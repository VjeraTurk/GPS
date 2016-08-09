/************************************************************************************************** 
*	  			  SSD1289 color Graphical LCD Display Driver 
* File name		: SSD1289x16.h 
* Programmer 	: jaruwit supa 
* Web presence  : www.thaibestlink.com 
* Note			: SSD1289 16 bit interface 
* Language		: avrGCC 
* Hardware		: atmega16 
* Date			: 01/05/2009 
************************************************************************************************/ 
 
 /**
 @defgroup SSD1289
 @code #include <SSD1289x16.h> @endcode
 
 @brief SSD1289 color Graphical LCD Display Driver 
       
 @authors Jaruwit Supa  

*/
 
 /*@{*/
 
#ifndef __SSD1289X16_H__ 
#define __SSD1289X16_H__ 
 
/* _____HARDWARE DEFINES_____________________________________________________ */ 
 
// LCM BL-TFT320420-32/28 
#define LCD_LO_DDR  DDRA
#define LCD_LO_PORT PORTA
#define LCD_LO_PIN  PINA

#define LCD_HI_DDR  DDRC
#define LCD_HI_PORT PORTC
#define LCD_HI_PIN  PINC

#define LCD_CS_DDR  DDRD
#define LCD_CS_PORT PORTD
#define LCD_CS_PIN  PIND
#define LCD_CS_BIT  3

#define LCD_RS_DDR  DDRD
#define LCD_RS_PORT PORTD
#define LCD_RS_PIN  PIND
#define LCD_RS_BIT  2

#define LCD_WR_DDR  DDRD
#define LCD_WR_PORT PORTD
#define LCD_WR_PIN  PIND
#define LCD_WR_BIT  5

#define LCD_RD_DDR  DDRD
#define LCD_RD_PORT PORTD
#define LCD_RD_PIN  PIND
#define LCD_RD_BIT  4

#define LCD_RST_DDR  DDRD
#define LCD_RST_PORT PORTD
#define LCD_RST_PIN  PIND
#define LCD_RST_BIT  7

/* 
// atmega 64 GSM topup 
#define LCD_LO_DDR  DDRA 
#define LCD_LO_PORT PORTA 
#define LCD_LO_PIN  PINA 
 
#define LCD_HI_DDR  DDRC 
#define LCD_HI_PORT PORTC 
#define LCD_HI_PIN  PINC 
 
#define LCD_CS_DDR  DDRG 
#define LCD_CS_PORT PORTG 
#define LCD_CS_PIN  PING 
#define LCD_CS_BIT  3 
 
#define LCD_RS_DDR  DDRD 
#define LCD_RS_PORT PORTD 
#define LCD_RS_PIN  PIND 
#define LCD_RS_BIT  4 
 
#define LCD_WR_DDR  DDRG 
#define LCD_WR_PORT PORTG 
#define LCD_WR_PIN  PING 
#define LCD_WR_BIT  0 
 
#define LCD_RD_DDR  DDRG 
#define LCD_RD_PORT PORTG 
#define LCD_RD_PIN  PING 
#define LCD_RD_BIT  1 
 
#define LCD_RST_DDR  DDRG 
#define LCD_RST_PORT PORTG 
#define LCD_RST_PIN  PING 
#define LCD_RST_BIT  2 
*/

#define LCD_INPUT()		{LCD_LO_DDR = 0x00; LCD_HI_DDR = 0x00;} 
#define LCD_OUTPUT()	{LCD_LO_DDR = 0xFF; LCD_HI_DDR = 0xFF;} 
 
#define LCD_SET_DBH		LCD_HI_PORT 
#define LCD_SET_DBL		LCD_LO_PORT 
 
#define LCD_GET_DBH()	LCD_HI_PIN 
#define LCD_GET_DBL()	LCD_LO_PIN 
 
#define LCD_SET_CS()  LCD_CS_PORT  |=   1<<LCD_CS_BIT 
#define LCD_CLR_CS()  LCD_CS_PORT  &= ~(1<<LCD_CS_BIT) 
#define LCD_DIR_CS(x) LCD_CS_DDR   |=   1<<LCD_CS_BIT 
 
#define LCD_SET_RS()  LCD_RS_PORT  |=   1<<LCD_RS_BIT 
#define LCD_CLR_RS()  LCD_RS_PORT  &= ~(1<<LCD_RS_BIT) 
#define LCD_DIR_RS(x) LCD_RS_DDR   |=   1<<LCD_RS_BIT 
 
#define LCD_SET_WR()  LCD_WR_PORT  |=   1<<LCD_WR_BIT 
#define LCD_CLR_WR()  LCD_WR_PORT  &= ~(1<<LCD_WR_BIT) 
#define LCD_DIR_WR(x) LCD_WR_DDR   |=   1<<LCD_WR_BIT 
 
#define LCD_SET_RD()  LCD_RD_PORT  |=   1<<LCD_RD_BIT 
#define LCD_CLR_RD()  LCD_RD_PORT  &= ~(1<<LCD_RD_BIT) 
#define LCD_DIR_RD(x) LCD_RD_DDR   |=   1<<LCD_RD_BIT 
 
#define LCD_SET_RST()  LCD_RST_PORT |=  1<<LCD_RST_BIT 
#define LCD_CLR_RST()  LCD_RST_PORT &= ~(1<<LCD_RST_BIT) 
#define LCD_DIR_RST(x) LCD_RST_DDR  |=  1<<LCD_RST_BIT 
 
/* _____PUBLIC DEFINE_____________________________________________________ */ 
#define Horizontal 
 
#ifdef Horizontal 
// Horizontal and vertical screen size 
#define SCREEN_HOR_SIZE    240UL 
#define SCREEN_VER_SIZE    320UL 
 
#else 
// Horizontal and vertical screen size 
#define SCREEN_HOR_SIZE    320UL 
#define SCREEN_VER_SIZE    240UL 
 
#endif 
 
// color 
#define BLACK                       RGB(0x00, 0x00, 0x00) 
#define WHITE                       RGB(0xFF, 0xFF, 0xFF) 
#define RED                         RGB(0xFF, 0x00, 0x00) 
#define GREEN                       RGB(0x00, 0xFF, 0x00) 
#define BLUE                        RGB(0x00, 0x00, 0xFF) 
#define YELLOW                      RGB(0xFF, 0xFF, 0x00) 
#define MAGENTA                     RGB(0xFF, 0x00, 0xFF) 
#define CYAN                        RGB(0x00, 0xFF, 0xFF) 
#define GRAY                        RGB(0x80, 0x80, 0x40) 
#define GOLD                        RGB(0xA0, 0xA0, 0x40)
#define WET_ASPHALT                 RGB(0x34, 0x49, 0x5E)
#define CLOUDS						RGB(0xEC, 0xF0, 0xF1)  
#define TURQUOISE					RGB(0x1A, 0xBC, 0x9C)
#define GREEN_SEA					RGB(0x16, 0xA0, 0x85)
#define SILVER						RGB(0xBD, 0xC3, 0xC7)
#define SEA							RGB(0x34, 0x98, 0xDB)
#define POMEGRANATE					RGB(0xC0, 0x39, 0x2B)
#define SUN_FLOWER					RGB(0xF1, 0xC4, 0xF)
#define EMERALD						RGB(0x2E, 0xCC, 0x71)
#define ALIZARIN					RGB(0xE7, 0x4C, 0x3C)

/* _____PUBLIC VARIABLE_____________________________________________________ */
extern unsigned int _color;

/* _____PUBLIC FUNCTIONS_____________________________________________________ */


/**
* @brief This function reset lcd module
*/
extern void LCD_Reset(void);

/**
* @brief set start address of lcd ram
* @param x Pixel coordinate X
* @param y Pixel coordinate Y
*/
extern void LCD_SetCursor(unsigned int x, unsigned int y);

/**
* @brief set paint area
* @param x1 Pixel 1 coordinate X
* @param y1 Pixel 1 coordinate Y
* @param x2 Pixel 2 coordinate X
* @param y2 Pixel 2 coordinate Y
*/
extern void LCD_SetArea(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2);

/**
* @brief puts pixel
* @param x Pixel coordinate X
* @param y Pixel coordinate Y
*/
extern void LCD_PutPixel(unsigned int x, unsigned int y);

/**
* @brief draws rectangle filled with current color
* @param left Pixel 1 coordinate X
* @param top Pixel 1 coordinate Y
* @param right Pixel 2 coordinate X
* @param bottom Pixel 2 coordinate Y
* @param color Bar color
*/
extern void LCD_Bar(unsigned int left, unsigned int top, unsigned int right, unsigned int bottom, unsigned int color);

/**
* @brief fill display with color
* @param color Paint color
*/
extern void LCD_Clear(unsigned int color);

/**
* @brief display image array to lcd
* @param x Pixel coordinate X
* @param y Pixel coordinate Y
* @param w Width
* @param h Height
* @param t compress type(0 = none(RGB565), 1 = compress(RGB5<compress bit>55)
* @param pImage - FLASH array of image
*/
extern void LCD_DrawSymbol(unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned char t, const unsigned char *pImage);

/**
* @brief display image array to lcd
* @param x Pixel coordinate X
* @param y Pixel coordinate Y
* @param pImage - FLASH array of image
*/
extern void LCD_DrawImage(unsigned int x, unsigned int y, const unsigned char *pImage);

/* _____DEFINE MACRO_________________________________________________________ */
#define GetMaxX() 		((unsigned int)SCREEN_HOR_SIZE-1)
#define GetMaxY() 		((unsigned int)SCREEN_VER_SIZE-1)

#define RGB(red, green, blue)	((unsigned int)( (( red >> 3 ) << 11 ) | \
								(( green >> 2 ) << 5  ) | \
								( blue  >> 3 )))

#define SetColor(color) _color = color
#define GetColor()      _color

#endif

