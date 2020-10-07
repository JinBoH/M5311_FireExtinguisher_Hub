#ifndef __M5311_BASIC_H
#define __M5311_BASIC_H

#include "m5311_opencpu.h"

#define M5311_DEBUG

#define M5311_TRACE(...) opencpu_printf(__VA_ARGS__)
// #ifdef M5311_DEBUG 
//     #define M5311_TRACE(...) opencpu_printf(__VA_ARGS__)
// #else
//     #define M5311_TRACE(...)
// #endif

#define VALID   0x53

#define M5311_FLASH_BASE_ADDR           0
#define M5311_FLASH_SECTOR_SIZE         4096
#define M5311_SETTING_ADDR              (M5311_FLASH_BASE_ADDR + M5311_FLASH_SECTOR_SIZE * 0)

typedef struct {
    int state;
    struct tm tm;
}m5311_time_t;


typedef struct {
    char camera_condition;
    char si7021_condition;
    char other_dev_condition;
}m5311_condition_t;

typedef struct {
    int device_name_state;
    char device_name[32];
    int temperature_range_state;
    float max_temperature_range;
    float min_temperature_range;
    int humidity_range_state;
    float max_humidity_range;
    float min_humidity_range;
    int upload_interval_state;
    int upload_interval;
    m5311_time_t m5311_time_to_upload;
    m5311_time_t m5311_time_uploaded;
}m5311_setting_t;

typedef struct {
    float temperature;
    char temperature_status;
    float humidity;
    char humidity_status;
    char pressure_status;
}m5311_info_t;

typedef struct {
    char rtc_expire_flag;
    char upload_flag;
    char ping_flag;
    char time_calibrate_flag;
}m5311_dispatch_flag_t;

extern m5311_condition_t m5311_condition;
extern m5311_setting_t m5311_setting;
extern m5311_info_t m5311_info;
extern m5311_dispatch_flag_t m5311_dispatch_flag;
extern struct tm m5311_tm;

void m5311_read_setting(void);
void m5311_get_info(void);
void m5311_get_time(struct tm *l_tm);
void m5311_set_new_upload_timestamp(void);
void m5311_set_device_name(char *device_name);
void m5311_set_new_upload_interval(int new_interval);
void m5311_set_new_temperature_range(float max, float min);
void m5311_set_new_humidity_range(float max, float min);

#endif