//Using  : The source code is intended to be used at selected operation mode of AD7843.
//Description : 

 /**
 @defgroup TC
 @code #include <tc_ad7843_avr_v1_00.h> @endcode
 
 @brief Touch Screen Controller Library
 
*/
 
 /*@{*/

#ifndef _TC_AD7843_H_
#define _TC_AD7843_H_

/**
* @brief initial touch screen controller
*/
void TCInit(void);

/**
* @brief call this after TCRead() to get X value
*/
unsigned short TCGetX(void); //

/**
* @brief call this after TCRead() to get Y value
*/
unsigned short TCGetY(void);

/**
* @brief TC_PEN_PIN is 0 when the screen is pressed
*/
unsigned char TCIsPenOn(void);

/**
* @brief read analog voltage
*/
void TCRead(void);

#endif
