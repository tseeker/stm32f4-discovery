#ifndef _H_XPT2046_STM32F4
#define _H_XPT2046_STM32F4


// Ports and controllers for the various touchscreen pins
//	IRQ	(any port will do)
#define XPT2046_IRQ_PORT	GPIOC
#define XPT2046_IRQ_PAD		4
//	NSS	(any port will do, this is done manually)
#define XPT2046_NSS_PORT	GPIOA
#define XPT2046_NSS_PAD		4
//	CLK	(must be a SPI clock port)
#define XPT2046_CLK_PORT	GPIOA
#define XPT2046_CLK_PAD		5
//	DIN	(must be a SPI MOSI port)
#define XPT2046_DIN_PORT	GPIOA
#define XPT2046_DIN_PAD		7
//	DOUT	(must be a SPI MISO port)
#define XPT2046_DOUT_PORT	GPIOA
#define XPT2046_DOUT_PAD	6

// SPI driver to use
#define XPT2046_SPI_DRIVER	SPID1


void xpt2046Init( void );
int xpt2046GetCoordinates( int * pX , int * pY );
int xpt2046GetAverageCoordinates( int * pX , int * pY , int nSamples );


#endif //_H_XPT2046_STM32F4
