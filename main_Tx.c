/* Copyright (c) 2015 Nordic Semiconductor. All Rights Reserved.
 *
 * The information contained herein is property of Nordic Semiconductor ASA.
 * Terms and conditions of usage are described in detail in NORDIC
 * SEMICONDUCTOR STANDARD SOFTWARE LICENSE AGREEMENT.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRANTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 *
 */

#include <stdio.h>
#include "nrf_drv_spi.h"
#include "app_util_platform.h"
#include "nrf_gpio.h"
#include "nrf_delay.h"
#include "app_uart.h"
#include "boards.h"
#include "app_error.h"
#include "nRF905.h"

#define SPI0_ENABLED 1
#if (SPI0_ENABLED == 1)
#define SPI0_USE_EASY_DMA 1
#endif

void Spi_init(void)
{
	  nrf_drv_spi_config_t spi_config = NRF_DRV_SPI_DEFAULT_CONFIG(SPI_INSTANCE);
		spi_config.sck_pin = SPI_SCK_PIN;
		spi_config.miso_pin = SPI_MISO_PIN;
		spi_config.mosi_pin = SPI_MOSI_PIN;
    spi_config.ss_pin = CSN;
		spi_config.mode = NRF_DRV_SPI_MODE_0;
		spi_config.bit_order = NRF_DRV_SPI_BIT_ORDER_MSB_FIRST;
		spi_config.frequency = NRF_DRV_SPI_FREQ_4M;
    APP_ERROR_CHECK(nrf_drv_spi_init(&spi, &spi_config, NULL));
}

int main(void)
{
		Spi_init();

		//nRF905 pin configure
		pin_configure();
	
		Write_Config();
		
		/***********************************************************
		read register configuration, check register value written */
		read_config();
	
    while(1)
    {
			nRF905_txData();
			
      nrf_delay_ms(200);
    }
}
