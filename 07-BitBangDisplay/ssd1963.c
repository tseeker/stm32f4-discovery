#include "ch.h"
#include "hal.h"

#include "ssd1963.h"


#define _command 0x40
#define _delay 0x80
#define _ndata(x) ( (u8)( (x) & 0x3f) )

static const u8 _ssd1963_reset_sequence[ ] = {
	/*
	 * PLL configuration
	 * -----------------
	 *
	 * PLL = input * ( P1 + 1 ) / ( P2 + 1 )
	 *	(P1 : 8 bits ; P2 : 4 bits; P3 : junk)
	 *
	 * 1/ The ITDB02-4.3 board has a 12MHz crystal (which contradicts the
	 * SSD1963 datasheet - wtf?).
	 * 2/ The SSD1963 must be clocked at *at most* 110MHz.
	 * 3/ Also, 250MHz < input * ( P1 + 1 ) < 800MHz
	 * 4/ The STM32F4 can communicate at 50MHz.
	 * 5/ The SSD1963 accepts PLL/2 accesses per second.
	 *
	 * 100MHz = 12MHz * ( P1 + 1 ) / ( P2 + 1 )
	 * 	=> P1 = 49, P2 = 5
	 * 100MHz = 10MHz * ( P1 + 1 ) / ( P2 + 1 )
	 * 	=> P1 = 19, P2 = 1
	 */
	_command | _ndata(3) ,
		SSD1963_CMD_SET_PLL_MN ,
		0x31 , 0x05 , 0x04 ,
//		0x1e , 0x02 , 0x04 ,

	// Enable PLL and wait until it's stable
	_command | _delay | _ndata(1) ,
		SSD1963_CMD_SET_PLL , 1 ,
		1 ,

	// Fully enable PLL
	_command | _ndata(1) ,
		SSD1963_CMD_SET_PLL , 3 ,
};


static const u8 _ssd1963_init_sequence[ ] = {
	// Software reset
	_command | _delay ,
		SSD1963_CMD_SOFT_RESET ,
		5 ,

	/* Pixel clock: screw finesse, crank that up to the max */
	_command | _ndata( 3 ) | _delay ,
		SSD1963_CMD_SET_LSHIFT_FREQ ,
		0x07 , 0xff , 0xff ,
		15 ,

	// Use 565 RGB format
	_command | _delay | _ndata(1) ,
		SSD1963_CMD_SET_PIXEL_FORMAT ,
		0x03 ,
		5 ,

	// Setup LCD panel
	_command | _ndata( 7 ) ,
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

	_command | _ndata( 8 ) ,
		SSD1963_CMD_SET_HORI_PERIOD ,
		0x02 , 0x13 ,	// Total period (PCLK)
		0x00 , 0x08 ,	// Non-display period
		0x2b ,		// Sync pulse width (PCLK)
		0x00 , 0x02 ,	// Start location (PCLK)
		0x00 ,		// Ignored (serial interfaces)

	_command | _ndata( 7 ) ,
		SSD1963_CMD_SET_VERT_PERIOD ,
		0x01 , 0x20 ,	// Vertical total period
		0x00 , 0x04 ,	// Non-display period
		0x0c ,		// Sync pulse width
		0x00 , 0x02 ,	// Start location

	_command | _ndata(1) ,
		0x36 ,
		0x00 ,

	_command ,
		SSD1963_CMD_SET_DISPLAY_ON ,

	_command | _ndata(6) ,
		SSD1963_CMD_SET_PWM_CONF ,
		0x06 , 0xf0 , 0x01 , 0xf0 , 0 , 0 ,

	_command | _ndata(1) ,
		0xd0 ,
		0x0d ,

	_command | _ndata(1) , SSD1963_CMD_SET_GAMMA_CURVE, 1 ,
	//_command | SSD1963_CMD_ENTER_NORMAL_MODE ,
	//_command | SSD1963_CMD_EXIT_INVERT_MODE ,
	_command , SSD1963_CMD_EXIT_IDLE_MODE ,
/*
	SSD1963_CMD_SET_TEAR_OFF ,
*/
};


#define _ssd1963_wcmd(command) \
	do { \
		_ssd1963_clear_rs; \
		_ssd1963_write( command ); \
		_ssd1963_clear_wr; \
		_ssd1963_set_wr; \
	} while ( FALSE )
#define _ssd1963_wdata(data) \
	do { \
		_ssd1963_set_rs; \
		_ssd1963_write( data ); \
		_ssd1963_clear_wr; \
		_ssd1963_set_wr; \
	} while ( FALSE )


void _ssd1963_run_sequence( const u8 * sequence , u32 size )
{
	_ssd1963_clear_cs;

	u32 addr = 0;
	u8 has_command = 0 , has_data = 0 , has_delay = 0;
	while ( addr < size ) {
		if ( has_command ) {
			u16 command = sequence[ addr ++ ];
			_ssd1963_wcmd( command );
			has_command = 0;
		} else if ( has_data ) {
			u16 data = sequence[ addr ++ ];
			_ssd1963_wdata( data );
			has_data --;
		} else if ( has_delay ) {
			u8 delay = sequence[ addr ++ ];
			chThdSleep( delay );
			has_delay = 0;
		} else {
			u8 value = sequence[ addr ++ ];
			has_command = ( ( value & _command ) == _command );
			has_delay = ( ( value & _delay ) == _delay );
			has_data = value & ~( _command | _delay );
		}
	}

	_ssd1963_set_cs;
}


void _ssd1963_reset_chip( void )
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
	_ssd1963_run_sequence( _ssd1963_reset_sequence ,
			sizeof( _ssd1963_reset_sequence ) );
}



void _ssd1963_set_pos( u32 x , u32 y )
{
	_ssd1963_wcmd( 0x2a );
	_ssd1963_wdata( x >> 8 );
	_ssd1963_wdata( x & 0xff );
	_ssd1963_wdata( ( ( SSD1963_SCR_WIDTH - 1 ) >> 8 ) & 0xff );
	_ssd1963_wdata( ( SSD1963_SCR_WIDTH - 1 ) & 0xff );
	_ssd1963_wcmd( 0x2b );
	_ssd1963_wdata( y >> 8 );
	_ssd1963_wdata( y & 0xff );
	_ssd1963_wdata( ( ( SSD1963_SCR_HEIGHT - 1 ) >> 8 ) & 0xff );
	_ssd1963_wdata( ( SSD1963_SCR_HEIGHT - 1 ) & 0xff );
	_ssd1963_wcmd( 0x2c ); 
}



void ssd1963Init( void )
{
	_ssd1963_init_gpio( PAL_STM32_OSPEED_LOWEST );
	chThdSleep( 10 );
	_ssd1963_reset_chip( );

	chThdSleep( 10 );
	_ssd1963_init_gpio( PAL_STM32_OSPEED_HIGHEST );
	_ssd1963_run_sequence( _ssd1963_init_sequence ,
			sizeof( _ssd1963_init_sequence ) );

	_ssd1963_clear_cs;
	_ssd1963_set_pos( 0 , 0 );
	int i;
	for ( i = 0 ; i < SSD1963_SCR_HEIGHT ; i ++ ) {
		u16 j , c = 0x8000;
		for ( j = 0 ; j < SSD1963_SCR_WIDTH ; j ++ ) {
			if ( ( j & 7 ) == 7 ) {
				c >>= 1;
				if ( !c ) {
					c = 0x8000;
				}
			}
			_ssd1963_wdata( c );
		}
	}
	_ssd1963_set_cs;
}


void display_test_pattern( u16 start_mask )
{
	_ssd1963_clear_cs;
	_ssd1963_set_pos( 0 , 0 );
	int i;
	for ( i = 0 ; i < SSD1963_SCR_HEIGHT ; i ++ ) {
		u16 j , c = start_mask;
		for ( j = 0 ; j < SSD1963_SCR_WIDTH ; j ++ ) {
			if ( ( j & 7 ) == 7 ) {
				c >>= 1;
				if ( !c ) {
					c = 0x8000;
				}
			}
			_ssd1963_wdata( c );
		}
	}
	_ssd1963_set_cs;
}


void display_off( void )
{
	static int display = 1;
	display = !display;
	_ssd1963_clear_cs;
	if ( display ) {
		_ssd1963_wcmd( SSD1963_CMD_SET_DISPLAY_ON );
	} else {
		_ssd1963_wcmd( SSD1963_CMD_SET_DISPLAY_OFF );
	}
	_ssd1963_set_cs;
}
