#include "m5311_tcp_utils.h"
#include "m5311_basic.h"
#include "parson.h"

enum m5311_tcp_errno tcp_errno;
m5311_tcp_info_t m5311_tcp_info = {
    .tcp_remote_name = "device.huozhiniao.cn",
    .tcp_remote_port = 18050,
    .http_remote_name = "img.xingketec.cn",
    .http_remote_port = 80
};

char tcp_remote_name[50];
char tcp_remote_ip[15];
int tcp_remote_port;
char http_remote_name[50];
char http_remote_ip[15];
int http_remote_port;
int tcp_sock_fd;  
int http_sock_fd;
char iccid_buf[30];
char http_token[30];

char host_ip_get_flag = 0;

 /**
 *  \brief ip解析回调函数
 *  
 *  \param [in] ip 解析得到的ip地址
 *  \return void
 */
static void cb_get_host_ip(char *ip)
{  
    memset(m5311_tcp_info.tcp_remote_ip, 0, sizeof(m5311_tcp_info.tcp_remote_ip));
    M5311_TRACE ("remote ip:%s\r\n", ip + 8); 
    strcpy(m5311_tcp_info.tcp_remote_ip, ip + 8);
    host_ip_get_flag = 1;
}

 /**
 *  \brief 获得设备iccid
 *  
 *  \param void
 *  \return void
 */
void m5311_get_iccid()
{
	int i = 0;
	memset(m5311_tcp_info.iccid_buf, 0, 30);
	while(opencpu_iccid(m5311_tcp_info.iccid_buf) !=  0) {
		i++;
		vTaskDelay(10);
		if(i > 20) {
			opencpu_printf("iccid timeout\r\n");
			return;
		}
	}
	opencpu_printf("ICCID:%s\r\n", m5311_tcp_info.iccid_buf);
}

 /**
 *  \brief 建立tcp连接
 *  
 *  \param [in] remote_name 服务器域名
 *  \param [in] remote_port 端口号
 *  \return 连接状态: <0>-成功 <-1>-失败
 */
int m5311_tcp_connect()
{
    struct sockaddr_in server_addr; 
    struct in_addr remote_addr; 

    if(-1 == opencpu_get_host_by_name(m5311_tcp_info.tcp_remote_name, 0, cb_get_host_ip)) {
        opencpu_printf( "tcp connect error\r\n");
		return -1;
    }

    while(!host_ip_get_flag)
        vTaskDelay(10);
    opencpu_printf("create tcp connection %s:%d\r\n", m5311_tcp_info.tcp_remote_ip, m5311_tcp_info.tcp_remote_port);
	inet_aton(m5311_tcp_info.tcp_remote_ip, &remote_addr); 

	m5311_tcp_info.tcp_sock_fd = socket(AF_INET, SOCK_STREAM, 0);  
	if (m5311_tcp_info.tcp_sock_fd == -1) {  
        opencpu_printf( "socket create error\r\n");
		return -1;
    }  

	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;  
    server_addr.sin_addr.s_addr = remote_addr.s_addr;
    server_addr.sin_port = htons(m5311_tcp_info.tcp_remote_port);
    if(connect(m5311_tcp_info.tcp_sock_fd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr))) {
		opencpu_printf( "tcp connect error\r\n");
		return -1;
	}
    else {
		opencpu_printf( "tcp connected\r\n");
        return 0;
    }
}

 /**
 *  \brief 通过tcp发送数据
 *  
 *  \param [in] sock_fd connect得到的tcp socket
 *  \param [in] len 欲发送数据长度
 *  \return 实际发送数据长度
 */
int m5311_tcp_send(int sock_fd, char *p, int len)
{
    int send_len;

    //有未被处理的tcp错误，终止发送
    // if(tcp_errno != no_err) {
    //     opencpu_printf("unhandled previous tcp error.\r\n");
    //     return -1;
    // }

    send_len = send(sock_fd, p, len, 0);
    if(send_len <= 0)
        tcp_errno = err_tcp_send_fail;
    else
        tcp_errno = err_waiting_response;

    return send_len;
}

 /**
 *  \brief 通过tcp socket上报环境数据
 *  
 *  \param [in] info 环境数据
 *  \return void
 */
void m5311_tcp_json_report_info(m5311_info_t info)
{
    int send_data_len = 0;
    char temperature_buf[5];
    char temperature_status_buf[2];
    char humidity_buf[5];
    char humidity_status_buf[2];
    char pressure_status_buf[2];
    char *p;

    memset(temperature_buf, 0, 5);
    memset(temperature_status_buf, 0, 2);
    memset(humidity_buf, 0, 5);
    memset(humidity_status_buf, 0, 2);
    memset(pressure_status_buf, 0, 2);

    sprintf(temperature_buf, "%.1f", info.temperature);
    sprintf(temperature_status_buf, "%d", info.temperature_status);
    sprintf(humidity_buf, "%.1f", info.humidity);
    sprintf(humidity_status_buf, "%d", info.humidity_status);
    sprintf(pressure_status_buf, "%d", info.pressure_status);

    JSON_Value *pJsonRoot = json_value_init_object();
    json_object_set_string(json_object(pJsonRoot), "type", "report");

    JSON_Value *pJsonSub = json_value_init_object();
    
    pJsonSub = json_value_init_array();
    json_object_set_value(json_object(pJsonRoot), "list", pJsonSub);

    JSON_Value *pJsonSubSub = json_value_init_object();
    json_array_append_value(json_array(pJsonSub), pJsonSubSub);
    json_object_set_string(json_object(pJsonSubSub), "item", "temperature");
    json_object_set_string(json_object(pJsonSubSub), "data", temperature_buf);
    json_object_set_string(json_object(pJsonSubSub), "status", temperature_status_buf);
    
    pJsonSubSub = json_value_init_object();
    json_array_append_value(json_array(pJsonSub), pJsonSubSub);
    json_object_set_string(json_object(pJsonSubSub), "item", "humidity");
    json_object_set_string(json_object(pJsonSubSub), "data", humidity_buf);
    json_object_set_string(json_object(pJsonSubSub), "status", humidity_status_buf);

    pJsonSubSub = json_value_init_object();
    json_array_append_value(json_array(pJsonSub), pJsonSubSub);
    json_object_set_string(json_object(pJsonSubSub), "item", "pressure");
    json_object_set_string(json_object(pJsonSubSub), "data", "");
    json_object_set_string(json_object(pJsonSubSub), "status", pressure_status_buf);
 
    p = json_serialize_to_string(pJsonRoot);
    send_data_len = m5311_tcp_send(m5311_tcp_info.tcp_sock_fd, (char *)p, strlen(p));
    if(send_data_len > 0)
        opencpu_printf("report data via socket %d: %s\r\n", m5311_tcp_info.tcp_sock_fd, p);
    else
        opencpu_printf("report data via socket %d fail\r\n", m5311_tcp_info.tcp_sock_fd);

    free(p); 
    json_value_free(pJsonRoot);

}

 /**
 *  \brief 通过tcp socket注册设备
 *  
 *  \param [in] name 设备名称
 *  \param [in] iccid 设备iccid
 *  \return void
 */
void m5311_tcp_json_register(const char *name, const char *iccid)
{
    char *p;
    int send_data_len = 0;

    JSON_Value *pJsonRoot = json_value_init_object();
    json_object_set_string(json_object(pJsonRoot), "type", "register");
    JSON_Value *pJsonSub = json_value_init_object();
    json_object_set_string(json_object(pJsonSub), "name", name);
    
    json_object_set_string(json_object(pJsonSub), "iccid", iccid);
    json_object_set_string(json_object(pJsonSub), "deviceType", "0");
    json_object_set_value(json_object(pJsonRoot), "info", pJsonSub);
    

    json_object_set_string(json_object(pJsonRoot), "status", "0");
    p = json_serialize_to_string(pJsonRoot);
    send_data_len = m5311_tcp_send(m5311_tcp_info.tcp_sock_fd, (char *)p, strlen(p));
    if(send_data_len > 0)
        opencpu_printf("device register success: %s\r\n", p);
    else
        opencpu_printf("device register fail\r\n");
    
    free(p); 
    json_value_free(pJsonRoot);
}

 /**
 *  \brief 通过tcp socket发送应答包
 *  
 *  \param [in] status 应答状态
 *  \return void
 */
void m5311_tcp_json_respond(int status)
{
    char *p;
    int send_data_len = 0;
    JSON_Value *pJsonRoot = json_value_init_object();
    json_object_set_string(json_object(pJsonRoot), "type", "response");
    json_object_set_string(json_object(pJsonRoot), "status", status);
    p = json_serialize_to_string(pJsonRoot);
    send_data_len = m5311_tcp_send(m5311_tcp_info.tcp_sock_fd, (char *)p, strlen(p));
    if(send_data_len > 0)
        opencpu_printf("respond success\r\n");
    else
        opencpu_printf("respond fail\r\n");
    
    free(p); 
    json_value_free(pJsonRoot);
}

 /**
 *  \brief 处理服务器通过tcp socket下发的数据
 *  
 *  \param [in] json_data 服务器下发数据
 *  \return void
 */
void m5311_tcp_json_data_process(const char *json_data)
{

    JSON_Value *pJsonRoot = json_parse_string(json_data);
    char *json_data_type = json_object_get_string(json_object(pJsonRoot), "type");
    char *json_data_item = NULL;
    char *json_data_data = NULL;
    char *json_data_data2 = NULL;
    char *json_data_token = NULL;

    float max_temperature_range;
    float min_temperature_range;
    float max_humidity_range;
    float min_humidity_range;
    int upload_interval = 0;

    if(NULL == json_data_type) 
        goto err_parse_json_data_fail;
    else if(!strcmp(json_data_type, "require")) {

    }
    else if(!strcmp(json_data_type, "modify")) {
        json_data_item = json_object_get_string(json_object(pJsonRoot), "item");
        if(NULL == json_data_item)
            goto err_parse_json_data_fail;
        if(!strcmp(json_data_item, "temperature")) {
            json_data_data = json_object_get_string(json_object_get_object(json_object(pJsonRoot), "data"), "max");
            json_data_data2 = json_object_get_string(json_object_get_object(json_object(pJsonRoot), "data"), "min");
            if(NULL == json_data_data || NULL == json_data_data2)
                goto err_parse_json_data_fail;
            m5311_set_new_temperature_range(atof(json_data_data), atof(json_data_data2));
            
            opencpu_printf("max temperature range <%.1f> updated.\r\n", m5311_setting.max_temperature_range);
            opencpu_printf("min temperature range <%.1f> updated.\r\n", m5311_setting.min_temperature_range);
        }
        else if(!strcmp(json_data_item, "humidity")) {
            json_data_data = json_object_get_string(json_object_get_object(json_object(pJsonRoot), "data"), "max");
            json_data_data2 = json_object_get_string(json_object_get_object(json_object(pJsonRoot), "data"), "min");
            if(NULL == json_data_data || NULL == json_data_data2)
                goto err_parse_json_data_fail;
            m5311_set_new_humidity_range(atof(json_data_data), atof(json_data_data2));
            
            opencpu_printf("max humidity range <%.1f> updated.\r\n", m5311_setting.max_humidity_range);
            opencpu_printf("min humidity range <%.1f> updated.\r\n", m5311_setting.min_humidity_range);
        }
        else if(!strcmp(json_data_item, "interval")) {
            json_data_data = json_object_get_string(json_object(pJsonRoot), "data");
            if(NULL == json_data_data)
                goto err_parse_json_data_fail;
            upload_interval = atoi(json_data_data);
            if(upload_interval > 0) {
                opencpu_printf("upload interval <%d> updated.\r\n", upload_interval);
                m5311_set_new_upload_interval(upload_interval);
            }
            else 
                opencpu_printf("invalid upload interval <%d>.\r\n", upload_interval);
        }
    }
    else if(!strcmp(json_data_type, "response")) {
        tcp_errno = no_err;
    }
    else if(!strcmp(json_data_type, "register-response")) {
        json_data_data = json_object_get_string(json_object(pJsonRoot), "token");
        if(NULL == json_data_data)
            goto err_parse_json_data_fail;
        strcpy(m5311_tcp_info.http_token, json_data_data);
        opencpu_printf("get http token <%s>.\r\n", m5311_tcp_info.http_token);
        tcp_errno = no_err;
    }
    json_value_free(pJsonRoot);
    return;

err_parse_json_data_fail: 
    opencpu_printf("json value parse fail.\r\n");
    json_value_free(pJsonRoot);
}

