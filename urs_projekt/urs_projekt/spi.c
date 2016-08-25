#include "app_config.h"

typedef enum
{
	SPI_IDLE,
	SPI_BUSY
} spi_status_t;

spi_status_t volatile spi_status = SPI_IDLE;
int spi_cnt;
int spi_ri;
int spi_wi;

void SPIMasterEnable(uint8_t clk_pol,uint8_t clk_phase,uint8_t clk_div,uint8_t order,uint8_t int_enable)
{
	uint8_t spcr_buf = SPI_MASTER;
	spcr_buf |= clk_pol | clk_phase | clk_div | order | int_enable;
	SPCR = spcr_buf;

	Orb(DDRB, PB5);	//MOSI = output
	Orb(DDRB, PB7);		//SCK = output
	SPCR |= SPI_ENABLE;
}

void SPIMasterDisable()
{
	SPCR &= ~SPI_ENABLE;
}

void SPIMasterTransfer(int len, unsigned char *buf)
{
	spi_status = SPI_BUSY;
	spi_cnt = len;
	spi_wi = 0;
	spi_ri = 0;

	while (spi_wi != spi_cnt)
	{
		SPDR = buf[spi_ri];
		spi_ri++;

		while (!(SPSR & 0x80)); //wait until SPIF is set

		buf[spi_wi] = SPDR;
		spi_wi++;
}
	spi_status = SPI_IDLE;
}

uint8_t SPIIsBusy()
{
	if (spi_status == SPI_IDLE) //spi_status MUST be volatile variable!!!
		return (0);
	else
		return (1);
}


