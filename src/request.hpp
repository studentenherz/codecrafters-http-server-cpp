#pragma once

#include <array>
#include <string>
#include <sstream>
#include <algorithm>
#include <iostream>
#include <map>
#include <sys/socket.h>

#include "http.hpp"
#include "util.hpp"

const size_t REQUEST_BUFFER_SIZE = 8192;

struct ParsingError {
	std::string msg;
};

class HttpRequest{
	std::array<std::string, HttpHeader::HttpHeaderEnumSize> _headers;
	std::string _buff;
	std::string _body;
	bool _body_received = false;
	int _socket_fd;
public:
	HttpMethod method;
	std::string path;
	std::string protocol;
	std::map<std::string, std::string> headers;

	/**
	 * Gets an HttpRequest object from a socket.
	 * This constructor gets and parses the header.
	 * 
	 * @param socket_fd socket file descriptor
	 * 
	 * @throws
	 *  - `"Failed to receive with code ..."` if it `recv()` fails to receive from the socket
	 * 	- `ParsingError` if it fails to parse the header
	*/
	HttpRequest(int socket_fd): _socket_fd(socket_fd){
		_buff.resize(REQUEST_BUFFER_SIZE + 1);

		ssize_t bytes;
		recv(_socket_fd, _buff.data(), REQUEST_BUFFER_SIZE, 0);
		if (bytes < 0){
			throw "Failed to receive with code " + std::to_string(bytes);
		}

		_parse_header(_buff);
		size_t pos = _buff.find("\r\n\r\n");
		_body = _buff.substr(pos + 4, bytes - pos - 4);
	}

	/**
	 * Get the value of a header
	*/
	std::string operator[](HttpHeader header){
		return _headers[(size_t) header];
	}

	/**
	 * Get the body of the request
	 * 
	 * @return the body of the request
	*/
	const std::string& body(){
		if (!_body_received) {
			while (true) {
				ssize_t bytes = recv(_socket_fd, _buff.data(), REQUEST_BUFFER_SIZE, 0);
				_body.append(_buff.substr(0, bytes));
				if (bytes < REQUEST_BUFFER_SIZE) break;
			}
		}
		
		return _body;
	}

private:
	void _parse_header(const std::string& s){
		std::cout << s.substr(0, s.find("\r\n\r\n")) << std::endl;

		std::istringstream iss(s.substr(0, s.find("\r\n\r\n")));
		std::string line, method;

		// Parse start line
		std::getline(iss, line);
		std::istringstream lineStream(line);
		lineStream >> method >> this->path >> this->protocol;

		// Validate HTTP method
		auto it = std::find_if(HttpMethodText.begin(), HttpMethodText.end(), 
			[&method](const std::string& s){return case_insensitive_equal(method, s);}
		);
		if (it == HttpMethodText.end()){
			throw ParsingError { "Unrecognized HTTP method `" + method + "`" };
		}
		this->method = HttpMethod(it - HttpHeadersText.begin());


		// Get headers
		std::map<std::string, std::string> headers;

		while(std::getline(iss, line)){
			size_t pos = line.find(':');
			if (pos != std::string::npos) {
				std::string key = line.substr(0, pos);
				std::string value = line.substr(pos + 2);
				this->headers.insert({key, value});
				headers.insert({tolower(key), value});
			}
		}

		for (size_t i = 0; i < HttpHeader::HttpHeaderEnumSize; i++){
			auto it = headers.find(tolower(HttpHeadersText[i]));
			if (it != headers.end()){
				this->_headers[i] = it->second;
			}
		}
	}
};