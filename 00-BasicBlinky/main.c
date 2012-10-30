#include <ch.h>
#include <hal.h>


int main( void )
{
	// Init ChibiOS stuff
	halInit( );
	chSysInit( );

	// Blink that green light!
	while ( 1 ) {
		palSetPad( GPIOD , GPIOD_LED4 );
		chThdSleepMilliseconds( 250 );
		palClearPad( GPIOD , GPIOD_LED4 );
		chThdSleepMilliseconds( 250 );
	}
}
