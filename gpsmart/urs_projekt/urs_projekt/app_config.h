//Using  : Configuration header for application
//Description : This file is to be included in all other .c files.
//User may put anything for configuration. Other files's header also
//declared in this file.

 /**
 @defgroup AppConfig
 @code #include <app_config.h> @endcode
 
 @brief Some additional defines.
 
*/

#ifndef _APP_CONFIG_H_
#define _APP_CONFIG_H_

#define F_CPU			8000000UL
#define F_CPU_MHZ		8

//device registers and speific macro
#define Orb(port,bitnum)		port |= _BV(bitnum)
#define Setb(port,bitnum)		port |= _BV(bitnum)
#define Clrb(port,bitnum)		port &= ~(_BV(bitnum))
#define Rdb(pinp,bitnum)		(pinp & _BV(bitnum))

#define delay_1ms(x)			_delay_ms(x)

#include <avr/io.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>


#include "spi.h"
#include "tc_ad7843_avr_v1_00.h"

//touch screen LCD configuration
//#define TS_ORN_PORTRAIT

#ifdef TS_ORN_PORTRAIT
#define TS_SIZE_X					240
#define TS_SIZE_Y					320
#define TS_VAL_ENTRY_MOD			0x0030
#define TS_INS_GRAM_ADX				TS_INS_GRAM_HOR_AD
#define TS_INS_GRAM_ADY				TS_INS_GRAM_VER_AD
#define TS_INS_START_ADX   			TS_INS_HOR_START_AD
#define TS_INS_END_ADX   			TS_INS_HOR_END_AD
#define TS_INS_START_ADY   			TS_INS_VER_START_AD
#define TS_INS_END_ADY   			TS_INS_VER_END_AD
#else
#define TS_SIZE_X					320
#define TS_SIZE_Y					240
#define TS_VAL_ENTRY_MOD			0x0028
#define TS_INS_GRAM_ADX				TS_INS_GRAM_VER_AD
#define TS_INS_GRAM_ADY				TS_INS_GRAM_HOR_AD
#define TS_INS_START_ADX   			TS_INS_VER_START_AD
#define TS_INS_END_ADX   			TS_INS_VER_END_AD
#define TS_INS_START_ADY   			TS_INS_HOR_START_AD
#define TS_INS_END_ADY   			TS_INS_HOR_END_AD
#endif

//pen status type definition
typedef enum
{
	PST_NOTFOUND,
	PST_DOWN,
	PST_HOLD,
	PST_UP
} pstatus_t;

#define so_pos_t		short

#endif
