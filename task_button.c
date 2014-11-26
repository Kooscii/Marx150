#include "cmsis_os.h"
#include "stm32f10x.h"
#include <string.h>
#include "LCD.h"
#include "periph.h"

/* -------------------------------------------------------------------------------------------------------------- *
			Uncomment THIS DEFINE to use the <left&right key> for changing device 
			and the <encoder> for changing parameter index, in this mode the 
			parameters are not allowed to be changed;
			
			Or to use the <left&right key> for selection and the <encoder> for 
			changing the selected item, in this mode the parameters of the devices
			can be changed.
 * -------------------------------------------------------------------------------------------------------------- */
#define NO_FLIP

//extern void task_lcd_ctrl(void const * arg);
//osThreadDef(task_lcd_ctrl, osPriorityNormal  , 1, 0);

void task_button (void const * arg)
{
	extern TYPE_DISP * dsp;
	extern osMutexId id_mtx_dsp;

	extern TYPE_DEVICE * dev;
	extern TYPE_CTRL * ctrl_status;
	
	extern osThreadId id_tsk_lcd_ctrl;
	extern osThreadId id_tsk_lcd_status;
	
	uint8_t enc=0, t=0;
	
#ifdef NO_FLIP
#else
	TIM2->ARR = 9;
	const uint8_t enc_range[] = {9, 2};
#endif
	
	for(;;)
	{
#ifdef NO_FLIP
		dsp->sel_choice = 0xff;		//disable twinkle on lcd
#endif
		
		
		if (GPIO_ReadInputDataBit(BTN_GP, BTN_LEFT)==RESET)
		{
			osMutexWait(id_mtx_dsp, osWaitForever);
#ifdef NO_FLIP
			if (dsp->sel_dev--==0) dsp->sel_dev = 9;
			dsp->updated |= LU_DSTAT | LU_PVAL | LU_CHO;		//0x08 | 0x04 | 0x01 -- dev on|off , param value and device index should be updated
#else
			dsp->sel_choice--;
			dsp->sel_choice %= 2;
			TIM2->ARR = enc_range[dsp->sel_choice];
			dsp->updated |= 0x04;		//0x04 -- device index should be updated
#endif
			osMutexRelease(id_mtx_dsp);
			osDelay(200);
		}
		else if (GPIO_ReadInputDataBit(BTN_GP, BTN_RIGHT)==RESET)
		{
			osMutexWait(id_mtx_dsp, osWaitForever);
#ifdef NO_FLIP
			if (dsp->sel_dev++==9) dsp->sel_dev = 0;
			dsp->updated |= LU_DSTAT | LU_PVAL | LU_CHO;		//0x08 | 0x04 | 0x01 -- dev on|off , param value and device index should be updated
#else
			dsp->sel_choice++;
			dsp->sel_choice %= 2;
			TIM2->ARR = enc_range[dsp->sel_choice];
			dsp->updated |= 0x04;		//0x04 -- device index should be updated
#endif
			osMutexRelease(id_mtx_dsp);
			osDelay(200);
		}
		else if (GPIO_ReadInputDataBit(BTN_GP, BTN_SET)==RESET)
		{
//			if (t) 
//			{
//				osSignalClear(id_tsk_lcd_status, 0x02);
//				osSignalSet(id_tsk_lcd_status, 0x01);		//stop lcd_status
//				osSignalClear(id_tsk_lcd_ctrl, 0x01);
//				osSignalSet(id_tsk_lcd_ctrl, 0x02);			//lcd_ctrl
//				t=0;
//			}
//			else 
//			{
//				osSignalClear(id_tsk_lcd_ctrl, 0x02);
//				osSignalSet(id_tsk_lcd_ctrl, 0x01);		//stop lcd_ctrl
//				osSignalClear(id_tsk_lcd_status, 0x01);
//				osSignalSet(id_tsk_lcd_status, 0x02);	//start_lcd_status
//				t=1;
//			}
			
			osDelay(200);
		}
		else if (GPIO_ReadInputDataBit(BTN_GP, BTN_BACK)==RESET)
		{
			osMutexWait(id_mtx_dsp, osWaitForever);
			dsp->updated = 0xff;
			osMutexRelease(id_mtx_dsp);
			osDelay(200);
		}
		
		if (enc != TIM2->CNT)
		{
			enc = TIM2->CNT;
#ifdef NO_FLIP
			dsp->sel_param = enc;
			dsp->updated |= LU_CHO | LU_PNAME | LU_PVAL;		//all should be updated
#else
			if (dsp->sel_choice==0) 
			{
				dsp->sel_dev = enc;
				dsp->updated |= 0x01;		//param value should be changed, because of sel_choice, the device index are always updated when twinkle
			}
			else
			{
				dsp->sel_param = enc;
				dsp->updated |= 0x03;		//param name and value should be changed
			}
#endif
		}			
		
		osDelay(100);
	}
}
