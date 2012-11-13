#include "ch.h"
#include "hal.h"

#include "chprintf.h"

#include "serial_output.h"
#include "sound.h"


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


static int increment;
static int curPos;
static const s16 sinTable[16384] = {
#include "sin_table.h"
};


void debug(void)
{
	chprintf( (void*)&SD2 , "I2S:\r\n" );
	chprintf( (void*)&SD2 , " CR2 %x\r\n" , SPI3->CR2 );
	chprintf( (void*)&SD2 , " SR  %x\r\n" , SPI3->SR );
	chprintf( (void*)&SD2 , " CFG %x\r\n" , SPI3->I2SCFGR );
	chprintf( (void*)&SD2 , " PR  %x\r\n" , SPI3->I2SPR );

	chprintf( (void*)&SD2 , "DMA:\r\n" );
	chprintf( (void*)&SD2 , " CR  %x\r\n" ,
			SPID3.dmatx->stream->CR );
	chprintf( (void*)&SD2 , " NDT %x\r\n" ,
			SPID3.dmatx->stream->NDTR );
	chprintf( (void*)&SD2 , " PAR %x\r\n" ,
			SPID3.dmatx->stream->PAR );
	chprintf( (void*)&SD2 , " M0A %x\r\n" ,
			SPID3.dmatx->stream->M0AR );
	chprintf( (void*)&SD2 , " M1A %x\r\n" ,
			SPID3.dmatx->stream->M1AR );
	chprintf( (void*)&SD2 , " FCR %x\r\n" ,
			SPID3.dmatx->stream->FCR );
	chprintf( (void*)&SD2 , "curPos: %d\r\n" , curPos );
}


__attribute__ ((noreturn))
int main( void )
{
	halInit();
	chSysInit();
	if ( palReadPad( GPIOA , GPIOA_BUTTON ) ) {
		lockdown( );
	}

	soInit( );
	chprintf( (void*)&SD2 , "Sound card initialisation\r\n" );
	sndInit( 11025 );
	chprintf( (void*)&SD2 ,
			"Sound card initialisation complete\r\n" );
	increment = 654; // 16384 * 440 / 11025
	curPos = 0;

	debug( );

#ifdef USE_DMA_FOR_SEXY_BEEPS
	int i = 0;
	while ( 1 ) {
		s16 * buffer = sndGetBuffer( );
		if ( buffer == NULL ) {
			i ++;
			if ( i == 2000000 ) {
				i = 0;
				palTogglePad( GPIOD , GPIOD_LED4 );
				debug( );
			}
			continue;
		}
		palTogglePad( GPIOD , GPIOD_LED3 );

		int z;
		for ( z = 0 ; z < SND_BUFSIZE / 2 ; z ++ ) {
			buffer[ z * 2 ] = buffer[ z * 2 + 1 ]
				= sinTable[ curPos ];
			curPos = ( curPos + increment / 2 ) % 16384;
		}
	}

#else // USE_DMA_FOR_SEXY_BEEPS

	int i = 0;
	while ( 1 ) {
		u16 sr = SPI3->SR;
		if ( ( sr & 0x2 ) == 0 ) {
			continue;
		}
		SPI3->DR = sinTable[ curPos ];

		if ( ( sr & 0x4 ) == 0 ) {
			curPos = ( curPos + increment ) % 16384;
		}
		i ++;
		if ( i == 22100 ) {
			i = 0;
			palTogglePad( GPIOD , GPIOD_LED4 );
			debug( );
		}
	}
#endif // USE_DMA_FOR_SEXY_BEEPS
}
