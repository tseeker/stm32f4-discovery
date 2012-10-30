
#include "ch.h"
#include "hal.h"


static int leds[] = { GPIOD_LED4 , GPIOD_LED3 , GPIOD_LED5 , GPIOD_LED6 };
static int speeds[] = { 50 , 250 , 500 , 1000 };
static int selectedSpeed = 0;
static Mutex speedMutex;


static WORKING_AREA( waInputThread , 128 );
static msg_t InputThread( void * arg  __attribute__((unused)) ) {
	int button = 0;
	chRegSetThreadName( "speed selector" );
	while ( TRUE ) {
		int gpioState = palReadPad(GPIOA, GPIOA_BUTTON);
		if ( !button && gpioState ) {
			chMtxLock( &speedMutex );
			selectedSpeed = ( selectedSpeed + 1 ) & 3;
			chMtxUnlock( );
			button = TRUE;
		} else if ( button && !gpioState ) {
			button = FALSE;
		}
		chThdYield( );
	}
}


int main(void)
{
	halInit();
	chSysInit();

	chMtxInit( &speedMutex );
	chThdCreateStatic( waInputThread , sizeof( waInputThread ) ,
			NORMALPRIO , InputThread , NULL );

	int i = 0;
	while (TRUE) {
		int speed;
		chMtxLock( &speedMutex );
		speed = speeds[selectedSpeed];
		chMtxUnlock( );

		palSetPad(GPIOD, leds[i]);
		chThdSleepMilliseconds(speed);
		palClearPad(GPIOD, leds[i++]);
		i &= 3;
	}
}
