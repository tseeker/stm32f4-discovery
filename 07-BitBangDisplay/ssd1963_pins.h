#ifndef _H_PINS_SSD1963_STM32F4
#define _H_PINS_SSD1963_STM32F4


// Pin definitions
#define SSD1963_D0_PORT	GPIOC
#define SSD1963_D0_PAD	6
#define SSD1963_D1_PORT	GPIOC
#define SSD1963_D1_PAD	7
#define SSD1963_D2_PORT	GPIOD
#define SSD1963_D2_PAD	0
#define SSD1963_D3_PORT	GPIOD
#define SSD1963_D3_PAD	1
#define SSD1963_D4_PORT	GPIOE
#define SSD1963_D4_PAD	7
#define SSD1963_D5_PORT	GPIOE
#define SSD1963_D5_PAD	8
#define SSD1963_D6_PORT	GPIOE
#define SSD1963_D6_PAD	9
#define SSD1963_D7_PORT	GPIOE
#define SSD1963_D7_PAD	10
#define SSD1963_D8_PORT	GPIOE
#define SSD1963_D8_PAD	11
#define SSD1963_D9_PORT	GPIOE
#define SSD1963_D9_PAD	12
#define SSD1963_D10_PORT	GPIOE
#define SSD1963_D10_PAD	13
#define SSD1963_D11_PORT	GPIOE
#define SSD1963_D11_PAD	14
#define SSD1963_D12_PORT	GPIOE
#define SSD1963_D12_PAD	15
#define SSD1963_D13_PORT	GPIOD
#define SSD1963_D13_PAD	8
#define SSD1963_D14_PORT	GPIOD
#define SSD1963_D14_PAD	9
#define SSD1963_D15_PORT	GPIOD
#define SSD1963_D15_PAD	10
#define SSD1963_CS_PORT	GPIOD
#define SSD1963_CS_PAD	7
#define SSD1963_RS_PORT	GPIOD
#define SSD1963_RS_PAD	11
#define SSD1963_RD_PORT	GPIOD
#define SSD1963_RD_PAD	4
#define SSD1963_WR_PORT	GPIOD
#define SSD1963_WR_PAD	5
#define SSD1963_RESET_PORT	GPIOC
#define SSD1963_RESET_PAD	5

// Initialise all GPIO ports used by the SSD1963
#define _ssd1963_init_gpio(spd) \
	do { \
		palSetGroupMode( GPIOD , 0x0fb3 , 0 , \
			PAL_MODE_OUTPUT_PUSHPULL | spd ); \
		palSetGroupMode( GPIOC , 0x00e0 , 0 , \
			PAL_MODE_OUTPUT_PUSHPULL | spd ); \
		palSetGroupMode( GPIOE , 0xff80 , 0 , \
			PAL_MODE_OUTPUT_PUSHPULL | spd ); \
	} while ( 0 )

#define _ssd1963_set_reset \
	palSetPad( GPIOC , 5 )
#define _ssd1963_clear_reset \
	palClearPad( GPIOC , 5 )

#define _ssd1963_set_cs \
	palSetPad( GPIOD , 7 )
#define _ssd1963_clear_cs \
	palClearPad( GPIOD , 7 )

#define _ssd1963_set_rd \
	palSetPad( GPIOD , 4 )
#define _ssd1963_clear_rd \
	palClearPad( GPIOD , 4 )

#define _ssd1963_set_wr \
	palSetPad( GPIOD , 5 )
#define _ssd1963_clear_wr \
	palClearPad( GPIOD , 5 )

#define _ssd1963_set_rs \
	palSetPad( GPIOD , 11 )
#define _ssd1963_clear_rs \
	palClearPad( GPIOD , 11 )

// Write to the SSD1963's data ports
#define _ssd1963_write(in) \
	do { \
		palWritePort( GPIOD , ( palReadLatch( GPIOD ) & ~( 0x0703 ) ) \
			| ( ( (in) >> 2 ) & 0x0003 ) \
			| ( ( ( (in) >> 13 ) & 0x0007 ) << 8 ) ); \
		palWritePort( GPIOC , ( palReadLatch( GPIOC ) & ~( 0x00c0 ) ) \
			| ( ( (in) & 0x0003 ) << 6 ) ); \
		palWritePort( GPIOE , ( palReadLatch( GPIOE ) & ~( 0xff80 ) ) \
			| ( ( ( (in) >> 4 ) & 0x01ff ) << 7 ) ); \
	} while ( 0 )

#endif //_H_PINS_SSD1963_STM32F4
