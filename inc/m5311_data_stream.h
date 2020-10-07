#ifndef __M5311_DATA_STREAM_H
#define __M5311_DATA_STREAM_H

#include "parson.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

typedef struct {
    char stream_status;
    char upload_status;
    int packet_total;
    int packet_now;
    int packet_size;
}image_stream_socket_t;

void m5311_stream_json_require(char *require_type);
void m5311_stream_json_respond(int status);
void m5311_stream_data_process(char *json_data);


#endif
