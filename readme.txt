�汾��M5311-opencpu_sdk_v3.1.0_release
���ڣ�2019\8\7
1.ʹ��ǰ���Ķ�M5311 OpenCPU ����ָ���ֲ�.pdf
2.�û�������void opencpu_task_main()����ӣ��ú�����Ϊһ���������У����ܳ�ʱ�����������������
3.�����srcĿ¼�¸��ļ���ʾ��������п�����onenet���������onenet_test.c��opencpu_onenet.h���п�����
4.�����������µ�nbiot_m2m_demo.bin������flashtool���ߣ����������ص�ģ�鼴��
5.��ʹ������ӿ��ĵ��ṩ�Ľӿں��������ĵ���ȷָ����ͷ�ļ������Ľӿں�������SDK��������ʹ�ó�������Ľӿڣ����ܱ�֤����ȶ��ԡ�
6.��SDK��֧����windows��ʹ��

������Ҫ���£�
	1.����TAU������ȡ֧�֣�CEREG���
	2.�޸�GPIO27���ͺ󱻸��ŵ�����
	3.����SPIֻ��ȡ��д��Ľӿ�
	4.����OneNET�����޸�
	5.FOTA�����ϱ�״̬���ϱ��������û���
	6.SDK ����Backtrace����
	7.AndLink�����޸�
	
Backtrace����˵����
��;�������������������ʱ�����ٶ�λ����ԭ�������ʱ�ĺ�������ջ��Ŀǰ�ݲ��ܶ�λ��Ϊ��ѭ�����µĿ��Ź�������
ʹ�÷�����
1.���밴����ʵ��opencpu_printf����
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
2.����ʵ�� OC_DEBUG_CHANNEL����
    unsigned int OC_DEBUG_CHANNEL;//0��MTKĬ��Fault���� 1��cm_backtrace fault����
3.���Է���
	a.����ԭʼ�̼����ڴ���1ִ�С�-�������ɿ���cm_backtrace�Ĵ�ӡ��Ϣ�������һ�У����������ݣ�
		addr2line -e nbiot_m2m_demo.elf -a -f 0804caee 080da1df 080da217 0804bf49
	b.��addr2line���ߣ���utilĿ¼�£��͹̼���Ӧ��.elf�ļ��ŵ�ͳһĿ¼�£���Ŀ¼A
	c.��Ŀ¼A��������ִ�д��ڴ�ӡ�������addr2line -e nbiot_m2m_demo.elf -a -f 0804caee 080da1df 080da217 0804bf49���Դ���ʵ�ʴ��������Ϊ׼��
	d.��������ִ�еĽ��������ȷ�����������λ��