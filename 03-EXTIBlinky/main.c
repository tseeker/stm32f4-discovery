
#include "ch.h"
#include "hal.h"

static void onButtonPressed( EXTDriver * driver , expchannel_t channel );


static const int leds[] = { GPIOD_LED4 , GPIOD_LED3 , GPIOD_LED5 , GPIOD_LED6 };
static const int speeds[] = { 50 , 250 , 500 , 1000 };

static const EXTConfig extConfig = {{
	{ EXT_CH_MODE_RISING_EDGE | EXT_CH_MODE_AUTOSTART , onButtonPressed } ,
	{ EXT_CH_MODE_DISABLED , NULL } ,
	{ EXT_CH_MODE_DISABLED , NULL } ,
	{ EXT_CH_MODE_DISABLED , NULL } ,
	{ EXT_CH_MODE_DISABLED , NULL } ,
	{ EXT_CH_MODE_DISABLED , NULL } ,
	{ EXT_CH_MODE_DISABLED , NULL } ,
	{ EXT_CH_MODE_DISABLED , NULL } ,
	{ EXT_CH_MODE_DISABLED , NULL } ,
	{ EXT_CH_MODE_DISABLED , NULL } ,
	{ EXT_CH_MODE_DISABLED , NULL } ,
	{ EXT_CH_MODE_DISABLED , NULL } ,
	{ EXT_CH_MODE_DISABLED , NULL } ,
	{ EXT_CH_MODE_DISABLED , NULL } ,
	{ EXT_CH_MODE_DISABLED , NULL } ,
	{ EXT_CH_MODE_DISABLED , NULL } ,
	{ EXT_CH_MODE_DISABLED , NULL } ,
	{ EXT_CH_MODE_DISABLED , NULL } ,
	{ EXT_CH_MODE_DISABLED , NULL } ,
	{ EXT_CH_MODE_DISABLED , NULL } ,
	{ EXT_CH_MODE_DISABLED , NULL } ,
	{ EXT_CH_MODE_DISABLED , NULL } ,
	{ EXT_CH_MODE_DISABLED , NULL } ,
}};

static int selectedSpeed = 0;


static void onButtonPressed( EXTDriver * driver __attribute__((unused)) ,
		expchannel_t channel __attribute__((unused)) )
{
	chSysLockFromIsr( );
	selectedSpeed = ( selectedSpeed + 1 ) & 3;
	chSysUnlockFromIsr( );
}


int main(void)
{
	halInit();
	chSysInit();

	extStart( &EXTD1 , &extConfig );

	int i = 0;
	while (TRUE) {
		palSetPad(GPIOD, leds[i]);
		chThdSleepMilliseconds(speeds[selectedSpeed]);
		palClearPad(GPIOD, leds[i]);
		i = ( i + 1 ) & 3;
	}
}
