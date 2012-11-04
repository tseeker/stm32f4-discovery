#include "ch.h"
#include "hal.h"

#include "ssd1963.h"

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


void display_off( void );
void display_test_pattern( u16 start_mask );

int main( void )
{
	halInit();
	chSysInit();
	if ( palReadPad( GPIOA , GPIOA_BUTTON ) ) {
		lockdown( );
	}

	ssd1963Init( );

	int i = 0;
	while (TRUE) {
		display_test_pattern( 1 << i );
		i = ( i + 1 ) % 16;
		if ( palReadPad( GPIOA , GPIOA_BUTTON ) ) {
			display_off( );
		}
	}
}
