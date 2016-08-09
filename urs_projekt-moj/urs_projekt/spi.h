#ifndef _SPI_H_
#define _SPI_H_

#define SPI_ENABLE			0x40

#define SPI_INT_ENABLE		0x80
#define SPI_INT_DISABLE		0x00

#define SPI_LSB_FIRST		0x20
#define SPI_MSB_FIRST		0x00

#define SPI_MASTER			0x10
#define SPI_SLAVE			0x00

#define SPI_FAL_FIRST		0x08
#define SPI_RIS_FIRST		0x00

#define SPI_SET_FIRST		0x04
#define SPI_SAM_FIRST		0x00

#define SPI_CLK_DIV4		0x00
#define SPI_CLK_DIV16		0x01
#define SPI_CLK_DIV64		0x02
#define SPI_CLK_DIV128		0x03

void SPIMasterEnable(uint8_t clk_pol,uint8_t clk_phase,uint8_t clk_div,uint8_t order,uint8_t int_enable);
void SPIMasterDisable(void);
void SPIMasterTransfer(int len, unsigned char *buf);
uint8_t SPIIsBusy(void);

#endif
