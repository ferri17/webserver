#pragma once

//#define	CONTINUE 100
//#define	SWITCHING_PROTOCOLS 101
//#define	OK 200
//#define	CREATED 201
//#define	ACCEPTED 202
//#define	NON_AUTHORITATIVE_INFORMATION 203
//#define	NO_CONTENT 204
//#define	RESET_CONTENT 205
//#define	PARTIAL_CONTENT 206
//#define	MULTIPLE_CHOICES 300
//#define	MOVED_PERMANENTLY 301
#define	FOUND 302
//#define	SEE_OTHER 303
//#define	NOT_MODIFIED 304
//#define	USE_PROXY 305
//#define	TEMPORARY_REDIRECT 307
#define	BAD_REQUEST 400
//#define	UNAUTHORIZED 401
//#define	PAYMENT_REQUIRED 402
//#define	FORBIDDEN 403
#define	NOT_FOUND 404
//#define	METHOD_NOT_ALLOWED 405
#define	NOT_ACCEPTABLE 406
//#define	PROXY_AUTHENTICATION_REQUIRED 407
//#define	REQUEST_TIMEOUT 408
//#define	CONFLICT 409
//#define	GONE 410
//#define	LENGTH_REQUIRED 411
//#define	PRECONDITION_FAILED 412
#define	REQUEST_ENTITY_TOO_LARGE 413
//s#define	REQUEST_URI_TOO_LONG 414
//#define	UNSUPPORTED_MEDIA_TYPE 415
//#define	REQUESTED_RANGE_NOT_SATISFIABLE 416
//#define	EXPECTATION_FAILED 417
//#define	INTERNAL_SERVER_ERROR 500
#define	NOT_IMPLEMENTED 501
//#define	BAD_GATEWAY 502
//#define	SERVICE_UNAVAILABLE 503
//#define	GATEWAY_TIMEOUT 504
#define	HTTP_VERSION_NOT_SUPPORTED 505

// Macro to find de correct msg
#define ERROR_MESSAGE(code) \
	((code) == 302 ? "Found" : \
    ((code) == 400 ? "Bad Request" : \
    ((code) == 404 ? "Not Found" : \
    ((code) == 406 ? "Not Acceptable" : \
    ((code) == 501 ? "Not Implemented" : \
    ((code) == 505 ? "HTTP Version Not Supported" : \
    "Unknown Error"))))))

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

// 413 REQUEST ENTITY TOO LARGE error messages
#define REQUEST_TOO_LARGE_STR "Request entity too large: Client max body size exceeded"


// 501 NOT IMPLEMENTED error messages
#define METHOD_NOT_IMPLEMENTED_STR "Not implemented: Method not implemented."
#define ENCODING_NOT_IMPLEMENTED_STR "Not implemented: Encoding directive not implemented."

// 505 HTTP VERSION NOT SUPPORTED errors messages
#define HTTP_VERSION_NOT_SUPPORTED_STR "Http version not supported: Invalid http protocol version."










