/************************************************************************************************** 
* File name		: graphic.h 
* Programmer 	: jaruwit supa 
* Web presence  : www.thaibestlink.com 
* Note			: lcd graphic support 
* Language		: avrGCC 
* Hardware		: atmega16 
* Date			: 01/05/2009 
***************************************************************************************************/ 

/**
 @defgroup LCDLibrary
 @code #include <Graphic.h> @endcode
 
 @brief LCD Graphic Support Library
       
 @authors Jaruwit Supa  

*/

/*@{*/
 
#ifndef _GRAPHIC_H_ 
#define _GRAPHIC_H_ 
 
/**
 * @name Project includes
 */
/*@{*/
// LCM driver define 
#include "SSD1289x16.h"  
/*@}*/ 
 
 
/* _____PUBLIC FUNCTIONS_____________________________________________________ */ 

/**
* @brief Draw a line on a graphic LCD using Bresenham's
* @param x1 Starting coordinate X
* @param y1 Starting coordinate Y
* @param x2 Ending coordinate X
* @param y2 Ending coordinate Y
*/
extern void Line(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2); 

/**
* @brief Draw a rectangle on a graphic LCD
* @param x1 Starting coordinate X
* @param y1 Starting coordinate Y
* @param x2 Ending coordinate X
* @param y2 Ending coordinate Y
*/
extern void Rectangle(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2); 

/**
* @brief Draws a beveled figure on the screen.
* @param x1 Starting coordinate X of the upper left center
* @param y1 Starting coordinate Y of the upper left center
* @param x2 Ending coordinate X of the lower right center
* @param y2 Ending coordinate Y of the lower right center
* @param rad Defines the radius of the circle
* @param fill Fill yes or no
*/
extern void RoundRectangle(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2, unsigned int radius, unsigned char fill); 

/**
* @brief Draw a circle on the screen.
* @param x Coordinate X of the center of the circle
* @param y Coordinate Y of the center of the circle
* @param rad Defines the radius of the circle
* @param fill Fill yes or no
*/
extern void Circle(unsigned int x, unsigned int y, unsigned int radius, unsigned char fill); 
 
/* _____PUBLIC DEFINE________________________________________________________ */ 
#define PutPixel(x,y)				LCD_PutPixel(x,y) 
#define FillRectangle(x1,y1,x2,y2)	LCD_Bar(x1, y1, x2, y2, GetColor()) 
 
#define Bevel(x1,y1,x2,y2,r) 		RoundRectangle(x1,y1,x2,y2,r,0) 
#define BevelFill(x1,y1,x2,y2,r)	RoundRectangle(x1,y1,x2,y2,r,1) 
 
 
#endif
