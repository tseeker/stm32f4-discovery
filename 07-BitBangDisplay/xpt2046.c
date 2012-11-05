#include "ch.h"
#include "hal.h"

#include "xpt2046.h"

static const SPIConfig _xpt2046_spi_config = {
	NULL ,
	XPT2046_NSS_PORT ,
	XPT2046_NSS_PAD ,
	SPI_CR1_BR_2 | SPI_CR1_BR_1 | SPI_CR1_BR_0
};



u16 _xpt2046_get_reading( u8 control )
{
	u8 tData[3] = { control , 0 , 0 };
	u8 rData[3] = { 0 , 0 , 0 };

#if SPI_USE_MUTUAL_EXCLUSION
	spiAcquireBus( &( XPT2046_SPI_DRIVER ) );
#endif

	palClearPad( XPT2046_NSS_PORT , XPT2046_NSS_PAD );
	spiExchange( &( XPT2046_SPI_DRIVER ) , 3 , tData , rData );
	palSetPad( XPT2046_NSS_PORT , XPT2046_NSS_PAD );

#if SPI_USE_MUTUAL_EXCLUSION
	spiReleaseBus( &( XPT2046_SPI_DRIVER ) );
#endif

	if ( ( control & 0x08 ) == 0 ) {
		return ( rData[1] << 5 ) | ( rData[2] >> 3 );
	}
	return ( rData[1] << 4 ) | ( rData[2] >> 4 );
}



void xpt2046Init( void )
{
	// Initialise SPI
	spiStart( &( XPT2046_SPI_DRIVER ) , &_xpt2046_spi_config );

	// NSS signal
	palSetPadMode( XPT2046_NSS_PORT , XPT2046_NSS_PAD ,
			PAL_MODE_OUTPUT_PUSHPULL );
	palSetPad( XPT2046_NSS_PORT , XPT2046_NSS_PAD );

	// Main SPI signals
	palSetPadMode( XPT2046_CLK_PORT , XPT2046_CLK_PAD ,
			PAL_MODE_ALTERNATE( 5 ) );
	palSetPadMode( XPT2046_DIN_PORT , XPT2046_DIN_PAD ,
			PAL_MODE_ALTERNATE( 5 ) );
	palSetPadMode( XPT2046_DOUT_PORT , XPT2046_DOUT_PAD ,
			PAL_MODE_ALTERNATE( 5 ) );

	// PENIRQ signal
	palSetPadMode( XPT2046_IRQ_PORT , XPT2046_IRQ_PAD , PAL_MODE_INPUT );

	// Read a sample, leaving PENIRQ active
	_xpt2046_get_reading( 0x90 );
}


int xpt2046GetCoordinates( int * pX , int * pY )
{
	int i;
	int allX[ 7 ] , allY[ 7 ];
	_xpt2046_get_reading( 0xd1 );
	_xpt2046_get_reading( 0x91 );
	for ( i = 0 ; i < 7 ; i ++ ) {
		allX[ i ] = _xpt2046_get_reading( 0xd1 );
		allY[ i ] = _xpt2046_get_reading( 0x91 );
	}

	int j;
	for ( i = 0 ; i < 4 ; i ++ ) {
		for ( j = i ; j < 7 ; j ++ ) {
			int temp = allX[ i ];
			if ( temp > allX[ j ] ) {
				allX[ i ] = allX[ j ];
				allX[ j ] = temp;
			}
			temp = allY[ i ];
			if ( temp > allY[ j ] ) {
				allY[ i ] = allY[ j ];
				allY[ j ] = temp;
			}
		}
	}
	_xpt2046_get_reading( 0x90 );

	if ( palReadPad( XPT2046_IRQ_PORT , XPT2046_IRQ_PAD ) ) {
		return 0;
	}

	*pX = allX[ 3 ];
	*pY = allY[ 3 ];

	return 1;
}

int xpt2046GetAverageCoordinates( int * pX , int * pY , int nSamples )
{
	int nRead = 0;
	int xAcc = 0 , yAcc = 0;
	int x , y;

	while ( nRead < nSamples ) {
		if ( !xpt2046GetCoordinates( &x , &y ) ) {
			break;
		}
		xAcc += x;
		yAcc += y;
		nRead ++;
	}

	if ( nRead == 0 ) {
		return 0;
	}
	*pX = xAcc / nRead;
	*pY = yAcc / nRead;
	return 1;
}
