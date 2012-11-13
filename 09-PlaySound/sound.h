#ifndef __H_SOUND
#define __H_SOUND

#define SND_BUFSIZE	2304
//#define USE_DMA_FOR_SEXY_BEEPS

/* Initialise the driver at a specific audio sampling rate */
void sndInit( u32 frequency );

/* Set the driver's output volume */
void sndOutputVolume( u8 volume );

/* Get the next writeable buffer; returns NULL if all buffers are full */
s16 * sndGetBuffer( void );

/* Get the sound chip's status register */
u8 sndGetStatus( void );

#endif //__H_SOUND
