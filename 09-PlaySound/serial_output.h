#ifndef __H_SERIALOUT
#define __H_SERIALOUT

#define SERIALOUT_DRIVER	SD2
#define SERIALOUT_TX_PORT	GPIOA
#define SERIALOUT_TX_PAD	2
#define SERIALOUT_RX_PORT	GPIOA
#define SERIALOUT_RX_PAD	3
#define SERIALOUT_AF		7


void soInit( void );


#endif // __H_SERIALOUT
