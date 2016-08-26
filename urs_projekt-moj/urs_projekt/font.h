 /**
 @defgroup FontLibrary
 @code #include <font.h> @endcode
 
 @brief Library for showing text on the LCD screen.
 
*/

#ifndef _FONT_H_
#define _FONT_H_

#define ALINE_LEFT		0
#define ALINE_CENTER	1
#define ALINE_RIGHT		2

/**
* @brief Set lcd font
* @param pointer Font
*/

extern void LcdFont(uint8_t *pointer);

extern void LcdRot(uint8_t r);
extern void LcdFontFixed(uint8_t typ);
extern void	LcdNonTransparence(uint8_t nt);

/**
* @brief Set font color
* @param color Font color
*/
extern void	LcdFgColor(uint16_t color);

/**
* @brief Set background color
* @param color Background color
*/
extern void	LcdBkColor(uint16_t color);

/**
* @brief Draw a char
* @param c The char
*/
extern uint8_t DrawChar(uint8_t c);

/**
* @brief Draw a string
* @param c The string
*/
extern unsigned int DrawStr(char *c);
extern unsigned int DrawStrP(char *Text);

/**
* @brief Calculate the width of a string
* @param Text The string
*/
extern unsigned int CalcTextWidth(char *Text);

/**
* @brief Calculate the height of a string
* @param Text The string
*/
extern unsigned int CalcTextHeight(char *Text);

/**
* @brief Draw text on a screen
* @param left x coordinate of the top left point
* @param top y coordinate of the top left point
* @param right x coordinate of the bottom right point
* @param bottom y coordinate of the bottom right point
* @param Text String
* @param aline The alignment of text
*/
extern void DrawText(uint16_t left, uint16_t top, uint16_t right, uint16_t bottom, char *Text, uint8_t aline);
//extern void DrawTextP(uint16_t left, uint16_t top, uint16_t right, uint16_t bottom, const char *Text, uint8_t aline);

extern uint8_t 	rot;
extern uint16_t FgColor;
extern uint16_t BkColor;
extern uint8_t 	FontFixed;
extern uint8_t	NonTransparence;

extern uint8_t FontWidth;
extern uint8_t FontHeight;
extern uint8_t FontXScale;
extern uint8_t FontYScale;
extern uint8_t FontSpace;

extern unsigned int cursorX;		// x position
extern unsigned int cursorY;		// y position

#define GetCursorX() 	cursorX
#define GetCursorY() 	cursorY
#define SetCursorX(x) 	cursorX = x
#define SetCursorY(y) 	cursorY = y
#define SetCursor(x,y) 	{cursorX = x; cursorY = y;}

#define LcdRot(n)				rot = n
#define LcdFontFixed(n)			FontFixed = n
#define LcdNonTransparence(n)	NonTransparenz = n
#define SetFgColor(n)			FgColor = n
#define SetBkColor(n)			BkColor = n
#define GetFgColor()			FgColor
#define GetBkColor()			BkColor


#define LcdFontXScale(n)	FontXScale = n
#define LcdFontYScale(n)	FontYScale = n
#define LcdFontSpace(n)		FontSpace  = n

#define LcdFontWidth() 	 	FontWidth
#define LcdFontHeight()		FontHeight

// complex function
#define DrawStringAt(x,y,s)	 SetCursorX(x); SetCursorY(y); DrawStr(s)
#define DrawStringAtP(x,y,s) SetCursorX(x); SetCursorY(y); DrawStrP(s)

#endif
