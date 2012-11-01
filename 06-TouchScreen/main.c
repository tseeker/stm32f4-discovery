#include "ch.h"
#include "hal.h"

#include "xpt2046.h"


// This is called when the board boots up with the user button pressed. The
// idea is to enter this mode if the wrong SPI device has been used and flashing
// is no longer possible.
__attribute__ ((noreturn))
static void lockdown( void )
{
	palSetPad( GPIOD , GPIOD_LED5 );
	while (TRUE) {
		// EMPTY
	}
}


static void checkTouchScreen( void )
{
	int x , y;
	if ( ! xpt2046GetAverageCoordinates( &x , &y , 3 ) ) {
		return;
	}

	palClearPad( GPIOD , GPIOD_LED4 );
	chThdSleep( 500 );
	int mask = 0x800;
	while ( mask != 0 ) {
		palTogglePad( GPIOD , GPIOD_LED5 );
		if ( ( x & mask ) != 0 ) {
			palSetPad( GPIOD , GPIOD_LED3 );
		} else {
			palClearPad( GPIOD , GPIOD_LED3 );
		}
		if ( ( y & mask ) != 0 ) {
			palSetPad( GPIOD , GPIOD_LED6 );
		} else {
			palClearPad( GPIOD , GPIOD_LED6 );
		}
		mask >>= 1;

		chThdSleep( 250 );
		palTogglePad( GPIOD , GPIOD_LED5 );
		palClearPad( GPIOD , GPIOD_LED3 );
		palClearPad( GPIOD , GPIOD_LED6 );
		chThdSleep( 250 );
	}
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



int main( void )
{
	halInit();
	chSysInit();

	if ( palReadPad( GPIOA , GPIOA_BUTTON ) ) {
		lockdown( );
	}

	xpt2046Init( );
	initIRQs();

	palSetPad( GPIOD , GPIOD_LED4 );
	while (TRUE) {
		if ( tryReading ) {
			checkTouchScreen( );
			tryReading = FALSE;
		}

		chThdSleep( 25 );
		palClearPad( GPIOD , GPIOD_LED4 );
		chThdSleep( 25 );
		palSetPad( GPIOD , GPIOD_LED4 );
	}
}
