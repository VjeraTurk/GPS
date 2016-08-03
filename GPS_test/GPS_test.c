/*
 * GPS_test.c
 *
 * Created: 5.4.2016. 19:00:31
 *  Author: Vjera
 * 
 */ 
//test2
/*
decimal  degrees    distance
places
-------------------------------
0        1.0        111 km
1        0.1        11.1 km
2        0.01       1.11 km
3        0.001      111 m
4        0.0001     11.1 m
5        0.00001    1.11 m
6        0.000001   0.111 m
7        0.0000001  1.11 cm
8        0.00000001 1.11 mm
*/
#define F_CPU			8000000UL
#define F_CPU_MHZ		8

#include <avr/io.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>


#include <stdlib.h>
#include <string.h>

#include "lcd.h"
#include "serial.h"

//my_serial
/*
#define BAUD            9600
#define BRC             ((F_CPU/16/BAUD)-1) //BAUD PRESCALAR (for Asynch. mode)
#define TX_BUFFER_SIZE  128
#define RX_BUFFER_SIZE  255

char txBuffer[TX_BUFFER_SIZE];
char rxBuffer[RX_BUFFER_SIZE];

volatile uint8_t txReadPos=0;
volatile uint8_t txWritePos=0;
volatile uint8_t rxReadPos=0;
volatile uint8_t rxWritePos=0;


ISR(USART_RXC_vect)
{
	rxBuffer[rxWritePos] = UDR;
	//na vrhu
	//lcd_putc (rxBuffer[rxWritePos]);
	
	if(rxBuffer[rxWritePos]=='C' && rxBuffer[rxWritePos-1]=='M' && rxBuffer[rxWritePos-2]=='R'&& rxBuffer[rxWritePos-3]=='P'&& rxBuffer[rxWritePos-4]=='G')
	{

	}
	
	rxWritePos++;
	if(rxWritePos>=RX_BUFFER_SIZE)
	{
		rxWritePos=0;
	}
}
static void UART_Init(void)
{
	// Set baud rate
	UBRRH = 0;
	UBRRL = BRC;

	//Enable reciver and transmitter
	UCSRB = (1 << RXEN) | (1 << TXEN)| (1<<RXCIE)| (1<<TXCIE);//|(1<<UDRIE); // interrupts

	//set frame format: 8 bit
	UCSRC = (1 << UCSZ0) | (1 << UCSZ1)|(1 << URSEL); // Character Size 8 bit
	//UPM1 UPM0,  even-1 0 odd- 1 1  ? sta je ovo, je to potrebno?
}
*/

uint8_t parseHex(char c);
static void LCD_Init(void);
struct Compass{

	long double slope;
	float angle;
	unsigned long previousAngle;
	
}compass;
struct CurrentGpsReading
{
	/**
	 * @name Time Values
	 */
	/*@{*/
	int hours; /**< Current hour */
	int minutes; /**< Current minute */
	int seconds; /**< Current second */
	/*@}*/
	
	/**
	 * @name Latitude Values
	 */
	/*@{*/
	unsigned long latitude; /**< Latitude value in degrees */
	unsigned long latitudeDegrees; /**< Only latitude degrees */
	unsigned long latitude_fixed; /**< Fixed point latitude value with degrees stored in units of 1/100000 degrees and minutes stored in units of 1/100000 degrees */
	unsigned long minutesLat; /**< Only latitude minutes */
	char lat[2]; /**< Hemisphere letter, N or S */
	/*@}*/
	
	/**
	 * @name Longitude Values
	 */
	/*@{*/
	unsigned long longitude; /**< Longitude value in degrees */
	unsigned long longitudeDegrees; /**< Only longitude degrees */
	unsigned long longitude_fixed; /**< Fixed point longitude value with degrees stored in units of 1/100000 degrees and minutes stored in units of 1/100000 degrees */
	unsigned long minutesLon; /**< Only longitude minutes */
	/*@}*/
	
	int j; /**< A counter that gets incremented on every symbol from the gps module */
	int satellites; /**< Current number of satellites */
	int altitude; /**< Altitude value in meters */
	int fixquality; /**< Fix quality, 0 for no fix, 1 for fix */

	//$GPRMC	
	char fix; //A-Active, V- Void in $GPRMC
	float speed;
	float angle;
	
} currentReading;

struct PreviousGpsReading
{
	/**
	 * @name Time Values
	 */
	/*@{*/
	int hours; /**< Current hour */
	int minutes; /**< Current minute */
	/*@}*/
	
	int satellites; /**< Current number of satellites */
	int altitude; /**< Altitude value in meters */
	unsigned long latitude; /**< Latitude value in degrees */
	unsigned long longitude; /**< Longitude value in degrees */
	//compass:
	unsigned long longitude_fixed;
	unsigned long latitude_fixed;
	
} previousReading;


void readGPGGA() {
		
		char degreebuff[10]; /**< A temporary buffer storing the degrees */
		char fullLine[120]; /**< A temporary array storing 120 chars received from the serial */
		//promijeniti logiku?!
		char tHours[3]; /**< A temporary string storing hours */
		char tMinutes[3]; /**< A temporary string storing minutes */
		char tSeconds[3]; /**< A temporary string storing seconds */
		char *p; /**< Pointer used for sliding through an array of chars */
		int32_t degree; /**< A degree from the degree buffer is stored here */
		currentReading.lat[1] = '\0';

	while(currentReading.j <= 119) {
		
	
	for (int i = 0; i < serialAvailable(); i++) {
			 if (serialHasChar(i)) {
				 fullLine[currentReading.j] = serialGet(i);
				 currentReading.j++;
			 }
		 }
		 
	}


		if (currentReading.j >= 119) {
			currentReading.j = 0;
			
			  // do checksum check

			  // first look if we even have one
			  if (fullLine[strlen(fullLine)-4] == '*') {
				  uint16_t sum = parseHex(fullLine[strlen(fullLine)-3]) * 16;
				  sum += parseHex(fullLine[strlen(fullLine)-2]);
				  
				  // check checksum
				  for (uint8_t i=1; i < (strlen(fullLine)-4); i++) {
					  sum ^= fullLine[i];
				  }
				  if (sum != 0) {
					  // bad checksum :(
					  return;
				  }
			  }
			
			memset(tHours, 0, 3);
			memset(tMinutes, 0, 3);
			memset(tSeconds, 0, 3);
			memset(degreebuff, 0, 10);
			memset(currentReading.lat, 0, 1);
			
			if (strstr(fullLine, "$GPGGA")) {
				// found GGA
				p = strstr(fullLine, "$GPGGA");
				p = strchr(p, ',')+1;
				strncpy(tHours, p, 2); // hours
				p = p + 2;
				strncpy(tMinutes, p, 2); // minutes
				p = p + 2;
				strncpy(tSeconds, p, 2); // seconds
				previousReading.hours = currentReading.hours;
				currentReading.hours = atoi(tHours);
				previousReading.minutes = currentReading.minutes;
				currentReading.minutes = atoi(tMinutes);
				currentReading.seconds = atoi(tSeconds);
				if (currentReading.hours < 22) {
					currentReading.hours = currentReading.hours + 2;
				} else if (currentReading.hours == 22){
					currentReading.hours = 0;
				} else {
					currentReading.hours = 1;
				}

				
				p = strchr(p, ',')+1;
				
				 if (',' != *p)
				 {
					 strncpy(degreebuff, p, 2);
					 p += 2;
					 degreebuff[2] = '\0';
					 //long degree = atol(degreebuff) * 10000000;
					 degree = atol(degreebuff) * 10000000;
					 strncpy(degreebuff, p, 2); // minutes
					 p += 3; // skip decimal point
					 strncpy(degreebuff + 2, p, 4);
					 degreebuff[6] = '\0';
					 currentReading.minutesLat = 50 * atol(degreebuff) / 3;
					 previousReading.latitude = currentReading.latitude;
					 currentReading.latitude = degree / 100000 + currentReading.minutesLat * 0.000006F;				 
					 currentReading.latitudeDegrees = (currentReading.latitude-100*(int)(currentReading.latitude/100))/60.0;
					 currentReading.latitudeDegrees += (int)(currentReading.latitude/100);
					 currentReading.latitude_fixed = degree + currentReading.minutesLat;		 
					 
					 
				 }
				 
				 
				 p = strchr(p, ',')+1;
				 if (',' != *p)
				 {
					 if (p[0] == 'S') currentReading.latitudeDegrees *= -1.0;
					 if (p[0] == 'N') currentReading.lat[0] = 'N';
					 else if (p[0] == 'S') currentReading.lat[0] = 'S';
					 else if (p[0] == ',') currentReading.lat[0] = '/';
				 }
				 
				 // parse out longitude
				 p = strchr(p, ',')+1;
				 if (',' != *p)
				 {
					 strncpy(degreebuff, p, 3);
					 p += 3;
					 degreebuff[3] = '\0';
					 degree = atol(degreebuff) * 10000000;
					 strncpy(degreebuff, p, 2); // minutes
					 p += 3; // skip decimal point
					 strncpy(degreebuff + 2, p, 4);
					 degreebuff[6] = '\0';
					 currentReading.minutesLon = 50 * atol(degreebuff) / 3;
					 previousReading.longitude = currentReading.longitude;
					 currentReading.longitude = degree / 100000 + currentReading.minutesLon * 0.000006F;
					 currentReading.longitudeDegrees = (currentReading.longitude-100*(int)(currentReading.longitude/100))/60.0;
					 currentReading.longitudeDegrees += (int)(currentReading.longitude/100);
					 currentReading.longitude_fixed = degree + currentReading.minutesLon;
				  }
				 
				p = strchr(p, ',')+1;
				if (',' != *p)
					{
						if (p[0] == 'W') currentReading.longitudeDegrees *= -1.0;
					}
				 
				 
				 
				 p = strchr(p, ',')+1;
				 if (',' != *p)
				 {
					 currentReading.fixquality = atoi(p);
				 }
				 
				 
				 
				 p = strchr(p, ',')+1;
				 if (',' != *p)
				 {
					 previousReading.satellites = currentReading.satellites;
					 currentReading.satellites = atoi(p);
				 }
				 
				 
				 p = strchr(p, ',')+1;
				 
				 p = strchr(p, ',')+1;
				 if (',' != *p)
				 {
					 previousReading.altitude = currentReading.altitude;
					 currentReading.altitude = atof(p);
				 }
				 
				 p = strchr(p, ',')+1;
				 p = strchr(p, ',')+1;

			}	
			
		}
		

}
char storesGPRMC[20];

#define MAXLINELENGHT 120

//#define MAXLINELENGHT 960
int readGPRMC() {
	
		
		char degreebuff[10]; /**< A temporary buffer storing the degrees */
		char fullLine[MAXLINELENGHT]; /**< A temporary array storing 120 chars received from the serial */
		//120 ->255
		char tHours[3]; /**< A temporary string storing hours */
		char tMinutes[3]; /**< A temporary string storing minutes */
		char tSeconds[3]; /**< A temporary string storing seconds */
		char *p; /**< Pointer used for sliding through an array of chars */
		int32_t degree; /**< A degree from the degree buffer is stored here */
		char s[MAXLINELENGHT];
		
		currentReading.lat[1] = '\0';
	while(currentReading.j <= MAXLINELENGHT-1) {
		
	
	for (int i = 0; i < serialAvailable(); i++) {
			 if (serialHasChar(i)) {
				 fullLine[currentReading.j] = serialGet(i);
				 currentReading.j++;
			 }
		 }
		 
	}


		if (currentReading.j >= MAXLINELENGHT-1) {
			currentReading.j = 0;
			
			  // do checksum check

			  // first look if we even have one
			  if (fullLine[strlen(fullLine)-4] == '*') {
				  uint16_t sum = parseHex(fullLine[strlen(fullLine)-3]) * 16;
				  sum += parseHex(fullLine[strlen(fullLine)-2]);
				  
				  // check checksum
				  for (uint8_t i=2; i < (strlen(fullLine)-4); i++) { //2!!! my
					  sum ^= fullLine[i];
				  }
				  if (sum != 0) {
					  // bad checksum :(
					  lcd_gotoxy(0,0);
					  lcd_puts("!bad check sum");
					  return 0;
				  }
			  }
			
			memset(tHours, 0, 3);
			memset(tMinutes, 0, 3);
			memset(tSeconds, 0, 3);
			memset(degreebuff, 0, 10);
			memset(currentReading.lat, 0, 1);
		
		/**get $GPRMC**/
			
			if (strstr(fullLine, "$GPRMC")) {
				// found RMC
		
				p = strstr(fullLine, "$GPRMC"); //1
				*p = *fullLine; //my
				
				strncpy(storesGPRMC, p, 20); 
				
				p = strchr(p, ',')+1;
				strncpy(tHours, p, 2); // hours
				p = p + 2;
				strncpy(tMinutes, p, 2); // minutes
				p = p + 2;
				strncpy(tSeconds, p, 2); // seconds
				
				previousReading.hours = currentReading.hours;
				currentReading.hours = atoi(tHours);
				previousReading.minutes = currentReading.minutes;
				currentReading.minutes = atoi(tMinutes);
				currentReading.seconds = atoi(tSeconds);
				
				if (currentReading.hours < 22) {
					currentReading.hours = currentReading.hours + 2;
					} else if (currentReading.hours == 22){
					currentReading.hours = 0;
					} else {
					currentReading.hours = 1;
				}
				
				p = strchr(p, ',')+1; //2
				if (p[0] == 'A')
				{
					currentReading.fix = p[0];
					currentReading.fixquality=1;
				}
				else if (p[0] == 'V')
				{
					currentReading.fix = p[0];
					currentReading.fixquality=0;
					return 0; //my
				}
				p = strchr(p, ',')+1; //3
				if (',' != *p)
				{
					strncpy(degreebuff, p, 2);
					p += 2;
					degreebuff[2] = '\0';
					long degree = atol(degreebuff) * 10000000;
					strncpy(degreebuff, p, 2); // minutes
					p += 3; // skip decimal point
					strncpy(degreebuff + 2, p, 4);
					degreebuff[6] = '\0';
					
					/*
					//!!!!!!
					 long minutes = 50 * atol(degreebuff) / 3;
					 latitude_fixed = degree + minutes;
					 latitude = degree / 100000 + minutes * 0.000006F;
					 latitudeDegrees = (latitude-100*int(latitude/100))/60.0;
					 latitudeDegrees += int(latitude/100);
					//!!!!!!
					*/

					previousReading.latitude_fixed=currentReading.latitude_fixed;
					previousReading.latitude = currentReading.latitude;
					
					currentReading.minutesLat = 50 * atol(degreebuff) / 3;
					currentReading.latitude_fixed = degree + currentReading.minutesLat;
					currentReading.latitude = degree / 100000 + currentReading.minutesLat * 0.000006F;
					currentReading.latitudeDegrees = (currentReading.latitude-100*(int)(currentReading.latitude/100))/60.0;
					currentReading.latitudeDegrees += (int)(currentReading.latitude/100);
						
					//compass:
				}
				
				
				p = strchr(p, ',')+1; //4
				if (',' != *p)
				{
					if (p[0] == 'S') currentReading.latitudeDegrees *= -1.0;
					if (p[0] == 'N') currentReading.lat[0] = 'N';
					else if (p[0] == 'S') currentReading.lat[0] = 'S';
					else if (p[0] == ',') currentReading.lat[0] = '/'; //???
				}
				
				// parse out longitude
				p = strchr(p, ',')+1; //5
				if (',' != *p)
				{
					strncpy(degreebuff, p, 3);
					p += 3;
					degreebuff[3] = '\0';
					long degree = atol(degreebuff) * 10000000;
					strncpy(degreebuff, p, 2); // minutes
					p += 3; // skip decimal point
					strncpy(degreebuff + 2, p, 4);
					degreebuff[6] = '\0';
					//compass:
					 previousReading.longitude_fixed=currentReading.longitude_fixed;
					 previousReading.longitude = currentReading.longitude;	
					
					currentReading.minutesLon = 50 * atol(degreebuff) / 3;
					currentReading.longitude_fixed = degree + currentReading.minutesLon;
					currentReading.longitude = degree / 100000 + currentReading.minutesLon * 0.000006F;
					currentReading.longitudeDegrees = (currentReading.longitude-100*(int)(currentReading.longitude/100))/60.0;
					currentReading.longitudeDegrees += (int)(currentReading.longitude/100);
					
				}
				p = strchr(p, ',')+1;
				if (',' != *p)
				{
					if (p[0] == 'W') currentReading.longitudeDegrees *= -1.0;
				}
				
				p = strchr(p, ',')+1;
				if (',' != *p)
				{
					currentReading.speed = atof(p);
				}
				
				p = strchr(p, ',')+1;
				if (',' != *p)
				{
					currentReading.angle = atof(p);
					//tracking angle!
				}
				p = strchr(p, ',')+1;
				
				return 1;
			}
		
		
				
		/** Compass **/
		compass.previousAngle=compass.angle;
		compass.slope = ((long double)currentReading.latitude_fixed-(long double)previousReading.latitude_fixed) - ((long double)currentReading.longitude_fixed-(long double)previousReading.longitude_fixed);
		//compass.angle= atan(compass.slope);
		//compass.angle= atan(compass.slope)*180/pi;
		//A minute (1/60th of a degree) equals 1.852 km :'(
		
		}
		
		return 1;
}

int main(void)
{
	LCD_Init();


	// Initialize UART modules
	for (int i = 0; i < serialAvailable(); i++) {
		serialInit(i, BAUD(9600, F_CPU));
		_delay_ms(100);
	}

	//my_serial UART_Init();
	// Enable Interrupts
	sei();
	char s[120];
	int MAX=0;
	int fix;
	while(1)
    {
		MAX++;
		fix=readGPRMC();
		//readGPGGA();
		//if(1){
		if(fix){
			/*
			lcd_puts(itoa(currentReading.hours, s, 10));
			lcd_puts(itoa(currentReading.minutes, s, 10));
			lcd_puts(itoa(currentReading.seconds, s, 10));
			*/
			//long latitude_fixed; /**< Fixed point latitude value with degrees stored in units of 1/100000 degrees and minutes stored in units of 1/100000 degrees */
			//lcd_puts(itoa(currentReading.latitude_fixed, s, 10)); //!!! promijenjiva vrijednost- zasto??
			lcd_clrscr();
			lcd_putc(currentReading.fix); // 'A' i 'V'
			
			
			//lcd_puts(ltoa(currentReading.latitude,s,10));
			//lcd_puts(ltoa(currentReading.minutesLat,s,10));
			
			//lcd_puts(itoa(currentReading.latitude,s,10));
			//lcd_puts(itoa(currentReading.minutesLat,s,10));
			
			
			//lcd_puts(itoa(currentReading.angle, s, 10)); //!!!!!
			//lcd_puts(itoa(currentReading.speed, s, 10)); //!!!!!
			
			
			lcd_puts(ltoa(currentReading.latitude_fixed,s,10));
			lcd_gotoxy(0,2);
			lcd_puts(ltoa(currentReading.longitude_fixed,s,10));
		
			lcd_gotoxy(11,1);
			lcd_puts(itoa(currentReading.hours,s,10));
			lcd_putc(':');
			lcd_puts(itoa(currentReading.minutes,s,10));
			_delay_ms(1000);
			if(currentReading.latitude_fixed == atol("7816")){
				lcd_gotoxy(0,1);
				lcd_puts_P("^");				
			}
			lcd_clrscr();
			lcd_puts(storesGPRMC);
			_delay_ms(2000);
			lcd_clrscr();
				
		}else lcd_puts("bcm");
		
    }
}

static void LCD_Init(void)
{
	// initialize display, cursor off
	lcd_init(LCD_DISP_ON);

	// clear display and home cursor
	lcd_clrscr();

	// put string to display (line 1) with linefeed
	// lcd_puts_P("Hello world\n");
}
uint8_t parseHex(char c) {
	if (c < '0')
	return 0;
	if (c <= '9')
	return c - '0';
	if (c < 'A')
	return 0;
	if (c <= 'F')
	return (c - 'A')+10;
	// if (c > 'F')
	return 0;
}