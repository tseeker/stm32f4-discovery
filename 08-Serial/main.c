#include "ch.h"
#include "hal.h"


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


void write( const char * string )
{
	while ( *string ) {
		sdPut( &SD2 , *( string++ ) );
	}
}


void writeLn( const char * string )
{
	write( string );
	sdWrite( &SD2 , (const u8 *) "\r\n" , 2 );
}


static const char * colours[] = {
	"orange" , "green" , "red" , "blue"
};

static const char keys[] = { 'o' , 'g' , 'r' , 'b' };
static const int pads[] = {
	GPIOD_LED3 , GPIOD_LED4 , GPIOD_LED5 , GPIOD_LED6
};

int main( void )
{
	int state[4] = { 0 , 0 , 0 , 0 };
	int changed = 1;

	halInit();
	chSysInit();
	if ( palReadPad( GPIOA , GPIOA_BUTTON ) ) {
		lockdown( );
	}

	// Set up serial driver
	sdStart( &SD2 , NULL );
	palSetPadMode( GPIOA , 2 , PAL_MODE_ALTERNATE(7) );
	palSetPadMode( GPIOA , 3 , PAL_MODE_ALTERNATE(7) );

	writeLn( "Program ready" );
	writeLn( "" );
	while ( 1 ) {
		int i;

		if ( changed ) {
			write( "Current state : " );
			for ( i = 0 ; i < 4 ; i ++ ) {
				write( colours[ i ] );
				write( " " );
				if ( state[ i ] ) {
					write( "on" );
				} else {
					write( "off" );
				}
				if ( i < 3 ) {
					write( "; " );
				}
			}
			writeLn( "" );
		}

		char c = sdGetTimeout( &SD2 , 250 );
		for ( i = 0 ; i < 4 ; i ++ ) {
			if ( c == ' ' || keys[i] == c ) {
				state[i] = !state[i];
				palTogglePad( GPIOD , pads[i] );
				if ( c != ' ' ) {
					break;
				}
			}
		}
		changed = ( i != 4 || c == ' ' );
	}
}
