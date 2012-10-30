#include <ch.h>
#include <hal.h>


static int leds[] = { GPIOD_LED4 , GPIOD_LED3 , GPIOD_LED5 , GPIOD_LED6 };

int main( void )
{
	// Init ChibiOS stuff
	halInit( );
	chSysInit( );

	// Blink that green light!
	int curLed = 0;
	while ( 1 ) {
		palSetPad( GPIOD , leds[ curLed ] );
		chThdSleepMilliseconds( 250 );
		palClearPad( GPIOD , leds[ curLed ] );
		curLed = ( curLed + 1 ) & 3;
	}
}
