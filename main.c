#include "cmsis_os.h"
#include "stm32f10x.h"
#include "periph.h"
#include "LCD.h"
#include <stdio.h>
#include <string.h>

/*----------------------------------------------------------------------------
     Variable Defination
 *---------------------------------------------------------------------------*/
TYPE_DEVICE * dev;
TYPE_CTRL * ctrl_param;
TYPE_DISP * dsp;

/* Forward reference */

//void task_idle (void const * arg);
//osThreadId id_tsk_idle;

/*----------------------------------------------------------------------------
     Thread
 *---------------------------------------------------------------------------*/
 
/* < task_lcd.c > -- lcd display tasks ------------------------------*/
extern void task_lcd_ctrl (void const * arg);
osThreadDef(task_lcd_ctrl, osPriorityNormal  , 1, 0);
osThreadId id_tsk_lcd_ctrl;
/* ----------------------------------------------------------------- */

/* < task_button.c > --  button checking task --------------------- */
//extern void task_button (void const * arg);
//osThreadDef(task_button, osPriorityNormal , 1, 0);
//osThreadId id_tsk_button;
/* ----------------------------------------------------------------- */

/* < task_fpga.c > --  fpga control task --------------------- */
extern void task_fpga_sending (void const * arg);
osThreadDef(task_fpga_sending, osPriorityHigh , 1, 0);
osThreadId id_tsk_fpga_sending;
/* ----------------------------------------------------------------- */

/* < task_isr_usart.c > --  fpga control task --------------------- */
extern void task_isr_usart (void const * arg);
osThreadDef(task_isr_usart, osPriorityHigh , 1, 300);
osThreadId id_tsk_isr_usart;
/* ----------------------------------------------------------------- */

/* < Timer_trigger.c > --  trigger task --------------------- */
extern void timer_trigger  (void const *arg);
osTimerDef (tmr_trg, timer_trigger);
osTimerId id_tmr_trg; 

extern void timer_wakeup  (void const *arg);
osTimerDef (tmr_wup, timer_wakeup);
osTimerId id_tmr_wup; 
/* ----------------------------------------------------------------- */

/* < Timer_trigger.c > --  trigger task --------------------- */
extern void task_flowing_led (void const * arg);
osThreadDef(task_flowing_led, osPriorityBelowNormal , 1, 0);
osThreadId id_tsk_flowing_led;
/* ----------------------------------------------------------------- */

extern void setTRGStatus(uint8_t idx,...);

/*----------------------------------------------------------------------------
     Mutex
 *---------------------------------------------------------------------------*/
osMutexId id_mtx_dsp;		// (TYPE_DISP*) dsp		
osMutexId id_mtx_dev;  	// (TYPE_DEVICE*) dev	
osMutexId id_mtx_ctrl;  	// (TYPE_TRG*) trg		
osMutexDef(Mutex_dsp);
osMutexDef(Mutex_dev);
osMutexDef(Mutex_ctrl);


/*----------------------------------------------------------------------------
     MemoryPool
 *---------------------------------------------------------------------------*/
osPoolId id_mp_dev;
osPoolId id_mp_dsp;
osPoolId id_mp_trg;
osPoolDef (MemPool_dev, 1, TYPE_DEVICE);
osPoolDef (MemPool_trg, 1, TYPE_CTRL);
osPoolDef (MemPool_dsp, 1, TYPE_DISP);

void task_idle(void const * arg)
{
	for(;;);
}

void struct_init()
{
	uint8_t i=0, j=0;
	/* STRUCT TYPE_DEVICE dev */
	id_mp_dev = osPoolCreate(osPool(MemPool_dev));
	dev = (TYPE_DEVICE *)osPoolAlloc(id_mp_dev);
	for (i=0;i<10;i++)
	{
		for (j=0;j<10;j++)
			dev->param[j][i]=0;
		dev->status[i]=0;
		dev->updated[i]=0;
	}
	dev->dz = 500;
	dev->pwm_ratio = 64;
	dev->cf = 20;
	
	
	/* STRUCT TYPE_TRIG trg */
	id_mp_trg = osPoolCreate(osPool(MemPool_trg));
	ctrl_param = (TYPE_CTRL *)osPoolAlloc(id_mp_trg);
	ctrl_param->mode=0;
	ctrl_param->trg=0;
	ctrl_param->freq=0;
	ctrl_param->times=0;
	for (i=0;i<10;i++)
		ctrl_param->param[i]=0;
	
	/* STRUCT TYPE_DISP dsp */
	id_mp_dsp = osPoolCreate(osPool(MemPool_dsp));
	dsp = (TYPE_DISP *)osPoolAlloc(id_mp_dsp);
	dsp->sel_choice=0xff;
	dsp->sel_dev=0;
	dsp->sel_param=0;
	dsp->updated=0xff;
	dsp->mode=3;
}

/*----------------------------------------------------------------------------
     Main Thread
 *---------------------------------------------------------------------------*/
int main (void) {
	osThreadId mid;
	uint8_t stat=0, ispressed=0, sel=0, i=0;
	uint16_t enc=0, param_limit[9] = {1500, 200, 9999, 100, 10000, 1, 100, 1500, 1500};
	
	RCC_Config();
	GPIO_Config();
	USART_Config();
	SPI_Config();
	TIM_Config();
	NVIC_Config();
	struct_init();
	
	/* Create Mutex */
	id_mtx_dsp = osMutexCreate(osMutex(Mutex_dsp));
	id_mtx_dev = osMutexCreate(osMutex(Mutex_dev));
	id_mtx_ctrl = osMutexCreate(osMutex(Mutex_ctrl));
	
	//osDelay(100);
	
	osKernelInitialize();
	id_tsk_flowing_led = osThreadCreate(osThread(task_flowing_led), NULL);
	osKernelStart();
	lcd_init();
	
	
	osDelay(30);
	/* Create thread X */
	id_tsk_isr_usart = osThreadCreate(osThread(task_isr_usart), NULL);
	id_tsk_fpga_sending = osThreadCreate(osThread(task_fpga_sending), NULL);
	id_tsk_lcd_ctrl = osThreadCreate(osThread(task_lcd_ctrl), NULL);
	/* Create timer X */
	id_tmr_trg = osTimerCreate (osTimer(tmr_trg), osTimerOnce, NULL);
	id_tmr_wup = osTimerCreate (osTimer(tmr_wup), osTimerOnce, NULL);
	osTimerStop(id_tmr_trg);
	osTimerStop(id_tmr_wup);
	
	mid = osThreadGetId();
	osThreadSetPriority(mid, osPriorityBelowNormal);
	
//	osKernelStart();

	for(;;)
	{
		dsp->sel_choice = 0xff;		//disable twinkle on lcd
		
//		osMutexWait(id_mtx_ctrl, osWaitForever);
//		if (ctrl_param->trg == 1)
//			ctrl_param->times = dev->param[0][4];
//		osMutexRelease(id_mtx_ctrl);
		
		if (GPIO_ReadInputDataBit(BTN_GP, BTN_LEFT)==RESET || GPIO_ReadInputDataBit(BTN_GP, BTN_RIGHT)==RESET)
		{
//			osMutexWait(id_mtx_dsp, osWaitForever);
//			if (dsp->sel_param--==0) dsp->sel_param = 2;
//			dsp->updated |= LU_CHO | LU_PNAME | LU_PVAL;		//0x08 | 0x04 | 0x01 -- dev on|off , param value and device index should be updated
//			osMutexRelease(id_mtx_dsp);
//			osDelay(200);
			osMutexWait(id_mtx_dsp, osWaitForever);
			if (dsp->mode != 7)
			{
				if (GPIO_ReadInputDataBit(BTN_GP, BTN_LEFT)==RESET)
					dsp->mode = (dsp->mode+5)%6;
				else 
					dsp->mode = (dsp->mode+1)%6;
				dsp->updated = 0xff;
				stat = dsp->mode;
			}
			osMutexRelease(id_mtx_dsp);
			if (stat==0 || stat==1 || stat==2)
			{
				TIM2->CNT = 0;
				TIM2->ARR = param_limit[stat*3];
			}
			else
			{
				TIM2->CNT = 0;
				TIM2->ARR = 7;
			}
			ispressed=1;
		}
		else if (GPIO_ReadInputDataBit(BTN_GP, BTN_SET)==RESET)
		{
			osMutexWait(id_mtx_dsp, osWaitForever);
			stat = dsp->mode;
			dsp->updated |= LU_DSTAT | LU_CTRG;
			osMutexRelease(id_mtx_dsp);
			if (stat == 3 || stat == 4)
			{
				osMutexWait(id_mtx_ctrl, osWaitForever);
				stat = ctrl_param->trg;
				osMutexRelease(id_mtx_ctrl);
				if (stat == 0 || stat == 1)
				{
					osMutexWait(id_mtx_dev, osWaitForever);
					if (dev->status[enc]&FS_24V)
					{
						dev->status[enc] &= ~(FS_24V|FS_CHG|FS_TRG);
					}
					else
					{
						dev->status[enc] |= FS_24V|FS_CHG|FS_TRG;
					}
					for (stat=0; stat<DEV_CNT; stat++)
					{
						if (dev->status[stat]&FS_24V)
							break;
					}
					osMutexRelease(id_mtx_dev);
					osMutexWait(id_mtx_ctrl, osWaitForever);
					if (stat >= DEV_CNT)
					{
						ctrl_param->trg = 0;
						ctrl_param->times = 0;
						GPIO_ResetBits(SIG_GP, SIG_K_CRG);
						GPIO_ResetBits(SIG_GP, SIG_EN);
						GPIO_ResetBits(SIG_GP, SIG_K_TRG);
					}
					else
					{
						ctrl_param->trg = 1;
						ctrl_param->times = 0;
						GPIO_ResetBits(SIG_GP, SIG_K_CRG);
						GPIO_SetBits(SIG_GP, SIG_EN);
						GPIO_SetBits(SIG_GP, SIG_K_TRG);
					}
					osMutexRelease(id_mtx_ctrl);
				}
			}
			ispressed=1;
		}
		else if (GPIO_ReadInputDataBit(BTN_GP, BTN_BACK)==RESET)
		{
			osMutexWait(id_mtx_dsp, osWaitForever);
			dsp->updated = 0xff;
			osMutexRelease(id_mtx_dsp);
			ispressed=1;
		}
		else if (GPIO_ReadInputDataBit(BTN_GP, BTN_ENCODER)==RESET)
		{
			osMutexWait(id_mtx_dsp, osWaitForever);
			stat = dsp->mode;
			if (stat==0 || stat==1 || stat==2 || stat==3)
			{
				dsp->sel_param = (dsp->sel_param+1)%3;
				dsp->updated |= LU_CHO|LU_PNAME|LU_PVAL;
			}
			sel = dsp->sel_param;
			osMutexRelease(id_mtx_dsp);
			if (stat==0 || stat==1 || stat==2)
			{
				osMutexWait(id_mtx_ctrl, osWaitForever);
				TIM2->ARR = param_limit[stat*3+sel];
				TIM2->CNT = ctrl_param->param[stat*3+sel];
				osMutexRelease(id_mtx_ctrl);
			}
			
			ispressed=1;
		}
		
		if (enc != TIM2->CNT)
		{
			osMutexWait(id_mtx_dsp, osWaitForever);
			stat = dsp->mode;
			sel = dsp->sel_param;
			enc = TIM2->CNT;
			if (stat==3 || stat==4)
				dsp->sel_dev = (uint8_t)enc;
			dsp->updated |= LU_DSTAT | LU_PVAL | LU_CHO;
			osMutexRelease(id_mtx_dsp);
			osMutexWait(id_mtx_ctrl, osWaitForever);
			if (stat==0 || stat==1 || stat==2)
			{
				ctrl_param->param[stat*3+sel] = enc;
			}
			osMutexRelease(id_mtx_ctrl);
			osMutexWait(id_mtx_dev, osWaitForever);
			if (stat==0 || stat==1 || stat==2)
			{
				if (stat*3+sel==0 || stat*3+sel==6 || stat*3+sel==7 || stat*3+sel==8)
					for (i=0; i<DEV_CNT; i++)
						dev->param[i][stat*3+sel] = enc/DEV_CNT;
				else
					for (i=0; i<DEV_CNT; i++)
						dev->param[i][stat*3+sel] = enc;
				for (i=0; i<DEV_CNT; i++)
					dev->updated[i] = 1;
			}
			osMutexRelease(id_mtx_dev);
		}		
		if (ispressed)	
		{
			ispressed=0;
			osDelay(300);
		}
		else
			osDelay(100);
		
	}
}
