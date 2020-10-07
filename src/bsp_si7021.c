#include "m5311_basic.h"
#include "bsp_si7021.h"

float si7021_get_temperature()//读取SI7021的温度数据
{
	int i,j,num_reasonable;
	uint16 temperature;
	uint16 f_temperature_array[6],f_temp;
	float f_temperature;
	float f_temperature_int10times;
	unsigned char data[6];
	static myFlag = 0;
	memset(data,0,6);
	opencpu_i2c_init();

	for(i = 0, j = 0; i < 6; i++) {
		data[0] = 0xF3;
		opencpu_i2c_write(0x40, data, 1);
		data[0] = 0x40;
		opencpu_i2c_write(0x40, data, 1);
		vTaskDelay(1);
		if(0 == opencpu_i2c_read(0x40, data, 4))//read ok
		{

			if((data[0] == data[2]) && (data[1] == data[3]))
			{
				temperature = data[0];
				temperature <<= 8;
				temperature += data[1];
				if(temperature < 2550 || temperature > 54768)//around f_temperature < -40 || f_temperature > 100
				{
					f_temperature = -40;
					continue;
				}
				else
				{
					f_temperature_array[j]=temperature;
					j++;
				}
			}
			else {
				f_temperature = -50;
			}
		}
		else
		{
			M5311_TRACE("read I2C failed\n");
			f_temperature = -60;
		}
		temperature = 0;
		vTaskDelay(1);
	}
;	if(j >= 3)
	{
		num_reasonable = j;
		for(i=0; i<num_reasonable; i++)
			for(j=i; j<num_reasonable; j++)
			{
				if(f_temperature_array[i] > f_temperature_array[j])
				{
					f_temp = f_temperature_array[i];
					f_temperature_array[i] = f_temperature_array[j];
					f_temperature_array[j] = f_temp;
				}
			}

		temperature = f_temperature_array[num_reasonable/2];
		f_temperature = (float)(temperature)*175.72/65536-46.85;
		f_temperature_int10times = (short)(f_temperature*10);
		M5311_TRACE("temperature: %.3f\r\n", f_temperature);
	}
	
	opencpu_i2c_deinit();
	return f_temperature;
}



float si7021_get_humidity()//读取SI7021的湿度数据
{
	int i,j,num_reasonable;
	uint16 humidity;
	uint16 f_humidity_array[6],f_temp;
	float f_humidity;
	unsigned char data[6];
	static myFlag = 0;
	memset(data, 0, 6);
	opencpu_i2c_init();

	for(i = 0, j = 0; i < 6; i++)
	{		
		data[0] = 0xF5;
		opencpu_i2c_write(0x40, data, 1);
		data[0] = 0x40;
		opencpu_i2c_write(0x40, data, 1);
		vTaskDelay(2);
		if(0 == opencpu_i2c_read(0x40, data, 4))
		{
			if((data[0] == data[2]) && (data[1] == data[3]))
			{
				humidity = data[0];
				humidity <<= 8;
				humidity += data[1];
				if(humidity < 3145 || humidity > 55573)//around f_temperature < -40 || f_temperature > 100
				{
					f_humidity = 0;
					continue;
				}
				else
				{
					f_humidity_array[j]=humidity;
					j++;
				}
			}
			else
				f_humidity = 0;
		}
		else
		{
			M5311_TRACE("read I2C failed\n");
			f_humidity = 0;
		}
		vTaskDelay(1);
	}

	if(j >= 3)
	{
		num_reasonable = j;
		for(i=0; i<num_reasonable; i++)
			for(j=i; j<num_reasonable; j++)
			{
				if(f_humidity_array[i] > f_humidity_array[j])
				{
					f_temp = f_humidity_array[i];
					f_humidity_array[i] = f_humidity_array[j];
					f_humidity_array[j] = f_temp;
				}
			}
		humidity = f_humidity_array[num_reasonable/2];
		f_humidity = (float)(humidity)*125.0/65535.0-6.0;
		M5311_TRACE("humidity:%.3f\r\n", f_humidity);
	}
	
	opencpu_i2c_deinit();
	return f_humidity;
}
