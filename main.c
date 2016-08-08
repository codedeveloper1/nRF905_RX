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
#include "boards.h"
#include "app_error.h"
#include "nRF905.h"


int main(void)
{
		//nRF905 pin configure
		pin_configure();
	
    nrf_drv_spi_config_t spi_config = NRF_DRV_SPI_DEFAULT_CONFIG(SPI_INSTANCE);
		spi_config.sck_pin = SPI_SCK_PIN;
		spi_config.miso_pin = SPI_MISO_PIN;
		spi_config.mosi_pin = SPI_MOSI_PIN;
    spi_config.ss_pin = SPI_SS_PIN;
    APP_ERROR_CHECK(nrf_drv_spi_init(&spi, &spi_config, spi_event_handler));

		Write_Config(US);
		
		/***********************************************************
		read register configuration, check register value written */
		read_config();
			 
    while(1)
    {
			nRF905_rxData();

      LEDS_INVERT(BSP_LED_0_MASK);
      nrf_delay_ms(200);
    }
}
