#ifndef __H_STM32F4_I2S
#define __H_STM32F4_I2S

/* I2S configuration */
#define STM32F4_I2S_OFFS_CFG 0x0000001c
/* I2S configuration - SPI mode */
#define STM32F4_I2S_CFG_MODE_SPI ((uint16_t)0x0000)
/* I2S configuration - I2S mode */
#define STM32F4_I2S_CFG_MODE_I2S ((uint16_t)0x0800)
/* I2S configuration - Peripheral enabled */
#define STM32F4_I2S_CFG_ENABLED ((uint16_t)0x0400)
/* I2S configuration - Peripheral disabled */
#define STM32F4_I2S_CFG_DISABLED ((uint16_t)0x0000)
/* I2S configuration - I2S configuration mode - Master/transmit */
#define STM32F4_I2S_CFG_CFG_MS_TX ((uint16_t)0x0200)
/* I2S configuration - I2S configuration mode - Slave/transmit */
#define STM32F4_I2S_CFG_CFG_SL_TX ((uint16_t)0x0000)
/* I2S configuration - I2S configuration mode - Slave/receive */
#define STM32F4_I2S_CFG_CFG_SL_RX ((uint16_t)0x0100)
/* I2S configuration - I2S configuration mode - Master/receive */
#define STM32F4_I2S_CFG_CFG_MS_RX ((uint16_t)0x0300)
/* I2S configuration - PCM frame sync - Long frame */
#define STM32F4_I2S_CFG_PCMSYNC_LONG ((uint16_t)0x0080)
/* I2S configuration - PCM frame sync - Short frame */
#define STM32F4_I2S_CFG_PCMSYNC_SHORT ((uint16_t)0x0000)
/* I2S configuration - I2S standard - Right justified */
#define STM32F4_I2S_CFG_STD_LSB ((uint16_t)0x0020)
/* I2S configuration - I2S standard - I2S Philips standard */
#define STM32F4_I2S_CFG_STD_I2S ((uint16_t)0x0000)
/* I2S configuration - I2S standard - Left justified */
#define STM32F4_I2S_CFG_STD_MSB ((uint16_t)0x0010)
/* I2S configuration - I2S standard - PCM standard */
#define STM32F4_I2S_CFG_STD_PCM ((uint16_t)0x0030)
/* I2S configuration - Reverse clock polarity - ON */
#define STM32F4_I2S_CFG_RCPOL_ON ((uint16_t)0x0008)
/* I2S configuration - Reverse clock polarity - OFF */
#define STM32F4_I2S_CFG_RCPOL_OFF ((uint16_t)0x0000)
/* I2S configuration - Data length - 32-bit */
#define STM32F4_I2S_CFG_DLEN_32 ((uint16_t)0x0004)
/* I2S configuration - Data length - 16-bit */
#define STM32F4_I2S_CFG_DLEN_16 ((uint16_t)0x0000)
/* I2S configuration - Data length - 24-bit */
#define STM32F4_I2S_CFG_DLEN_24 ((uint16_t)0x0002)
/* I2S configuration - Channel length - 32-bit */
#define STM32F4_I2S_CFG_CLEN_32 ((uint16_t)0x0001)
/* I2S configuration - Channel length - 16-bit */
#define STM32F4_I2S_CFG_CLEN_16 ((uint16_t)0x0000)

/* I2S prescaler */
#define STM32F4_I2S_OFFS_PR 0x00000020
/* I2S prescaler - Master clock output - ON */
#define STM32F4_I2S_PR_MCLK_ON ((uint16_t)0x0200)
/* I2S prescaler - Master clock output - OFF */
#define STM32F4_I2S_PR_MCLK_OFF ((uint16_t)0x0000)
/* I2S prescaler - Odd factor - Divider is I2SDIV * 2 */
#define STM32F4_I2S_PR_EVEN ((uint16_t)0x0000)
/* I2S prescaler - Odd factor - Divider is 1 + I2SDIV * 2 */
#define STM32F4_I2S_PR_ODD ((uint16_t)0x0100)
/* I2S prescaler - Divider - Mask */
#define STM32F4_I2S_PR_DIV_MASK ((uint16_t)0x00ff)

#endif //__H_STM32F4_I2S
