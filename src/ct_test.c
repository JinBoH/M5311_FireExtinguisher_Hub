#include "m5311_opencpu.h"

void onNmi_cb(int data_length)
{
    //TODO
    //...
    opencpu_printf("onNmi_cb:%d\n",data_length);
    return;
}
void onRead_cb(int read_actual_length, int remain_length, char *data)
{
    //TODO
    //...
    opencpu_printf("onRead_cb:%d,%d,%s", read_actual_length, remain_length, data);
    return;
}
void onEvtind_cb(int type)
{
    //TODO
    //...
    opencpu_printf("onEvtind_cb:%d\n",type);
    return;
}
void onStr_cb(int seq, int status)
{
    //TODO
    //...
    opencpu_printf("onStr_cb:%d, %d", seq, status);
    return;
}
void onDrop_cb(int length)
{
    //TODO
    //...
    opencpu_printf("onDrop_cb:%d\n",length);
    return;
}

void test_ct()
{
	ct_cb_t callback;
    callback.onNmi = onNmi_cb;
    callback.onRead = onRead_cb;
    callback.onEvtind = onEvtind_cb;
    callback.onStr = onStr_cb;
    callback.onDrop = onDrop_cb;

    opencpu_ct_create(&callback);//初始化任务

    if (APB_PROXY_RESULT_OK == opencpu_ct_new("180.101.147.115", "5683"))
    {
        opencpu_printf("ct_new OK\n");
    }
    else
    {
        opencpu_printf("ct_new fail\n");
    }
    vTaskDelay(1000 / portTICK_RATE_MS); 

    if(APB_PROXY_RESULT_OK == opencpu_ct_open(0, 90))
    {
        opencpu_printf("ct_open OK\n");
    }
    else
    {
        opencpu_printf("ct_open fail\n");
    }
    vTaskDelay(4000 / portTICK_RATE_MS);

    if(APB_PROXY_RESULT_OK == opencpu_ct_update())
    {
        opencpu_printf("ct_update OK\n");
    }
    else
    {
        opencpu_printf("ct_update fail\n");
    }
    vTaskDelay(1000 / portTICK_RATE_MS);

    if(APB_PROXY_RESULT_OK == opencpu_ct_setcfg(1,1))
    {
        opencpu_printf("ct_setcfg OK\n");
    }
    else
    {
        opencpu_printf("ct_setcfg fail\n");
    }

    if(APB_PROXY_RESULT_OK == opencpu_ct_send_ex(10, "1234567891", 0, 1))
    {
        opencpu_printf("ct_send1 OK\n");
    }
    else
    {
        opencpu_printf("ct_send1 fail\n");
    }
    vTaskDelay(1000 / portTICK_RATE_MS);

    if(APB_PROXY_RESULT_OK == opencpu_ct_send_ex(10, "1234567892", 0, 1))
    {
        opencpu_printf("ct_send1 OK\n");
    }
    else
    {
        opencpu_printf("ct_send1 fail\n");
    }

    if(APB_PROXY_RESULT_OK == opencpu_ct_close())
    {
        opencpu_printf("ct_close OK\n");
    }
    else
    {
        opencpu_printf("ct_close fail\n");
    }
    vTaskDelay(3000 / portTICK_RATE_MS);
    if(APB_PROXY_RESULT_OK == opencpu_ct_client_delte())
    {
        opencpu_printf("ct_delte OK\n");
    }
    else
    {
        opencpu_printf("ct_delte fail\n");
    }
	
}