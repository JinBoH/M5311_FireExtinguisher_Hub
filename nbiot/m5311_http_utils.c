#include "m5311_tcp_utils.h"
#include "m5311_http_utils.h"

HTTPRequest_t *HTTPRequest;
char http_host_ip_get_flag = 0;

const char *http_request_header = "POST /device/upload.do HTTP/1.1\r\nHOST: img.xingketec.cn\r\nConnection: keep-alive\r\nContent-Type: multipart/form-data; boundary=----WebKitFormBoundary7MA4YWxkTrZu0gW\r\nContent-Length: 800\r\n\r\n";
const char *http_first_boundary = "------WebKitFormBoundary7MA4YWxkTrZu0gW\r\n";
const char *http_multipart_content = "Content-Disposition: form-data; name=\"file\"; filename=\"test.bmp\"\r\nContent-Type: image/bmp\r\n\r\n";
const char *http_last_boundary = "----WebKitFormBoundary7MA4YWxkTrZu0gW--\r\n\r\n";

static void cb_get_host_ip(char *ip)
{  
    memset(m5311_tcp_info.http_remote_ip, 0, sizeof(m5311_tcp_info.http_remote_ip));
    M5311_TRACE ("remote ip:%s\r\n", ip + 8); 
    strcpy(m5311_tcp_info.http_remote_ip, ip + 8);
    http_host_ip_get_flag = 1;
}

int HTTPGetFileContentLength(int filesize, char* boundary, char* name, char* filename, char *file_type)
{
	int nSize = 0;
	nSize += strlen("--\r\n") + strlen(boundary);
	nSize += strlen("Content-Disposition: form-data; name=\"\"; filename=\"\"\r\n") + strlen(name) + strlen(filename);
	nSize += strlen("Content-Type: \r\n\r\n") + strlen(file_type);
	nSize += filesize;
	nSize += strlen("--\r\n") + strlen(boundary) ;
	return nSize;
}

HTTPRequest_t *HTTPRequestCreate()
{
	HTTPRequest_t *HTTPRequest = util_alloc(1, HTTPRequest_t);
	if(NULL == HTTPRequest)
		return NULL;
	
	util_memset(HTTPRequest, 0, sizeof(HTTPRequest_t));
	HTTPRequest->list_head = util_alloc(1, struct list_head);
	if(NULL == HTTPRequest->list_head) {
		util_free(HTTPRequest);
		return NULL;
	}
	
	list_head_init(HTTPRequest->list_head);
	return HTTPRequest;
}

int HTTPRequestAddType(HTTPRequest_t *HTTPRequest, char *http_request_type, char *request_param, char *http_request_version)
{
	int length = 0;
	
	if(NULL == http_request_type || NULL == request_param || NULL == http_request_version || NULL == HTTPRequest)
		return -1;
	
	length += strlen(http_request_type);
	length += strlen(request_param);
	length += strlen(http_request_version);
	length += 4;							//strlen(' ') * 2 + strlen("\r\n");
	
	HTTPRequest->request_type = util_alloc(length + 1, char);
	if(!ASSERT_UTILS(HTTPRequest->request_type))
		return -1;
	
	util_memset(HTTPRequest->request_type, 0, length + 1);
	
	sprintf(HTTPRequest->request_type, "%s %s %s\r\n", http_request_type, request_param, http_request_version);
	
	HTTPRequest->total_length += length;
}

int HTTPRequestAddHeaders(HTTPRequest_t *HTTPRequest, char *key, char *value)
{
	int key_length;
	int value_length;
	
	if(NULL == key || NULL == value || NULL == HTTPRequest)
		return -1;
	
	HTTP_CREATE_UTILS(new_header, HTTPRequestHeader_t);
	if(!ASSERT_UTILS(new_header)) {
		M5311_TRACE("memory alloc fail.\r\n");
		return -1;
	}
	LIST_CREATE_UTILS(new_header->list_entry, new_header);
	if(!ASSERT_UTILS(new_header->list_entry)) {
		M5311_TRACE("memory alloc fail.\r\n");
        return -1;
	}
	
	key_length = strlen(key);
	new_header->key = util_alloc(key_length, char);
	if(!ASSERT_UTILS(new_header->key)) {
		M5311_TRACE("memory alloc fail.\r\n");
		util_free(new_header->list_entry);
		util_free(new_header);
		return -1;
	}
	
	value_length = strlen(value);
	new_header->value = util_alloc(value_length, char);
	if(!ASSERT_UTILS(new_header->value)) {
		M5311_TRACE("memory alloc fail.\r\n");
		util_free(new_header->list_entry);
		util_free(new_header->key);
		util_free(new_header);
		return -1;
	}
	
	strcpy(new_header->key, key);
	strcpy(new_header->value, value);
	
	list_add_tail(new_header->list_entry, HTTPRequest->list_head);
	HTTPRequest->total_length += key_length + value_length + 4;			//strlen(": ") + strlen("\r\n");
	return 0;			
}

char *HTTPRequestSerializeToString(HTTPRequest_t *HTTPRequest)
{
	struct list_head *p = NULL;
	char *http_serialized_string = NULL;
	
	if(NULL == HTTPRequest)
		return NULL;
	if(NULL == HTTPRequest->request_type)
		return NULL;
	http_serialized_string = util_alloc(HTTPRequest->total_length + 2, char);
	if(!ASSERT_UTILS(http_serialized_string))
		return NULL;
	
	util_memset(http_serialized_string, 0, HTTPRequest->total_length + 2);
	
	strcpy(http_serialized_string, HTTPRequest->request_type);
	
	list_for_each(p, HTTPRequest->list_head) {
		strcat(http_serialized_string, (list_entry(p, HTTPRequestHeader_t))->key);
		strcat(http_serialized_string, ": ");
		strcat(http_serialized_string, (list_entry(p, HTTPRequestHeader_t))->value);
		strcat(http_serialized_string, "\r\n");
	}
	
	strcat(http_serialized_string, "\r\n");
	M5311_TRACE("%s\r\n", http_serialized_string);
	return http_serialized_string;
}

int HTTPRequestSocketCreate()
{
    if(-1 == opencpu_get_host_by_name(m5311_tcp_info.http_remote_name, 0, cb_get_host_ip)) {
        M5311_TRACE( "http connect error\r\n");
		return -1;
    }

    while(!http_host_ip_get_flag)
        vTaskDelay(10);
    M5311_TRACE("create http socket %s:%d\r\n", m5311_tcp_info.http_remote_ip, m5311_tcp_info.http_remote_port);
	
	m5311_tcp_info.http_sock_fd = socket(AF_INET, SOCK_STREAM, 0);  
	if (m5311_tcp_info.http_sock_fd == -1) {  
        M5311_TRACE( "http socket create error\r\n");
		return -1;
    }  
	else  {
		
		return 0;
	}
}

int HTTPRequestSend(HTTPRequest_t *HTTPRequest)
{
	struct sockaddr_in server_addr;
	struct in_addr remote_addr; 
	char *http_request = NULL;
	int send_data_len = 0;

	inet_aton(m5311_tcp_info.http_remote_ip, &remote_addr); 

	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;  
    server_addr.sin_addr.s_addr = remote_addr.s_addr;
    server_addr.sin_port = htons(m5311_tcp_info.http_remote_port);
    if(connect(m5311_tcp_info.http_sock_fd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr))) {
		M5311_TRACE( "http server connect error\r\n");
		return -1;
	}
    else {
		M5311_TRACE( "http server connected\r\n");
    }

	http_request = HTTPRequestSerializeToString(HTTPRequest);
	send_data_len = send(m5311_tcp_info.http_sock_fd, http_request, strlen(http_request_header), 0);
	if(send_data_len > 0)
        M5311_TRACE("send http request via socket %d: %s\r\n", m5311_tcp_info.http_sock_fd, http_request_header);
    else
        M5311_TRACE("send http request via socket %d fail\r\n", m5311_tcp_info.http_sock_fd);
	
	send_data_len = send(m5311_tcp_info.http_sock_fd, http_first_boundary, strlen(http_first_boundary), 0);
	if(send_data_len > 0)
        M5311_TRACE("send http request via socket %d: %s\r\n", m5311_tcp_info.http_sock_fd, http_first_boundary);
    else
        M5311_TRACE("send http request via socket %d fail\r\n", m5311_tcp_info.http_sock_fd);
	
	send_data_len = send(m5311_tcp_info.http_sock_fd, http_multipart_content, strlen(http_multipart_content), 0);
	if(send_data_len > 0)
        M5311_TRACE("send http request via socket %d: %s\r\n", m5311_tcp_info.http_sock_fd, http_multipart_content);
    else
        M5311_TRACE("send http request via socket %d fail\r\n", m5311_tcp_info.http_sock_fd);
	
	free(http_request);

}


void HTTPRequestDestroy()
{

}

int HTTPDataSend(char *p, int len)
{
	return send(m5311_tcp_info.http_sock_fd, p, len, 0);

}

void HTTPSocketClose()
{
	send(m5311_tcp_info.http_sock_fd, http_last_boundary, strlen(http_last_boundary), 0);
	close(m5311_tcp_info.http_sock_fd);
}