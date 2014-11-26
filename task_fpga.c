#include "cmsis_os.h"
#include "stm32f10x.h"
#include "periph.h"
#include "LCD.h"

extern TYPE_DEVICE * dev;
extern osMutexId id_mtx_dev;
extern TYPE_DISP * dsp;
extern osMutexId id_mtx_dsp;
extern TYPE_CTRL * ctrl_param;
extern osMutexId id_mtx_ctrl;
extern osTimerId id_tmr_trg;


void send_data(uint8_t addr, uint8_t cmd, uint16_t data)
{
	while(USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET);
	USART_SendData(USART2, addr);
	while(USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET);
	USART_SendData(USART2, cmd);
	while(USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET);
	USART_SendData(USART2, (data>>8)&0xff);
	while(USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET);
	USART_SendData(USART2, data&0xff);
	while(USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET);
	USART_SendData(USART2, 0x00);
	while(USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET);
	USART_SendData(USART2, 0x00);
}

void task_fpga_sending (void const * arg)
{
	uint8_t fpga_status=0, fpga_updated=0;
	uint16_t fpga_pwm=0, fpga_data=0, pr=0, dz=0, fpga_polar=0, cf=0;
	uint8_t i;
	
	for(;;)
	{
		
		for (i=0;i<DEV_CNT;i++)
		{
			osMutexWait(id_mtx_dev, osWaitForever);
			fpga_status = dev->status[i];
			fpga_updated = dev->updated[i];
			fpga_pwm = dev->param[i][0];
			dev->updated[i] = 0;
			pr=dev->pwm_ratio;
			osMutexRelease(id_mtx_dev);
			
			/* fpga status updating */
			fpga_data = 0;
			if (fpga_status&FS_24V) fpga_data |= 0x0500;
			if (fpga_status&FS_TRG) fpga_data |= 0x5000;
			send_data(0x80|i, 0xCA, fpga_data);
			
			/* fpga pwm updating */
			fpga_data = 0;
			if (fpga_status&FS_CHG) fpga_data = fpga_pwm*pr;
			send_data(0x80|i, 0xC1, fpga_data);
			
			if (fpga_updated)
			//if (1)
			{
				
//				/* fpga status updating */
//				fpga_data = 0;
//				if (fpga_status&FS_24V) fpga_data |= 0x0500;
//				if (fpga_status&FS_TRG) fpga_data |= 0x5000;
//				send_data(0x80|i, 0xCA, fpga_data);
//				
//				/* fpga pwm updating */
//				fpga_data = 0;
//				if (fpga_status&FS_CHG) fpga_data = fpga_pwm*pr;
//				send_data(0x80|i, 0xC1, fpga_data);
				
				osMutexWait(id_mtx_dev, osWaitForever);
				fpga_data = dev->param[i][1];
				fpga_pwm = dev->param[i][2];
				fpga_polar = dev->param[i][5];
				dz=dev->dz;
				cf=dev->cf;
				osMutexRelease(id_mtx_dev);
//				if (fpga_polar == 0)
//				{
//					send_data(0x80|i, 0xC2, fpga_data*5 + dz/20 + cf*50);
//					send_data(0x80|i, 0xC3, cf*50);
//					send_data(0x80|i, 0xC4, 0);
//					send_data(0x80|i, 0xC5, fpga_data*5);
//					
//					send_data(0x80|i, 0xC6, fpga_pwm*5);
//					send_data(0x80|i, 0xC7, fpga_pwm*5 + fpga_data*5+dz/20);
//					send_data(0x80|i, 0xC8, 0);
//					send_data(0x80|i, 0xC9, fpga_pwm*5);
//				}
//				else
//				{
//					send_data(0x80|i, 0xC2, 0);
//					send_data(0x80|i, 0xC3, fpga_data*5);
//					send_data(0x80|i, 0xC4, fpga_data*5 + dz/20 + cf*50);
//					send_data(0x80|i, 0xC5, cf*50);
//					
//					send_data(0x80|i, 0xC6, 0);
//					send_data(0x80|i, 0xC7, fpga_pwm*5);
//					send_data(0x80|i, 0xC8, fpga_pwm*5);
//					send_data(0x80|i, 0xC9, fpga_pwm*5 + fpga_data*5+dz/20);
//				}
					send_data(0x80|i, 0xC2, fpga_data*5);
					send_data(0x80|i, 0xC3, cf*50);
					send_data(0x80|i, 0xC4, 0);
					send_data(0x80|i, 0xC5, 0);
					
					send_data(0x80|i, 0xC6, fpga_pwm*5);
					send_data(0x80|i, 0xC7, fpga_pwm*5+fpga_data*5+dz/20);
					send_data(0x80|i, 0xC8, 0);
					send_data(0x80|i, 0xC9, 0);
				
			} 
			
			osDelay(10);
		}
	}
}
