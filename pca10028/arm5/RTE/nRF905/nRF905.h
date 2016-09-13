/* 
 * File 		: nRF905.h
 */
#ifndef NRF905_H
#define NRF905_H

#define SPI_INSTANCE  0 /**< SPI instance index. */
static const nrf_drv_spi_t spi = NRF_DRV_SPI_INSTANCE(SPI_INSTANCE);  /**< SPI instance. */

/* nrf905 instruction set */
#define WC								0x00
#define RC								0x10
#define WTP								0x20
#define RTP								0x21
#define WTA								0x22
#define RTA								0x23
#define RRP								0x24

/* GPIO pin mapping */
#define TXEN							14
#define TRX_CE							12
#define PWR								15
#define CSN								24
#define AM								13
#define DR								17
#define CD								16
#define SPI_SCK_PIN       	            29     /**< SPI clock GPIO pin number. */
#define SPI_MOSI_PIN      	            25     /**< SPI Master Out Slave In GPIO pin number. */
#define SPI_MISO_PIN      	            28     /**< SPI Master In Slave Out GPIO pin number. */

extern void pin_configure(void);
extern void nRF905_txData(void);
extern void Write_Config(void);
extern void read_config(void);
extern void nRF905_rxData(void);

#endif	//end NRF905_H
