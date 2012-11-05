#include "ch.h"
#include "hal.h"

#include "ssd1963.h"


static const u8 _ssd1963_reset_sequence[ ] = {
	// PLL configuration
	SSD1963_COMMAND | SSD1963_DATA(3) ,
		SSD1963_CMD_SET_PLL_MN ,
		0x1e , 0x02 , 0x04 ,

	// Enable PLL and wait until it's stable
	SSD1963_COMMAND | SSD1963_DELAY | SSD1963_DATA(1) ,
		SSD1963_CMD_SET_PLL , 1 ,
		1 ,

	// Fully enable PLL
	SSD1963_COMMAND | SSD1963_DATA(1) ,
		SSD1963_CMD_SET_PLL , 3 ,
};


static const u8 _ssd1963_init_sequence[ ] = {
	// Software reset
	SSD1963_COMMAND | SSD1963_DELAY ,
		SSD1963_CMD_SOFT_RESET ,
		5 ,

	// Pixel clock: screw finesse, crank that up to the max
	SSD1963_COMMAND | SSD1963_DATA( 3 ) | SSD1963_DELAY ,
		SSD1963_CMD_SET_LSHIFT_FREQ ,
		0x07 , 0xff , 0xff ,
		15 ,

	// Use 565 RGB format
	SSD1963_COMMAND | SSD1963_DELAY | SSD1963_DATA(1) ,
		SSD1963_CMD_SET_PIXEL_FORMAT ,
		0x03 ,
		5 ,

	// Setup LCD panel
	SSD1963_COMMAND | SSD1963_DATA( 7 ) ,
		SSD1963_CMD_SET_LCD_MODE ,
		0x20 ,	// Data width 24-bit, FRC and dithering disabled
			// Data latch on falling edge
			// HSync polarity: active low
			// VSync polarity: active low
		0x00 ,	// TFT mode
		// Width
		( ( ( SSD1963_SCR_WIDTH - 1 ) >> 8 ) & 0xff ) ,
		( ( SSD1963_SCR_WIDTH - 1 ) & 0xff ) ,
		// Height
		( ( ( SSD1963_SCR_HEIGHT - 1 ) >> 8 ) & 0xff ) ,
		( ( SSD1963_SCR_HEIGHT - 1 ) & 0xff ) ,
		0x00 , // Ignored (serial interface)

	SSD1963_COMMAND | SSD1963_DATA( 8 ) ,
		SSD1963_CMD_SET_HORI_PERIOD ,
		0x02 , 0x13 ,	// Total period (PCLK)
		0x00 , 0x08 ,	// Non-display period
		0x2b ,		// Sync pulse width (PCLK)
		0x00 , 0x02 ,	// Start location (PCLK)
		0x00 ,		// Ignored (serial interfaces)

	SSD1963_COMMAND | SSD1963_DATA( 7 ) ,
		SSD1963_CMD_SET_VERT_PERIOD ,
		0x01 , 0x20 ,	// Vertical total period
		0x00 , 0x04 ,	// Non-display period
		0x0c ,		// Sync pulse width
		0x00 , 0x02 ,	// Start location

	SSD1963_COMMAND | SSD1963_DATA(1) ,
		SSD1963_CMD_SET_ADDRESS_MODE ,
		0x00 ,		// Top to bottom, left to right, no reversing,
				// RGB framebuffer, LCD l-to-r refresh, no
				// flipping

	// TE signal is not connected on the ITDB02-4.3
	SSD1963_COMMAND ,
		SSD1963_CMD_SET_TEAR_OFF ,

	// Set standard gamma curve
	SSD1963_COMMAND | SSD1963_DATA(1) ,
		SSD1963_CMD_SET_GAMMA_CURVE ,
		1 ,

	// Enable display
	SSD1963_COMMAND ,
		SSD1963_CMD_SET_DISPLAY_ON ,
};


static void _ssd1963_reset_chip( void )
{
	// Select and reset the chip
	_ssd1963_set_reset;
	_ssd1963_set_cs;
	_ssd1963_set_rd;
	_ssd1963_set_wr;
	_ssd1963_clear_reset;
	chThdSleep( 100 );
	_ssd1963_set_reset;
	chThdSleep( 100 );

	// Run the PLL init sequence
	ssd1963RunSequence( _ssd1963_reset_sequence ,
			sizeof( _ssd1963_reset_sequence ) );
}



void ssd1963Init( void )
{
	_ssd1963_init_gpio( PAL_STM32_OSPEED_LOWEST );
	chThdSleep( 10 );
	_ssd1963_reset_chip( );

	chThdSleep( 10 );
	_ssd1963_init_gpio( PAL_STM32_OSPEED_HIGHEST );
	ssd1963RunSequence( _ssd1963_init_sequence ,
			sizeof( _ssd1963_init_sequence ) );

	_ssd1963_clear_cs;
	ssd1963StartWriting( 0 , 0 );
	int i;
	for ( i = 0 ; i < SSD1963_SCR_HEIGHT * SSD1963_SCR_WIDTH ; i ++ ) {
		ssd1963WriteData( 0 );
	}
	_ssd1963_set_cs;
}


void ssd1963RunSequence( const u8 * sequence , u32 size )
{
	_ssd1963_clear_cs;

	u32 addr = 0;
	u8 has_command = 0 , has_data = 0 , has_delay = 0;
	while ( addr < size ) {
		if ( has_command ) {
			u16 command = sequence[ addr ++ ];
			ssd1963WriteCommand( command );
			has_command = 0;
		} else if ( has_data ) {
			u16 data = sequence[ addr ++ ];
			ssd1963WriteData( data );
			has_data --;
		} else if ( has_delay ) {
			u8 delay = sequence[ addr ++ ];
			chThdSleep( delay );
			has_delay = 0;
		} else {
			u8 value = sequence[ addr ++ ];
			has_command = ( ( value & SSD1963_COMMAND )
					== SSD1963_COMMAND );
			has_delay = ( ( value & SSD1963_DELAY )
					== SSD1963_DELAY );
			has_data = value & ~( SSD1963_COMMAND | SSD1963_DELAY );
		}
	}

	_ssd1963_set_cs;
}


void ssd1963StartWriting( u32 x , u32 y )
{
	ssd1963WriteCommand( 0x2a );
	ssd1963WriteData( x >> 8 );
	ssd1963WriteData( x & 0xff );
	ssd1963WriteData( ( ( SSD1963_SCR_WIDTH - 1 ) >> 8 ) & 0xff );
	ssd1963WriteData( ( SSD1963_SCR_WIDTH - 1 ) & 0xff );
	ssd1963WriteCommand( 0x2b );
	ssd1963WriteData( y >> 8 );
	ssd1963WriteData( y & 0xff );
	ssd1963WriteData( ( ( SSD1963_SCR_HEIGHT - 1 ) >> 8 ) & 0xff );
	ssd1963WriteData( ( SSD1963_SCR_HEIGHT - 1 ) & 0xff );
	ssd1963WriteCommand( 0x2c ); 
}
