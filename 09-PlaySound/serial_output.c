#include "ch.h"
#include "hal.h"

#include "serial_output.h"


void soInit( void )
{
	sdStart( &SERIALOUT_DRIVER , NULL );
	palSetPadMode( SERIALOUT_TX_PORT , SERIALOUT_TX_PAD ,
			PAL_MODE_ALTERNATE( SERIALOUT_AF ) );
	palSetPadMode( SERIALOUT_RX_PORT , SERIALOUT_RX_PAD ,
			PAL_MODE_ALTERNATE( SERIALOUT_AF ) );
}
