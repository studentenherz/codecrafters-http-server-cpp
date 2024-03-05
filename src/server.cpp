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
#include <format>
#include <algorithm>

const size_t REQUEST_BUFFER_SIZE = 2048;

enum HttpStatus{
	Ok,
	NotFound
};


const std::array<int, 2> HttpStatusCode = {200, 404};
const std::array<std::string, 2> HttpStatusText = {"Ok", "Not Found"};

std::string response_status_line(HttpStatus status){
	return std::format("HTTP/1.1 {} {}\r\n\r\n", HttpStatusCode[status], HttpStatusText[status]);
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

RequestStartLine parse_request_start_line(std::string line){
	RequestStartLine rsl;
	
	size_t pos = line.find(' ');
	auto method = line.substr(0, pos);
	for(size_t i = 0; i < HttpMethodText.size(); i++){
		if (case_insensitive_equal(method, HttpMethodText[i])){
			rsl.method = HttpMethod(i);
		}
	}

	size_t new_pos = line.find(' ', pos + 1);
	rsl.path = line.substr(pos + 1, new_pos - pos - 1);

	rsl.protocol = line.substr(new_pos + 1);

	return rsl;
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
  
  auto client_fd = accept(server_fd, (struct sockaddr *) &client_addr, (socklen_t *) &client_addr_len);
  std::cout << "Client connected\n";

	std::string buff;
	buff.resize(REQUEST_BUFFER_SIZE);
	if (ssize_t bytes = recv(client_fd, buff.data(), REQUEST_BUFFER_SIZE, 0); bytes > 0){
		std::cout << bytes << " bytes received:\n\n";
	}
	else{
		std::cerr << "Failed to receive with code " << bytes << std::endl;
		return 1;
	}

	std::cout << buff;

	std::string sline = buff.substr(0, buff.find('\r'));
	std::cout << sline << std::endl;

	auto start_line = parse_request_start_line(sline);

	std::string response;
	std::cout << start_line.path << " " << start_line.path.length() << std::endl;
	if (start_line.path == "/"){
		response = response_status_line(HttpStatus::Ok);
	}
	else{
		response = response_status_line(HttpStatus::NotFound);
	}
	

	send(client_fd, (void *) response.c_str(), response.size(), 0);
  
  close(server_fd);

  return 0;
}
