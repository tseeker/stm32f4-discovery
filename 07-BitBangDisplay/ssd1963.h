#ifndef _H_SSD1963_STM32F4
#define _H_SSD1963_STM32F4

#include "ssd1963_pins.h"


// Screen width (pixels)
#define SSD1963_SCR_WIDTH	480
// Screen height (pixels)
#define SSD1963_SCR_HEIGHT	272


// Commands
#define SSD1963_CMD_			0x00
#define SSD1963_CMD_NOP			0x00
#define SSD1963_CMD_SOFT_RESET		0x01
#define SSD1963_CMD_ENTER_SLEEP_MODE	0x10
#define SSD1963_CMD_ENTER_NORMAL_MODE	0x13
#define SSD1963_CMD_EXIT_INVERT_MODE	0x20
#define SSD1963_CMD_SET_GAMMA_CURVE	0x26
#define SSD1963_CMD_SET_DISPLAY_OFF	0x28
#define SSD1963_CMD_SET_DISPLAY_ON	0x29
#define SSD1963_CMD_SET_TEAR_OFF	0x34
#define SSD1963_CMD_SET_ADDRESS_MODE	0x36
#define SSD1963_CMD_EXIT_IDLE_MODE	0x38
#define SSD1963_CMD_SET_LCD_MODE	0xb0
#define SSD1963_CMD_SET_HORI_PERIOD	0xb4
#define SSD1963_CMD_SET_VERT_PERIOD	0xb6
#define SSD1963_CMD_SET_GPIO_VALUE	0xba
#define SSD1963_CMD_SET_POST_PROC	0xbc
#define SSD1963_CMD_SET_PWM_CONF	0xbe
#define SSD1963_CMD_SET_PLL		0xe0
#define SSD1963_CMD_SET_PLL_MN		0xe2
#define SSD1963_CMD_SET_LSHIFT_FREQ	0xe6
#define SSD1963_CMD_SET_PIXEL_FORMAT	0xf0


// Command sequences
#define SSD1963_DATA(x)		( (u8)( (x) & 0x3f) )
#define SSD1963_COMMAND		0x40
#define SSD1963_DELAY		0x80


// Initialise the display controller
void ssd1963Init( void );

// Execute a sequence of commands from the specified address
void ssd1963RunSequence( const u8 * sequence , u32 size );


// Write a command to the SSD1963
#define ssd1963WriteCommand(command) \
	do { \
		_ssd1963_clear_rs; \
		_ssd1963_write( command ); \
		_ssd1963_clear_wr; \
		_ssd1963_set_wr; \
	} while ( FALSE )

// Write data to the SSD1963
#define ssd1963WriteData(data) \
	do { \
		_ssd1963_set_rs; \
		_ssd1963_write( data ); \
		_ssd1963_clear_wr; \
		_ssd1963_set_wr; \
	} while ( FALSE )


// Start writing data at the specified location
void ssd1963StartWriting( u32 x , u32 y );


#endif // _H_SSD1963_STM32F4
