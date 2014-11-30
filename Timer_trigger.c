#include "stm32f10x.h"
#include "cmsis_os.h"
#include "periph.h"
#include "LCD.h"

extern TYPE_DEVICE * dev;
extern osMutexId id_mtx_dev;
extern TYPE_DISP * dsp;
extern osMutexId id_mtx_dsp;
extern TYPE_CTRL * ctrl_param;
extern osMutexId id_mtx_ctrl;
extern osTimerId id_tmr_trg;
extern osTimerId id_tmr_wup;

extern void setTRGStatus(uint8_t idx,...);
extern void dspUpdate(uint8_t upd);

uint16_t freq=0, times=0, cnt=0, trigger_cnt=0;
uint32_t period=0; 
uint8_t mode=0, trg_stat=0, i=0;

uint8_t len;
uint8_t ptr[4];

void pulse_delay()
{
	uint16_t t=300;
	while (t--) __nop();
}

void TRIGGER()
{
	GPIO_ResetBits(LED_GP, LED_RD);
	GPIO_SetBits(SIG_GP, SIG_TRG);
	setTRGStatus(3);
	pulse_delay();
	GPIO_ResetBits(SIG_GP, SIG_TRG);
	osTimerStart(id_tmr_wup, 500);
}

void timer_trigger  (void const *arg)
{
	osMutexWait(id_mtx_ctrl, osWaitForever);
	mode = ctrl_param->mode;
	trg_stat = ctrl_param->trg;
	osMutexRelease(id_mtx_ctrl);
	
	if (mode == 0)
	{
		if (trg_stat == 1)	//the first trigger
		{
			setTRGStatus(3);
			osMutexWait(id_mtx_ctrl, osWaitForever);
			freq = ctrl_param->param[3];
			times = ctrl_param->param[4];
			osMutexRelease(id_mtx_ctrl);
			
			if (freq == 0 || times == 1) //trigger only once
			{
				TRIGGER();
				osDelay(500);
				setTRGStatus(1,0);
			}
			else 
			{
				period = (uint32_t)(100000.0/freq);
				cnt = times;
					
				TRIGGER();
				cnt--;
				
				osMutexWait(id_mtx_ctrl, osWaitForever);
				ctrl_param->times = cnt;
				osMutexRelease(id_mtx_ctrl);
				dspUpdate(LU_CTRG);

				osTimerStart(id_tmr_trg, period);
			}
		}
		else if (trg_stat == 3)	//the later trigger
		{
			if (cnt != 0) //more tigger needed
			{
				TRIGGER();
				cnt--;
				
				osMutexWait(id_mtx_ctrl, osWaitForever);
				ctrl_param->times = cnt;
				osMutexRelease(id_mtx_ctrl);
				dspUpdate(LU_CTRG);
				
				if (cnt != 0 || times == 0)
					osTimerStart(id_tmr_trg, period);
				else
				{
					period = 0;
					setTRGStatus(1,0);
				}
			}
		}
		else if (trg_stat == 2)	//pause
		{
			osTimerStart(id_tmr_trg, period);
		}
	}
}

void timer_wakeup (void const *arg)
{
	GPIO_SetBits(LED_GP, LED_RD);
	
	osMutexWait(id_mtx_dev, osWaitForever);
	for (i=0; i<10; i++)
		dev->updated[i] = 1;
	osMutexRelease(id_mtx_dev);
}
