#ifndef __H_PINS_CS43L22
#define __H_PINS_CS43L22

// Alternate function used for I2S to the CS43L22
#define I2S_OUT_FUNCTION	6

// CS43L22 reset pin and delay
#define I2S_ORESET_PORT		GPIOD
#define I2S_ORESET_PAD		4
#define I2S_ORESET_DELAY	10

// CS43L22 SCL pin
#define I2C_OSCL_PORT		GPIOB
#define I2C_OSCL_PAD		6

// CS43L22 SDA pin
#define I2C_OSDA_PORT		GPIOB
#define I2C_OSDA_PAD		9

// CS43L22 WS pin
#define I2S_OWS_PORT		GPIOA
#define I2S_OWS_PAD		4

// CS43L22 MCK pin
#define I2S_OMCK_PORT		GPIOC
#define I2S_OMCK_PAD		7

// CS43L22 SCK pin
#define I2S_OSCK_PORT		GPIOC
#define I2S_OSCK_PAD		10

// CS43L22 SD pin
#define I2S_OSD_PORT		GPIOC
#define I2S_OSD_PAD		12

// Output control I2C parameters
#define I2S_OI2C_DRIVER		I2CD1
#define I2S_OI2C_SPEED		400000
#define I2S_OI2C_ADDRESS	0x4a
#define I2S_OI2C_TIMEOUT	1000

// Base address of the SPI port
#define I2S_SPI_BASE		SPI3_BASE



#endif //__H_PINS_CS43L22
