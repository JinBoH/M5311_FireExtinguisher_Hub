/*
   opencpu_iic.c
   created by xgl,2019/1/22
*/

#include "m5311_opencpu.h"
#include <math.h>

typedef struct __BMP180
{
	short AC1;
	short AC2;
	short AC3;
	unsigned short AC4;
	unsigned short AC5;
	unsigned short AC6;
	short B1;
	short B2;
	short MB;
	short MC;
	short MD;
	long UT;
	long UP;
	long X1;
	long X2;
	long X3;
	long B3;
	unsigned long B4;
	long B5;
	long B6;
	long B7;
	long p;
	long Temp;
	float altitude;
}_bmp180;

_bmp180 bmp180;

void BMP_ReadCalibrationData(void);
long BMP_Read_UT(void);
long BMP_Read_UP(void);
void BMP_UncompemstatedToTrue(void);

void BMP_ReadCalibrationData()
{
	unsigned char temp[3];
	memset(temp,0,3);
	opencpu_i2c_write_read(0x77,0xAA,temp,2);
	bmp180.AC1 = (temp[0]<<8)|temp[1];
	
	memset(temp,0,3);
	opencpu_i2c_write_read(0x77,0xAC,temp,2);
	bmp180.AC2 = (temp[0]<<8)|temp[1];
	
	memset(temp,0,3);
	opencpu_i2c_write_read(0x77,0xAE,temp,2);
	bmp180.AC3 = (temp[0]<<8)|temp[1];
	
	memset(temp,0,3);
	opencpu_i2c_write_read(0x77,0xB0,temp,2);
	bmp180.AC4 = (temp[0]<<8)|temp[1];
	
	memset(temp,0,3);
	opencpu_i2c_write_read(0x77,0xB2,temp,2);
	bmp180.AC5 = (temp[0]<<8)|temp[1];
	
	memset(temp,0,3);
	opencpu_i2c_write_read(0x77,0xB4,temp,2);
	bmp180.AC6 = (temp[0]<<8)|temp[1];
	
	memset(temp,0,3);
	opencpu_i2c_write_read(0x77,0xB6,temp,2);
	bmp180.B1 = (temp[0]<<8)|temp[1];
	
	memset(temp,0,3);
	opencpu_i2c_write_read(0x77,0xB8,temp,2);
	bmp180.B2 = (temp[0]<<8)|temp[1];
	
	memset(temp,0,3);
	opencpu_i2c_write_read(0x77,0xBA,temp,2);
	bmp180.MB = (temp[0]<<8)|temp[1];
	
	memset(temp,0,3);
	opencpu_i2c_write_read(0x77,0xBC,temp,2);
	bmp180.MC = (temp[0]<<8)|temp[1];
	
	memset(temp,0,3);
	opencpu_i2c_write_read(0x77,0xBE,temp,2);
	bmp180.MD = (temp[0]<<8)|temp[1];
	
}


long BMP_Read_UT(void)
{
	unsigned char buffer[3];
	memset(buffer,0,3);
	buffer[0] = 0xF4;
	buffer[1] = 0x2E;
	opencpu_i2c_write(0x77,buffer,2);	
	vTaskDelay(1);
	memset(buffer,0,3);
	opencpu_i2c_write_read(0x77,0xF6,buffer,2);
	return (buffer[0]<<8)|buffer[1];
}
long BMP_Read_UP(void)
{
	long pressure = 0;
	unsigned char buffer[3];
	memset(buffer,0,3);
	buffer[0] = 0xF4;
	buffer[1] = 0x34;
	opencpu_i2c_write(0x77,buffer,2);	
	vTaskDelay(1);
	memset(buffer,0,3);
	opencpu_i2c_write_read(0x77,0xF6,buffer,2);
	
	return (buffer[0]<<8)|buffer[1];
}
//ÓÃ»ñÈ¡µÄ²ÎÊý¶ÔÎÂ¶ÈºÍ´óÆøÑ¹½øÐÐÐÞÕý£¬²¢¼ÆËãº£°Î
void BMP_UncompemstatedToTrue(void)
{
	bmp180.UT = BMP_Read_UT();//µÚÒ»´Î¶ÁÈ¡´íÎó
	bmp180.UT = BMP_Read_UT();//½øÐÐµÚ¶þ´Î¶ÁÈ¡ÐÞÕý²ÎÊý
	bmp180.UP = BMP_Read_UP();
	
	bmp180.X1 = ((bmp180.UT - bmp180.AC6) * bmp180.AC5) >> 15;
	bmp180.X2 = (((long)bmp180.MC) << 11) / (bmp180.X1 + bmp180.MD);
	bmp180.B5 = bmp180.X1 + bmp180.X2;
	bmp180.Temp  = (bmp180.B5 + 8) >> 4;
	
	bmp180.B6 = bmp180.B5 - 4000;
	bmp180.X1 = ((long)bmp180.B2 * (bmp180.B6 * bmp180.B6 >> 12)) >> 11;
	bmp180.X2 = ((long)bmp180.AC2) * bmp180.B6 >> 11;
	bmp180.X3 = bmp180.X1 + bmp180.X2;
	
	bmp180.B3 = ((((long)bmp180.AC1) * 4 + bmp180.X3) + 2) /4;
	bmp180.X1 = ((long)bmp180.AC3) * bmp180.B6 >> 13;
	bmp180.X2 = (((long)bmp180.B1) *(bmp180.B6*bmp180.B6 >> 12)) >>16;
	bmp180.X3 = ((bmp180.X1 + bmp180.X2) + 2) >> 2;
	bmp180.B4 = ((long)bmp180.AC4) * (unsigned long)(bmp180.X3 + 32768) >> 15;
	bmp180.B7 = ((unsigned long)bmp180.UP - bmp180.B3) * 50000;
	
	if(bmp180.B7 < 0x80000000)
	{
		bmp180.p = (bmp180.B7 * 2) / bmp180.B4;		
	}
	else
	{
		bmp180.p = (bmp180.B7 / bmp180.B4) * 2;
	}
	
	bmp180.X1 = (bmp180.p >> 8) * (bmp180.p >>8);
	bmp180.X1 = (((long)bmp180.X1) * 3038) >> 16;
	bmp180.X2 = (-7357 * bmp180.p) >> 16;
	
	bmp180.p = bmp180.p + ((bmp180.X1 + bmp180.X2 + 3791) >> 4);
	
	bmp180.altitude = 44330 * (1-pow(((bmp180.p) / 101325.0),(1.0/5.255)));  
}
int BMP180_get_id()
{
	unsigned char id[3];
	opencpu_i2c_init();
	opencpu_i2c_set_freq(HAL_I2C_FREQUENCY_50K);
	memset(id,0,2);
	opencpu_i2c_write_read(0x77,0xD0,id,1);
	if(id[0] != 0)
	{
		opencpu_printf("ID:%d\n",id[0]);
		return 0;
	}
	else
	{
		return -1;
	}
}
void bmp180_test()
{
	int i;
	if(BMP180_get_id())
	{
		opencpu_printf("no bmp180 found!\n");
	}
	BMP_ReadCalibrationData();
	for(i=0;i<10;i++)
	{
		BMP_UncompemstatedToTrue();
		opencpu_printf("temp = %d.%dC\t   Pressure = %ldPa\t   Altitude = %.5fm\r\n",bmp180.Temp/10,bmp180.Temp%10,bmp180.p,bmp180.altitude);
		vTaskDelay(100);
	}
}
