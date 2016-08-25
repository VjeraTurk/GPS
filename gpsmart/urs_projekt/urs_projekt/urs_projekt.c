/**
 * @file urs_projekt.c
 * @author Sandro Babic, David Kreso, Marin Krmpotic, Ana Tomas
 * @date 6 Feb 2015
 * @brief The main file receiving data from the gps module, showing graphics on the screen and managing touch events on the screen.
 */

#include "app_config.h"

#include <util/delay.h>
#include <avr/io.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <math.h>


#include "SSD1289x16.h"
#include "font.h"
#include "ft18x34.h"
#include "tc_ad7843_avr_v1_00.h"
#include "serial.h"
#include "Graphic.h"


#ifdef TS_ORN_PORTRAIT
#define TCGetH()	TCGetX()
#define TCGetV()	TCGetY()
#else
#define TCGetH()	TCGetY()
#define TCGetV()	TCGetX()
#endif

#define pi 3.14159265358979323846 /**< Value of the PI constant */

const long chalfx = TS_SIZE_X/2;
const long chalfy = TS_SIZE_Y/2;


long ccx = 0x00000834;
long cm1x = 0x00000007;
long cm2x = 0xFFFFFFB8;

long ccy = 0x00000815;
long cm1y = 0x0000000C;
long cm2y = 0x000000A3;

/**
 * @brief A function for getting the exact x-position of the touch event.
 *
 * @param x x-coordinate buffer value
 * @return Position X
 */
long cal_posx(unsigned short x);

/**
 * @brief A function for getting the exact y-position of the touch event.
 *
 * @param y y-coordinate buffer value
 * @return Position Y
 */
long cal_posy(unsigned short y);

/**
 * @brief A function for showing a selectable menu to the other screens.
 *
 * This screen is shown when the app is started.
 */
void showMenu();

/**
 * @brief A function for showing a live GPS readings on a dedicated screen.
 *
 * This screen is shown when a user selects it from the menu.
 */
void showLiveGPS();

/**
 * @brief A function for showing distances from the current GPS location to some hardcoded cities on a dedicated screen.
 *
 * This screen is shown when a user selects it from the menu.
 */
void showDistances();

/**
 * @brief A function for graphical representation of the current altitude data on a dedicated screen.
 *
 * This screen is shown when a user selects it from the menu.
 */
void showAltitude();

/**
 * @brief A function for showing a Distance Calculator creen.
 *
 * This screen is shown when a user selects it from the menu.
 */
void showCalc();

/**
 * @brief A function for receiving GPS data and assigning the values to current and previous GPS struct variables.
 */
void readGPS();

/**
 * @brief A function for calculating a distance between two GPS points
 *
 * @param lat1 First point latitude
 * @param lon1 First point longitude
 * @param lat2 Second point latitude
 * @param lon2 Second point longitude
 * @param unit Distance unit. M for miles, K for kilometers, N for nautical miles
 * @return Distance in desired distance unit
 */
double distance(long lat1, long lon1, double lat2, double lon2, char unit);

/**
 * @brief A function for getting radians from degrees
 *
 * @param deg Double precision degrees
 * @return Radians
 */
double deg2rad(double deg);

/**
 * @brief A function for getting degrees from radians
 *
 * @param deg Double precision radians
 * @return Degrees
 */
double rad2deg(double rad);

/**
 * @brief A function that returns the decimal equivalent of a Hex value
 *
 * @param c Hex value
 * @return The decimal equivalent
 */
uint8_t parseHex(char c);

/**
 * @brief A structure to keep  the current GPS data
 *
 * This data gets updated every time new data is received.
 */
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
	long latitude; /**< Latitude value in degrees */
	long latitudeDegrees; /**< Only latitude degrees */
	long latitude_fixed; /**< Fixed point latitude value with degrees stored in units of 1/100000 degrees and minutes stored in units of 1/100000 degrees */
	long minutesLat; /**< Only latitude minutes */
	char lat[2]; /**< Hemisphere letter, N or S */
	/*@}*/
	
	/**
	 * @name Longitude Values
	 */
	/*@{*/
	long longitude; /**< Longitude value in degrees */
	long longitudeDegrees; /**< Only longitude degrees */
	long longitude_fixed; /**< Fixed point longitude value with degrees stored in units of 1/100000 degrees and minutes stored in units of 1/100000 degrees */
	long minutesLon; /**< Only longitude minutes */
	/*@}*/
	
	int j; /**< A counter that gets incremented on every symbol from the gps module */
	int satellites; /**< Current number of satellites */
	int altitude; /**< Altitude value in meters */
	int fixquality; /**< Fix quality, 0 for no fix, 1 for fix */

} currentReading;

/**
 * @brief A structure to keep one previous GPS data
 *
 * This data gets replaced by the current reading every time the current reading is to be updated.
 */
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
	long latitude; /**< Latitude value in degrees */
	long longitude; /**< Longitude value in degrees */

} previousReading;


int currentScreen; /**< The screen currently being shown */
char latitudeInput[11]; /**< Latitude input field content, defined by the user */
char longitudeInput[11]; /**< Longitude input field content, defined by the user */
int focusedField = 0; /**< Which input field is focused */


unsigned char last_pen_check = 0; /**< A flag used to define if touch has just been made or hold */
so_pos_t hpos; /**< Horizontal position of the touch event */
so_pos_t vpos; /**< Vertical position of the touch event */

pstatus_t p_stat; /**< Status of event */


long cal_posx(unsigned short x)
{
	long buf;
	buf = x - ccx;
	buf = buf*cm1x;
	buf = buf/cm2x;
	buf = buf + chalfx;
	return (buf);
}

long cal_posy(unsigned short y)
{
	long buf;
	buf = y - ccy;
	buf = buf*cm1y;
	buf = buf/cm2y;
	buf = buf + chalfy;
	return (buf);
}

/**
 * @brief A function for detecting touch events and calling the appropriate functions depending on the touch event
 *
 * This function should be called in an indefinite loop.
 */
void ScanPen(void)
{
	unsigned long x,y;
	unsigned short tc_x_buf,tc_y_buf;
	unsigned short tc_x_max,tc_y_max;
	unsigned short tc_x_min,tc_y_min;
	if (TCIsPenOn())
	{
		TCRead();
		tc_x_buf = TCGetH();
		tc_y_buf = TCGetV();

		tc_x_max = TCGetH();
		tc_y_max = TCGetV();
		tc_x_min = TCGetH();
		tc_y_min = TCGetV();

		TCRead();
		tc_x_buf += TCGetH();
		tc_y_buf += TCGetV();

		if (TCGetH() > tc_x_max)
		tc_x_max = TCGetH();
		if (TCGetH() < tc_x_min)
		tc_x_min = TCGetH();
		if (TCGetV() > tc_y_max)
		tc_y_max = TCGetV();
		if (TCGetV() < tc_y_min)
		tc_y_min = TCGetV();

		TCRead();
		tc_x_buf += TCGetH();
		tc_y_buf += TCGetV();

		if (TCGetH() > tc_x_max)
		tc_x_max = TCGetH();
		if (TCGetH() < tc_x_min)
		tc_x_min = TCGetH();
		if (TCGetV() > tc_y_max)
		tc_y_max = TCGetV();
		if (TCGetV() < tc_y_min)
		tc_y_min = TCGetV();

		TCRead();
		tc_x_buf += TCGetH();
		tc_y_buf += TCGetV();

		if (TCGetH() > tc_x_max)
		tc_x_max = TCGetH();
		if (TCGetH() < tc_x_min)
		tc_x_min = TCGetH();
		if (TCGetV() > tc_y_max)
		tc_y_max = TCGetV();
		if (TCGetV() < tc_y_min)
		tc_y_min = TCGetV();

		tc_x_buf -= tc_x_max;
		tc_x_buf -= tc_x_min;
		tc_y_buf -= tc_y_max;
		tc_y_buf -= tc_y_min;

		tc_x_buf = tc_x_buf >> 1;
		tc_y_buf = tc_y_buf >> 1;

		if (TCIsPenOn())
		{
			x = cal_posx(tc_x_buf);
			y = cal_posy(tc_y_buf);

			if (last_pen_check)
			{
				p_stat = PST_HOLD;
			}
			else
			{
				p_stat = PST_DOWN;
			}

			if ((x) && (y) && (x < TS_SIZE_X) && (y < TS_SIZE_Y))
			{
				hpos = x;
				vpos = y;

				if (p_stat == PST_DOWN)
				{
					
					switch (currentScreen) {
						case 0:
						if (vpos >= 5 && vpos <= GetMaxX() - 5 && hpos >= 62 && hpos <= 117) {
							SetColor(GREEN_SEA);
							FillRectangle(GetMaxX() - 5, 62, GetMaxX(), 117);
							SetColor(CLOUDS);
							currentScreen = 1;
							showLiveGPS();
							} else if (vpos >= 5 && vpos <= GetMaxX() - 5 && hpos >= 122 && hpos <= 177) {
							SetColor(GREEN_SEA);
							FillRectangle(GetMaxX() - 5, 122, GetMaxX(), 177);
							SetColor(CLOUDS);
							currentScreen = 2;
							BevelFill(0, 41, GetMaxX(), GetMaxY(), 0);
							showDistances();
							} else if (vpos >= 5 && vpos <= GetMaxX() - 5 && hpos >= 182 && hpos <= 237) {
							SetColor(GREEN_SEA);
							FillRectangle(GetMaxX() - 5, 182, GetMaxX(), 237);
							SetColor(CLOUDS);
							currentScreen = 4;
							showAltitude();
							} else if (vpos >= 5 && vpos <= GetMaxX() - 5 && hpos >= 242 && hpos <= 297) {
							SetColor(GREEN_SEA);
							FillRectangle(GetMaxX() - 5, 242, GetMaxX(), 297);
							SetColor(CLOUDS);
							currentScreen = 5;
							showCalc();
						}
						break;
						case 1:
						if (vpos >= 0 && vpos <= 40 && hpos >= 40 && hpos <= 80) {
							SetColor(WET_ASPHALT);
							FillRectangle(0, 40, 40, 80);
							SetFgColor(CLOUDS);
							DrawText(0, 40, 40, 80, "<", ALINE_CENTER);
							SetColor(CLOUDS);
							currentScreen = 0;
							showMenu();
						}
						break;
						case 2:
						if (vpos >= 0 && vpos <= 40 && hpos >= 40 && hpos <= 80) {
							SetColor(WET_ASPHALT);
							FillRectangle(0, 40, 40, 80);
							SetFgColor(CLOUDS);
							DrawText(0, 40, 40, 80, "<", ALINE_CENTER);
							SetColor(CLOUDS);
							currentScreen = 0;
							showMenu();
							} else if (vpos >= GetMaxX() / 2 && vpos <= GetMaxX() - 20 && hpos >= GetMaxY() - 50 && hpos <= GetMaxY() - 20) {
							currentScreen = 3;
							showDistances();
						}
						break;
						case 3:
						if (vpos >= 0 && vpos <= 40 && hpos >= 40 && hpos <= 80) {
							SetColor(WET_ASPHALT);
							FillRectangle(0, 40, 40, 80);
							SetFgColor(CLOUDS);
							DrawText(0, 40, 40, 80, "<", ALINE_CENTER);
							SetColor(CLOUDS);
							currentScreen = 0;
							showMenu();
							} else if (vpos >= 20 && vpos <= GetMaxX() / 2 && hpos >= GetMaxY() - 50 && hpos <= GetMaxY() - 20) {
							currentScreen = 2;
							showDistances();
						}
						break;
						case 4:
						if (vpos >= 0 && vpos <= 40 && hpos >= 40 && hpos <= 80) {
							SetColor(WET_ASPHALT);
							FillRectangle(0, 40, 40, 80);
							SetFgColor(CLOUDS);
							DrawText(0, 40, 40, 80, "<", ALINE_CENTER);
							SetColor(CLOUDS);
							currentScreen = 0;
							showMenu();
						}
						break;
						case 5:
						if (vpos >= 0 && vpos <= 40 && hpos >= 40 && hpos <= 80) {
							SetColor(WET_ASPHALT);
							FillRectangle(0, 40, 40, 80);
							SetFgColor(CLOUDS);
							DrawText(0, 40, 40, 80, "<", ALINE_CENTER);
							SetColor(CLOUDS);
							currentScreen = 0;
							showMenu();
							} else if (focusedField < 2 && vpos >= 10 && vpos <= 50 && hpos >= GetMaxY() - 125 && hpos <= GetMaxY() - 90) {
							if (focusedField == 0) {
								strcat(latitudeInput, "7");
								} else {
								strcat(longitudeInput, "7");
							}
							} else if (focusedField < 2 && vpos >= 10 && vpos <= 50 && hpos >= GetMaxY() - 85 && hpos <= GetMaxY() - 50) {
							if (focusedField == 0) {
								strcat(latitudeInput, "4");
								} else {
								strcat(longitudeInput, "4");
							}
							} else if (focusedField < 2 && vpos >= 10 && vpos <= 50 && hpos >= GetMaxY() - 45 && hpos <= GetMaxY() - 10) {
							if (focusedField == 0) {
								strcat(latitudeInput, "1");
								} else {
								strcat(longitudeInput, "1");
							}
							} else if (focusedField < 2 && vpos >= 55 && vpos <= 95 && hpos >= GetMaxY() - 125 && hpos <= GetMaxY() - 90) {
							if (focusedField == 0) {
								strcat(latitudeInput, "8");
								} else {
								strcat(longitudeInput, "8");
							}
							} else if (focusedField < 2 && vpos >= 55 && vpos <= 95 && hpos >= GetMaxY() - 85 && hpos <= GetMaxY() - 50) {
							if (focusedField == 0) {
								strcat(latitudeInput, "5");
								} else {
								strcat(longitudeInput, "5");
							}
							} else if (focusedField < 2 && vpos >= 55 && vpos <= 95 && hpos >= GetMaxY() - 45 && hpos <= GetMaxY() - 10) {
							if (focusedField == 0) {
								strcat(latitudeInput, "2");
								} else {
								strcat(longitudeInput, "2");
							}
							} else if (focusedField < 2 && vpos >= 100 && vpos <= 140 && hpos >= GetMaxY() - 125 && hpos <= GetMaxY() - 90) {
							if (focusedField == 0) {
								strcat(latitudeInput, "9");
								} else {
								strcat(longitudeInput, "9");
							}
							} else if (focusedField < 2 && vpos >= 100 && vpos <= 140 && hpos >= GetMaxY() - 85 && hpos <= GetMaxY() - 50) {
							if (focusedField == 0) {
								strcat(latitudeInput, "6");
								} else {
								strcat(longitudeInput, "6");
							}
							} else if (focusedField < 2 && vpos >= 100 && vpos <= 140 && hpos >= GetMaxY() - 45 && hpos <= GetMaxY() - 10) {
							if (focusedField == 0) {
								strcat(latitudeInput, "3");
								} else {
								strcat(longitudeInput, "3");
							}
							} else if (focusedField < 2 && vpos >= 150 && vpos <= GetMaxX() - 10 && hpos >= GetMaxY() - 125 && hpos <= GetMaxY() - 90) {
							strcat(longitudeInput, "3");
							} else if (focusedField < 2 && vpos >= 150 && vpos <= GetMaxX() - 10 && hpos >= GetMaxY() - 85 && hpos <= GetMaxY() - 50) {
							if (focusedField == 0) {
								strcat(latitudeInput, ".");
								} else {
								strcat(longitudeInput, ".");
							}
							} else if (focusedField < 2 && vpos >= 150 && vpos <= 190 && hpos >= GetMaxY() - 45 && hpos <= GetMaxY() - 10) {
							if (focusedField == 0) {
								strcat(latitudeInput, "0");
								} else {
								strcat(longitudeInput, "0");
							}
							} else if (focusedField < 2 && vpos >= 195 && vpos <= GetMaxX() - 10 && hpos >= GetMaxY() - 45 && hpos <= GetMaxY() - 10) {
							focusedField++;
							} else if (focusedField >= 2 && vpos >= 0 && vpos <= GetMaxX() && hpos >= GetMaxY() - 50 && hpos <= GetMaxY()) {
							showCalc();
						}
						break;
					}
					
					
					
				}
			}

			last_pen_check = 1;
		}
	}
	else
	{
		if (last_pen_check)
		{
			p_stat = PST_UP;
		}
		else
		{
			p_stat = PST_NOTFOUND;
		}

		last_pen_check = 0;
	}
}

void showMenu() {
	
	//int dotFlag = 0;

	currentScreen = 0;
	SetColor(CLOUDS);
	BevelFill(0, 40, GetMaxX(), GetMaxY(), 0);
	SetColor(TURQUOISE);
	FillRectangle(5, 62, GetMaxX() - 5, 117);
	FillRectangle(5, 122, GetMaxX() - 5, 177);
	FillRectangle(5, 182, GetMaxX() -5, 237);
	FillRectangle(5, 242, GetMaxX() -5, 297);
	
	SetColor(GREEN_SEA);
	FillRectangle(GetMaxX() - 60, 62, GetMaxX() - 5, 117);
	FillRectangle(GetMaxX() - 60, 122, GetMaxX() - 5, 177);
	FillRectangle(GetMaxX() - 60, 182, GetMaxX() -5, 237);
	FillRectangle(GetMaxX() - 60, 242, GetMaxX() -5, 297);
	
	SetFgColor(CLOUDS);
	DrawText(25, 62, GetMaxX() - 5, 114, "Live GPS", ALINE_LEFT);
	DrawText(25, 122, GetMaxX() - 5, 174, "Distances", ALINE_LEFT);
	DrawText(25, 182, GetMaxX() -5, 234, "Altitude", ALINE_LEFT);
	DrawText(25, 242, GetMaxX() -5, 294, "Distance Calc.", ALINE_LEFT);
	
	
	DrawText(GetMaxX() - 60, 62, GetMaxX() - 5, 114, ">", ALINE_CENTER);
	DrawText(GetMaxX() - 60, 122, GetMaxX() - 5, 174, ">", ALINE_CENTER);
	DrawText(GetMaxX() - 60, 182, GetMaxX() -5, 234, ">", ALINE_CENTER);
	DrawText(GetMaxX() - 60, 242, GetMaxX() -5, 294, ">", ALINE_CENTER);
	
	
	
	while(currentScreen == 0) {
		ScanPen();		
		readGPS();
				
				/*
				if (fixquality == 0) {
					
					if (fixquality != fixqualityOld) {
						SetColor(BLACK);
						FillRectangle(5, 120, GetMaxX() - 5, 200);
						SetColor(CLOUDS);
						FillRectangle(6, 125, GetMaxX() - 6, 199);
						SetFgColor(BLACK);
						DrawText(5, 128, GetMaxX() - 5, 138, "Searching for Satellites", ALINE_CENTER);
						DrawText(5, 180, GetMaxX() - 5, 190, "Please wait", ALINE_CENTER);
					}
					
					
					switch(dotFlag) {
						case 0:
						SetColor(SILVER);
						FillRectangle(71, 156, 161, 166);
						dotFlag = 1;
						break;
						case 1:
						SetColor(SUN_FLOWER);
						FillRectangle(71, 156, 101, 166);
						//DrawText(107, 138, GetMaxX() - 5, 148, "_", ALINE_LEFT);
						dotFlag = 2;
						break;
						case 2:
						SetColor(EMERALD);
						FillRectangle(101, 156, 131, 166);
						//DrawText(107, 138, GetMaxX() - 5, 148, "__", ALINE_LEFT);
						dotFlag = 3;
						break;
						case 3:
						SetColor(ALIZARIN);
						FillRectangle(131, 156, 161, 166);
						//DrawText(107, 138, GetMaxX() - 5, 148, "___", ALINE_LEFT);
						dotFlag = 0;
						break;
					}
				} else if (fixquality == 1 && fixquality != fixqualityOld) {
					showMenu();
				}
				*/
}

}

/**
 * @brief A function that initializes touch screen, UART and interrupts
 *
 * This function is called only once on the start of the app.
 */
int main(void)
{
	TCInit();

	 // Disable JTAG Interface
	 MCUCSR |= _BV(JTD);
	 MCUCSR |= _BV(JTD);

	// Initialize and clear LCD
	LCD_Reset();
	LCD_Clear(CLOUDS);	
	LcdFont(ft18x34);

	SetColor(WET_ASPHALT);
	FillRectangle(0, 0, GetMaxX(), 40);
	SetFgColor(CLOUDS);
	DrawText(0, 0, GetMaxX(), 40, "GPSmart", ALINE_CENTER);

	 // Initialize UART modules
	 for (int i = 0; i < serialAvailable(); i++) {
		 serialInit(i, BAUD(9600, F_CPU));
		 _delay_ms(100);
	 }

	 // Enable Interrupts
	 sei();

	 showMenu();
	
	 return 0;
}


void readGPS() {
	
		char degreebuff[10]; /**< A temporary buffer storing the degrees */
		char fullLine[120]; /**< A temporary array storing 120 chars received from the serial */
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

void showLiveGPS() {
	
	currentReading.lat[1] = '\0';
	int firstShow = 1; /**< A flag showing if this is the first time showing the screen */
	char str[20]; /**< A helper string used to store text that is to be shown on the screen */
	
	SetColor(CLOUDS);
	BevelFill(0, 41, GetMaxX(), GetMaxY(), 0);
	SetColor(TURQUOISE);
	BevelFill(0, 40, GetMaxX(), 80, 0);
	SetColor(GREEN_SEA);
	BevelFill(0, 40, 40, 80, 0);
	SetFgColor(CLOUDS);
	DrawText(0, 40, 40, 80, "<", ALINE_CENTER);
	DrawText(0, 40, GetMaxX(), 80, "Live GPS", ALINE_CENTER);
	SetColor(CLOUDS);
	
	
	while(currentScreen == 1) {
		
		ScanPen();
		readGPS();

				memset(str, 0, 20);
				
				SetFgColor(WET_ASPHALT);
				
				sprintf(str, "Time: %02d:%02d:%02d", currentReading.hours, currentReading.minutes, currentReading.seconds);
				if (currentReading.hours != previousReading.hours || firstShow) {
					BevelFill(70, 83, 95, 110, 0);
				}
				if (currentReading.minutes != previousReading.minutes || firstShow) {
					BevelFill(101, 83, 121, 110, 0);
				}
				
				BevelFill(127, 83, GetMaxX(), 110, 0);
				DrawText(20, 100, GetMaxX() - 20, 100, str, ALINE_LEFT);
				
				if (currentReading.latitude != previousReading.latitude || firstShow) {
					sprintf(str, "Latitude: %lu.%lu", currentReading.latitudeDegrees, currentReading.minutesLat);
					BevelFill(97, 130, GetMaxX(), 150, 0);
					DrawText(20, 140, GetMaxX() - 20, 140, str, ALINE_LEFT);
				}
				
				if (currentReading.longitude != previousReading.longitude || firstShow) {
					sprintf(str, "Longitude: %lu.%lu", currentReading.longitudeDegrees, currentReading.minutesLon);
					BevelFill(110, 160, GetMaxX(), 180, 0);
					DrawText(20, 170, GetMaxX() - 20, 170, str, ALINE_LEFT);
				}
				
				if (currentReading.altitude != previousReading.altitude || firstShow) {
					sprintf(str, "Altitude: %d m", currentReading.altitude);
					BevelFill(90, 190, GetMaxX(), 210, 0);
					DrawText(20, 200, GetMaxX() - 20, 200, str, ALINE_LEFT);
				}
				
				
				if (currentReading.satellites != previousReading.satellites || firstShow) {
					sprintf(str, "Satellite Count: %d", currentReading.satellites);
					BevelFill(155, 220, GetMaxX(), 240, 0);
					DrawText(20, 230, GetMaxX() - 20, 230, str, ALINE_LEFT);
				}

				firstShow = 0;

			}
			
			
}

void showDistances() {
	
	currentReading.lat[1] = '\0';
	int firstShow = 1; /**< A flag showing if this is the first time showing the screen */
	char str[20]; /**< A helper string used to store text that is to be shown on the screen */
	
	if (currentScreen == 2) {
		SetColor(WHITE);
		FillRectangle(GetMaxX() / 2 + 1, GetMaxY() - 49, GetMaxX() - 21, GetMaxY() - 21);
		SetColor(SILVER);
		Rectangle(20, GetMaxY() - 50, GetMaxX() / 2, GetMaxY() - 20);
		FillRectangle(21, GetMaxY() - 49, GetMaxX() / 2 -1, GetMaxY() - 21);
		SetFgColor(WHITE);
		DrawText(20, GetMaxY() - 50, GetMaxX() / 2, GetMaxY() - 20, "Croatia", ALINE_CENTER);
		SetFgColor(SILVER);
		Rectangle(GetMaxX() / 2, GetMaxY() - 50, GetMaxX() - 20, GetMaxY() - 20);
		DrawText(GetMaxX() / 2, GetMaxY() - 50, GetMaxX() - 20, GetMaxY() - 20, "World", ALINE_CENTER);
		SetColor(CLOUDS);
	} else if (currentScreen == 3) {
		SetColor(WHITE);
		FillRectangle(21, GetMaxY() - 49, GetMaxX() / 2 -1, GetMaxY() - 21);
		SetColor(SILVER);
		Rectangle(20, GetMaxY() - 50, GetMaxX() / 2, GetMaxY() - 20);
		SetFgColor(SILVER);
		DrawText(20, GetMaxY() - 50, GetMaxX() / 2, GetMaxY() - 20, "Croatia", ALINE_CENTER);
		Rectangle(GetMaxX() / 2, GetMaxY() - 50, GetMaxX() - 20, GetMaxY() - 20);
		FillRectangle(GetMaxX() / 2 + 1, GetMaxY() - 49, GetMaxX() - 21, GetMaxY() - 21);
		SetFgColor(WHITE);
		DrawText(GetMaxX() / 2, GetMaxY() - 50, GetMaxX() - 20, GetMaxY() - 20, "World", ALINE_CENTER);
		SetColor(CLOUDS);
	}
	
		SetColor(TURQUOISE);
		FillRectangle(0, 40, GetMaxX(), 80);
		SetColor(GREEN_SEA);
		FillRectangle(0, 40, 40, 80);
		SetFgColor(CLOUDS);
		DrawText(0, 40, 40, 80, "<", ALINE_CENTER);
		DrawText(0, 40, GetMaxX(), 80, "Distances", ALINE_CENTER);
		SetColor(CLOUDS);
		BevelFill(0, 81, GetMaxX(), GetMaxY()-51, 0);
	
	while(currentScreen == 2 || currentScreen == 3) {
		
		ScanPen();
		readGPS();
		
		memset(str, 0, 20);

				SetFgColor(WET_ASPHALT);
				
				
				if (currentScreen == 2) {
					if (currentReading.latitude != previousReading.latitude || currentReading.longitude != previousReading.longitude || firstShow) {
						sprintf(str, "Zagreb: %d km", (int) round(distance(currentReading.latitude_fixed, currentReading.longitude_fixed, 45.8144400, 15.9779800, 'K')));
						BevelFill(90, 100, GetMaxX(), 120, 0);
						BevelFill(75, 130, GetMaxX(), 150, 0);
						BevelFill(65, 160, GetMaxX(), 180, 0);
						BevelFill(80, 190, GetMaxX(), 210, 0);
						BevelFill(110, 220, GetMaxX(), 240, 0);
						DrawText(20, 110, GetMaxX() - 20, 110, str, ALINE_LEFT);
						sprintf(str, "Rijeka: %d km", (int) round(distance(currentReading.latitude_fixed, currentReading.longitude_fixed, 45.328979, 14.457664, 'K')));
						DrawText(20, 140, GetMaxX() - 20, 140, str, ALINE_LEFT);
						sprintf(str, "Split: %d km", (int) round(distance(currentReading.latitude_fixed, currentReading.longitude_fixed, 43.5089100, 16.4391500, 'K')));
						DrawText(20, 170, GetMaxX() - 20, 170, str, ALINE_LEFT);
						sprintf(str, "Osijek: %d km", (int) round(distance(currentReading.latitude_fixed, currentReading.longitude_fixed, 45.5511100, 18.6938900, 'K')));
						DrawText(20, 200, GetMaxX() - 20, 200, str, ALINE_LEFT);
						sprintf(str, "Dubrovnik: %d km", (int) round(distance(currentReading.latitude_fixed, currentReading.longitude_fixed, 42.6480700, 18.0921600, 'K')));
						DrawText(20, 230, GetMaxX() - 20, 230, str, ALINE_LEFT);
					}
				} else if (currentScreen == 3) {
					if (currentReading.latitude != previousReading.latitude || currentReading.longitude != previousReading.longitude || firstShow) {
						sprintf(str, "Tokyo: %d km", (int) round(distance(currentReading.latitude_fixed, currentReading.longitude_fixed, 35.6895000, 139.6917100, 'K')));
						BevelFill(83, 100, GetMaxX(), 120, 0);
						BevelFill(115, 130, GetMaxX(), 150, 0);
						BevelFill(95, 160, GetMaxX(), 180, 0);
						BevelFill(85, 190, GetMaxX(), 210, 0);
						BevelFill(93, 220, GetMaxX(), 240, 0);
						DrawText(20, 110, GetMaxX() - 20, 110, str, ALINE_LEFT);
						sprintf(str, "New York: %d km", (int) round(distance(currentReading.latitude_fixed, currentReading.longitude_fixed, 40.7142700, -74.0059700, 'K')));
						DrawText(20, 140, GetMaxX() - 20, 140, str, ALINE_LEFT);
						sprintf(str, "Moscow: %d km", (int) round(distance(currentReading.latitude_fixed, currentReading.longitude_fixed, 55.7522200, 37.6155600, 'K')));
						DrawText(20, 170, GetMaxX() - 20, 170, str, ALINE_LEFT);
						sprintf(str, "London: %d km", (int) round(distance(currentReading.latitude_fixed, currentReading.longitude_fixed, 51.5085300, -0.1257400, 'K')));
						DrawText(20, 200, GetMaxX() - 20, 200, str, ALINE_LEFT);
						sprintf(str, "Sydney: %d km", (int) round(distance(currentReading.latitude_fixed, currentReading.longitude_fixed, -33.8678500, 151.2073200, 'K')));
						DrawText(20, 230, GetMaxX() - 20, 230, str, ALINE_LEFT);
					}
				}
						
				
	firstShow = 0;

		
	}
	

}

void showAltitude() {
	
	currentReading.lat[1] = '\0';
	int firstShow = 1; /**< A flag showing if this is the first time showing the screen */
	char str[20]; /**< A helper string used to store text that is to be shown on the screen */
	
	SetColor(TURQUOISE);
	FillRectangle(0, 40, GetMaxX(), 80);
	SetColor(GREEN_SEA);
	FillRectangle(0, 40, 40, 80);
	SetFgColor(CLOUDS);
	DrawText(0, 40, 40, 80, "<", ALINE_CENTER);
	DrawText(0, 40, GetMaxX(), 80, "Altitude", ALINE_CENTER);
	SetColor(CLOUDS);
	BevelFill(0, 81, GetMaxX(), GetMaxY(), 0);
	
	
	SetColor(BLACK);
	SetFgColor(BLACK);
	DrawText(15, 90, GetMaxX() - 15, 100, "1472 m", ALINE_CENTER);
	Line(5, 85, 10, 85);
	Line(5, 85, 5, GetMaxY() - 40);
	Line(GetMaxX() - 5, 85, GetMaxX() - 10, 85);
	Line(GetMaxX() - 5, 85, GetMaxX() - 5, GetMaxY() - 40);
	SetColor(SEA);
	FillRectangle(5, GetMaxY() - 40, GetMaxX() - 5, GetMaxY() - 5);
	SetFgColor(CLOUDS);
	DrawText(15, GetMaxY() - 27, GetMaxX() - 15, GetMaxY() - 17, "SEA LEVEL", ALINE_CENTER);
	
	while(currentScreen == 4) {
		
		ScanPen();
		readGPS();
		
		memset(str, 0, 20);
	
				SetFgColor(WET_ASPHALT);
				
				
				if ((currentReading.altitude != previousReading.altitude || firstShow) && currentReading.altitude > 0) {

					SetColor(CLOUDS);
					for (int i = 11; i <= GetMaxX() - 11; i = i + 12) {
						Circle(i, GetMaxY() - 40, 5, 1);
					}
					
					SetColor(CLOUDS);
					FillRectangle(6, 110, GetMaxX() - 6, GetMaxY() - 40);
					SetColor(RED);
					SetFgColor(RED);
					sprintf(str, "%d m", currentReading.altitude);
					DrawText(15, GetMaxY() - 60 - (int) (currentReading.altitude * 0.163), GetMaxX(), GetMaxY() - 50 - (int) (currentReading.altitude * 0.163), str, ALINE_LEFT);
					Line(6, GetMaxY() - 40 - (int) (currentReading.altitude * 0.163), 100, GetMaxY() - 40 - (int) (currentReading.altitude * 0.163));
					Line(GetMaxX() - 6, GetMaxY() - 40 - (int) (currentReading.altitude * 0.163), GetMaxX() - 101, GetMaxY() - 40 - (int) (currentReading.altitude * 0.163));
					Circle(GetMaxX() / 2, GetMaxY() - 40 - (int) (currentReading.altitude * 0.163), 5, 1);

				}
				
				firstShow = 0;
	}
}

void showCalc() {
	
	currentReading.lat[1] = '\0';
	int firstShow = 1; /**< A flag showing if this is the first time showing the screen */
	char str[20]; /**< A helper string used to store text that is to be shown on the screen */
	double latDouble; /**< Double precision latitude value */
	double lonDouble; /**< Double precision longitude value */
	
	focusedField = 0; /**< A flag showing which input field is focused */
	memset(longitudeInput, 0, 11);
	memset(latitudeInput, 0, 11);
	
	SetColor(TURQUOISE);
	FillRectangle(0, 40, GetMaxX(), 80);
	SetColor(GREEN_SEA);
	FillRectangle(0, 40, 40, 80);
	SetFgColor(CLOUDS);
	DrawText(0, 40, 40, 80, "<", ALINE_CENTER);
	DrawText(0, 40, GetMaxX(), 80, "Distance Calc.", ALINE_CENTER);
	SetColor(CLOUDS);
	BevelFill(0, 81, GetMaxX(), GetMaxY(), 0);
	
	SetColor(WHITE);
	FillRectangle(5, 85, GetMaxX() - 5, 115);
	FillRectangle(5, 120, GetMaxX() - 5, 150);
	SetFgColor(BLACK);
	DrawText(5, 85, GetMaxX() - 10, 115, "LAT", ALINE_RIGHT);
	DrawText(5, 120, GetMaxX() - 10, 150, "LON", ALINE_RIGHT);
	
	SetColor(BLACK);
	Rectangle(5, 85, GetMaxX() - 5, 115);
	Rectangle(5, 120, GetMaxX() - 5, 150);
	

		SetColor(TURQUOISE);
		FillRectangle(0, GetMaxY() - 135, GetMaxX(), GetMaxY());

		SetColor(GREEN_SEA);
		FillRectangle(10, GetMaxY() - 125, 50, GetMaxY() - 90);
		FillRectangle(10, GetMaxY() - 85, 50, GetMaxY() - 50);
		FillRectangle(10, GetMaxY() - 45, 50, GetMaxY() - 10);
		
		FillRectangle(55, GetMaxY() - 125, 95, GetMaxY() - 90);
		FillRectangle(55, GetMaxY() - 85, 95, GetMaxY() - 50);
		FillRectangle(55, GetMaxY() - 45, 95, GetMaxY() - 10);
		
		FillRectangle(100, GetMaxY() - 125, 140, GetMaxY() - 90);
		FillRectangle(100, GetMaxY() - 85, 140, GetMaxY() - 50);
		FillRectangle(100, GetMaxY() - 45, 140, GetMaxY() - 10);
		
		FillRectangle(150, GetMaxY() - 125, GetMaxX() - 10, GetMaxY() - 90);
		FillRectangle(150, GetMaxY() - 85, GetMaxX() - 10, GetMaxY() - 50);
		
		FillRectangle(150, GetMaxY() - 45, 190, GetMaxY() - 10);
		FillRectangle(195, GetMaxY() - 45, GetMaxX() - 10, GetMaxY() - 10);
		
		SetFgColor(CLOUDS);
		DrawText(10, GetMaxY() - 125, 50, GetMaxY() - 90, "7", ALINE_CENTER);
		DrawText(10, GetMaxY() - 85, 50, GetMaxY() - 50, "4", ALINE_CENTER);
		DrawText(10, GetMaxY() - 45, 50, GetMaxY() - 10, "1", ALINE_CENTER);
		
		DrawText(55, GetMaxY() - 125, 95, GetMaxY() - 90, "8", ALINE_CENTER);
		DrawText(55, GetMaxY() - 85, 95, GetMaxY() - 50, "5", ALINE_CENTER);
		DrawText(55, GetMaxY() - 45, 95, GetMaxY() - 10, "2", ALINE_CENTER);
		
		DrawText(100, GetMaxY() - 125, 140, GetMaxY() - 90, "9", ALINE_CENTER);
		DrawText(100, GetMaxY() - 85, 140, GetMaxY() - 50, "6", ALINE_CENTER);
		DrawText(100, GetMaxY() - 45, 140, GetMaxY() - 10, "3", ALINE_CENTER);
		
		DrawText(150, GetMaxY() - 125, GetMaxX() - 10, GetMaxY() - 90, "<", ALINE_CENTER);
		DrawText(150, GetMaxY() - 85, GetMaxX() - 10, GetMaxY() - 50, ".", ALINE_CENTER);
		
		DrawText(150, GetMaxY() - 45, 190, GetMaxY() - 10, "0", ALINE_CENTER);
		DrawText(195, GetMaxY() - 45, GetMaxX() - 10, GetMaxY() - 10, "->", ALINE_CENTER);
	
	while(1) {
		
		ScanPen();
		
		memset(str, 0, 20);
		
		SetFgColor(WET_ASPHALT);
		

			SetColor(BLACK);
			if (focusedField == 0) {
				SetColor(BLACK);
				Rectangle(6, 86, GetMaxX() - 6, 114);
				SetColor(WHITE);
				Rectangle(6, 121, GetMaxX() - 6, 149);
			} else if (focusedField == 1) {
				SetColor(BLACK);
				Rectangle(6, 121, GetMaxX() - 6, 149);
				SetColor(WHITE);
				Rectangle(6, 86, GetMaxX() - 6, 114);
			} else if (focusedField == 2) {
				readGPS();
				latDouble = atof(latitudeInput);
				lonDouble = atof(longitudeInput);
				sprintf(str, "Distance: %d km", (int) round(distance(currentReading.latitude_fixed, currentReading.longitude_fixed, latDouble, lonDouble, 'K')));
				SetColor(WHITE);
				Rectangle(6, 121, GetMaxX() - 6, 149);
				Rectangle(6, 86, GetMaxX() - 6, 114);
				SetColor(CLOUDS);
				FillRectangle(90, 152, GetMaxX() - 5, 180);
				FillRectangle(0, GetMaxY() - 135, GetMaxX(), GetMaxY());
				DrawText(10, 150, GetMaxX() - 10, 180, str, ALINE_LEFT);
				SetColor(ALIZARIN);
				FillRectangle(0, GetMaxY() - 50, GetMaxX(), GetMaxY());
				SetFgColor(CLOUDS);
				DrawText(0, GetMaxY() - 50, GetMaxX(), GetMaxY(), "RESET", ALINE_CENTER);
				focusedField++;
			} else {
				readGPS();
				if (currentReading.latitude != previousReading.latitude || currentReading.longitude != previousReading.longitude) {
					sprintf(str, "Distance: %d km", (int) round(distance(currentReading.latitude_fixed, currentReading.longitude_fixed, latDouble, lonDouble, 'K')));
					SetColor(CLOUDS);
					FillRectangle(90, 152, GetMaxX() - 5, 180);
					DrawText(10, 150, GetMaxX() - 5, 180, str, ALINE_LEFT);
				}
			}


			memset(str, 0, 20);
			SetColor(RED);
			SetFgColor(BLACK);
			sprintf(str, "%s", latitudeInput);
			DrawText(10, 85, GetMaxX() - 5, 115, str, ALINE_LEFT);
			
			memset(str, 0, 20);
			sprintf(str, "%s", longitudeInput);
			DrawText(10, 120, GetMaxX() - 5, 150, str, ALINE_LEFT);
		
		firstShow = 1;
		
	}
}


double distance(long lat1, long lon1, double lat2, double lon2, char unit) {
	double theta; /**< Longitude difference  */
	double dist; /**< Distance that is going to be returned to the caller */
	double dLat1 = lat1 / 10000000.0; /**< Double precision of first point latitude */
	double dLon1 = lon1 / 10000000.0; /**< Double precision of first point longitude */
	theta = dLon1 - lon2;
	dist = sin(deg2rad(dLat1)) * sin(deg2rad(lat2)) + cos(deg2rad(dLat1)) * cos(deg2rad(lat2)) * cos(deg2rad(theta));
	dist = acos(dist);
	dist = rad2deg(dist);
	dist = dist * 60 * 1.1515;
	switch(unit) {
		case 'M':
		break;
		case 'K':
		dist = dist * 1.609344;
		break;
		case 'N':
		dist = dist * 0.8684;
		break;
	}
	return (dist);
}


double deg2rad(double deg) {
	return (deg * pi / 180);
}

double rad2deg(double rad) {
	return (rad * 180 / pi);
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