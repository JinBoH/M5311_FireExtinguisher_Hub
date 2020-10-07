/*
   M5311_opencpu.c
   created by xgl,2018/4/2
*/

#include "bsp_si7021.h"
#include "parson.h"
#include "m5311_tcp_utils.h"
#include "m5311_http_utils.h"

#define INIT_TASK_PRIO			1
#define TCP_TASK_PRIO			5
#define REPORT_TASK_PRIO		6
#define STREAM_TASK_PRIO		5

#define INIT_STK_SIZE 			1024
#define TCP_STK_SIZE 			2048
#define REPORT_STK_SIZE 		1024
#define STREAM_STK_SIZE 		1024

TaskHandle_t InitTask_Handler;
TaskHandle_t TCPTask_Handler;
TaskHandle_t ReportTask_Handler;
TaskHandle_t StreamTask_Handler;

extern unsigned int OC_DEBUG_CHANNEL;

char tcp_recv_buf[500];
char http_recv_buf[1000];
unsigned char imsi_buf[40] = {0};
unsigned char imei_buf[40] = {0};

int sec_remain;
int info_upload_flag;
unsigned int rtc_timer_handle;

static void rtc_timer_callback()
{ 
	++sec_remain;
	opencpu_printf("%d ", sec_remain);
	if(sec_remain == 60) {
		sec_remain = 0;
		m5311_dispatch_flag.rtc_expire_flag = 1;
	}
}

void m5311_bsp_init()
{
	int retry_cnt = 0;
	int rssi, rxlevel;
	char length_buf[6];

	unsigned char base_version[30] = {0};
	memset(length_buf, 0, 6);

	custom_uart_init();
	
	opencpu_get_base_version(base_version);
	opencpu_printf("BASE_VERSION:%s\r\n",base_version);
	opencpu_printf("HW VERSION:%d\r\n",get_band_version());
	opencpu_printf("update status:%d\r\n",update_status);
	opencpu_printf("run mode:%d\r\n",get_run_mode());
	m5311_read_setting();
	
	// HTTPRequest = HTTPRequestCreate();
	// HTTPRequestAddType(HTTPRequest, "POST", "/device/upload.do", "HTTP/1.1");
	// HTTPRequestAddHeaders(HTTPRequest, "HOST", "http://img.xingketec.cn");
	// HTTPRequestAddHeaders(HTTPRequest, "xktoken", "ODk4NjA0MTExMTE4ODAxMTg5MDI=");
	// HTTPRequestAddHeaders(HTTPRequest, "Connection", "keep-alive");
	// sprintf(length_buf, "%d", HTTPGetFileContentLength(80066, "----WebKitFormBoundary7MA4YWxkTrZu0gW", "file", "test.bmp", "image/bmp"));
	// HTTPRequestAddHeaders(HTTPRequest, "Content-Length", length_buf);
	// HTTPRequestAddHeaders(HTTPRequest, "Content-Type", "multipart/form-data; boundary=----WebKitFormBoundary7MA4YWxkTrZu0gW");

	if(opencpu_is_boot_from_sleep()==1)
		opencpu_printf("BOOT CAUSE:WAKE FROM SLEEP\r\n");
	else
		opencpu_printf("BOOT CAUSE:POWER_ON OR RESET\r\n");
	
	opencpu_printf("M5311 opencpu ready\r\n");

	opencpu_printf("waiting for network...\r\n");
    opencpu_lock_light_sleep();
	
	opencpu_csq(&rssi,&rxlevel);
	opencpu_printf("%d,%d\r\n",rssi, rxlevel);

	//阻塞方式获取ICCID，必须要有SIM卡才能读到
	m5311_get_iccid();

	memset(imei_buf, 0, 40);
	opencpu_get_imei(imei_buf);
	opencpu_printf("IMEI:%s\r\n",imei_buf);

	memset(imsi_buf, 0, 40);
	opencpu_get_imsi(imsi_buf);
	opencpu_printf("IMSI:%s\r\n",imsi_buf);

	//获取网络注册状态，并阻塞等待网络注册成功
	opencpu_printf("network registering...");
	while(opencpu_cgact() != 1) {
		vTaskDelay(50);
		opencpu_printf(".");
		++retry_cnt;
		if(retry_cnt > 20) {
			opencpu_printf("network register failed, system reboot\r\n");
			opencpu_reboot();
		}
	}
	opencpu_printf("\r\n");
	opencpu_printf("network register success\r\n");
	opencpu_printf("network ready\r\n");
	m5311_get_time(&m5311_tm);
	opencpu_rtc_timer_create(&rtc_timer_handle, 10, true, rtc_timer_callback);
	while(m5311_tcp_connect()) {
		++retry_cnt;
		if(retry_cnt > 5) {
			opencpu_printf("tcp connect failed, system reboot\r\n");
			opencpu_reboot();
		}
	}
	retry_cnt = 0;
	m5311_tcp_json_register(m5311_setting.device_name, m5311_tcp_info.iccid_buf);
	opencpu_rtc_timer_start(rtc_timer_handle);
	// opencpu_printf("system boot.\r\n");
	// vTaskDelay(100);
	// m5311_stream_json_require("image");
}

static void m5311_tcp_task()
{
	int data_len;
	int i = 0;

	while(1) {
		data_len = recv(m5311_tcp_info.tcp_sock_fd, tcp_recv_buf, 200, MSG_TRUNC | MSG_DONTWAIT);
		if(data_len > 0) {
			opencpu_printf("receive %d bytes data : %s\r\n",data_len, tcp_recv_buf);
			if(strcmp(tcp_recv_buf, "Heartbeat"))
				m5311_tcp_json_data_process(tcp_recv_buf);
			memset(tcp_recv_buf, 0, sizeof(tcp_recv_buf));
		}

		data_len = recv(m5311_tcp_info.http_sock_fd, http_recv_buf, 1000, MSG_TRUNC | MSG_DONTWAIT);
			
		if(data_len > 0) {
			opencpu_printf("receive data : %s\r\n",http_recv_buf);
			memset(http_recv_buf, 0, sizeof(http_recv_buf));
		}
		vTaskDelay(5);
	}
}

static void m5311_report_task()
{
	while(1) {
		if(m5311_dispatch_flag.rtc_expire_flag) {
			m5311_dispatch_flag.rtc_expire_flag = 0;
			opencpu_printf("get info\r\n");
			m5311_get_info();
			m5311_tcp_json_report_info(m5311_info);
		}
		vTaskDelay(5);
	}
}

static void m5311_stream_task()
{
	while(1) {
		if(usart_rx_sta > 0) {
			usart_rx_sta = 0;
			//M5311_TRACE("%s",usart_rx_buffer);
			m5311_stream_data_process(usart_rx_buffer);
			memset(usart_rx_buffer, 0, 1025);
		}
		vTaskDelay(2);
	}
}

static void m5311_init_task()
{
	m5311_bsp_init();

	xTaskCreate((TaskFunction_t )m5311_tcp_task,
			(const char*    )"m5311 tcp task", 
			(uint16_t       )TCP_STK_SIZE, 
			(void*          )NULL, 
			(UBaseType_t    )TASK_PRIORITY_NORMAL, 
			(TaskHandle_t*  )&TCPTask_Handler);

	xTaskCreate((TaskFunction_t )m5311_report_task,
			(const char*    )"m5311 report task", 
			(uint16_t       )REPORT_STK_SIZE, 
			(void*          )NULL, 
			(UBaseType_t    )TASK_PRIORITY_NORMAL, 
			(TaskHandle_t*  )&ReportTask_Handler);

	xTaskCreate((TaskFunction_t )m5311_stream_task,
			(const char*    )"m5311 stream task", 
			(uint16_t       )STREAM_STK_SIZE, 
			(void*          )NULL, 
			(UBaseType_t    )TASK_PRIORITY_NORMAL, 
			(TaskHandle_t*  )&StreamTask_Handler);

	vTaskDelete(InitTask_Handler);     
}

/*
 新建opencpu任务，这个函数用户不可更改
*/
void test_opencpu_start()
{
	xTaskCreate((TaskFunction_t )m5311_init_task,
				(const char*    )"m5311 init task", 
				(uint16_t       )INIT_STK_SIZE, 
				(void*          )NULL, 
				(UBaseType_t    )TASK_PRIORITY_NORMAL, 
				(TaskHandle_t*  )&InitTask_Handler);
}
