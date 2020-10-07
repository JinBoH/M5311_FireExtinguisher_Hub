#ifndef __M5311_TCP_UTILS_H
#define __M5311_TCP_UTILS_H

#include "m5311_opencpu.h"
#include "m5311_basic.h"

enum m5311_tcp_packet_type {
    m5311_tcp_report = 0,
    m5311_tcp_ask,
    m5311_tcp_require,
    m5311_tcp_modify,
    m5311_tcp_response
};

enum m5311_tcp_errno {
    no_err = 0,
    err_waiting_response,
    err_tcp_send_fail
};

typedef struct {
    char tcp_remote_name[50];
    int tcp_remote_port;
    char http_remote_name[50];
    int http_remote_port;
    
    char http_remote_ip[15];
    char tcp_remote_ip[15];
    int tcp_sock_fd;  
    int http_sock_fd;
    char iccid_buf[30];
    char http_token[30];
}m5311_tcp_info_t;

extern m5311_tcp_info_t m5311_tcp_info;

void m5311_get_iccid();
int m5311_tcp_connect();
int m5311_tcp_send(int sock_fd, char *p, int len);
void m5311_tcp_json_report_info(m5311_info_t info);
void m5311_tcp_json_register(const char *name, const char *iccid);
void m5311_tcp_json_data_process(const char *json_data);
#endif