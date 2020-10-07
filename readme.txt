版本：M5311-opencpu_sdk_v3.1.0_release
日期：2019\8\7
1.使用前请阅读M5311 OpenCPU 开发指导手册.pdf
2.用户函数在void opencpu_task_main()中添加，该函数作为一个任务运行，不能长时间阻塞，否则会死机
3.请参照src目录下各文件中示例代码进行开发，onenet功能请参照onenet_test.c及opencpu_onenet.h进行开发。
4.编译后会生成新的nbiot_m2m_demo.bin，运行flashtool工具，将程序下载到模组即可
5.请使用软件接口文档提供的接口函数及该文档明确指定的头文件包含的接口函数进行SDK开发，如使用除此以外的接口，不能保证软件稳定性。
6.本SDK仅支持在windows下使用

本次主要更新：
	1.增加TAU参数读取支持（CEREG命令）
	2.修复GPIO27拉低后被干扰的问题
	3.增加SPI只读取不写入的接口
	4.部分OneNET问题修复
	5.FOTA增加上报状态和上报参数配置机制
	6.SDK 增加Backtrace机制
	7.AndLink问题修复
	
Backtrace机制说明：
用途：用于在死机情况发生时，快速定位死机原因和死机时的函数调用栈（目前暂不能定位因为死循环导致的看门狗死机）
使用方法：
1.必须按如下实现opencpu_printf函数
void opencpu_printf (const char *str, ...)
{

    static unsigned char s[600]; //This needs to be large enough to store the string TODO Change magic number
	int i,j;
	unsigned char *p;
    va_list args;
    int str_len;

    if ((str == NULL) || (strlen(str) == 0))
    {
        return;
    }
    va_start (args, str);
    str_len = (unsigned int)vsprintf ((char*)s, str, args);
    va_end (args);
    p =s;
	while(str_len > 0)
	{
		j = opencpu_uart_send(OPENCPU_MAIN_UART, p, str_len);
		p=p+j;
		str_len = str_len - j;
	}
}
2.必须实现 OC_DEBUG_CHANNEL变量
    unsigned int OC_DEBUG_CHANNEL;//0：MTK默认Fault处理 1：cm_backtrace fault处理
3.测试方法
	a.编译原始固件后，在串口1执行‘-’，即可看到cm_backtrace的打印信息，在最后一行，有如下内容：
		addr2line -e nbiot_m2m_demo.elf -a -f 0804caee 080da1df 080da217 0804bf49
	b.将addr2line工具（在util目录下）和固件对应的.elf文件放到统一目录下，如目录A
	c.在目录A下命令行执行串口打印出的命令：addr2line -e nbiot_m2m_demo.elf -a -f 0804caee 080da1df 080da217 0804bf49（以串口实际打出的内容为准）
	d.根据命令执行的结果，即可确定死机代码的位置