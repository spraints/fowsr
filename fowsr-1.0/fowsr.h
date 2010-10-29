/* Fine Offset Weather Station Reader - Header file

   (C) Arne-J�rgen Auberg (arne.jorgen.auberg@gmail.com)

  - Wireless Weather Station Data Block Definition
  - Wireless Weather Station Record Format Definition
  - Wunderground Record Format
  - PYWWS Record Format
  - PWS Weather Record Format

  - CUSB class for open, initialize and close of USB interface
  - CWS class for open, read, and close of WS buffer
  - CWF class for write to selected log file format
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <signal.h>
#include <ctype.h>
#include <time.h>
#include <usb.h>


// Parameters used by the cache file
#define ISREADING	0
#define ISWRITING	1

// Weather Station buffer parameters
#define WS_BUFFER_SIZE		0x10000	// Size of total buffer
#define WS_BUFFER_START		0x100	// Size of fixed block, start of up to 4080 buffer records
#define WS_BUFFER_END		0xFFF0	// Last buffer record
#define WS_BUFFER_RECORD	0x10	// Size of one buffer record
#define WS_BUFFER_CHUNK		0x20	// Size of chunk received over USB

// Weather Station buffer memory positions
#define WS_DELAY		 0	// Position of delay parameter
#define WS_HUMIDITY_IN		 1	// Position of inside humidity parameter
#define WS_TEMPERATURE_IN	 2	// Position of inside temperature parameter
#define WS_HUMIDITY_OUT		 4	// Position of outside humidity parameter
#define WS_TEMPERATURE_OUT	 5	// Position of outside temperature parameter
#define WS_ABS_PRESSURE		 7	// Position of absolute pressure parameter
#define WS_WIND_AVE		 9	// Position of wind direction parameter
#define WS_WIND_GUST		10	// Position of wind direction parameter
#define WS_WIND_DIR		12	// Position of wind direction parameter
#define WS_RAIN			13	// Position of rain parameter
#define WS_STATUS		15	// Position of status parameter
#define WS_DATA_COUNT		27	// Position of data_count parameter
#define WS_CURRENT_POS		30	// Position of current_pos parameter

// Calculated rain parameters
// NOTE: These positions are NOT stored in the Weather Station
#define WS_RAIN_HOUR		0x08	// Position of hourly calculated rain
#define WS_RAIN_DAY		0x0A	// Position of daily calculated rain
#define WS_RAIN_WEEK		0x0C	// Position of weekly calculated rain
#define WS_RAIN_MONTH		0x0E	// Position of monthly calculated rain


// The following setting parameters are for reference only
// A future user interface could interpret these parameters
// Unit settings
#define WS_UNIT_SETTING_IN_T_C_F	0x01
#define WS_UNIT_SETTING_OUT_T_C_F	0x02
#define WS_UNIT_SETTING_RAIN_FALL_CM_IN	0x04
#define WS_UNIT_SETTING_PRESSURE_HPA	0x20
#define WS_UNIT_SETTING_PRESSURE_INHG	0x40
#define WS_UNIT_SETTING_PRESSURE_MMHG	0x80
// Unit wind speed settings
#define WS_UNIT_SETTING_WIND_SPEED_MS	0x01
#define WS_UNIT_SETTING_WIND_SPEED_KMH	0x02
#define WS_UNIT_SETTING_WIND_SPEED_KNOT	0x04
#define WS_UNIT_SETTING_WIND_SPEED_MH	0x08
#define WS_UNIT_SETTING_WIND_SPEED_BFT	0x10
// Display format 0
#define WS_DISPLAY_FORMAT_P_ABS_REL	0x01
#define WS_DISPLAY_FORMAT_WSP_AVG_GUST	0x02
#define WS_DISPLAY_FORMAT_H_24_12	0x04
#define WS_DISPLAY_FORMAT_DDMMYY_MMDDYY	0x08
#define WS_DISPLAY_FORMAT_TS_H_12_24	0x10
#define WS_DISPLAY_FORMAT_DATE_COMPLETE	0x20
#define WS_DISPLAY_FORMAT_DATE_AND_WK	0x40
#define WS_DISPLAY_FORMAT_ALARM_TIME	0x80
// Display format 1
#define WS_DISPLAY_FORMAT_OUT_T		0x01
#define WS_DISPLAY_FORMAT_OUT_WINDCHILL	0x02
#define WS_DISPLAY_FORMAT_OUT_DEW_POINT	0x04
#define WS_DISPLAY_FORMAT_RAIN_FALL_1H	0x08
#define WS_DISPLAY_FORMAT_RAIN_FALL_24H	0x10
#define WS_DISPLAY_FORMAT_RAIN_FALL_WK	0x20
#define WS_DISPLAY_FORMAT_RAIN_FALL_MO	0x40
#define WS_DISPLAY_FORMAT_RAIN_FALL_TOT	0x80
// Alarm enable 0
#define WS_ALARM_ENABLE_TIME		0x02
#define WS_ALARM_ENABLE_WIND_DIR	0x04
#define WS_ALARM_ENABLE_IN_RH_LO	0x10
#define WS_ALARM_ENABLE_IN_RH_HI	0x20
#define WS_ALARM_ENABLE_OUT_RH_LO	0x40
#define WS_ALARM_ENABLE_OUT_RH_HI	0x80
// Alarm enable 1
#define WS_ALARM_ENABLE_WSP_AVG		0x01
#define WS_ALARM_ENABLE_WSP_GUST	0x02
#define WS_ALARM_ENABLE_RAIN_FALL_1H	0x04
#define WS_ALARM_ENABLE_RAIN_FALL_24H	0x08
#define WS_ALARM_ENABLE_ABS_P_LO	0x10
#define WS_ALARM_ENABLE_ABS_P_HI	0x20
#define WS_ALARM_ENABLE_REL_P_LO	0x40
#define WS_ALARM_ENABLE_REL_P_HI	0x80
// Alarm enable 2
#define WS_ALARM_ENABLE_IN_T_LO		0x01
#define WS_ALARM_ENABLE_IN_T_HI		0x02
#define WS_ALARM_ENABLE_OUT_T_LO	0x04
#define WS_ALARM_ENABLE_OUT_T_HI	0x08
#define WS_ALARM_ENABLE_WINDCHILL_LO	0x10
#define WS_ALARM_ENABLE_WINDCHILL_HI	0x20
#define WS_ALARM_ENABLE_DEWPOINT_LO	0x40
#define WS_ALARM_ENABLE_DEWPOINT_HI	0x80


// Conversion parameters for english units
// Second and optional third factor is for adapting to actual stored values
#define WS_SCALE_DEFAULT	 1.0	// No scaling
#define WS_SCALE_MS_TO_MPH	 2.2369363         * 0.1
#define WS_SCALE_C_TO_F		 1.8               * 0.1
#define WS_SCALE_CM_TO_IN	 0.39370079        * 0.1 * 0.3
#define WS_SCALE_HPA_TO_INHG	 0.029530058646697 * 0.1
#define WS_SCALE_OFFS_TO_DEGREE	22.5

#define WS_OFFSET_DEFAULT	 0.0	// No offset
#define WS_OFFSET_C_TO_F	32.0


// Table for decoding raw weather station data.
// Each key specifies a (pos, type, scale) tuple that is understood by CWS_decode().
// See http://www.jim-easterbrook.me.uk/weather/mm/ for description of data

enum ws_types {ub,sb,us,ss,dt,tt,pb,wa,wg,dp};

struct ws_record {
	char name[22];
	int pos;
	enum ws_types ws_type;
	float scale;
} ws_format[] = {
// Up to 4080 records with this format
	{"delay"	,  0, ub,  1.0}, // Minutes since last stored reading (1:240)
	{"hum_in"       ,  1, ub,  1.0}, // Indoor relative humidity %        (1:99)    , 0xFF means invalid
	{"temp_in"      ,  2, ss,  0.1}, // Multiply by 0.1 to get �C         (-40:+60) , 0xFFFF means invalid
	{"hum_out"      ,  4, ub,  1.0}, // Outdoor relative humidity %       (1:99)    , 0xFF means invalid
	{"temp_out"     ,  5, ss,  0.1}, // Multiply by 0.1 to get �C         (-40:+60) , 0xFFFF means invalid
	{"abs_pressure" ,  7, us,  0.1}, // Multiply by 0.1 to get hPa        (920:1080), 0xFFFF means invalid
	{"wind_ave"     ,  9, wa,  0.1}, // Multiply by 0.1 to get m/s        (0:50)    , 0xFF means invalid
	{"wind_gust"    , 10, wg,  0.1}, // Multiply by 0.1 to get m/s        (0:50)    , 0xFF means invalid
	// 11, wind speed, high bits     // Lower 4 bits are the average wind speed high bits, upper 4 bits are the gust wind speed high bits
	{"wind_dir"     , 12, ub, 22.5}, // Multiply by 22.5 to get � from north (0-15), 7th bit indicates invalid data
	{"rain"         , 13, us,  0.3}, // Multiply by 0.3 to get mm
	{"status"       , 15, pb,  1.0}, // 6th bit indicates loss of contact with sensors, 7th bit indicates rainfall overflow
// The lower fixed block
	{"read_period"      , 16, ub, 1.0}, // Minutes between each stored reading (1:240)
	{"units0"           , 17, ub, 1.0}, // Unit setting flags       (Bits 0,1,2,    5,6,7)
	{"units_wind_speed" , 18, ub, 1.0}, // Unit wind speed settings (Bits 0,1,2,3,4      )
	{"display_format0"  , 19, ub, 1.0}, // Unit display settings    (Bits 0,1,2,3,4,5,6,7)
	{"display_format1"  , 20, ub, 1.0}, // Unit display settings    (Bits 0,1,2,3,4,5,6,7)
	{"alarm_enable0"    , 21, ub, 1.0}, // Unit alarm settings      (Bits   1,2,  4,5,6,7)
	{"alarm_enable1"    , 22, ub, 1.0}, // Unit alarm settings      (Bits 0,1,2,3,4,5,6,7)
	{"alarm_enable2"    , 23, ub, 1.0}, // Unit alarm settings      (Bits 0,1,2,3,4,5,6,7)
	{"timezone"         , 24, sb, 1.0}, // Hours offset from Central European Time, so in the UK this should be set to -1. In stations without a radio controlled clock this is always zero. 7th bit is sign bit
	{"data_refreshed"   , 26, us, 1.0}, // PC write AA indicating setting changed, base unit clear this byte for reading back the change
	{"data_count"       , 27, us, 1.0}, // Number of stored readings. Starts at zero, rises to 4080
	{"current_pos"      , 30, us, 1.0}, // Address of the stored reading currently being created. Starts at 256, rises to 65520 in steps of 16, then loops back to 256. The data at this address is updated every 48 seconds or so, until the read period is reached. Then the address is incremented and the next record becomes current.
	{"rel_pressure"     , 32, us, 0.1}, // Current relative (sea level) atmospheric pressure, multiply by 0.1 to get hPa
	{"abs_pressure"     , 34, us, 0.1}, // Current absolute atmospheric pressure, multiply by 0.1 to get hPa
	{"date_time"        , 43, dt, 1.0}, // Current date & time
// Alarm settings
	{"alarm.hum_in.hi"       , 48, ub,  1.0}, {"alarm.hum_in.lo"       , 49, ub, 1.0}, // Indoor relative humidity %
	{"alarm.temp_in.hi"      , 50, ss,  0.1}, {"alarm.temp_in.lo"      , 52, ss, 0.1}, // Multiply by 0.1 to get �C
	{"alarm.hum_out.hi"      , 54, ub,  1.0}, {"alarm.hum_out.lo"      , 55, ub, 1.0}, // Indoor relative humidity %
	{"alarm.temp_out.hi"     , 56, ss,  0.1}, {"alarm.temp_out.lo"     , 58, ss, 0.1}, // Multiply by 0.1 to get �C
	{"alarm.windchill.hi"    , 60, ss,  0.1}, {"alarm.windchill.lo"    , 62, ss, 0.1}, // Multiply by 0.1 to get �C
	{"alarm.dewpoint.hi"     , 64, ss,  0.1}, {"alarm.dewpoint.lo"     , 66, ss, 0.1}, // Multiply by 0.1 to get �C
	{"alarm.abs_pressure.hi" , 68, ss,  0.1}, {"alarm.abs_pressure.lo" , 70, ss, 0.1}, // Multiply by 0.1 to get hPa
	{"alarm.rel_pressure.hi" , 72, ss,  0.1}, {"alarm.rel_pressure.lo" , 74, ss, 0.1}, // Multiply by 0.1 to get hPa
	{"alarm.wind_ave.bft"    , 76, ub,  1.0}, {"alarm.wind_ave.ms"     , 77, ub, 0.1}, // Multiply by 0.1 to get m/s
	{"alarm.wind_gust.bft"   , 79, ub,  1.0}, {"alarm.wind_gust.ms"    , 80, ub, 0.1}, // Multiply by 0.1 to get m/s
	{"alarm.wind_dir"        , 82, ub, 22.5},                                          // Multiply by 22.5 to get � from north
	{"alarm.rain.hour"       , 83, us,  0.3}, {"alarm.rain.day"        , 85, us, 0.3}, // Multiply by 0.3 to get mm
	{"alarm.time"            , 87, tt,  1.0},
// Maximums with timestamps
	{"max.hum_in.val"       ,  98, ub, 1.0}, {"max.hum_in.date"       , 141, dt, 1.0},
	{"max.hum_out.val"      , 100, ub, 1.0}, {"max.hum_out.date"      , 151, dt, 1.0},
	{"max.temp_in.val"      , 102, ss, 0.1}, {"max.temp_in.date"      , 161, dt, 1.0}, // Multiply by 0.1 to get �C
	{"max.temp_out.val"     , 106, ss, 0.1}, {"max.temp_out.date"     , 171, dt, 1.0}, // Multiply by 0.1 to get �C
	{"max.windchill.val"    , 110, ss, 0.1}, {"max.windchill.date"    , 181, dt, 1.0}, // Multiply by 0.1 to get �C
	{"max.dewpoint.val"     , 114, ss, 0.1}, {"max.dewpoint.date"     , 191, dt, 1.0}, // Multiply by 0.1 to get �C
	{"max.abs_pressure.val" , 118, us, 0.1}, {"max.abs_pressure.date" , 201, dt, 1.0}, // Multiply by 0.1 to get hPa
	{"max.rel_pressure.val" , 122, us, 0.1}, {"max.rel_pressure.date" , 211, dt, 1.0}, // Multiply by 0.1 to get hPa
	{"max.wind_ave.val"     , 126, us, 0.1}, {"max.wind_ave.date"     , 221, dt, 1.0}, // Multiply by 0.1 to get m/s
	{"max.wind_gust.val"    , 128, us, 0.1}, {"max.wind_gust.date"    , 226, dt, 1.0}, // Multiply by 0.1 to get m/s
	{"max.rain.hour.val"    , 130, us, 0.3}, {"max.rain.hour.date"    , 231, dt, 1.0}, // Multiply by 0.3 to get mm
	{"max.rain.day.val"     , 132, us, 0.3}, {"max.rain.day.date"     , 236, dt, 1.0}, // Multiply by 0.3 to get mm
	{"max.rain.week.val"    , 134, us, 0.3}, {"max.rain.week.date"    , 241, dt, 1.0}, // Multiply by 0.3 to get mm
	{"max.rain.month.val"   , 136, us, 0.3}, {"max.rain.month.date"   , 246, dt, 1.0}, // Multiply by 0.3 to get mm
	{"max.rain.total.val"   , 138, us, 0.3}, {"max.rain.total.date"   , 251, dt, 1.0}, // Multiply by 0.3 to get mm
// Minimums with timestamps
	{"min.hum_in.val"       ,  99, ub, 1.0}, {"min.hum_in.date"       , 146, dt, 1.0},
	{"min.hum_out.val"      , 101, ub, 1.0}, {"min.hum_out.date"      , 156, dt, 1.0},
	{"min.temp_in.val"      , 104, ss, 0.1}, {"min.temp_in.date"      , 166, dt, 1.0}, // Multiply by 0.1 to get �C
	{"min.temp_out.val"     , 108, ss, 0.1}, {"min.temp_out.date"     , 176, dt, 1.0}, // Multiply by 0.1 to get �C
	{"min.windchill.val"    , 112, ss, 0.1}, {"min.windchill.date"    , 186, dt, 1.0}, // Multiply by 0.1 to get �C
	{"min.dewpoint.val"     , 116, ss, 0.1}, {"min.dewpoint.date"     , 196, dt, 1.0}, // Multiply by 0.1 to get �C
	{"min.abs_pressure.val" , 120, us, 0.1}, {"min.abs_pressure.date" , 206, dt, 1.0}, // Multiply by 0.1 to get hPa
	{"min.rel_pressure.val" , 124, us, 0.1}, {"min.rel_pressure.date" , 216, dt, 1.0}, // Multiply by 0.1 to get hPa
// Calculated rainfall, must be calculated prior to every record
	{"rain.hour"  , WS_RAIN_HOUR , us, 0.3}, // Multiply by 0.3 to get mm
	{"rain.day"   , WS_RAIN_DAY  , us, 0.3}, // Multiply by 0.3 to get mm
	{"rain.week"  , WS_RAIN_WEEK , us, 0.3}, // Multiply by 0.3 to get mm
	{"rain.month" , WS_RAIN_MONTH, us, 0.3}  // Multiply by 0.3 to get mm
};


// Table for creating pywws format string
// Each key specifies a (pos, type, scale) tuple that is understood by CWS_decode().
// See http://www.jim-easterbrook.me.uk/weather/mm/ for description of data

struct pywws_record {
	char name[22];
	int pos;
	enum ws_types ws_type;
	float scale;
} pywws_format[] = {
// Up to 4080 records with this format
	{"delay"	,  0, ub,  1.0}, // Minutes since last stored reading (1:240)
	{"hum_in"       ,  1, ub,  1.0}, // Indoor relative humidity %        (1:99)    , 0xFF means invalid
	{"temp_in"      ,  2, ss,  0.1}, // Multiply by 0.1 to get �C         (-40:+60) , 0xFFFF means invalid
	{"hum_out"      ,  4, ub,  1.0}, // Outdoor relative humidity %       (1:99)    , 0xFF means invalid
	{"temp_out"     ,  5, ss,  0.1}, // Multiply by 0.1 to get �C         (-40:+60) , 0xFFFF means invalid
	{"abs_pressure" ,  7, us,  0.1}, // Multiply by 0.1 to get hPa        (920:1080), 0xFFFF means invalid
	{"wind_ave"     ,  9, wa,  0.1}, // Multiply by 0.1 to get m/s        (0:50)    , 0xFF means invalid
	{"wind_gust"    , 10, wg,  0.1}, // Multiply by 0.1 to get m/s        (0:50)    , 0xFF means invalid
	// 11, wind speed, high bits     // Lower 4 bits are the average wind speed high bits, upper 4 bits are the gust wind speed high bits
	{"wind_dir"     , 12, ub,  1.0}, // Multiply by 22.5 to get � from north (0-15), 7th bit indicates invalid data
	{"rain"         , 13, us,  0.3}, // Multiply by 0.3 to get mm
	{"status"       , 15, pb,  1.0}  // 6th bit indicates loss of contact with sensors, 7th bit indicates rainfall overflow
};


// Table for creating Wunderground format string
// Each key specifies a (pos, type, scale, offset) tuple that is understood by CWS_decode().
// See http://weatherstation.wunderground.com/weatherstation/updateweatherstation.php

struct wug_record {
	char name[13];
	int pos;
	enum ws_types ws_type;
	float scale;
	float offset;
} wug_format[] = {
	// (name, pos, type, scale, offset)
	// action [action=updateraw]
	// ID [ID as registered by wunderground.com]
	// PASSWORD [PASSWORD registered with this ID]
	// dateutc - [YYYY-MM-DD HH:MM:SS (mysql format)]
	{"winddir"      , WS_WIND_DIR        , ub , WS_SCALE_OFFS_TO_DEGREE , WS_OFFSET_DEFAULT},	// - [0-360]
	{"windspeedmph" , WS_WIND_AVE        , wa , WS_SCALE_MS_TO_MPH      , WS_OFFSET_DEFAULT},	// - [mph]
	{"windgustmph"  , WS_WIND_GUST       , wg , WS_SCALE_MS_TO_MPH      , WS_OFFSET_DEFAULT},	// - [windgustmph]
	{"humidity"     , WS_HUMIDITY_OUT    , ub , WS_SCALE_DEFAULT        , WS_OFFSET_DEFAULT},	// - [%]
	{"tempf"        , WS_TEMPERATURE_OUT , ss , WS_SCALE_C_TO_F         , WS_OFFSET_C_TO_F},	// - [temperature F]
	{"rainin"       , WS_RAIN_HOUR       , us , WS_SCALE_CM_TO_IN       , WS_OFFSET_DEFAULT},	// - [hourly rain in]
	{"dailyrainin"  , WS_RAIN_DAY        , us , WS_SCALE_CM_TO_IN       , WS_OFFSET_DEFAULT},	// - [daily rain in accumulated]
	{"baromin"      , WS_ABS_PRESSURE    , us , WS_SCALE_HPA_TO_INHG    , WS_OFFSET_DEFAULT},	// - [barom in]
	{"dewptf"       , 0                  , dp , WS_SCALE_C_TO_F         , WS_OFFSET_C_TO_F}		// - [dewpoint F]
	// weather - [text] -- metar style (+RA)
	// clouds - [text] -- SKC, FEW, SCT, BKN, OVC
	// softwaretype - [text] ie: vws or weatherdisplay
};


// Table for creating PWS Weather format string
// Each key specifies a (pos, type, scale, offset) tuple that is understood by CWS_decode().

struct pws_record {
	char name[13];
	int pos;
	enum ws_types ws_type;
	float scale;
	float offset;
} pws_format[] = {
	// (name, pos, type, scale, offset)
	// ID [STATIONID as registered by pwsweather.com]
	// PASSWORD [PASSWORD registered with this ID]
	// dateutc - [YYYY-MM-DD+HH%3AMM%3ASS]
	{"winddir"      , WS_WIND_DIR        , ub , WS_SCALE_OFFS_TO_DEGREE , WS_OFFSET_DEFAULT},	// - [0-360]
	{"windspeedmph" , WS_WIND_AVE        , wa , WS_SCALE_MS_TO_MPH      , WS_OFFSET_DEFAULT},	// - [mph]
	{"windgustmph"  , WS_WIND_GUST       , wg , WS_SCALE_MS_TO_MPH      , WS_OFFSET_DEFAULT},	// - [windgustmph]
	{"tempf"        , WS_TEMPERATURE_OUT , ss , WS_SCALE_C_TO_F         , WS_OFFSET_C_TO_F},	// - [temperature F]
	{"rainin"       , WS_RAIN_HOUR       , us , WS_SCALE_CM_TO_IN       , WS_OFFSET_DEFAULT},	// - [hourly rain in]
	{"dailyrainin"  , WS_RAIN_DAY        , us , WS_SCALE_CM_TO_IN       , WS_OFFSET_DEFAULT},	// - [daily rain in accumulated]
	// monthrainin - [monthly rain in accumulated]
	// yearrainin - [yearly rain in accumulated]
	{"baromin"      , WS_ABS_PRESSURE    , us , WS_SCALE_HPA_TO_INHG    , WS_OFFSET_DEFAULT},	// - [barom in]
	{"dewptf"       , 0                  , dp , WS_SCALE_C_TO_F         , WS_OFFSET_C_TO_F},	// - [dewpoint F]
	{"humidity"     , WS_HUMIDITY_OUT    , ub , WS_SCALE_DEFAULT        , WS_OFFSET_DEFAULT}	// - [%]
	// weather - [text] -- metar style (+RA)
	// solarradiation
	// UV
	// softwaretype - [text] ie: vws or weatherdisplay
	// action [action=updateraw]
	
	// http://www.pwsweather.com/pwsupdate/pwsupdate.php?ID=STATIONID&PASSWORD=password&dateutc=2000-12-01+15%3A20%3A01&winddir=225&windspeedmph=0.0&windgustmph=0.0&tempf=34.88&rainin=0.06&dailyrainin=0.06&monthrainin=1.02&yearrainin=18.26&baromin=29.49&dewptf=30.16&humidity=83&weather=OVC&solarradiation=183&UV=5.28&softwaretype=Examplever1.1&action=updateraw
};


// Weather Station properties
unsigned char m_buf[WS_BUFFER_SIZE] = {0};	// Raw WS data

time_t m_previous_timestamp = 0;		// Previous readout
time_t m_timestamp = 0;				// Current readout


// libusb structures and functions
struct usb_dev_handle *devh;
struct usb_device *dev;

void list_devices();
struct usb_device *find_device(int vendor, int product);
void print_bytes(char *address, int len);

// USB class
int CUSB_Open(int vendor, int product);
int CUSB_Close();

// Weather Station class
void CWS_Cache(char isStoring);
void CWS_print_decoded_data();

int CWS_Open();
int CWS_Close();
int CWS_Read();

unsigned short CWS_dec_ptr(unsigned short ptr);
unsigned short CWS_read_block(unsigned short ptr, char* buf);
unsigned short CWS_write_byte(unsigned short ptr, char* buf);
unsigned short CWS_write_block(unsigned short ptr, char* buf);
unsigned short CWS_read_fixed_block();	

char CWS_calculate_rain_period(char done, unsigned short pos, unsigned short begin, unsigned short end);
unsigned int CWS_calculate_rain(unsigned short current_pos, unsigned short data_count, unsigned short start);

float CWS_dew_point(char* raw, float scale, float offset);

unsigned char CWS_bcd_decode(unsigned char byte);
unsigned short CWS_unsigned_short(char* raw);
signed short CWS_signed_short(char* raw);
int CWS_decode(char* raw, enum ws_types ws_type, float scale, float offset, char* result);

// Weather File class
int CWF_Write(char arg, char* fname);
