#pragma once

#include <array>
#include <string>

enum HttpMethod{
	GET,
	HEAD,
	POST,
	PUT,
	DELETE,
	CONNECT,
	OPTIONS,
	TRACE,
	PATCH,
	HttpMethodEnumSize
};

const std::array<std::string, HttpMethod::HttpMethodEnumSize> HttpMethodText = {
	"GET", "HEAD", "POST", "PUT", "DELETE", "CONNECT", "OPTIONS", "TRACE", "PATCH"
};

enum HttpHeader{
	AIM,
	Accept,
	AcceptCharset,
	AcceptEncoding,
	AcceptLanguage,
	AcceptDatetime,
	AccessControlRequestMethod,
	AccessControlRequestHeaders,
	Authorization,
	CacheControl,
	Connection,
	ContentLength,
	ContentType,
	Cookie,
	Date,
	Expect,
	Forwarded,
	From,
	Host,
	IfMatch,
	IfModifiedSince,
	IfNoneMatch,
	IfRange,
	IfUnmodifiedSince,
	MaxForwards,
	Origin,
	Pragma,
	ProxyAuthorization,
	Range,
	Referer,
	TE,
	UserAgent,
	Upgrade,
	Via,
	Warning,
	HttpHeaderEnumSize
};

const std::array<std::string, HttpHeader::HttpHeaderEnumSize> HttpHeadersText = {
	"A-IM", "Accept", "Accept-Charset", "Accept-Encoding", "Accept-Language", "Accept-Datetime", "Access-Control-Request-Method",
	"Access-Control-Request-Headers", "Authorization", "Cache-Control", "Connection", "Content-Length", "Content-Type", "Cookie",
	"Date", "Expect", "Forwarded", "From", "Host", "If-Match", "If-Modified-Since", "If-None-Match", "If-Range", 
	"If-Unmodified-Since", "Max-Forwards", "Origin", "Pragma", "Proxy-Authorization", "Range", "Referer", "TE", "User-Agent", 
	"Upgrade", "Via", "Warning"
};

enum HttpStatus{
	Ok,
	NotFound,
	InternalError,
	HttpStatusEnumSize,
};

const std::array<int, HttpStatus::HttpStatusEnumSize> HttpStatusCode = {200, 404, 500};
const std::array<std::string, HttpStatus::HttpStatusEnumSize> HttpStatusText = {"Ok", "Not Found", "Internal Error"};

enum HttpProtocol{
	HTTP1,
	HTTP1_1,
	HttpProtocolEnumSize
};

const std::array<std::string, HttpProtocol::HttpProtocolEnumSize> HttpProtocolText = {"HTTP/1", "HTTP/1.1"};