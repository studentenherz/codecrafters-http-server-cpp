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

const size_t REQUEST_BUFFER_SIZE = 2048;

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

	std::array<char, REQUEST_BUFFER_SIZE> buff;
	if (ssize_t bytes = recv(client_fd, buff.data(), REQUEST_BUFFER_SIZE, 0); bytes > 0){
		std::cout << bytes << " bytes received:\n\n";
		std::cout << (char *) buff.data();
	}
	else{
		std::cerr << "Failed to receive with code " << bytes << std::endl;
		return 1;
	}

	const std::string response = "HTTP/1.1 200 OK\r\n\r\n";
	send(client_fd, (void *) response.c_str(), response.size(), 0);
  
  close(server_fd);

  return 0;
}
