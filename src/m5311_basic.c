#include "m5311_basic.h"
#include "bsp_si7021.h"

m5311_condition_t m5311_condition;
m5311_setting_t m5311_setting;
m5311_info_t m5311_info;
m5311_dispatch_flag_t m5311_dispatch_flag;

struct tm m5311_tm;

static char days_per_month[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

void m5311_read_setting()
{
    opencpu_flash_read (M5311_SETTING_ADDR, (u8 *)&m5311_setting, sizeof(m5311_setting_t));

    if(m5311_setting.device_name_state != VALID) 
        strcpy(m5311_setting.device_name, "test device");
     M5311_TRACE("device name: %s\r\n", m5311_setting.device_name);

    if(m5311_setting.temperature_range_state == VALID) {
        M5311_TRACE("max temperature range: %.1f\r\n", m5311_setting.max_temperature_range);
        M5311_TRACE("min temperature range: %.1f\r\n", m5311_setting.min_temperature_range);
    }
    else
        M5311_TRACE("invalid temperature range\r\n");

    if(m5311_setting.humidity_range_state == VALID) {
        M5311_TRACE("max humidity range: %.1f\r\n", m5311_setting.max_humidity_range);
        M5311_TRACE("min humidity range: %.1f\r\n", m5311_setting.min_humidity_range);
    }
    else
        M5311_TRACE("invalid humidity range\r\n");
    
    if(m5311_setting.upload_interval_state == VALID)  
        M5311_TRACE("upload interval: %d hours\r\n", m5311_setting.upload_interval);
    else
        M5311_TRACE("invalid upload interval\r\n");
}

void m5311_get_info()
{
    m5311_info.temperature = si7021_get_temperature();
    m5311_info.humidity = si7021_get_humidity();

    if(m5311_info.temperature < m5311_setting.min_temperature_range)
        m5311_info.temperature_status = 1;
    else if(m5311_info.temperature > m5311_setting.max_temperature_range)
        m5311_info.temperature_status = 2;
    else
        m5311_info.temperature_status = 0;

    if(m5311_info.humidity < m5311_setting.min_humidity_range)
        m5311_info.humidity_status = 1;
    else if(m5311_info.humidity > m5311_setting.max_humidity_range)
        m5311_info.humidity_status = 2;
    else
        m5311_info.humidity_status = 0;
    m5311_info.temperature_status = 0;
    m5311_info.pressure_status = 0;
}

void m5311_get_time(struct tm *l_tm)
{
    unsigned char time_string[50];
	unsigned char *p1;
	unsigned char *p2;
	
	memset(time_string,0,50);
	opencpu_rtc_get_time(time_string);
	M5311_TRACE("TIME:%s\r\n",time_string);
	p1 = strchr(time_string,'/');
	*p1 = 0;
	l_tm->tm_year = atoi(time_string)-1900;
	p2 = strchr(p1+1,'/');
	*p2 = 0;
	l_tm->tm_mon = atoi(p1+1)-1;	
	p1 = strchr(p2+1,',');
	*p1 =0;
	l_tm->tm_mday = atoi(p2+1);	
	p2 = strchr(p1+1,':');
	*p2 = 0;
	l_tm->tm_hour = atoi(p1+1);
	p1 = strchr(p2+1,':');
	*p1 = 0;
	l_tm->tm_min = atoi(p2+1);
	p2  = strchr(p1+1,'G');
	*p2 = 0;
	l_tm->tm_sec = atoi(p1+1);
	M5311_TRACE("secs:%d\r\n",mktime(l_tm));
}

static void get_future_timestamp_by_hour(struct tm now_tm, struct tm *future_tm, int hour)
{
    int i;
    int remain_hour_in_a_day = hour % 24;
    int day = hour / 24;
    int remain_day_in_this_month;
    int remain_day;
    int year = now_tm.tm_year + 1900;

    if(now_tm.tm_hour + remain_hour_in_a_day >= 24) {
        now_tm.tm_mday += 1;
        now_tm.tm_hour = (now_tm.tm_hour + remain_hour_in_a_day) - 24;
    }
    else
    now_tm.tm_hour += remain_hour_in_a_day;

    //当月剩余天数
    remain_day_in_this_month = days_per_month[now_tm.tm_mon - 1] - now_tm.tm_mday;

    //在本月外
    if(day > remain_day_in_this_month) {

        //除当月剩余天数外的天数
        remain_day = day - remain_day_in_this_month;
        //月份先+1
        ++now_tm.tm_mon;
        //从下个月开始计算剩余天数
        i = now_tm.tm_mon-1;
        while(1) {
            //在下个月内
            if(remain_day < days_per_month[i] + 1)  //每个月没有第0天
                break;
            else {  //在下个月之后，获得除下个月之外的天数
                remain_day -= days_per_month[i++];
                if(now_tm.tm_mon == 12) {
                    now_tm.tm_mon = 1;
                    ++now_tm.tm_year;

                    //判断下一年是否为闰年
                    year = now_tm.tm_year + 1900;
                    if((year % 400 == 0) || (year % 4 == 0 && year % 100 != 0))
                        days_per_month[1] = 29;
                    else
                        days_per_month[1] = 28;
                    i = 0;
                }
                else
                    ++now_tm.tm_mon;
            }
        }
        //加上月份内的剩余天数
        now_tm.tm_mday = remain_day;
    }
    else    //在本月内，直接加上天数
        now_tm.tm_mday += day;

    memcpy(future_tm, &now_tm, sizeof(struct tm));
        
}

void m5311_set_new_upload_timestamp()
{
    m5311_setting_t m5311_tmp_setting;
    struct tm tmp_tm;
    int hour, day, month, year;

    opencpu_flash_read (M5311_SETTING_ADDR, (u8 *)&m5311_tmp_setting, sizeof(m5311_setting_t));
    
    //上传间隔无效
    if(m5311_setting.upload_interval_state != VALID) {  
       //flash中也无有效上传间隔，说明本设备还未获取上传间隔
        if(m5311_tmp_setting.upload_interval_state != VALID) {
           
           //向服务器请求上传间隔

           return;
        }
        else {
            m5311_setting.upload_interval_state = VALID;
            m5311_setting.upload_interval = m5311_tmp_setting.upload_interval;
        }
    }
    
    //上次上传时间戳无效
    if(m5311_setting.m5311_time_uploaded.state != VALID) {  
       //flash中无有效的上次上传时间戳，设备还未进行过上传
        if(m5311_tmp_setting.m5311_time_uploaded.state != VALID) {
            //从当前时间算起
            m5311_get_time(&tmp_tm);

            //运行到此处时，设备必然已经获取到上传间隔
            //构建下次上传时间戳
            get_future_timestamp_by_hour(tmp_tm, &(m5311_tmp_setting.m5311_time_to_upload.tm), m5311_tmp_setting.upload_interval);
            m5311_tmp_setting.m5311_time_to_upload.state = VALID;
            memcpy(&(m5311_setting.m5311_time_to_upload), &(m5311_tmp_setting.m5311_time_to_upload), sizeof(m5311_time_t));
           
        }
        else {  //flash存在有效的上次上传时间戳
            get_future_timestamp_by_hour(m5311_tmp_setting.m5311_time_uploaded.tm, &(m5311_tmp_setting.m5311_time_to_upload.tm), m5311_tmp_setting.upload_interval);
            m5311_tmp_setting.m5311_time_to_upload.state = VALID;
            memcpy(&(m5311_setting.m5311_time_to_upload), &(m5311_tmp_setting.m5311_time_to_upload), sizeof(m5311_time_t));
        }
    }
    else {
         get_future_timestamp_by_hour(m5311_tmp_setting.m5311_time_uploaded.tm, &(m5311_tmp_setting.m5311_time_to_upload.tm), m5311_tmp_setting.upload_interval);
    }
    opencpu_flash_erase(M5311_SETTING_ADDR, sizeof(m5311_setting_t));
    //将新时间戳写入flash
    opencpu_flash_write(M5311_SETTING_ADDR, (u8 *)&m5311_tmp_setting, sizeof(m5311_setting_t));

}

void m5311_set_device_name(char *device_name)
{
    m5311_setting_t m5311_tmp_setting;
    opencpu_flash_read (M5311_SETTING_ADDR, (u8 *)&m5311_tmp_setting, sizeof(m5311_setting_t));
    strcpy(m5311_tmp_setting.device_name, device_name);
    m5311_tmp_setting.device_name_state = VALID;
    opencpu_flash_erase(M5311_SETTING_ADDR, sizeof(m5311_setting_t));
    opencpu_flash_write(M5311_SETTING_ADDR, (u8 *)&m5311_tmp_setting, sizeof(m5311_setting_t));

}

void m5311_set_new_upload_interval(int new_interval)
{
    m5311_setting.upload_interval = new_interval;
    m5311_setting.upload_interval_state = VALID;
    opencpu_flash_erase(M5311_SETTING_ADDR, sizeof(m5311_setting_t));
    opencpu_flash_write(M5311_SETTING_ADDR, (u8 *)&m5311_setting, sizeof(m5311_setting_t));
}

void m5311_set_new_temperature_range(float max, float min)
{
    m5311_setting.max_temperature_range = max;
    m5311_setting.min_temperature_range = min;    
    m5311_setting.temperature_range_state = VALID;
    opencpu_flash_erase(M5311_SETTING_ADDR, sizeof(m5311_setting_t));
    opencpu_flash_write(M5311_SETTING_ADDR, (u8 *)&m5311_setting, sizeof(m5311_setting_t));
}

void m5311_set_new_humidity_range(float max, float min)
{
    m5311_setting.max_humidity_range = max;
    m5311_setting.min_humidity_range  = min;
    m5311_setting.humidity_range_state = VALID;
    opencpu_flash_erase(M5311_SETTING_ADDR, sizeof(m5311_setting_t));
    opencpu_flash_write(M5311_SETTING_ADDR, (u8 *)&m5311_setting, sizeof(m5311_setting_t));
}
