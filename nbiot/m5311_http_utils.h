#ifndef __M5311_HTTP_UTILS_H
#define __M5311_HTTP_UTILS_H

#include "list_utils.h"
#include "m5311_opencpu.h"

typedef struct {
	int headers;
	int total_length;
	char *request_type;
	char *boundary;
	struct list_head *list_head;
}HTTPRequest_t;

typedef struct {
	char *key;
	char *value;
	struct list_head *list_entry;
}HTTPRequestHeader_t;

enum http_request_type {
	POST = 0,
	GET
};

enum http_request_version {
	HTTP_1_0 = 0,
	HTTP_1_1
};

#define HTTP_CREATE_UTILS(name, type)    \
        type *name = util_alloc(1, type)

extern HTTPRequest_t *HTTPRequest;
extern const char *http_request_header;
extern const char *http_request_tail;

HTTPRequest_t *HTTPRequestCreate(void);
int HTTPRequestAddType(HTTPRequest_t *HTTPRequest, char *http_request_type, char *request_param, char *http_request_version);
int HTTPRequestAddHeaders(HTTPRequest_t *HTTPRequest, char *key, char *value);
int HTTPGetFileContentLength(int filesize, char* boundary, char* name, char* filename, char *file_type);
char *HTTPRequestSerializeToString(HTTPRequest_t *HTTPRequest);

int HTTPRequestSocketCreate(void);
int HTTPRequestSend(HTTPRequest_t *HTTPRequest);
void HTTPSocketClose(void);


#endif