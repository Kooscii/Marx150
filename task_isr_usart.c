#include "stm32f10x.h"
#include "cmsis_os.h"
#include "periph.h"
#include "stdarg.h"

#define RES_OK 	0
#define RES_ERR 1
#define RES_OPD 2

const uint8_t param_mapping[9] = {0,7,8,6,4,3,1,2,5};


/* respons string */									//	0			1			2			3			4			5			6			7			8			9			10		11		12		13		14		15		16		17		18		19		20		21		22		23		24		25		26		27		28		29		
const uint8_t  response_str[3][30]	 = {{0x3a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x17, 0xbb, 0xa9, 0xee, 0x0d, 0x0a},
										{0x3a, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xee, 0xa1, 0x2d, 0x0c, 0x0d, 0x0a},
										{0x3a, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe1, 0x4f, 0xbd, 0x9d, 0x0d, 0x0a}};																				
/*--------------------------------------------*/
																					
/* check status and mode table */
/* const [mode][stat] */			
						//	stat 0 1 2 3 4	mode						
const uint8_t CHK_AUTO[2][5] = {{1,1,0,0,0}, //0
								{1,1,0,0,0}};//1
						//	stat 0 1 2 3 4	mode						
const uint8_t CHK_MANU[2][5] = {{1,1,0,0,0}, //0
								{1,1,0,0,0}};//1
						//	stat 0 1 2 3 4	mode						
const uint8_t CHK_ON[2][5]   = {{1,1,0,0,0}, //0
								{1,1,0,0,0}};//1
						//	stat 0 1 2 3 4	mode						
const uint8_t CHK_OFF[2][5]  = {{1,1,0,0,0}, //0
								{1,1,0,0,0}};//1
						//	stat 0 1 2 3 4	mode						
const uint8_t CHK_PAU[2][5]  = {{0,1,1,1,0}, //0
								{0,1,1,1,0}};//1
						//	stat 0 1 2 3 4	mode						
const uint8_t CHK_CHG[2][5]  = {{1,1,0,0,1}, //0
								{1,1,0,0,1}};//1
						//	stat 0 1 2 3 4	mode						
const uint8_t CHK_TRG[2][5]  = {{0,1,1,1,0}, //0
								{0,1,1,1,0}};//1
/*------------------------------------------------*/																					
																					
osMessageQDef(MsgBox_usart, 90, uint8_t); 
osMessageQId id_Msg_usart;
extern TYPE_DEVICE * dev;
extern TYPE_DISP * dsp;
extern TYPE_CTRL * ctrl_param;
extern osMutexId id_mtx_dsp;
extern osMutexId id_mtx_dev;
extern osMutexId id_mtx_ctrl;
extern osThreadId id_tsk_fpga_trigger;
extern osTimerId id_tmr_trg;
																				

																					
/* functions */
uint32_t cal_crc(uint8_t * ptr, uint8_t len)
{
    uint32_t xbit, i = 0;
    uint32_t data;
    uint32_t crc = 0xffffffff;
    uint32_t dwpoly = 0x04c11db7;
    uint8_t bits;
    while (len-- > 0)
    {
        xbit = 0x80000000;
        data = ptr[i++];
        for (bits = 0; bits < 32; bits++)
        {
            if ((crc & 0x80000000) == 0x80000000)
            {
                crc <<= 1;
                crc ^= dwpoly;
            }
            else
                crc <<= 1;
            if ((data & xbit) == xbit)
                crc ^= dwpoly;
            xbit >>= 1;
        }
    }
    return crc;
}

void sendback(uint8_t idx_response)
{
	uint8_t i=0;
	for (i = 0; i<30; i++)
	{
		while(USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);
		USART_SendData(USART1, response_str[idx_response][i]);
	}
}

void dspUpdate(uint8_t upd)
{
	osMutexWait(id_mtx_dsp, osWaitForever);
	dsp->updated |= upd;
	osMutexRelease(id_mtx_dsp);
}
void setDEVStatus(uint8_t idx, uint8_t stat)
{
	osMutexWait(id_mtx_dev, osWaitForever);
	dev->status[idx] |= stat;
	osMutexRelease(id_mtx_dev);
	
	dspUpdate(LU_DSTAT);
}
void resetDEVStatus(uint8_t idx, uint8_t stat)
{
	osMutexWait(id_mtx_dev, osWaitForever);
	dev->status[idx] &= ~stat;
	osMutexRelease(id_mtx_dev);
	
	dspUpdate(LU_DSTAT);
}
void setTRGMode(uint8_t md)
{
	osMutexWait(id_mtx_ctrl, osWaitForever);
	ctrl_param->mode = md;
	osMutexRelease(id_mtx_ctrl);
	
	dspUpdate(LU_CMODE);
}

void setTRGStatus(uint8_t idx, ...)
{
	va_list ap;
	uint8_t i=0;	
	int u16t=0;
	
	osMutexWait(id_mtx_ctrl, osWaitForever);
	switch (idx)
	{
		case 0:
			ctrl_param->trg = idx;
			ctrl_param->times = 0;
			GPIO_ResetBits(SIG_GP, SIG_K_CRG);
			GPIO_ResetBits(SIG_GP, SIG_EN);
			GPIO_ResetBits(SIG_GP, SIG_K_TRG);
			for (i=0; i<DEV_CNT; i++)
				resetDEVStatus( i, FS_24V | FS_CHG | FS_TRG );
			GPIO_SetBits(LED_GP, LED_YL);
			break;
		case 1:
			ctrl_param->trg = idx;
			ctrl_param->times = 0;
			for (i=0; i<DEV_CNT; i++)
				setDEVStatus( i, FS_24V | FS_CHG | FS_TRG );
			GPIO_ResetBits(SIG_GP, SIG_K_CRG);
			GPIO_SetBits(SIG_GP, SIG_EN);
			GPIO_SetBits(SIG_GP, SIG_K_TRG);
			GPIO_SetBits(LED_GP, LED_YL);
			break;
		case 2:
			ctrl_param->trg = idx;
			GPIO_ResetBits(SIG_GP, SIG_K_CRG);
			GPIO_ResetBits(SIG_GP, SIG_EN);
			GPIO_ResetBits(SIG_GP, SIG_K_TRG);
			break;
		case 3:
			ctrl_param->trg = idx;
			GPIO_ResetBits(SIG_GP, SIG_K_CRG);
			GPIO_SetBits(SIG_GP, SIG_EN);
			GPIO_SetBits(SIG_GP, SIG_K_TRG);
			GPIO_ResetBits(LED_GP, LED_YL);
			break;
		case 4:
			ctrl_param->trg = idx;
			GPIO_ResetBits(SIG_GP, SIG_EN);
			GPIO_ResetBits(SIG_GP, SIG_K_TRG);
			GPIO_SetBits(SIG_GP, SIG_K_CRG);
			GPIO_SetBits(LED_GP, LED_YL);
			break;
		case 5:
			ctrl_param->trg = 1;
			ctrl_param->times = 0;
			va_start(ap, idx);
			u16t = va_arg(ap, int);
			va_end(ap);
			for (i=0; i<DEV_CNT; i++)
				if ((u16t>>i)&0x01)
					setDEVStatus( i, FS_24V | FS_CHG | FS_TRG );
				else 
					resetDEVStatus( i, FS_24V | FS_CHG | FS_TRG );
			GPIO_ResetBits(SIG_GP, SIG_K_CRG);
			GPIO_SetBits(SIG_GP, SIG_EN);
			GPIO_SetBits(SIG_GP, SIG_K_TRG);
			GPIO_SetBits(LED_GP, LED_YL);
			break;
		default:
			break;
	}
	osMutexRelease(id_mtx_ctrl);
	
	dspUpdate(LU_CTRG);
}


/* -----------------------------------------------*/


void USART1_IRQHandler(void) 
{
	uint8_t recv;
	extern osThreadId id_tsk_isr_usart; 

	if(USART_GetFlagStatus(USART1,USART_IT_RXNE)==SET) 
	{
		USART_ClearITPendingBit(USART1, USART_IT_RXNE);
		recv = USART_ReceiveData(USART1);
		osMessagePut(id_Msg_usart, recv, 0);
	}
}



void task_isr_usart (void const * arg)
{
	uint8_t buf=0, i=0, recv_pgc=0, recv_index=1; 
	uint8_t commd[30];
	osEvent evt;
	uint32_t recv_crc=0, calc_crc=0;
	uint8_t stat=0, mode=0;	
		
	id_Msg_usart = osMessageCreate(osMessageQ(MsgBox_usart), NULL);
	
	for(;;)
	{
		/* wait for the start code */
		if (recv_pgc == 0)
		{
			evt = osMessageGet(id_Msg_usart, osWaitForever);
			if (evt.status == osEventMessage) 
			{
				buf = (uint8_t)evt.value.v;
				if (buf == 0x3A) 
				{
					recv_pgc = 1;
					recv_index = 1;
					commd[0]=buf;
				}
			}
		}
		/* receive the body of info */
		else 
		{
			evt = osMessageGet(id_Msg_usart, 2);
			if (evt.status == osEventMessage) 
			{
				buf = (uint8_t)evt.value.v;
				if (recv_pgc == 1)
				{
					commd[recv_index++]=buf;
					/* check if the info received is correct */
					if (recv_index>=30)
					{
						recv_pgc = 0;
						if (commd[28] != 0x0D && commd[29] != 0x0A)
						{
							continue;
						}
						/* handle the info received*/
						else		//the info received is correct
						{
							recv_crc = 0;
							for (buf = 24; buf<28; buf++)
								recv_crc = recv_crc<<8 | commd[buf];
							calc_crc = cal_crc(commd, 23);
							commd[0] = 0;
							commd[28] = 0;
							commd[29] = 0;
							if (recv_crc == calc_crc)
							//if (1)
							{
								if (commd[1] == 0x01 || commd[1] == 0x00)
								{
									/* get the current trigger status and control mode */
									osMutexWait(id_mtx_ctrl, osWaitForever);
									stat = ctrl_param->trg;
									mode = ctrl_param->mode;
									osMutexRelease(id_mtx_ctrl);
									
									if (commd[2] == CMD_PAR)
									{
										/* setting the global parameters */
										osMutexWait(id_mtx_ctrl, osWaitForever);
										for (buf=0; buf<10; buf++)
											ctrl_param->param[param_mapping[buf]] = ((uint16_t)commd[buf*2+3]<<8)|commd[buf*2+4];
										if (ctrl_param->param[4] == 1 || ctrl_param->param[3] == 0)
											GPIO_SetBits(LED_GP, LED_BL);
										else
											GPIO_ResetBits(LED_GP, LED_BL);
										osMutexRelease(id_mtx_ctrl);
										/* convert the global parameters into level paramdters */
										//dev_param[0,6..8] = ctrl_param[0,6..8]/DEVCNT
										//dev_param[1..5] = ctrl_param[1..6]
										osMutexWait(id_mtx_dev, osWaitForever);
										for (i=0; i<10; i++)
											dev->param[i][0] = ctrl_param->param[0]/DEV_CNT;
										for (i=0; i<10; i++)
											for (buf=1; buf<6; buf++)
												dev->param[i][buf] = ctrl_param->param[buf];
										for (i=0; i<10; i++)
											for (buf=6; buf<10; buf++)
												dev->param[i][buf] = ctrl_param->param[buf]/DEV_CNT;
										for (i=0; i<10; i++)
											dev->updated[i] = 1;
										osMutexRelease(id_mtx_dev);

										/* |-- dsp->updated */
										dspUpdate(LU_PVAL);
									}
									/* --------------------------------------------------------
											setting parameters 
									---------------------------------------------------------*/
									else if (commd[2] == CMD_LPAR)
									{
										/* setting each level parameters , commd[21] is the device index */
										osMutexWait(id_mtx_dev, osWaitForever);								
										for (buf=3; buf<20; buf+=2)
											dev->param[commd[21]-1][param_mapping[(buf-3)/2]] = ((uint16_t)commd[buf]<<8)|commd[buf+1];
										dev->updated[commd[21]-1] = 1;
										osMutexRelease(id_mtx_dev);
										/* convert the level parameters into global parameter */
										//ctrl_param[0] = SUM (dev_param[0])
										//ctrl_param[1..5] = dev_param[1..5]
										osMutexWait(id_mtx_ctrl, osWaitForever);
										ctrl_param->param[0] = 0;
										for (buf=0; buf<DEV_CNT; buf++)
											ctrl_param->param[0] += dev->param[buf][0];
										for (buf=1; buf<6; buf++)
										ctrl_param->param[buf] = dev->param[0][buf];
										//check if multi trigger , and turn on the blue led
										if (ctrl_param->param[4] == 1 || ctrl_param->param[3] == 0)
											GPIO_SetBits(LED_GP, LED_BL);
										else
											GPIO_ResetBits(LED_GP, LED_BL);
										osMutexRelease(id_mtx_ctrl);
										
										/* |-- dsp->updated */
										dspUpdate(LU_PVAL);
									}
									/* -------------------------------------------------------- 
											setting command 
									---------------------------------------------------------*/
									else if (commd[2] == CMD_SET)
									{
										/* setting the control mode -- 0x11 for Manual and 0x12 for Auto */
										if (commd[3] == CMD_AUTO && CHK_AUTO[mode][stat])
										{
											/* |-- ctrl_param->mode changed */
											setTRGMode(1);
										}
										else if (commd[3] == CMD_MANU && CHK_MANU[mode][stat])
										{
											/* |-- ctrl_param->mode changed */
											setTRGMode(0);
										}
										/* power-on or shut-down devices -- 0x13 for power-on and 0x14 for shut-down */
										else if (commd[3] == CMD_LON && CHK_ON[mode][stat])
										{
											/* |-- ctrl_param (uint16_t)commd[4,5] marks the device needed power-on , if any devices powered-on , then the ctrl_param-> will be set and allow trigger */
											if (commd[4]!=0 || commd[5]!=0)
												setTRGStatus(5, (int)((int)commd[4])<<8|commd[5]);
											else
												setTRGStatus(0);
											/* |-- dev->status (uint16_t)commd[4,5] marks the device needed power-on */
										}
										else if (commd[3] == CMD_LOFF && CHK_OFF[mode][stat])
										{
											setTRGStatus(0);
										}
										else if (commd[3] == CMD_ON && CHK_ON[mode][stat])
										{
											/* |-- ctrl_param (uint16_t)commd[4,5] marks the device needed power-on , if any devices powered-on , then the ctrl_param-> will be set and allow trigger */
											setTRGStatus ( 1 );
										}
										else if (commd[3] == CMD_OFF && CHK_OFF[mode][stat])
										{
											setTRGStatus(0);
										}
										else if (commd[3] == CMD_CHG && CHK_CHG[mode][stat])
										{
											if (stat != 4)
											{
												setTRGStatus (4);
											}
											else
											{
												setTRGStatus(0);
											}
										}
										/* pause or continue */
										else if (commd[3] == CMD_PAU && CHK_PAU[mode][stat])
										{
											/*  warning warning warning warning warning warning warning warning warning warning
													|-- this command only set ctrl_param->trg to 2 and forbiden trigger by user, the fpga itself maintain trigger-allowed and charged */
												/* ctrl_param->trg */
											if (stat != 2)
												setTRGStatus(2);
											else 
											{
												if (GPIO_ReadOutputDataBit(LED_GP, LED_YL) == RESET)
													setTRGStatus(3);
												else 
													setTRGStatus(1);
											}
												
											/*  */
	//											osMutexWait(id_mtx_dev, osWaitForever);
	//											for (buf=0;buf<10;buf++)
	//											{
	//												if (commd[3] == 0x15) 
	//													dev->status[buf] &= ~FS_TRG; 
	//												else if (dev->status[buf]&FS_24V)
	//													dev->status[buf] |= FS_TRG; 
	//											}
	//											osMutexRelease(id_mtx_dev);
										}
										else if ((commd[3] == 0xB0 || commd[3] == 0xB1) && (stat == 0 || stat == 1 || (mode == 0 && stat == 2)))
										{
											for (buf=0; buf<10; buf++)
											{
												if (commd[3] == 0xB0 && (dev->status[buf]&FS_24V))
													setDEVStatus ( buf, FS_CHG );
												else
													resetDEVStatus ( buf, FS_CHG );
													
											}
										}
										else if ((commd[3] == CMD_DZ || commd[3] == CMD_PWM || commd[3] == CMD_CF) && (stat == 0 || stat == 1 || (mode == 0 && stat == 2)))
										{
											/* |-- ctrl_param->mode changed */
											osMutexWait(id_mtx_dev, osWaitForever);
											if (commd[3] == CMD_DZ) 
											{
												dev->dz = ((uint16_t)commd[4]<<8)|commd[5];
												if (dev->dz%20 != 0) dev->dz = dev->dz - dev->dz%20 +20;
											}
											else if (commd[3] == CMD_PWM)
												dev->pwm_ratio = ((uint16_t)commd[4]<<8)|commd[5];
											else if (commd[3] == CMD_CF)
												dev->cf = ((uint16_t)commd[4]<<8)|commd[5];
											osMutexRelease(id_mtx_dev);
											/* |-- dsp->updated */
											dspUpdate(LU_DSTAT);
										}
										/* TRIGGER */
										else if (commd[3] == CMD_TRG && CHK_TRG[mode][stat])
										{
											if (stat == 1)
											{
												osTimerStart(id_tmr_trg, 1);
											}
											else if (GPIO_ReadOutputDataBit(LED_GP, LED_YL) == RESET)
											{
												osTimerStop(id_tmr_trg);
												setTRGStatus(1);
											}
										}
										else
										{
											sendback(RES_OPD);
											continue;
										}
									}
									else
									{
										sendback(RES_OPD);
										continue;
									}
									sendback(RES_OK);
									continue;
								}
							}
							else 
							{
								sendback(RES_ERR);
								continue;
							}
						}
					}
				}
			}
			/* timeout */
			else 
			{
				recv_pgc = 0;
				recv_index = 1;
				continue;
			}
		}
	}
}

