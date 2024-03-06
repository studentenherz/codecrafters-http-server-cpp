#include <iostream>
#include <cstdlib>
#include <string>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <array>
// #include <format>
#include <algorithm>
#include <regex>
#include <thread>

const size_t REQUEST_BUFFER_SIZE = 2048;

enum HttpStatus{
	Ok,
	NotFound
};


const std::array<int, 2> HttpStatusCode = {200, 404};
const std::array<std::string, 2> HttpStatusText = {"Ok", "Not Found"};

std::string response_status_line(HttpStatus status){
	return "HTTP/1.1 " + std::to_string(HttpStatusCode[status]) + " " + HttpStatusText[status] + "\r\n";
}

enum HttpMethod{
	GET,
	POST
};

const std::array<std::string, 2> HttpMethodText = {"GET", "POST"};

struct RequestStartLine{
	HttpMethod method;
	std::string path;
	std::string protocol;
};

struct ParsingError {};

bool case_insensitive_equal(std::string s1, std::string s2){
	if (s1.length() != s2.length()) return false;
	for(size_t i = 0; i < s1.length(); i++){
		if (toupper(s1[i]) != toupper(s2[i])) return false;
	}
	return true;
}

RequestStartLine parse_request_start_line(const std::string& req){
	RequestStartLine rsl;

	std::regex re("(GET|POST) (.+) (.+)", std::regex::icase);
	std::smatch sm;
	if (std::regex_search(req, sm, re)){
		for (size_t i = 0; i < HttpMethodText.size(); i++){
			if (case_insensitive_equal(HttpMethodText[i], sm[1])){
				rsl.method = HttpMethod(i);
				break;
			}
		}

		rsl.path = sm[2];
		rsl.protocol = sm[3];
	}

	return rsl;
}

enum HttpRequestHeader{
	UserAgent,
	Host
};

class RequestHeaders{
	std::array<std::string, 2> _headers;
public:
	std::string& operator[](HttpRequestHeader header){
		return _headers[(size_t) header];
	}

	const std::string& operator[](HttpRequestHeader header) const {
		return _headers[(size_t) header];
	}
};

const std::array<std::string, 2> RequestHeadersText = {"User-Agent", "Host"};

RequestHeaders parse_request_headers(const std::string& req){
	RequestHeaders req_headers;

	for (size_t i = 0; i < RequestHeadersText.size(); i++){
		std::string reg = RequestHeadersText[i] + ": (.+)";
		std::regex re(reg, std::regex::icase);
		std::smatch sm;
		if (std::regex_search(req, sm, re)){
			req_headers[HttpRequestHeader(i)] = sm[1];
		}
	}

	return req_headers;
}

void handleConnection(int client_fd){
	  std::cout << "Client connected\n";

	std::string buff;
	buff.resize(REQUEST_BUFFER_SIZE);
	if (ssize_t bytes = recv(client_fd, buff.data(), REQUEST_BUFFER_SIZE, 0); bytes > 0){
		std::cout << bytes << " bytes received:\n\n";
	}
	else if (bytes < 0){
		std::cerr << "Failed to receive with code " << bytes << std::endl;
		exit(1);
	}

	std::cout << buff;

	auto start_line = parse_request_start_line(buff);

	std::string response;
	if (start_line.path == "/"){
		response = response_status_line(HttpStatus::Ok) + "Content-Length: 0\r\n\r\n";
	}
	else if (start_line.path.starts_with("/echo/")){
		response = response_status_line(HttpStatus::Ok);
		std::string payload = start_line.path.substr(6);
		response = response + "Content-Type: text/plain\r\nContent-Length: " + std::to_string(payload.length()) + "\r\n\r\n" + payload;

	}
	else if (start_line.path == "/user-agent"){
		auto req_headers = parse_request_headers(buff);
		std::cout << req_headers[HttpRequestHeader::UserAgent] << std::endl;
		std::cout << req_headers[HttpRequestHeader::Host] << std::endl;

		std::string payload = req_headers[HttpRequestHeader::UserAgent];
		response = response_status_line(HttpStatus::Ok);
		response = response + "Content-Type: text/plain\r\nContent-Length: " + std::to_string(payload.length()) + "\r\n\r\n" + payload;
	}
	else{
		response = response_status_line(HttpStatus::NotFound) + "Content-Length: 0\r\n\r\n";
	}

	send(client_fd, (void *) response.c_str(), response.size(), 0);
	close(client_fd);
}

int main(int argc, char **argv) {

  // Uncomment this block to pass the first stage
  
  int server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd < 0) {
   std::cerr << "Failed to create server socket\n";
   return 1;
  }
  
  // Since the tester restarts your program quite often, setting REUSE_PORT
  // ensures that we don't run into 'Address already in use' errors
  int reuse = 1;
  if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEPORT, &reuse, sizeof(reuse)) < 0) {
    std::cerr << "setsockopt failed\n";
    return 1;
  }
  
  struct sockaddr_in server_addr;
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(4221);
  
  if (bind(server_fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) != 0) {
    std::cerr << "Failed to bind to port 4221\n";
    return 1;
  }
  
  int connection_backlog = 5;
  if (listen(server_fd, connection_backlog) != 0) {
    std::cerr << "listen failed\n";
    return 1;
  }
  
  struct sockaddr_in client_addr;
  int client_addr_len = sizeof(client_addr);
  
  std::cout << "Waiting for a client to connect...\n";
  

	while(true){
  	int client_fd = accept(server_fd, (struct sockaddr *) &client_addr, (socklen_t *) &client_addr_len);

		std::thread new_thread(handleConnection, client_fd);
		new_thread.detach();
	}
  
  close(server_fd);

  return 0;
}
