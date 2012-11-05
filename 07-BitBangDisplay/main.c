#include "ch.h"
#include "hal.h"

#include "ssd1963.h"
#include "xpt2046.h"

// This is called when the board boots up with the user button pressed. The
// idea is to enter this mode if the wrong SPI device has been used and flashing
// is no longer possible.
__attribute__ ((noreturn))
static void lockdown( void )
{
	palSetPad( GPIOD , GPIOD_LED3 );
	while (TRUE) {
		// EMPTY
	}
}


static void displayTestPattern( u16 start_mask )
{
	_ssd1963_clear_cs;
	ssd1963StartWriting( 0 , 0 );
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
			ssd1963WriteData( c );
		}
	}
	_ssd1963_set_cs;
}


static int tryReading;

static void onTouchscreenIRQ( EXTDriver * driver __attribute__((unused)) ,
		expchannel_t channel __attribute__((unused)) )
{
	tryReading = TRUE;
}


#define PORT(P) \
	case (int) GPIO##P : \
		port = EXT_MODE_GPIO##P; \
		break;

static void initIRQs( void )
{
	EXTConfig config;
	u32 i;

	for ( i = 0 ; i < EXT_MAX_CHANNELS ; i ++ ) {
		if ( i == XPT2046_IRQ_PAD ) {
			int port;
			switch ( (int) XPT2046_IRQ_PORT ) {
				default: PORT(A); PORT(B);
				PORT(C); PORT(D); PORT(E);
				PORT(F); PORT(G); PORT(H);
			}
			config.channels[ i ].mode = EXT_CH_MODE_FALLING_EDGE
				| EXT_CH_MODE_AUTOSTART | port;
			config.channels[ i ].cb = &onTouchscreenIRQ;
		} else {
			config.channels[ i ].mode = EXT_CH_MODE_DISABLED;
			config.channels[ i ].cb = NULL;
		}
	}
	extStart( &EXTD1 , &config );
}


static void checkTouchScreen( void )
{
	int x , y;
	if ( ! xpt2046GetAverageCoordinates( &x , &y , 3 ) ) {
		return;
	}

	x >>= 8;

	_ssd1963_clear_cs;
	ssd1963WriteCommand( SSD1963_CMD_SET_POST_PROC );
	ssd1963WriteData( 0x40 );
	ssd1963WriteData( x );
	ssd1963WriteData( 0x40 );
	if ( y > 2048 ) {
		ssd1963WriteData( 0 );
	} else {
		ssd1963WriteData( 1 );
	}
	_ssd1963_set_cs;
}


int main( void )
{
	halInit();
	chSysInit();
	if ( palReadPad( GPIOA , GPIOA_BUTTON ) ) {
		lockdown( );
	}

	xpt2046Init( );
	initIRQs();
	ssd1963Init( );

	int i = 0;
	while (TRUE) {
		displayTestPattern( 1 << i );
		chThdSleep(100);
		i = ( i + 1 ) % 16;
		if ( tryReading ) {
			checkTouchScreen( );
			tryReading = FALSE;
		}
	}
}
