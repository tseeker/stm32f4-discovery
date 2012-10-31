
#include "ch.h"
#include "hal.h"

static void onDataAvailable( EXTDriver * driver , expchannel_t channel );


static const int leds[] = { GPIOD_LED4 , GPIOD_LED3 , GPIOD_LED5 , GPIOD_LED6 };
static const int speeds[] = { 50 , 250 , 500 , 1000 };

static const EXTConfig extConfig = {{
	{ EXT_CH_MODE_DISABLED , NULL } ,	//00
	{ EXT_CH_MODE_DISABLED , NULL } ,	//01
	{ EXT_CH_MODE_DISABLED , NULL } ,	//02
	{ EXT_CH_MODE_DISABLED , NULL } ,	//03
	{ EXT_CH_MODE_DISABLED , NULL } ,	//04
	{ EXT_CH_MODE_DISABLED , NULL } ,	//05
	{ EXT_CH_MODE_DISABLED , NULL } ,	//06
	{ EXT_CH_MODE_DISABLED , NULL } ,	//07
	{ EXT_CH_MODE_DISABLED , NULL } ,	//08
	{ EXT_CH_MODE_DISABLED , NULL } ,	//09
	{ EXT_CH_MODE_DISABLED , NULL } ,	//0a
	{ EXT_CH_MODE_DISABLED , NULL } ,	//0b
	{ EXT_CH_MODE_DISABLED , NULL } ,	//0c
	{ EXT_CH_MODE_DISABLED , NULL } ,	//0d
	{ EXT_CH_MODE_DISABLED , NULL } ,	//0e
	{ EXT_CH_MODE_RISING_EDGE | EXT_CH_MODE_AUTOSTART , onDataAvailable } ,
	{ EXT_CH_MODE_DISABLED , NULL } ,
	{ EXT_CH_MODE_DISABLED , NULL } ,
	{ EXT_CH_MODE_DISABLED , NULL } ,
	{ EXT_CH_MODE_DISABLED , NULL } ,
	{ EXT_CH_MODE_DISABLED , NULL } ,
	{ EXT_CH_MODE_DISABLED , NULL } ,
	{ EXT_CH_MODE_DISABLED , NULL } ,
}};

static int selectedSpeed = 0;
static systime_t lastPressed = 0;


static void onDataAvailable( EXTDriver * driver __attribute__((unused)) ,
		expchannel_t channel __attribute__((unused)) )
{
	chSysLockFromIsr( );
	if ( chTimeNow( ) - lastPressed >= 500 ) {
		selectedSpeed = ( selectedSpeed + 1 ) & 3;
		lastPressed = chTimeNow( );
	}
	chSysUnlockFromIsr( );
}


int main(void)
{
	halInit();
	chSysInit();

	lastPressed = chTimeNow( );
	palSetPadMode( GPIOA , 15 , PAL_MODE_INPUT );
	extStart( &EXTD1 , &extConfig );

	int i = 0;
	while (TRUE) {
		palSetPad(GPIOD, leds[i]);
		chThdSleepMilliseconds(speeds[selectedSpeed]);
		palClearPad(GPIOD, leds[i]);
		i = ( i + 1 ) & 3;
	}
}
