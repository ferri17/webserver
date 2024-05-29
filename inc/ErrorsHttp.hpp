#pragma once

#define	NO_CONTENT 204
#define	FOUND 302
#define	BAD_REQUEST 400
#define	NOT_FOUND 404
#define	METHOD_NOT_ALLOWED 405
#define	NOT_ACCEPTABLE 406
#define	REQUEST_TIMEOUT 408
#define	REQUEST_ENTITY_TOO_LARGE 413
#define	INTERNAL_SERVER_ERROR 500
#define	NOT_IMPLEMENTED 501
#define	HTTP_VERSION_NOT_SUPPORTED 505
#define HTTP_ERROR_START 400

// Macro to find de correct msg
#define ERROR_MESSAGE(code) \
    ((code) == 204 ? "No Content" : \
	((code) == 302 ? "Found" : \
    ((code) == 400 ? "Bad Request" : \
    ((code) == 404 ? "Not Found" : \
    ((code) == 405 ? "Method not allowed" : \
    ((code) == 406 ? "Not Acceptable" : \
    ((code) == 408 ? "Request Timeout" : \
    ((code) == 413 ? "Request enntity too large" : \
    ((code) == 500 ? "Internal server error" : \
    ((code) == 501 ? "Not Implemented" : \
    ((code) == 505 ? "HTTP Version Not Supported" : \
    "Unknown Error")))))))))))

// 400 BAD REQUEST error messages
#define BAD_REQUEST_STR "Bad request: "
#define EMPTY_REQUEST_STR BAD_REQUEST_STR "Empty request."
#define SYNTAX_ERROR_HEADER_STR BAD_REQUEST_STR "Syntax error on header fields."
#define INVALID_CONTENT_LENGTH_STR BAD_REQUEST_STR "Invalid content-length."
#define WRONG_CONTENT_LENGTH_STR BAD_REQUEST_STR "Content-length doesn't match real body length."
#define SYNTAX_ERROR_REQLINE_STR BAD_REQUEST_STR "Syntax error on request-line."
#define REQLINE_LONG_STR BAD_REQUEST_STR "Request-line too long. Max lenght: 160000."
#define SYNTAX_ERROR_REQTARGET_STR BAD_REQUEST_STR "Syntax error on request-target."
#define SYNTAX_ERROR_METHOD_STR BAD_REQUEST_STR "Syntax error on method token."
#define HOST_NOT_FOUND_STR BAD_REQUEST_STR "Host header not found."
#define CONTRADICTORY_HEADERS_STR BAD_REQUEST_STR "Content-length and transfer-encoding header found."
#define INVALID_CHUNK_STR BAD_REQUEST_STR "Invalid chunked message."
#define TIMEOUT_STR BAD_REQUEST_STR "Request timeout."

// 413 REQUEST ENTITY TOO LARGE error messages
#define REQUEST_TOO_LARGE_STR "Request entity too large: Client max body size exceeded"


// 501 NOT IMPLEMENTED error messages
#define METHOD_NOT_IMPLEMENTED_STR "Not implemented: Method not implemented."
#define ENCODING_NOT_IMPLEMENTED_STR "Not implemented: Encoding directive not implemented."

// 505 HTTP VERSION NOT SUPPORTED errors messages
#define HTTP_VERSION_NOT_SUPPORTED_STR "Http version not supported: Invalid http protocol version."
