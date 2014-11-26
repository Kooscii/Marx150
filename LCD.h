#include "stm32f10x.h"

#define LCD_REG 0xF8
#define LCD_DAT 0xFA

#define LCD_DELAY_RST_LOW	1000
#define LCD_DELAY_RST			3000	//CAUTION!!!:after rst change to high, a long delay must be done !!!
#define LCD_DELAY_REG			30
#define LCD_DELAY_CLR			1000
#define LCD_DELAY_DAT			30

#define LCD_NONE 0x00
#define LCD_1 	 0x01
#define LCD_2 	 0x02
#define LCD_BOTH 0x03

void delay(uint16_t n);
void lcd_spi(uint8_t cmd, uint8_t dat);
void lcd_init(void); 
void lcd_print(uint8_t line, uint8_t pos, void* str);
void lcd_select(uint8_t sel);
