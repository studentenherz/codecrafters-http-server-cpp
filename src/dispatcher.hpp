#pragma once

#include <array>
#include <functional>
#include <vector>
#include <regex>
#include <utility>

#include "request.hpp"
#include "response.hpp"

using handler_t = std::function<HttpResponse(HttpRequest&, std::vector<std::string>)>;

class Dispatcher {
	std::array<std::vector<std::pair<std::string, handler_t>>, HttpMethod::HttpMethodEnumSize> _routes;
public:
	Dispatcher& route(std::string path, handler_t handler, HttpMethod method = HttpMethod::GET){
		_routes[method].push_back({path, handler});
		return *this;
	}

	Dispatcher& get(std::string path, handler_t handler){
		return route(path, handler);
	}

	Dispatcher& post(std::string path, handler_t handler){
		return route(path, handler, HttpMethod::POST);
	}

	void dispatch(HttpRequest& req, int socket_fd){
		for (auto [path, handler]: _routes[req.method]){
			std::regex re(path);
			std::smatch sm;
			if (std::regex_match(req.path, sm, re)){
				std::vector<std::string> params;
				for(size_t i = 1; i < sm.size(); i++){
					params.push_back(sm[i]);
				}

				HttpResponse res = handler(req, params);
				send(res, socket_fd);
				return;
			}
		}

		send(HttpResponse(HttpStatus::NotFound), socket_fd);
	}
};