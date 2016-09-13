/* 
 * File 	: nRF905.c
 * Purpose	: Implement nRF905 as a receiver mode. Here nRF905 connected with nRF51 DK.
 * 		  Read nRF905 datasheet from http://infocenter.nordicsemi.com/pdf/nRF905_PS_v1.5.pdf
 *
 * Additional information: 
 *		"Pin connections"
		nRF905					nRF51DK(GPIO Pin mapping)
		CE						P0.12
		AM						P0.13
		MISO						P0.28
		MOSI						P0.25
		CSN						P0.24
		SCK						P0.29
		TXEN						P0.14
		PWR						P0.15
		CD						P0.16
		DR						P0.17
 */

#include "nrf_drv_spi.h"
#include "nrf_gpio.h"
#include "nrf_delay.h"
#include "app_error.h"
#include "string.h"
#include "nRF905.h"

/**
 * @brief nRF905 pins configure
 * @param none
 */
void pin_configure(void)
{
	nrf_gpio_cfg_output(CSN);
	nrf_gpio_pin_set(CSN);
	
	nrf_gpio_cfg_output(PWR);
	nrf_gpio_pin_set(PWR);
	
	nrf_gpio_cfg_output(TRX_CE);
	nrf_gpio_pin_clear(TRX_CE);
	
	nrf_gpio_cfg_output(TXEN);
	nrf_gpio_pin_clear(TXEN);
	
	nrf_gpio_cfg_input(DR, NRF_GPIO_PIN_PULLDOWN);
	
	nrf_gpio_cfg_input(AM, NRF_GPIO_PIN_PULLDOWN);
	
	nrf_gpio_cfg_input(CD, NRF_GPIO_PIN_PULLDOWN);
	
	nrf_delay_ms(1000);
}

/**
 * @brief Write_Config : send write configuration (WC) command and data.
 * @param none
 */
void Write_Config(void)
{	
	uint32_t err_code;
	
	unsigned char config_data[11]={
	0x00,
  	0x6A,                   //CH_NO,433MHZ
	0x0c,                   //output power 10db, resend disable, Current Normal operation
	0x44,                   //4-byte address
    	0x03,0x03,              //receive or send data length 32 bytes
	0xcc,0xcc,0xcc,0xcc,    //receiving address
    	0x58,									  //CRC enable,8bit CRC,external clock disable,16MHZ Oscillator               
	};
		
	/* Spi enable for write a spi command */
	nrf_gpio_pin_clear(CSN);

	err_code = nrf_drv_spi_transfer(&spi, config_data, 11, NULL, 0);
	APP_ERROR_CHECK(err_code);
		
	nrf_gpio_pin_set(CSN);					/* Disable Spi */
}


/**
 * @brief read_config : send read configuration (RC) command.
 * @param none
 */
void read_config(void)
{	
	unsigned char txCmd[1];
	unsigned char data[11];
	
	nrf_gpio_pin_clear(CSN);
	
	txCmd[0] = RC;
		
	/* send read configuration command */
	uint32_t err_code = nrf_drv_spi_transfer(&spi, txCmd, 1, data, 11);
    	APP_ERROR_CHECK(err_code);
	
	nrf_gpio_pin_set(CSN);					/* Disable Spi */
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
	nrf_delay_us(800);
}

/**
 * @brief check_ready : check valid packet is received (DR = 1 indicate valid packet has been received)
 * @param none
 */
unsigned char check_ready(void)
{	
	if(nrf_gpio_pin_read(DR) && nrf_gpio_pin_read(TRX_CE) && !(nrf_gpio_pin_read(TXEN)))	{
		return 1;
	}  
	else {
		return 0;
	}
}

/**
 * @brief RxPacket : receive payload data
 * @param none
 */
void RxPacket(void)
{	
	unsigned char rxBuffer[3] = "";
	unsigned char txBuf[1];

	nrf_delay_ms(100);
	
	nrf_gpio_pin_clear(TRX_CE);
	nrf_gpio_pin_clear(CSN);
	nrf_delay_ms(1);
	
	txBuf[0] = RRP;
		
	uint32_t err_code = nrf_drv_spi_transfer(&spi, txBuf, 1, rxBuffer, 3);
	APP_ERROR_CHECK(err_code);
	
	nrf_gpio_pin_set(CSN);
	nrf_delay_ms(10);
	nrf_gpio_pin_set(TRX_CE);
}

/**
 * @param none
 */
void nRF905_rxData(void)
{
    /* Set nRF905 in Rx mode */
    set_rx();
    while (check_ready()==0);
    nrf_delay_ms(10);
    RxPacket();
    nrf_delay_ms(10);
}

/**
 * @brief TxPacket : transmit TX address and payload data
 * @param none
 */
void TxPacket(void)
{
	uint32_t err_code;
	
	/* transmiting address */
	unsigned char txAddr[5]={
		0x22,0xcc,0xcc,0xcc,0xcc,
	};
	/* transmit Payload data */
	unsigned char txBuf[4];
	txBuf[0] = 0x20;
	txBuf[1] = 'R';
	txBuf[2] = 'F';
	
	nrf_gpio_pin_clear(CSN);
	
	/* Write payload data	*/
	err_code = nrf_drv_spi_transfer(&spi, txBuf, 4, NULL, 0);
	APP_ERROR_CHECK(err_code);
	
	nrf_gpio_pin_set(CSN);
	nrf_delay_ms(10);
	
  	/* Spi enable for write a spi command */
	nrf_gpio_pin_clear(CSN);
	
	/* Write 4 bytes address */
	err_code = nrf_drv_spi_transfer(&spi, txAddr, 5, NULL, 0);
	APP_ERROR_CHECK(err_code);
	
	/* Spi disable */
	nrf_gpio_pin_set(CSN);
		
	/* Set TRX_CE high,start Tx data transmission, CE pulse */
	nrf_gpio_pin_set(TRX_CE);
	nrf_delay_ms(10);
	
	nrf_gpio_pin_clear(TRX_CE);
}

/**
 * @brief set_tx : set_rx : TXEN = HIGH and TRX_CE = LOW Set nRF905 in transmit mode
 * @param none
 */
void set_tx(void)
{
	nrf_gpio_pin_set(TXEN);
	nrf_gpio_pin_clear(TRX_CE);
	
	/* delay for mode change(>=650us) */
	nrf_delay_us(800);
}

/**
 * @param none
 */
void nRF905_txData(void)
{
	/* Set nRF905 in Tx mode */
	set_tx();
	nrf_delay_ms(10);
	
	TxPacket();
	nrf_delay_ms(10);
}
