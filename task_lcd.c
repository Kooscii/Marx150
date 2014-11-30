#include "cmsis_os.h"
#include "stm32f10x.h"
#include <string.h>
#include "LCD.h"
#include "menu.h"
#include "periph.h"

uint8_t convert_status(TYPE_DEVICE * dev, uint8_t dev_index, uint8_t dev_status)
{
	extern osMutexId id_mtx_dev;
	uint8_t stat;
	
	osMutexWait(id_mtx_dev, osWaitForever);
	stat = dev->status[dev_index];
	osMutexRelease(id_mtx_dev);
	
	if (stat&dev_status) return 1;
	else return 0;
}


void convert_param(char * str, void * dev, uint8_t dev_index, uint8_t dev_param)
{
	extern osMutexId id_mtx_dev;
	extern osMutexId id_mtx_ctrl;
	uint16_t val=0;
	uint8_t i;
	
	if (dev_index<10)
	{
		osMutexWait(id_mtx_dev, osWaitForever);
		val = ((TYPE_DEVICE*)dev)->param[dev_index][dev_param];
		osMutexRelease(id_mtx_dev);
	}
	else
	{
		osMutexWait(id_mtx_ctrl, osWaitForever);
		val = ((TYPE_CTRL*)dev)->param[dev_param];
		osMutexRelease(id_mtx_ctrl);
	}
	//val=123;
	
	/* Presicion = 1 */
	if (dev_param==4)
	{
		for (i=6; i>0; i--)
		{
			str[i-1]= val%10 + 48;
			val /= 10;
			if (val==0) break;
		}
		for (;i>1; i--)
		{
			str[i-2]= ' ';
		}
	}
	/* Presicion = 0.1 */
	else if (dev_param==0 || dev_param==1 || dev_param==2 || dev_param==6 || dev_param==7 || dev_param==8)
	{
		str[5]= val%10 + 48;
		val /= 10;
		str[4]= '.';
		for (i=4; i>0; i--)
		{
			str[i-1]= val%10 + 48;
			val /= 10;
			if (val==0) break;
		}
		for (;i>1; i--)
		{
			str[i-2]= ' ';
		}
	}
	/* Presicion = 0.01 */
	else if (dev_param==3)
	{
		str[5]= val%10 + 48;
		val /= 10;
		str[4]= val%10 + 48;
		val /= 10;
		str[3]= '.';
		for (i=3; i>0; i--)
		{
			str[i-1]= val%10 + 48;
			val /= 10;
			if (val==0) break;
		}
		for (;i>1; i--)
		{
			str[i-2]= ' ';
		}
	}	
	/* Diaplay as Char */
	else if (dev_param==5)
	{
		for (i=6; i>0; i--)
			str[i-1]= ' ';
		if (val==0) str[4] = '+';
		else str[4] = '-';
	}			
}


void task_lcd_ctrl (void const * arg)
{
	extern TYPE_DISP * dsp;
	extern TYPE_DEVICE * dev;
	extern TYPE_CTRL * ctrl_param;
	extern osMutexId id_mtx_dsp;
	extern osMutexId id_mtx_ctrl;
	extern osMutexId id_mtx_dev;
	
	char str_dsp[6];
	uint8_t flip = 0;
	uint8_t len;
	uint8_t * ptr;
	uint8_t sel[3];
	int8_t i=0, j=0;
	uint8_t	is_upd=1;
	uint8_t trg_status=0, ctrl_mode=0;
	uint8_t stat;
	uint16_t val;
	
  for(;;)
	{
		osMutexWait(id_mtx_dsp, osWaitForever);
		stat = dsp->mode;
		is_upd = dsp->updated;
		dsp->updated = 0;
		osMutexRelease(id_mtx_dsp);
		
		osMutexWait(id_mtx_ctrl, osWaitForever);
		trg_status=ctrl_param->trg;
		ctrl_mode=ctrl_param->mode;
		val = ctrl_param->times;
		osMutexRelease(id_mtx_ctrl);
		
		/* Global Display */
		if (is_upd==0xff)
			{
				for (i=0; i<2; i++)
				{
					lcd_select(LCD_BOTH);
					if (!i) lcd_spi(LCD_REG, 0x80);
					else lcd_spi(LCD_REG, 0x90);
					delay(LCD_DELAY_REG);
					len = strlen(str_BLANK);
					ptr = (uint8_t*) str_BLANK;
					while(len--) {
							lcd_spi(LCD_DAT, *ptr++);
						delay(LCD_DELAY_DAT);
					}
					delay(LCD_DELAY_DAT);
				}
			}
			if (is_upd&LU_CHO) flip=4;
			/*--------------------------------------------
					str_TRG_STATUS
			*--------------------------------------------*/
			if (is_upd&LU_CTRG)
			{
				lcd_select(LCD_1);
				lcd_spi(LCD_REG, 0x80);
				delay(LCD_DELAY_REG);
				len = strlen(str_TRG_STATUS[trg_status]);
				ptr = (uint8_t*) str_TRG_STATUS[trg_status];
				while(len--) {
					if (flip>4 && sel[0]==0xff)
						lcd_spi(LCD_DAT, ' ');
					else
						lcd_spi(LCD_DAT, *ptr++);
					delay(LCD_DELAY_DAT);
				}
				delay(LCD_DELAY_DAT);
			}
			/*--------------------------------------------
					str_CTRL_MODE
			*--------------------------------------------*/
			if (is_upd&LU_CMODE)
			{
				lcd_select(LCD_1);
				lcd_spi(LCD_REG, 0x8A);
				delay(LCD_DELAY_REG);
				len = strlen(str_CTRL_MODE[ctrl_mode]);
				ptr = (uint8_t*) str_CTRL_MODE[ctrl_mode];
				while(len--) {
						lcd_spi(LCD_DAT, *ptr++);
					delay(LCD_DELAY_DAT);
				}
				delay(LCD_DELAY_DAT);
			}
			/*--------------------------------------------
					str_CTRL_TIMES
			*--------------------------------------------*/
			if (is_upd&LU_CTRG)
			{
				lcd_select(LCD_1);
				lcd_spi(LCD_REG, 0x84);
				delay(LCD_DELAY_REG);
				for (i=3; i>=0; i--)
				{
					if (val==0) break;
					str_dsp[i]= val%10 + 48;
					val /= 10;
				}
				for (;i>=0; i--)
				{
					str_dsp[i]= ' ';
				}
				i=4;
				ptr = (uint8_t*) str_dsp; 
				while(i--) {
					lcd_spi(LCD_DAT, *ptr++);
					delay(LCD_DELAY_DAT);
				}
				delay(LCD_DELAY_DAT);
			}
		/* ----------------------------------------------------------------------------------------- *
					Standard Display
		 * ----------------------------------------------------------------------------------------- */
		if (stat==3)
		{
			osMutexWait(id_mtx_dsp, osWaitForever);
			sel[0]=dsp->sel_choice;
			sel[1]=dsp->sel_dev;
			sel[2]=dsp->sel_param;
			osMutexRelease(id_mtx_dsp);
			
			/*--------------------------------------------
					str_DEV_STATUS
			*--------------------------------------------*/
			if (is_upd&LU_DSTAT)
			{
				lcd_select(LCD_2);
				lcd_spi(LCD_REG, 0x80);
				delay(LCD_DELAY_REG);
				lcd_spi(LCD_DAT, 0xA9);
				delay(LCD_DELAY_DAT);
				lcd_spi(LCD_DAT, 0xC0);
				delay(LCD_DELAY_DAT);
				len = strlen(str_DEV_STATUS[convert_status(dev, sel[1], FS_24V)]);
				ptr = (uint8_t*) str_DEV_STATUS[convert_status(dev, sel[1], FS_24V)];
				while(len--) {
					lcd_spi(LCD_DAT, *ptr++);
					delay(LCD_DELAY_DAT);
				}
				delay(LCD_DELAY_DAT);
				
				lcd_select(LCD_2);
				lcd_spi(LCD_REG, 0x90);
				delay(LCD_DELAY_REG);
				lcd_spi(LCD_DAT, 0xA9);
				delay(LCD_DELAY_DAT);
				lcd_spi(LCD_DAT, 0xB8);
				delay(LCD_DELAY_DAT);
				len = strlen(str_CHG_STATUS[convert_status(dev, sel[1], FS_CHG)]);
				ptr = (uint8_t*) str_CHG_STATUS[convert_status(dev, sel[1], FS_CHG)];
				while(len--) {
					lcd_spi(LCD_DAT, *ptr++);
					delay(LCD_DELAY_DAT);
				}
				delay(LCD_DELAY_DAT);
			}
			/*--------------------------------------------
					str_DEVICE
			*--------------------------------------------*/
			if (is_upd&LU_CHO || sel[0]==0)		//if selection changed
			{
				lcd_select(LCD_1);
				lcd_spi(LCD_REG, 0x90);
				delay(LCD_DELAY_REG);
				len = strlen(str_DEVICE[sel[1]]);
				ptr = (uint8_t*) str_DEVICE [sel[1]];
				while(len--) {
					if (flip>4 && (!is_upd&0x04 || sel[0]==0))		//only when is selected and not in updating
						lcd_spi(LCD_DAT, ' ');
					else
						lcd_spi(LCD_DAT, *ptr++);
					delay(LCD_DELAY_DAT);
				}
				delay(LCD_DELAY_DAT);
			}
			
			
			
			/*--------------------------------------------
					str_PARAM_NAME
			*--------------------------------------------*/
			if (is_upd&LU_PNAME)		//if sel_param changed
			{
				for (j=0; j<3; j++)
				{
					if (j==0) lcd_select(LCD_1);
					else lcd_select(LCD_2);
					if (j==1) lcd_spi(LCD_REG, 0x84);
					else lcd_spi(LCD_REG, 0x94);
					delay(LCD_DELAY_REG);
					len = strlen(str_PARAM_NAME[sel[2]*3+j]);
					ptr = (uint8_t*) str_PARAM_NAME [sel[2]*3+j];
					while(len--) 
					{
						lcd_spi(LCD_DAT, *ptr++);
						delay(LCD_DELAY_DAT);
					}
					delay(LCD_DELAY_DAT);
				}
			}
			/*--------------------------------------------
					parameter's value
			*--------------------------------------------*/
			if (is_upd&LU_PVAL)		//if sel_param changed or sel_dev changed
			{
				for (j=0; j<3; j++)
				{
					if (j==0) lcd_select(LCD_1);
					else lcd_select(LCD_2);
					if (j==1) lcd_spi(LCD_REG, 0x88);
					else lcd_spi(LCD_REG, 0x98);
					delay(LCD_DELAY_REG);
					convert_param(str_dsp, dev, sel[1], sel[2]*3+j);
					i=6;
					ptr = (uint8_t*) str_dsp; 
					while(i--) {
						lcd_spi(LCD_DAT, *ptr++);
						delay(LCD_DELAY_DAT);
					}
				}
			}
			
			//if (++flip>5) flip=0;
		}
		/* ----------------------------------------------------------------------------------------- *
					Summary Display
		 * ----------------------------------------------------------------------------------------- */
		else if (stat == 4)
		{		
			if (is_upd&LU_PNAME)
			{
				lcd_select(LCD_1);
				lcd_spi(LCD_REG, 0x90);
				delay(LCD_DELAY_REG);
				len = strlen(str_SEP);
				ptr = (uint8_t*) str_SEP;
				while(len--)
				{
					lcd_spi(LCD_DAT, *ptr++);
					delay(LCD_DELAY_DAT);
				}
				delay(LCD_DELAY_DAT);
				lcd_select(LCD_2);
				lcd_spi(LCD_REG, 0x81);
				delay(LCD_DELAY_REG);
				lcd_spi(LCD_DAT, 0xA9);
				delay(LCD_DELAY_DAT);
				lcd_spi(LCD_DAT, 0xC0);
				delay(LCD_DELAY_DAT);
				lcd_select(LCD_2);
				lcd_spi(LCD_REG, 0x91);
				delay(LCD_DELAY_REG);
				lcd_spi(LCD_DAT, 0xA9);
				delay(LCD_DELAY_DAT);
				lcd_spi(LCD_DAT, 0xB8);
				delay(LCD_DELAY_DAT);
			}
			if (is_upd&LU_DSTAT)
			{
				for (i=0; i<10; i++)
				{
					osMutexWait(id_mtx_dev, osWaitForever);
					stat = dev->status[i];
					osMutexRelease(id_mtx_dev);
					lcd_select(LCD_2);
					lcd_spi(LCD_REG, 0x82+i);
					delay(LCD_DELAY_REG);
					len = strlen(str_ON[stat&FS_24V?1:0]);
					ptr = (uint8_t*) str_ON[stat&FS_24V?1:0];
					while(len--) 
					{
						lcd_spi(LCD_DAT, *ptr++);
						delay(LCD_DELAY_DAT);
					}
					delay(LCD_DELAY_DAT);
				}
				for (i=0; i<10; i++)
				{
					osMutexWait(id_mtx_dev, osWaitForever);
					stat = dev->status[i];
					osMutexRelease(id_mtx_dev);

					lcd_select(LCD_2);
					lcd_spi(LCD_REG, 0x92+i);
					delay(LCD_DELAY_REG);
					len = strlen(str_CHG[stat&FS_CHG?1:0]);
					ptr = (uint8_t*) str_CHG[stat&FS_CHG?1:0];
					while(len--) 
					{
						lcd_spi(LCD_DAT, *ptr++);
						delay(LCD_DELAY_DAT);
					}
					delay(LCD_DELAY_DAT);
				}
			}
			if (is_upd&LU_CHO)
			{
				lcd_select(LCD_2);
				lcd_spi(LCD_REG, 0x90);
				delay(LCD_DELAY_REG);
				len = dsp->sel_dev;
				lcd_spi(LCD_DAT, len/10==0?' ':len/10+'0');
				delay(LCD_DELAY_DAT);
				lcd_spi(LCD_DAT, len%10+'0');
				delay(LCD_DELAY_DAT);
			}
		}
		else if (stat==0 || stat==1 || stat==2)
		{
			osMutexWait(id_mtx_dsp, osWaitForever);
			sel[0]=dsp->sel_choice;
			sel[1]=dsp->sel_dev;
			sel[2]=dsp->sel_param;
			osMutexRelease(id_mtx_dsp);
			
			/*--------------------------------------------
					str_PARAM_NAME
			*--------------------------------------------*/
			if (is_upd&LU_PNAME)		//if sel_param changed
			{
				for (j=0; j<3; j++)
				{
					if (j==0) lcd_select(LCD_1);
					else lcd_select(LCD_2);
					if (j==1) lcd_spi(LCD_REG, 0x82);
					else lcd_spi(LCD_REG, 0x92);
					delay(LCD_DELAY_REG);
					len = strlen(str_PARAM_NAME[stat*3+j]);
					ptr = (uint8_t*) str_PARAM_NAME [stat*3+j];
					while(len--) 
					{
						lcd_spi(LCD_DAT, *ptr++);
						delay(LCD_DELAY_DAT);
					}
					delay(LCD_DELAY_DAT);
				}
			}
			/*--------------------------------------------
					parameter's value
			*--------------------------------------------*/
			if (is_upd&LU_PVAL)		//if sel_param changed or sel_dev changed
			{
				for (j=0; j<3; j++)
				{
					if (j==0) lcd_select(LCD_1);
					else lcd_select(LCD_2);
					if (j==1) lcd_spi(LCD_REG, 0x86);
					else lcd_spi(LCD_REG, 0x96);
					delay(LCD_DELAY_REG);
					convert_param(str_dsp, ctrl_param, 100, stat*3+j);
					i=6;
					ptr = (uint8_t*) str_dsp; 
					while(i--) {
						lcd_spi(LCD_DAT, *ptr++);
						delay(LCD_DELAY_DAT);
					}
				}
			}
			if (is_upd&LU_CHO && ctrl_param->admin == 1)
			{
				osMutexWait(id_mtx_dsp, osWaitForever);
				sel[0]=dsp->sel_choice;
				sel[1]=dsp->sel_dev;
				sel[2]=dsp->sel_param;
				osMutexRelease(id_mtx_dsp);


				for (len =0; len<3; len++)
				{
					if (len == 0) lcd_select(LCD_1);
					else lcd_select(LCD_2);
					if (len == 1) lcd_spi(LCD_REG, 0x81);
					else lcd_spi(LCD_REG, 0x91);
					delay(LCD_DELAY_REG);
					lcd_spi(LCD_DAT, len==sel[2]?0x10:' ');
					delay(LCD_DELAY_DAT);
				}
			}
			
		}
		/* ----------------------------------------------------------------------------------------- *
		 *		Testing Display
		 * ----------------------------------------------------------------------------------------- */
		else if (stat == 5)
		{
			if (is_upd&LU_DSTAT)
			{
				lcd_select(LCD_1);
				lcd_spi(LCD_REG, 0x90);
				delay(LCD_DELAY_REG);
				len = strlen(str_DZ);
				ptr = (uint8_t*) str_DZ;
				while(len--) {
						lcd_spi(LCD_DAT, *ptr++);
					delay(LCD_DELAY_DAT);
				}
				delay(LCD_DELAY_DAT);
				osMutexWait(id_mtx_dev, osWaitForever);
				val = dev->dz;
				osMutexRelease(id_mtx_dev);
				lcd_spi(LCD_REG, 0x92);
				delay(LCD_DELAY_REG);
				for (i=4; i>0; i--)
				{
					str_dsp[i-1]= val%10 + 48;
					val /= 10;
					if (val==0) break;
				}
				for (;i>1; i--)
				{
					str_dsp[i-2]= ' ';
				}
				len = 4;
				ptr = (uint8_t*) str_dsp;
				while(len--) {
						lcd_spi(LCD_DAT, *ptr++);
					delay(LCD_DELAY_DAT);
				}
				delay(LCD_DELAY_DAT);
			}
			if (is_upd&LU_DSTAT)
			{
				lcd_select(LCD_1);
				lcd_spi(LCD_REG, 0x96);
				delay(LCD_DELAY_REG);
				len = strlen(str_CF);
				ptr = (uint8_t*) str_CF;
				while(len--) {
						lcd_spi(LCD_DAT, *ptr++);
					delay(LCD_DELAY_DAT);
				}
				delay(LCD_DELAY_DAT);
				osMutexWait(id_mtx_dev, osWaitForever);
				val = dev->cf;
				osMutexRelease(id_mtx_dev);
				lcd_spi(LCD_REG, 0x98);
				delay(LCD_DELAY_REG);
				for (i=4; i>0; i--)
				{
					str_dsp[i-1]= val%10 + 48;
					val /= 10;
					if (val==0) break;
				}
				for (;i>1; i--)
				{
					str_dsp[i-2]= ' ';
				}
				len = 4;
				ptr = (uint8_t*) str_dsp;
				while(len--) {
						lcd_spi(LCD_DAT, *ptr++);
					delay(LCD_DELAY_DAT);
				}
				delay(LCD_DELAY_DAT);
			}
			if (is_upd&LU_DSTAT)
			{
				lcd_select(LCD_2);
				lcd_spi(LCD_REG, 0x80);
				delay(LCD_DELAY_REG);
				len = strlen(str_PR);
				ptr = (uint8_t*) str_PR;
				while(len--) {
						lcd_spi(LCD_DAT, *ptr++);
					delay(LCD_DELAY_DAT);
				}
				delay(LCD_DELAY_DAT);
				osMutexWait(id_mtx_dev, osWaitForever);
				val = dev->pwm_ratio;
				osMutexRelease(id_mtx_dev);
				lcd_spi(LCD_REG, 0x82);
				delay(LCD_DELAY_REG);
				for (i=3; i>=0; i--)
				{
					str_dsp[i]= val%10 + 48;
					val /= 10;
					if (val==0) break;
				}
				for (i--;i>=0; i--)
				{
					str_dsp[i]= ' ';
				}
				len = 4;
				ptr = (uint8_t*) str_dsp;
				while(len--) {
						lcd_spi(LCD_DAT, *ptr++);
					delay(LCD_DELAY_DAT);
				}
				delay(LCD_DELAY_DAT);
			}
		}
		/* ----------------------------------------------------------------------------------------- *
					Auto-control Display
		 * ----------------------------------------------------------------------------------------- */
		else if (stat == 7)
		{
			
		}
		
		osDelay(100);
	}
}


/*--------------------------------------------
					str_PARAM_NO
			
			if (is_upd&LU_CHO || sel[0]==1)
			{
				lcd_select(LCD_2);
				lcd_spi(LCD_REG, 0x90);
				delay(LCD_DELAY_REG);
				lcd_spi(LCD_DAT, 0xA9);
				delay(LCD_DELAY_DAT);
				lcd_spi(LCD_DAT, 0xB8);
				delay(LCD_DELAY_DAT);
				len = strlen(str_PARAM_NO[sel[2]]);
				ptr = (uint8_t*) str_PARAM_NO [sel[2]];
				while(len--) {
					if (flip>4 && (!is_upd&0x04 || sel[0]==1))
						lcd_spi(LCD_DAT, ' ');
					else
						lcd_spi(LCD_DAT, *ptr++);
					delay(LCD_DELAY_DAT);
				}
				delay(LCD_DELAY_DAT);
			}
			*--------------------------------------------*/

