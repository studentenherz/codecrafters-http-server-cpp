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
#include <algorithm>
#include <regex>
#include <thread>
#include <filesystem>
#include <fstream>
#include <iterator>

#include "request.hpp"

namespace fs = std::filesystem;

enum HttpStatus{
	Ok,
	NotFound,
	InternalError,
	HttpStatusEnumSize,
};


const std::array<int, HttpStatus::HttpStatusEnumSize> HttpStatusCode = {200, 404, 500};
const std::array<std::string, HttpStatus::HttpStatusEnumSize> HttpStatusText = {"Ok", "Not Found", "Internal Error"};

std::string response_status_line(HttpStatus status){
	return "HTTP/1.1 " + std::to_string(HttpStatusCode[status]) + " " + HttpStatusText[status] + "\r\n";
}

void handleConnection(int client_fd, std::string base_filepath){
	using namespace std::chrono_literals;

	std::cout << "Client connected\n";

	try{
		HttpRequest req(client_fd);

		std::string response;
		if (req.path == "/"){
			response = response_status_line(HttpStatus::Ok) + "Content-Length: 0\r\n\r\n";
		}
		else if (req.path.starts_with("/echo/")){
			response = response_status_line(HttpStatus::Ok);
			std::string payload = req.path.substr(6);
			response = response + "Content-Type: text/plain\r\nContent-Length: " + std::to_string(payload.length()) + "\r\n\r\n" + payload;

		}
		else if (req.path == "/user-agent"){
			std::string payload = req[HttpHeader::UserAgent];
			response = response_status_line(HttpStatus::Ok);
			response = response + "Content-Type: text/plain\r\nContent-Length: " + std::to_string(payload.length()) + "\r\n\r\n" + payload;
		}
		else if (req.path.starts_with("/files/")){
			std::string filepath = req.path.substr(7);

			fs::path fullpath(base_filepath);
			fullpath /= filepath;

			std::cout << "Asking for file: " << fullpath << std::endl;

			if (fs::exists(fullpath)){
				std::ifstream fi(fullpath);
				if (!fi.is_open()){
					std::cerr << "Error opening file " << fullpath << std::endl;
					response = response_status_line(HttpStatus::InternalError) + "Content-Length: 0\r\n\r\n";
				}
				else{
					std::string payload(std::istreambuf_iterator<char>{fi}, {});

					response = response_status_line(HttpStatus::Ok);
					response = response + "Content-Type: application/octet-stream\r\nContent-Length: " + std::to_string(payload.length()) + "\r\n\r\n";
					
					
					send(client_fd, (void *) response.c_str(), response.size(), 0);

					size_t n_blocks = 10;
					size_t block_size = payload.size() / n_blocks;
					n_blocks += (payload.size() % n_blocks > 0);
					for(size_t i = 0; i < n_blocks; i++){
						std::this_thread::sleep_for(200ms);
						response = payload.substr(i * block_size, block_size);
						send(client_fd, (void *) response.c_str(), response.size(), 0);
					}
				
					return;
				}
			}
			else {
				goto R404;
			}
		}
		else{
			R404:
			response = response_status_line(HttpStatus::NotFound) + "Content-Length: 0\r\n\r\n";
		}

		send(client_fd, (void *) response.c_str(), response.size(), 0);
		close(client_fd);
	}
	catch (std::string s){
		std::cerr << s << std::endl;
	}
	catch (ParsingError p){
		std::cerr << p.msg << std::endl;
	}
}

int main(int argc, char **argv) {
	std::string base_filepath;

	if (argc > 1){
		if (strcmp(argv[1],"--directory") == 0){
			if (argc < 3){
				std::cerr << "Expected <directory>\nUsage: ./your_server.sh --directory <directory>\n";
				return 1;
			}
			else{
				base_filepath = argv[2];
			}
		}
	}

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

		std::thread new_thread(handleConnection, client_fd, base_filepath);
		new_thread.detach();
	}
  
  close(server_fd);

  return 0;
}
