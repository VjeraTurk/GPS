#include "app_config.h"

#define TC_CR_Y			0x90 //DFR = 0, PD = 00
#define TC_CR_X			0xD0 //DFR = 0, PD = 00

volatile unsigned short tc_x,tc_y;

unsigned short TCGetX(void) //call this after TCRead() to get X value
{
	return (tc_x);
}

unsigned short TCGetY(void) //call this after TCRead() to get Y value
{
	return (tc_y);
}

unsigned char TCIsPenOn(void) //TC_PEN_PIN is 0 when the screen is pressed
{	
	if (!Rdb(PINB, PB2))
		return (1); 
	return (0);
}

unsigned char spi_buf[8];

void TCInit(void)
{
	Setb(PORTB, PB4);
	TCRead(); //Wake it up
}

void TCRead(void) //read analog voltage
{
	spi_buf[0] = TC_CR_Y; //read Y first
	spi_buf[1] = 0; //2nd byte is not used
	spi_buf[2] = TC_CR_X; //then read X
	spi_buf[3] = 0; //3rd byte is not used
	spi_buf[4] = 0; //4th byte is not used

	SPIMasterEnable(SPI_RIS_FIRST,SPI_SAM_FIRST,SPI_CLK_DIV64,SPI_MSB_FIRST,SPI_INT_DISABLE);
																			 
	Orb(DDRB, PB4);	//set TC_CS to be output port
	Clrb(PORTB, PB4); //clear TC_CS to 0

	SPIMasterTransfer(5,spi_buf); //transfer data for 5 cycles

	Setb(PORTB, PB4); //set TC_CS to 1

	SPIMasterDisable(); //disable interrupt

	tc_y = (spi_buf[1] << 5); //arrange received data
	tc_y |= (spi_buf[2] >> 3); //arrange received data
	tc_x = (spi_buf[3] << 5);  //arrange received data
	tc_y |= (spi_buf[4] >> 3); //arrange received data
}
