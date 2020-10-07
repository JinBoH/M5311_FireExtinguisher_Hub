#include "m5311_basic.h"
#include "m5311_data_stream.h"
#include "m5311_tcp_utils.h"
#include "m5311_http_utils.h"

#define stream_send(fmt, arg...)  opencpu_printf(fmt, ##arg)

image_stream_socket_t image_socket;

void m5311_stream_json_require(char *require_type)
{
	char *p;

	JSON_Value *pJsonRoot = json_value_init_object();
	json_object_set_string(json_object(pJsonRoot), "type", "require");
	json_object_set_string(json_object(pJsonRoot), "item", require_type);

	p = json_serialize_to_string(pJsonRoot);
	stream_send("%s\r\n", p);

	free(p); 
	json_value_free(pJsonRoot);
}

void m5311_stream_json_respond(int status)
{
	char *p;

	JSON_Value *pJsonRoot = json_value_init_object();
	json_object_set_string(json_object(pJsonRoot), "type", "response");
	json_object_set_number(json_object(pJsonRoot), "status", status);
	p = json_serialize_to_string(pJsonRoot);
	stream_send("%s\r\n", p);

	free(p);
	json_value_free(pJsonRoot);
}
 
void m5311_stream_data_process(char *stream_data)
{
	JSON_Value *pJsonRoot = NULL;
	char *json_data_type = NULL;
	char *json_data_item = NULL;
	int json_data_data = 0;
	int json_data_status = 0;
	int http_send_len = 0;
	static int image_upload_done = 0;
	
	pJsonRoot = json_parse_string(stream_data);
	if(NULL == pJsonRoot) {
		/*有未处理的图像包信息，表明当前数据为图像数据*/
		if(image_socket.stream_status) {
			http_send_len = HTTPDataSend(stream_data, image_socket.packet_size);
			M5311_TRACE("http_send_len:%d.\r\n", http_send_len);
			if(http_send_len == image_socket.packet_size) {
				/*所有图像包全部上传完毕，发送尾boundary和请求尾*/
				if(image_upload_done) {
					HTTPDataSend("\r\n", 2);
					HTTPSocketClose();
					/*通知Exchanger图像包上传结束*/
					m5311_stream_json_require("done");
					image_upload_done = 0;
				}
				else
					m5311_stream_json_respond(0);
			}
			else 
				m5311_stream_json_respond(1);
			image_socket.stream_status = 0;
		}
	}
	else {
		json_data_type = json_object_get_string(json_object(pJsonRoot), "type");
		if(NULL == json_data_type) 
			goto err_parse_json_data_fail;
		else if(!strcmp(json_data_type, "condition")) {
			json_data_item = json_object_get_string(json_array_get_object(json_array(json_object_get_value(json_object(pJsonRoot), "list")), 0), "item");
			if(!strcmp(json_data_item, "camera")) {
				json_data_status = json_object_get_number(json_array_get_object(json_array(json_object_get_value(json_object(pJsonRoot), "list")), 0), "status");
				m5311_condition.camera_condition = json_data_status;
			}

			json_data_item = json_object_get_string(json_array_get_object(json_array(json_object_get_value(json_object(pJsonRoot), "list")), 1), "item");
			if(!strcmp(json_data_item, "dht11")) {
				json_data_status = json_object_get_number(json_array_get_object(json_array(json_object_get_value(json_object(pJsonRoot), "list")), 1), "status");
				m5311_condition.si7021_condition = json_data_status;
			}

			json_data_item = json_object_get_string(json_array_get_object(json_array(json_object_get_value(json_object(pJsonRoot), "list")), 2), "item");
			if(!strcmp(json_data_item, "others")) {
				json_data_status = json_object_get_number(json_array_get_object(json_array(json_object_get_value(json_object(pJsonRoot), "list")), 2), "status");
				m5311_condition.other_dev_condition = json_data_status;
			}
		}
		else if(!strcmp(json_data_type, "report")) {
			
		}
		/*采集表盘图像并上传*/
		else if(!strcmp(json_data_type, "report-image")) {
			json_data_data = json_object_get_number(json_object(pJsonRoot), "size");
			if(json_data_data > 0)
				image_socket.packet_size = json_data_data;
			else {
				memset(&image_socket, 0, sizeof(image_stream_socket_t));
				goto err_parse_json_data_fail;
			}

			json_data_data = json_object_get_number(json_object_get_object(json_object(pJsonRoot), "ctrl"), "total");
			if(json_data_data > 0)
				image_socket.packet_total = json_data_data;
			else {
				memset(&image_socket, 0, sizeof(image_stream_socket_t));
				goto err_parse_json_data_fail;
			}

			json_data_data = json_object_get_number(json_object_get_object(json_object(pJsonRoot), "ctrl"), "now");
			if(json_data_data > 0)
				image_socket.packet_now = json_data_data;
			else {
				memset(&image_socket, 0, sizeof(image_stream_socket_t));
				goto err_parse_json_data_fail;
			}
			/*收到第一个图像包信息，创建HTTP连接并发送请求头*/
			if(1 == image_socket.packet_now) {
				HTTPRequestSocketCreate();
				HTTPRequestSend(HTTPRequest);
			}
			/*收到最后一个图像包信息，图像包接收即将结束*/
			else if(image_socket.packet_now == image_socket.packet_total)
				image_upload_done = 1;

			m5311_stream_json_respond(0);
			image_socket.stream_status = 1;
			M5311_TRACE("process packet %d.\r\n", image_socket.packet_now);
		}
		else if(!strcmp(json_data_type, "rename")) {
			json_data_item = json_object_get_string(json_object(pJsonRoot), "item");
			if(strlen(json_data_item) > 0 && strlen(json_data_item) < 32) {
				M5311_TRACE("set device name: %s\r\n", json_data_item);
				m5311_set_device_name(json_data_item);
			}

		}
		else if(!strcmp(json_data_type, "response")) {
			
		}

		err_parse_json_data_fail:
			
			json_value_free(pJsonRoot);
	}
}

