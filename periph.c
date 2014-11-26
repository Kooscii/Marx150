#include "periph.h"

void RCC_Config()
{
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA |
                         RCC_APB2Periph_GPIOB |
                         RCC_APB2Periph_GPIOC |
                         RCC_APB2Periph_SPI1 |
                         RCC_APB2Periph_USART1 |
                         RCC_APB2Periph_AFIO
                         , ENABLE);
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2 |
                         RCC_APB1Periph_TIM2
                         , ENABLE);

  GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);
}

void GPIO_Config()
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	
	/* USART1 */
	GPIO_InitStructure.GPIO_Pin = USART1_TX;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_Init(USART1_GP, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = USART1_RX;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(USART1_GP, &GPIO_InitStructure);
	
	/* USART2 */
	GPIO_InitStructure.GPIO_Pin = USART2_TX;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_Init(USART2_GP, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = USART2_RX;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(USART2_GP, &GPIO_InitStructure);
	
//	/* USART3 */
//	GPIO_InitStructure.GPIO_Pin = USART3_TX;
//  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
//  GPIO_Init(USART3_GP, &GPIO_InitStructure);
//	GPIO_InitStructure.GPIO_Pin = USART3_RX;
//  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
//  GPIO_Init(USART3_GP, &GPIO_InitStructure);
	
	/* LCD */
	GPIO_InitStructure.GPIO_Pin = LCD_CLK | LCD_SDA;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = LCD_RST;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = LCD_E1 | LCD_E2;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	/* BUTTON */
	GPIO_InitStructure.GPIO_Pin = BTN_LEFT | BTN_RIGHT | BTN_BACK | BTN_SET | BTN_ENCODER;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
  GPIO_Init(BTN_GP, &GPIO_InitStructure);
	
	/* ENCODER */ 
  GPIO_InitStructure.GPIO_Pin = BTN_ENC_A | BTN_ENC_B;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(BTN_ENC_GP, &GPIO_InitStructure);
	
	/* LEDs */
	GPIO_InitStructure.GPIO_Pin = LED_RD | LED_GR | LED_YL | LED_BL;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
  GPIO_Init(LED_GP, &GPIO_InitStructure);
	GPIO_SetBits(LED_GP, LED_RD);
	GPIO_SetBits(LED_GP, LED_GR);
	GPIO_SetBits(LED_GP, LED_YL);
	GPIO_SetBits(LED_GP, LED_BL);
	
	/* SIGNAL CTRL FPGA */
	GPIO_InitStructure.GPIO_Pin = SIG_EN | SIG_TRG | SIG_K_CRG | SIG_K_TRG;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(SIG_GP, &GPIO_InitStructure);
	GPIO_ResetBits(SIG_GP, SIG_EN);
	GPIO_ResetBits(SIG_GP, SIG_TRG);
	GPIO_ResetBits(SIG_GP, SIG_K_CRG);
	GPIO_ResetBits(SIG_GP, SIG_K_TRG);
}

void USART_Config()
{
  USART_InitTypeDef USART_InitStructure;

	/* USART1 */
	USART_InitStructure.USART_BaudRate = 115200;
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;
  USART_InitStructure.USART_StopBits = USART_StopBits_1;
  USART_InitStructure.USART_Parity = USART_Parity_No;
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
  USART_Init(USART1, &USART_InitStructure);
	USART_Cmd(USART1, ENABLE);
	
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE); 	// enable usart1 rx it
	
	/* USART2 */
  USART_InitStructure.USART_BaudRate = 19200;
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;
  USART_InitStructure.USART_StopBits = USART_StopBits_2;
  USART_InitStructure.USART_Parity = USART_Parity_No;
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
  USART_Init(USART2, &USART_InitStructure);
	USART_Cmd(USART2, ENABLE);
	
//	/* USART3 */
//	USART_InitStructure.USART_BaudRate = 19200;
//  USART_InitStructure.USART_WordLength = USART_WordLength_8b;
//  USART_InitStructure.USART_StopBits = USART_StopBits_2;
//  USART_InitStructure.USART_Parity = USART_Parity_No;
//  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
//  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
//  USART_Init(USART3, &USART_InitStructure);
//  USART_Cmd(USART3, ENABLE);
}

void SPI_Config(void)
{
  SPI_InitTypeDef  SPI_InitStructure;

	/* LCD */
  SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
  SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
  SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
  SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
  SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
  SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
  SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_128;
  SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
  SPI_InitStructure.SPI_CRCPolynomial = 7;
  SPI_Init(SPI1, &SPI_InitStructure);

  SPI_Cmd(SPI1, ENABLE);
}

void TIM_Config(void)
{
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
  TIM_ICInitTypeDef  TIM_ICInitStructure;

	/* ENCODER */
  TIM_TimeBaseStructure.TIM_Prescaler = 1;  // No prescaling
  TIM_TimeBaseStructure.TIM_Period = DEV_CNT-1;
  TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);

  TIM_ICInitStructure.TIM_Channel = TIM_Channel_1;
  TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Falling;
  TIM_ICInitStructure.TIM_ICFilter = 0x0d;
  TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;
  TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;
  TIM_ICInit(TIM2, &TIM_ICInitStructure);

  TIM_ICInitStructure.TIM_Channel = TIM_Channel_2;
  TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Falling;
  TIM_ICInitStructure.TIM_ICFilter = 0x0d;
  TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;
  TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;
  TIM_ICInit(TIM2, &TIM_ICInitStructure);

  TIM_EncoderInterfaceConfig(TIM2, TIM_EncoderMode_TI1, TIM_ICPolarity_Falling, TIM_ICPolarity_Falling);

  //TIM2->CNT = 100;

  TIM_Cmd(TIM2, ENABLE);
}

void NVIC_Config(void) 
{ 
  NVIC_InitTypeDef NVIC_InitStructure;
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0); 
  NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0; 
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; 
  NVIC_Init(&NVIC_InitStructure); 

}
