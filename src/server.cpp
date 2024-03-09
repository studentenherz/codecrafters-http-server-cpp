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
#include "response.hpp"
#include "dispatcher.hpp"

namespace fs = std::filesystem;

void handleConnection(int client_fd, std::string base_filepath){
	using namespace std::chrono_literals;

	std::cout << "Client connected\n";

	try{
		HttpRequest req(client_fd);

		std::ifstream fi;

		Dispatcher()
			.route("/", [](HttpRequest /* r */, std::vector<std::string> /* v */){return HttpStatus::Ok;})
			.route("/echo/(.*)", [](HttpRequest /* r */, std::vector<std::string> v){ return HttpResponse().text(v[0]);})
			.route("/user-agent", [](const HttpRequest& req, auto /* v */){return HttpResponse().text(req[HttpHeader::UserAgent]);})
			.route("/files/(.*)", [&base_filepath, &fi](HttpRequest /* r */, std::vector<std::string> params){
				std::string filepath = params[0];
				fs::path fullpath(base_filepath);
				fullpath /= filepath;

				if (fs::exists(fullpath)){
					if (fi.is_open()) fi.close();
					fi.open(fullpath);
					if (!fi.is_open()){
						std::cerr << "Error opening file " << fullpath << std::endl;
						return HttpResponse(HttpStatus::InternalError);
					}
					else{
						size_t length = fs::file_size(fullpath);
						return HttpResponse()
							.stream(fi, length);
					}
				}
				
				return HttpResponse(HttpStatus::NotFound);
			})
			.dispatch(req, client_fd);

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
