Read coordinates from an XPT2046 touchscreen and displays the coordinates by
flashing the blue and orange LEDs (MSB to LSB, red LED serves as a clock).

Sounds simple enough. Yeah, right.

Mistakes made (in addition to misreading docs):
	- Trying to sample in 8-bit mode. This is damned useless.
	- Trying to use the BUSY signal. Pointless, better left alone.
	- Not oversampling.

Connections to use: see xpt2046.h
Requires an SPI driver with SPI_USE_WAIT; SPI_USE_MUTUAL_EXCLUSION supported.

Program includes some idiot-proofing to prevent being unable to flash the board
(press user button, press reset button, release reset button, red LED lights up
to indicate idiot-proof mode).
