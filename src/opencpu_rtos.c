#include "m5311_opencpu.h"


void opencpu_stack_overflow_hook(xTaskHandle *pxTask, signed portCHAR * pcTaskName)
{
	opencpu_printf("stack overflow");
}

void vApplicationTickHook( void )
{

}

void opencpu_task_idle_hook(void)
{
	
}


