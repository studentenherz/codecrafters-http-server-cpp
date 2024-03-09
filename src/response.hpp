#pragma once

#include <array>
#include <fstream>
#include <string>
#include <iostream>
#include <sys/socket.h>

#include "http.hpp"

const size_t RESPONSE_BUFFER_SIZE = 8192;

class HttpResponse {
	std::string _header;
	std::string _content_type;
	std::string _body;
	std::istream *_is;
	size_t _length;
public:
	HttpResponse(HttpStatus status = HttpStatus::Ok){
		_header = "HTTP/1.1 " + std::to_string(HttpStatusCode[status]) + " " + HttpStatusText[status] + "\r\n";
	}

	HttpResponse& text(const std::string& body){
		_content_type = "text/plain";
		_body = body;

		_header += "Content-Type: text/plain\r\n";
		_header += "Content-Length: " + std::to_string(_body.length()) + "\r\n";

		return *this;
	}

	HttpResponse& stream(std::istream& is, size_t length){
		_content_type = "application/octet-stream";
		_is = &is;
		_length = length;

		_header += "Content-Type: application/octet-stream\r\n";
		_header += "Content-Length: " + std::to_string(length) + "\r\n";

		return *this;
	}

 	friend void send(HttpResponse res, int socket_fd);

private:
	void send(int socket_fd) {
		::send(socket_fd, (void *) _header.data(), _header.size(), 0);
		::send(socket_fd, (void *) "\r\n", 2, 0);

		if (_content_type == "text/plain"){
			ssize_t out_value = ::send(socket_fd, (void *) _body.data(), _body.size(), 0);
		}
		else if (_content_type == "application/octet-stream"){
			size_t n_blocks = _length / RESPONSE_BUFFER_SIZE;
			char buff[RESPONSE_BUFFER_SIZE];
			while(n_blocks--){
				_is->read(buff, RESPONSE_BUFFER_SIZE);
				::send(socket_fd, (void *) buff, RESPONSE_BUFFER_SIZE, 0);
			}

			size_t rem = _length % RESPONSE_BUFFER_SIZE;
			_is->read(buff, rem);
			::send(socket_fd, (void *) buff, rem, 0);
		}
	}
};

void send(HttpResponse res, int socket_fd) {
	res.send(socket_fd);
}