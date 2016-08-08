/* 
 * File 		: nRF905.c
 * Purpose	: Implement nRF905 as a receiver mode. Here nRF905 connected with nRF51 DK.
 * 						Read nRF905 datasheet from http://infocenter.nordicsemi.com/pdf/nRF905_PS_v1.5.pdf
 *
 * Additional information: 
 *		"Pin connections"
					nRF905					nRF51DK(GPIO Pin mapping)
					CE							P0.12
					AM							P0.13
					MISO						P0.28
					MOSI						P0.25
					CSN							P0.24
					SCK							P0.29
					TXEN						P0.14
					PWR							P0.15
					CD							P0.16
					DR							P0.17
 */
 
#include "nrf_drv_spi.h"
#include "nrf_gpio.h"
#include "nrf_delay.h"
#include "app_error.h"
#include "NRF_LOG.h"
#include "nRF905.h"


unsigned char config_info_buf[10]={
    0x76,                   //CH_NO,868.4MHZ
    0x0E,                   //output power 10db, resend disable, Current Normal operation
    0x44,                   //4-byte address
    0x20,0x20,              //receive or send data length 32 bytes
    0xCC,0xCC,0xCC,0xCC,    //receiving address
    0x58,                   //CRC enable,8bit CRC,external clock disable,16MHZ Oscillator
};

unsigned int freq_tab[10] = {
    0x13e,
    0x076,
    0x076,
    0x076,
    0x177,
    0x02b,
    0x17f,
    0x17f,
    0x17f,
    0x100,
};

#define BUF_LEN     								32

unsigned char rx_buf[BUF_LEN]= {0};

static uint8_t       m_tx_buf[50] = "";           		/**< TX buffer. */
static uint8_t       m_rx_buf[sizeof(m_tx_buf)+1];    /**< RX buffer. */
static const uint8_t m_length = sizeof(m_tx_buf);     /**< Transfer length. */


/**
 * @brief nRF905 pins configure
 * @param none
 */
void pin_configure(void)
{
		nrf_gpio_cfg_output(CSN);
		nrf_gpio_pin_set(CSN);
	
		nrf_gpio_cfg_input(DR, NRF_GPIO_PIN_NOPULL);
	
		nrf_gpio_cfg_input(AM, NRF_GPIO_PIN_NOPULL);
	
		nrf_gpio_cfg_input(CD, NRF_GPIO_PIN_NOPULL);
	
		nrf_gpio_cfg_output(PWR);
		nrf_gpio_pin_set(PWR);
	
		nrf_gpio_cfg_output(TRX_CE);
		nrf_gpio_pin_clear(TRX_CE);
	
		nrf_gpio_cfg_output(TXEN);
		nrf_gpio_pin_clear(TXEN);
		
		nrf_delay_ms(2000);
}

/**
* @brief Write_Config : send write configuration (WC) command.
 * @param frequency band
 */
void Write_Config(unsigned char freq_band)
{
		if(freq_band < 10){
			config_info_buf[0] = (unsigned char)freq_tab[freq_band];
			if(freq_tab[freq_band]&0x100){
				config_info_buf[1] |= 0x01;
			}
			else{
				config_info_buf[1] &= ~0x01;
			}
		}
	
		// Spi enable for write a spi command
		nrf_gpio_pin_clear(CSN);
		/** send write configuration command */
		m_tx_buf[0] = WC;
		uint32_t err_code = nrf_drv_spi_transfer(&spi, m_tx_buf, m_length, m_rx_buf, m_length);
		APP_ERROR_CHECK(err_code);	
		
		err_code = nrf_drv_spi_transfer(&spi, config_info_buf, m_length, m_rx_buf, m_length);
		APP_ERROR_CHECK(err_code);
		
		nrf_gpio_pin_set(CSN);					// Disable Spi
}

/**
 * @brief SPI user event handler.
 * @param event
 */
void spi_event_handler(nrf_drv_spi_evt_t const * p_event)
{
    NRF_LOG_PRINTF(" Transfer completed.\r\n");
    if (rx_buf[0] != 0)
    {
			NRF_LOG_PRINTF("Data received\r\n");
    }
}

/**
 * @brief read_config : send read configuration (RC) command.
 * @param none
 */
void read_config(void)
{		
		nrf_gpio_pin_clear(CSN);
		m_tx_buf[0] = RC;
		
		/** send read configuration command */
		uint32_t err_code = nrf_drv_spi_transfer(&spi, m_tx_buf, m_length, m_rx_buf, m_length);
    APP_ERROR_CHECK(err_code);
	
		nrf_gpio_pin_set(CSN);					// Disable Spi
}

/**
 * @brief set_rx : TXEN = LOW and TRX_CE = HIGH Set nRF905 in receive mode
 * @param none
 */
void set_rx(void)
{
		nrf_gpio_pin_clear(TXEN);
		nrf_gpio_pin_set(TRX_CE);
	
		/* delay for mode change(>=650us) */
		nrf_delay_ms(1);
}

/**
 * @brief check_ready : check valid packet is received (DR = 1 indicate valid packet has been received)
 * @param none
 */
unsigned char check_ready(void)
{
		// DR set to high When a valid packet has been received
    if(nrf_gpio_pin_read(DR) == HIGH)	{
			return 1;
		}
		else{
			return 0;
		}
}

void RxPacket(unsigned char *TxRxBuffer)
{	
		nrf_gpio_pin_clear(TRX_CE);
		nrf_gpio_pin_clear(CSN);
		nrf_delay_ms(1000);
		
		m_tx_buf[0] = RRP;
		
		uint32_t err_code = nrf_drv_spi_transfer(&spi, m_tx_buf, m_length, TxRxBuffer, BUF_LEN);
		APP_ERROR_CHECK(err_code);
		nrf_delay_ms(1000);
		
		nrf_gpio_pin_set(CSN);
		nrf_delay_ms(1000);
		nrf_gpio_pin_set(TRX_CE);
		nrf_delay_ms(1000);	
}

void RX(unsigned char *TxRxBuffer)
{
    set_rx();			// Set nRF905 in Rx mode
    while (check_ready()==0);
    nrf_delay_ms(1000);
    RxPacket(TxRxBuffer);
    nrf_delay_ms(1000);
}

/**
 * @brief nRF905_rxData : nRF905 is continous receive data packet with default RX address 
 * @param none
 */
void nRF905_rxData(void)
{
		RX(rx_buf);
		
		nrf_delay_ms(1000);
}
