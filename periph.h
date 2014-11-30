#ifndef _PERIPH_H
#define _PERIPH_H

#include "stm32f10x.h"
#include "misc.h"

#define DEV_CNT 8

/* USART1 A9 A10 */
#define USART1_TX GPIO_Pin_9
#define USART1_RX GPIO_Pin_10
#define USART1_GP GPIOA

/* USART2 A2 A3 */
#define USART2_TX GPIO_Pin_2
#define USART2_RX GPIO_Pin_3
#define USART2_GP GPIOA

/* USART3 B10 B11 */
#define USART3_TX GPIO_Pin_10
#define USART3_RX GPIO_Pin_11
#define USART3_GP GPIOB

/* LCD A4 A5 A7 B0 B1*/
#define LCD_RST GPIO_Pin_4
#define LCD_CLK GPIO_Pin_5
#define LCD_SDA GPIO_Pin_7
#define LCD_GP  GPIOA
#define LCD_E1 	GPIO_Pin_0
#define LCD_E2 	GPIO_Pin_1
#define LCD_GPE GPIOB

/* LEDs B12 B13 AB14 B15 */
#define LED_RD 	GPIO_Pin_15
#define LED_YL 	GPIO_Pin_14
#define LED_BL 	GPIO_Pin_13
#define LED_GR  GPIO_Pin_12
#define LED_GP 	GPIOB

/* BUTTON B9 B8 B7 B6 B5 */
#define BTN_LEFT 		GPIO_Pin_8
#define BTN_RIGHT 	GPIO_Pin_7
#define BTN_BACK 		GPIO_Pin_6
#define BTN_SET 		GPIO_Pin_5
#define BTN_ENCODER	GPIO_Pin_9
#define BTN_ENC_A		GPIO_Pin_0
#define BTN_ENC_B		GPIO_Pin_1
#define BTN_GP			GPIOB
#define BTN_ENC_GP	GPIOA

/* SIGNAL CTRL FPGA */
#define SIG_EN		GPIO_Pin_10
#define SIG_TRG		GPIO_Pin_11
#define SIG_K_CRG	GPIO_Pin_3
#define SIG_K_TRG	GPIO_Pin_4
#define SIG_GP		GPIOB

/* TYPE_DEVICE -> status */
#define FS_24V 0x01
#define FS_TRG 0x02
#define FS_CHG 0x04

/* TYPE_DISP -> updated */
#define LU_PVAL		0x01
#define LU_PNAME	0x02
#define LU_CHO		0x04
#define LU_DSTAT	0x08
#define LU_CMODE	0x10
#define LU_CTRG		0x80

/* Command */
#define CMD_PAR		0x20
#define CMD_SET		0x21
#define CMD_LPAR	0x22
#define CMD_LON		0xC0
#define CMD_LOFF	0xC1
#define CMD_MANU	0x11
#define CMD_AUTO	0x12
#define CMD_ON		0x13
#define CMD_OFF		0x14
#define CMD_PAU		0x15
#define CMD_CHG		0x16
#define CMD_TRG 	0x17
#define CMD_CF		0xA2
#define CMD_DZ		0xA0
#define CMD_PWM		0xA1

//void delay_us(uint32_t us);
void RCC_Config(void);
void GPIO_Config(void);
void SPI_Config(void);
void EXTI_Config(void);
void TIM_Config(void);
void USART_Config(void);
void NVIC_Config(void);
//void PWM_Config(u16 PWM_Period, u16 Duty);
//void PWM_ChangeDuty(u16 Duty);


/*----------------------------------------------------------------------------
     TYPE_DEVICE * dev
 *---------------------------------------------------------------------------*/
/* param[dev_index][param_index] -- the name of each parameters can be found in " menu.h "*/
/* status[dev_index] -- F_CHG | F_TRG | F_24V */
/* updated -- used for mark if parameters should be updated to fpga */
typedef struct
{
	uint16_t param[10][10]; 	
	uint8_t status[10];	//updated all time ( every 10*10 ms )
	uint8_t updated[10];	// mark if parameters are changed
	uint16_t dz, pwm_ratio, cf;
} TYPE_DEVICE;

typedef struct
{
	uint8_t trg, mode, admin;
	uint16_t freq, times, param[10];
} TYPE_CTRL;

/*----------------------------------------------------------------------------
     TYPE_DISP * dsp
 *---------------------------------------------------------------------------*/
/* sel_dev -- the index of the device which is selected to be shown on LCD " */
/* sel_param -- the index of the parameter page which is selected to be shown on LCD , three parameters in each page */
/* sel_choice -- the index of the choosed selection */
/* updated -- L_CTRG |  |  |  | L_DSTAT | L_CHO | L_PNAME | L_PVAL */
typedef struct
{
	uint8_t sel_dev, sel_param, sel_choice, updated, mode;
} TYPE_DISP;

#endif
