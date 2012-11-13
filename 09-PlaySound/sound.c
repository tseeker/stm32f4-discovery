#include "ch.h"
#include "hal.h"
#include "chprintf.h"
#include "stm32f4xx.h"

#include "sound.h"
#include "i2s_bits.h"
#include "cs43l22_pins.h"
#include "cs43l22_commands.h"


/* I2C driver configuration */
static const I2CConfig _i2s_i2c_config = {
	OPMODE_I2C ,		// Operation mode
	I2S_OI2C_SPEED ,	// Clock frequency
	FAST_DUTY_CYCLE_2	// Duty cycle
};


/* Initialise one of the I2C pads */
#define _i2s_init_i2c_pad(port,pad) \
	palSetPadMode( port , pad , \
			PAL_STM32_OTYPE_OPENDRAIN | PAL_STM32_OSPEED_MID2 \
			| PAL_MODE_ALTERNATE(4) )

/* Initialise an I2S pad */
#define _i2s_init_i2s_pad(port,pad) \
	palSetPadMode( port , pad , \
			PAL_STM32_OTYPE_PUSHPULL | PAL_STM32_OSPEED_MID2 \
			| PAL_STM32_ALTERNATE( I2S_OUT_FUNCTION ) )


/* Currently playing? */
static int _snd_playing;
/* May write next buffer? */
static int _snd_next_ok;
/* Last initialised buffer */
static int _snd_init_buffer;
/* Buffers */
static s16 _snd_buffers[ 2 * SND_BUFSIZE ];

/* Compute a buffer's address */
#define _snd_buffer_address(idx) \
	(& _snd_buffers[ (idx) * SND_BUFSIZE ] )


#ifdef USE_DMA_FOR_SEXY_BEEPS
/* SPI transfer callback, used when buffers need to be rotated */
static void _cs43l22_spi_callback( SPIDriver * driver , u32 flags )
{
	if ( driver != &SPID3 || !( flags & STM32_DMA_ISR_TCIF ) ) {
		return;
	}
	extern u32 isrFlags;
	isrFlags = flags;
	_snd_next_ok = TRUE;
}
#endif // USE_DMA_FOR_SEXY_BEEPS



/*
 * Initialise GPIO ports to handle:
 *	- the CS43L22's I2C channel,
 *	- the CS43L22's I2S channel.
 */
static void _i2s_init_gpio( void )
{
	// Reset pin
	palSetPadMode( I2S_ORESET_PORT , I2S_ORESET_PAD ,
			PAL_MODE_OUTPUT_PUSHPULL | PAL_STM32_OSPEED_MID2 );

	// SDL/SDA pins
	_i2s_init_i2c_pad( I2C_OSCL_PORT , I2C_OSCL_PAD );
	_i2s_init_i2c_pad( I2C_OSDA_PORT , I2C_OSDA_PAD );

	// I2S WS/MCK/SCK/SD pins
	_i2s_init_i2s_pad( I2S_OWS_PORT , I2S_OWS_PAD );
	_i2s_init_i2s_pad( I2S_OMCK_PORT , I2S_OMCK_PAD );
	_i2s_init_i2s_pad( I2S_OSCK_PORT , I2S_OSCK_PAD );
	_i2s_init_i2s_pad( I2S_OSD_PORT , I2S_OSD_PAD );
}


/* Reset the CS43L22 */
static void _i2s_reset_output( void )
{
	palClearPad( I2S_ORESET_PORT , I2S_ORESET_PAD );
	chThdSleep( I2S_ORESET_DELAY );
	palSetPad( I2S_ORESET_PORT , I2S_ORESET_PAD );
	chThdSleep( I2S_ORESET_DELAY );
}


/* Send a command to the CS43L22 through I2C */
static msg_t _cs43l22_set( u8 reg , u8 value )
{
	u8 txBuffer[ 2 ];
	txBuffer[0] = reg;
	txBuffer[1] = value;
	msg_t rv = i2cMasterTransmitTimeout(
			& I2S_OI2C_DRIVER , I2S_OI2C_ADDRESS ,
			txBuffer , 2 ,
			NULL , 0 ,
			I2S_OI2C_TIMEOUT );
	if ( rv ) {
		chprintf( (void*)&SD2 ,
				"I2C 0x%0.2x <- 0x%0.2x ERROR %d\r\n" ,
				reg , value , rv );
		chprintf( (void*)&SD2 ,
				"    status = 0x%x\r\n" ,
				i2cGetErrors( & I2S_OI2C_DRIVER ) );
	} else {
		chprintf( (void*)&SD2 ,
				"I2C 0x%0.2x <- 0x%0.2x OK\r\n" ,
				reg , value );
	}
	return rv;
}


/* Get a register from the CS43L22 through I2C */
static u8 _cs43l22_get( u8 reg )
{
	u8 data;
	msg_t rv = i2cMasterTransmitTimeout(
			& I2S_OI2C_DRIVER , I2S_OI2C_ADDRESS ,
			&reg , 1 ,
			&data , 1 ,
			I2S_OI2C_TIMEOUT );
	if ( rv ) {
		chprintf( (void*)&SD2 ,
				"I2C 0x%0.2x >- ??? ERROR %d\r\n" ,
				reg , rv );
		chprintf( (void*)&SD2 ,
				"    status = 0x%x\r\n" ,
				i2cGetErrors( & I2S_OI2C_DRIVER ) );
	}
	return data;
}


/* Initialise the CS43L22 through its I2C interface */
static void _cs43l22_init( void )
{
	i2cStart( &( I2S_OI2C_DRIVER ) , &_i2s_i2c_config );

	// Make sure the device is powered down
	_cs43l22_set( CS43L22_REG_PWR_CTL1 , CS43L22_PWR1_DOWN );
	// Activate headphone channels
	_cs43l22_set( CS43L22_REG_PWR_CTL2 ,
			CS43L22_PWR2_SPKA_OFF | CS43L22_PWR2_SPKB_OFF
			| CS43L22_PWR2_HDA_ON | CS43L22_PWR2_HDB_ON );
	// Set serial clock
	_cs43l22_set( CS43L22_REG_CLOCK_CTL , CS43L22_CLK_AUTO_ON
			| CS43L22_CLK_MCDIV_ON );
	// Set input data format
	_cs43l22_set( CS43L22_REG_INT_CTL1 , CS43L22_IC1_SLAVE
			| CS43L22_IC1_SCPOL_OFF | CS43L22_IC1_DSP_OFF
			| CS43L22_IC1_DIF_I2S | CS43L22_IC1_AWL_32 );
	// Fire it up
	_cs43l22_set( CS43L22_REG_PWR_CTL1 , CS43L22_PWR1_UP );
	// Analog soft ramp/zero cross disabled
	_cs43l22_set( CS43L22_REG_AZCSR ,
			CS43L22_AZCSR_SRB_OFF | CS43L22_AZCSR_SRA_OFF
			| CS43L22_AZCSR_ZCB_OFF | CS43L22_AZCSR_ZCA_OFF );
	// Digital soft ramp disabled
	_cs43l22_set( CS43L22_REG_MISC_CTL , CS43L22_MISC_DEEMPHASIS_ON );
	// Limiter: no soft ramp/zero cross, no attack level
	_cs43l22_set( CS43L22_REG_LIM_CTL1 , CS43L22_LIM1_SRD_OFF
			| CS43L22_LIM1_ZCD_OFF );
	// Initial volume and tone controls
	_cs43l22_set( CS43L22_REG_TONE_CTL , 0xf );
	_cs43l22_set( CS43L22_REG_PCM_A , 0x00 );
	_cs43l22_set( CS43L22_REG_PCM_B , 0x00 );
	sndOutputVolume( 200 );
}


/* Initialise the I2S interface to the CS43L22 */
static void _cs43l22_init_i2s( u32 frequency )
{
	rccEnableSPI3(FALSE);

	u32 plln = ( RCC->PLLI2SCFGR & RCC_PLLI2SCFGR_PLLI2SN ) >> 6;
	u32 pllr = ( RCC->PLLI2SCFGR & RCC_PLLI2SCFGR_PLLI2SR ) >> 28;
	u32 pllm = (u32)( RCC->PLLCFGR & RCC_PLLCFGR_PLLM );
	u32 clock = (u32)( ( ( HSE_VALUE / pllm ) * plln ) / pllr );
	chprintf( (void*)&SD2 , "PLL REG=0x%.x N=%d R=%d M=%d CLK = %d\r\n" ,
			RCC->PLLI2SCFGR , plln , pllr , pllm , clock );

	u16 frq = (u16)(( ( ( clock / 256 ) * 10 ) / frequency ) + 5 ) / 10;
	chprintf( (void*)&SD2 , "frq base = 0x%x\r\n" , frq );
	if ( ( frq & 1 ) == 0 ) {
		frq = frq >> 1;
	} else {
		frq = STM32F4_I2S_PR_ODD | ( frq >> 1 );
	}
	chprintf( (void*)&SD2 , "frq = 0x%x\r\n" , frq );

	*((u32*)(SPI3_BASE + STM32F4_I2S_OFFS_CFG)) =
		  STM32F4_I2S_CFG_MODE_I2S
		| STM32F4_I2S_CFG_STD_I2S
		| STM32F4_I2S_CFG_CFG_MS_TX
		| STM32F4_I2S_CFG_RCPOL_OFF
		| STM32F4_I2S_CFG_DLEN_16
		| STM32F4_I2S_CFG_CLEN_16;
	*((u32*)(SPI3_BASE + STM32F4_I2S_OFFS_PR)) = frq
		| STM32F4_I2S_PR_MCLK_ON;
	*((u32*)(SPI3_BASE + STM32F4_I2S_OFFS_CFG)) |= STM32F4_I2S_CFG_ENABLED;
}

#ifdef USE_DMA_FOR_SEXY_BEEPS
static void _cs43l22_init_dma( void )
{
	dmaStreamAllocate( SPID3.dmatx , 1 ,
			(stm32_dmaisr_t) _cs43l22_spi_callback ,
			&SPID3 );
	SPID3.txdmamode |= ( STM32_DMA_CR_PSIZE_HWORD | 0x30000
			| STM32_DMA_CR_MINC | STM32_DMA_CR_MSIZE_HWORD
			| STM32_DMA_CR_DBM | STM32_DMA_CR_TCIE );
	dmaStreamSetMode( SPID3.dmatx , SPID3.txdmamode );
	dmaStreamSetFIFO( SPID3.dmatx , STM32_DMA_FCR_DMDIS
			| STM32_DMA_FCR_FTH_HALF );
	dmaStreamSetPeripheral( SPID3.dmatx , &( SPID3.spi->DR ) );
	SPID3.spi->CR2 |= SPI_CR2_TXDMAEN;
}
#endif // USE_DMA_FOR_SEXY_BEEPS


void sndInit( u32 frequency )
{
	_i2s_init_gpio( );
	_i2s_reset_output( );
	_cs43l22_init( );
	_cs43l22_init_i2s( frequency );
#ifdef USE_DMA_FOR_SEXY_BEEPS
	_cs43l22_init_dma( );
#endif // USE_DMA_FOR_SEXY_BEEPS
	_snd_next_ok = TRUE;
}


void sndOutputVolume( u8 volume )
{
	if ( volume > 0xe6 ) {
		volume -= 0xe7;
	} else {
		volume += 0x19;
	}
	_cs43l22_set( CS43L22_REG_MASTER_VOLUME_A , volume );
	_cs43l22_set( CS43L22_REG_MASTER_VOLUME_B , volume );
}

s16 * sndGetBuffer( void )
{
	if ( ! _snd_next_ok ) {
		return NULL;
	}

	int next_write;
	if ( _snd_playing ) {
		next_write = ( SPID3.dmatx->stream->CR & 0x80000 )
			? 0 : 1;
		_snd_next_ok = FALSE;
	} else if ( _snd_init_buffer == 0 ) {
		next_write = _snd_init_buffer++;
	} else {
		next_write = _snd_init_buffer++;
		_snd_playing = TRUE;
		_snd_next_ok = FALSE;
		dmaStreamSetMemory0( SPID3.dmatx , _snd_buffer_address(0) );
		dmaStreamSetMemory1( SPID3.dmatx , _snd_buffer_address(1) );
		dmaStreamSetTransactionSize( SPID3.dmatx , SND_BUFSIZE );
		dmaStreamEnable( SPID3.dmatx );
	}
	return _snd_buffer_address( next_write );
}

u8 sndGetStatus( void )
{
	return _cs43l22_get( CS43L22_REG_STATUS );
}
