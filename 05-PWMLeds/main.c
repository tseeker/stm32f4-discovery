
#include "ch.h"
#include "hal.h"

// Green, Orange, Red, Blue
static const int leds[] = { GPIOD_LED4 , GPIOD_LED3 , GPIOD_LED5 , GPIOD_LED6 };
static const int speeds[] = { 50 , 250 , 500 , 1000 };


static PWMConfig pwmConfig = {
	2000 ,
	2000 ,
	NULL ,
	{
		{ PWM_OUTPUT_ACTIVE_HIGH , NULL } ,
		{ PWM_OUTPUT_ACTIVE_HIGH , NULL } ,
		{ PWM_OUTPUT_ACTIVE_HIGH , NULL } ,
		{ PWM_OUTPUT_ACTIVE_HIGH , NULL } ,
	} ,
	0 ,	// CR2
};


static void setupPWM( void )
{
	int i;

	// Initialise PWM, route output to LEDs, setup channels
	pwmStart( &PWMD4 , &pwmConfig );
	for ( i = 0 ; i < 4 ; i ++ ) {
		palSetPadMode( GPIOD , leds[ i ] , PAL_MODE_ALTERNATE( 2 ) );
		pwmEnableChannel( &PWMD4 , i , speeds[ i ] );
	}
}

static void pwmReset( void * ptr __attribute__((unused)) )
{
	pwmStop( &PWMD4 );
	setupPWM( );
}

static VirtualTimer pwmResetTimer;

static void onButtonPressed( EXTDriver * driver __attribute__((unused)) ,
		expchannel_t channel __attribute__((unused)) )
{
	int i;

	chSysLockFromIsr( );
	for ( i = 0 ; i < 4 ; i ++ ) {
		pwmConfig.channels[ i ].mode = 3 - pwmConfig.channels[ i ].mode;
	}
	chVTSetI( &pwmResetTimer , 1 , &pwmReset , NULL );
	chSysUnlockFromIsr( );
}

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


int main(void)
{
	halInit();
	chSysInit();

	setupPWM( );
	extStart( &EXTD1 , &extConfig );

	while (TRUE) {
		chThdYield( );
	}
}
