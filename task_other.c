#include "stm32f10x.h"
#include "cmsis_os.h"
#include "periph.h"

#define LEDDY 50

void task_flowing_led (void const * arg)
{
	osThreadId mid;
	uint8_t i=0;
	
	for (i=0; i<2; i++)
	{
		GPIO_ResetBits(LED_GP, LED_GR);
		osDelay(LEDDY);
		GPIO_SetBits(LED_GP, LED_GR);
		GPIO_ResetBits(LED_GP, LED_BL);
		osDelay(LEDDY);
		GPIO_SetBits(LED_GP, LED_BL);
		GPIO_ResetBits(LED_GP, LED_YL);
		osDelay(LEDDY);
		GPIO_SetBits(LED_GP, LED_YL);
		GPIO_ResetBits(LED_GP, LED_RD);
		osDelay(LEDDY);
		GPIO_SetBits(LED_GP, LED_RD);
		osDelay(LEDDY);
	}
	osDelay(LEDDY*2);
	
	GPIO_ResetBits(LED_GP, LED_GR);
	GPIO_ResetBits(LED_GP, LED_BL);
	GPIO_ResetBits(LED_GP, LED_YL);
	GPIO_ResetBits(LED_GP, LED_RD);
	osDelay(LEDDY);
	GPIO_SetBits(LED_GP, LED_GR);
	GPIO_SetBits(LED_GP, LED_BL);
	GPIO_SetBits(LED_GP, LED_YL);
	GPIO_SetBits(LED_GP, LED_RD);
	
	mid = osThreadGetId();
	osThreadTerminate(mid);
}
