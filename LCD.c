#include "LCD.h"
#include "periph.h"
#include <string.h>

void delay(uint16_t n)
{
   u16 i=0;  
   while(n--)
   {
      i=10;
      while(i--) ;    
   }
}

void lcd_select(uint8_t sel)
{
	GPIO_WriteBit(LCD_GPE, LCD_E1, (BitAction)(sel&LCD_1));
	GPIO_WriteBit(LCD_GPE, LCD_E2, (BitAction)(sel&LCD_2));
}

void lcd_spi(uint8_t cmd, uint8_t dat)
{
	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET)
		;
  SPI_I2S_SendData(SPI1, cmd);
	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET)
		;
	//delay(10);
	SPI_I2S_SendData(SPI1,(uint8_t)(dat&0xf0));
	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET)
		;
	//delay(10);
	SPI_I2S_SendData(SPI1,(uint8_t)(dat<<4&0xf0));
//	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET)
//		;
}

void lcd_init() 
{
	lcd_select(LCD_BOTH);
	
	GPIO_WriteBit(LCD_GP, LCD_RST, Bit_RESET);
	delay(LCD_DELAY_RST_LOW);
	GPIO_WriteBit(LCD_GP, LCD_RST, Bit_SET);
	delay(LCD_DELAY_RST);
	
	lcd_spi(LCD_REG, 0x38);
	delay(LCD_DELAY_REG);
	lcd_spi(LCD_REG, 0x0c);
	delay(LCD_DELAY_REG);
	lcd_spi(LCD_REG, 0x06);
	delay(LCD_DELAY_REG);
	lcd_spi(LCD_REG, 0x02);
	delay(LCD_DELAY_REG);
	lcd_spi(LCD_REG, 0x01);
	delay(LCD_DELAY_CLR);
}
