const String FirmwareVersion = "020000";
#define HardwareVersion "NCS314 for HW 3.x"
//Format                _X.XXX_
//2.00 12.12.2020
//Modded according to personal preferences
//NIXIE CLOCK SHIELD NCS314 v 3.x by GRA & AFCH (fominalec@gmail.com)
//1.90 08.06.2020
//Fixed: GPS timezone issue: added breakTime(now(), tm) to adjustTime function at Time.cpp
//1.89 03.04.2020
//Dots sync with seconds
//1.88 26.03.2020
//GPS synchronization algorithm has been changed
//1.87 11.03.2020
//SPI MODE was changed from MODE1 to MODE3
//1.86 23.02.2020
//GPS synchronization algorithm changed
//GPS parser has been replaced by NEOGPS
//1.85.2 23.02.2020
//Added: DS3231 internal temperature sensor self test: 5 beeps if fail.
//1.85 21.02.2020
//Fixed: Bug with time zones more than +-9
//1.84 08.04.2018
//LEDs functions moved to external file
//LEDs freezing while music (or sound) played.
//SPI Setup moved driver's file
//1.83 02.08.2018 (Driver v 1.1 is required)
//Fixed: Temp. reading speed fixed
//Fixed: Dots mixed up (driver was updated to v. 1.1)
//Fixed: RGB LEDs reading from EEPROM
//Fixed: Check for entering data from GPS in range
//1.82  18.07.2018 Dual Date Format
//1.81  18.02.2018 Temp. sensor present analyze
//1.80   06.08.2017
//Added: Date and Time GPS synchronization
//1.70   30.07.2017
//Added  IR remote control support (Sony RM-X151) ("MODE", "UP", "DOWN")
//1.60   24_07_2017
//Added: Temperature reading mode in menu and slot machine transaction
//1.0.31 27_04_2017
//Added: antipoisoning effect - slot machine
//1.021 31.01.2017
//Added: time synchronizing each 10 seconds
//Fixed: not correct time reading from RTC while start up
//1.02 17.10.2016
//Fixed: RGB color controls
//Update to Arduino IDE 1.6.12 (Time.h replaced to TimeLib.h)
//1.01
//Added RGB LEDs lock(by UP and Down Buttons)
//Added Down and Up buttons pause and resume self testing
//25.09.2016 update to HW ver 1.1
//25.05.2016

//#define tubes8
#define tubes6
//#define tubes4

#include <SPI.h>
#include <Wire.h>
#include <ClickButton.h>
#include <TimeLib.h>
#include <Tone.h>
#include <EEPROM.h>
#include "doIndication314_HW3.x.h"
#include <OneWire.h>
//IR remote control /////////// START /////////////////////////////
#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)

#define GPS_SYNC_INTERVAL 1800000 // in milliseconds
unsigned long Last_Time_GPS_Sync = 0;
bool GPS_Sync_Flag = 1;
//uint32_t GPS_Sync_Interval=120000; // 2 minutes
uint32_t GPS_Sync_Interval = 60000; // 1 minutes
#include <NMEAGPS.h>
static NMEAGPS  gps;
static gps_fix  fix;

#endif


//int ModeButtonState = 0;
//int UpButtonState = 0;
//int DownButtonState = 0;

//IR remote control /////////// START /////////////////////////////

#define GPS_BUFFER_LENGTH 83

char GPS_Package[GPS_BUFFER_LENGTH];
byte GPS_position = 0;

struct GPS_DATE_TIME
{
  byte GPS_hours;
  byte GPS_minutes;
  byte GPS_seconds;
  byte GPS_day;
  byte GPS_mounth;
  int GPS_year;
  bool GPS_Valid_Data = false;
  unsigned long GPS_Data_Parsed_time;
};

GPS_DATE_TIME GPS_Date_Time;

boolean UD, LD; // DOTS control;

byte data[12];
byte addr[8];
int celsius, fahrenheit;

/*#define RedLedPin 9 //MCU WDM output for red LEDs 9-g
  #define GreenLedPin 6 //MCU WDM output for green LEDs 6-b
  #define BlueLedPin 3 //MCU WDM output for blue LEDs 3-r*/
#define pinSet A0
#define pinUp A2
#define pinDown A1
//#define pinBuzzer 2
const byte pinBuzzer = 2; // pomenyal
//#define pinUpperDots 12 //HIGH value light a dots
//#define pinLowerDots 8 //HIGH value light a dots
#define pinTemp 7
#define pinSHDN 5
#define pinMotDet 8
bool RTC_present;
#define US_DateFormat 1
#define EU_DateFormat 0
//bool DateFormat=EU_DateFormat;

OneWire ds(pinTemp);
bool TempPresent = false;
#define CELSIUS 0
#define FAHRENHEIT 1

String stringToDisplay = "000000"; // Conten of this string will be displayed on tubes (must be 6 chars length)
int menuPosition = 0;
// 0 - time
// 1 - date
// 2 - alarm
// 3 - 12/24 hours mode
// 4 - Temperature

byte blinkMask = B00000000; //bit mask for blinkin digits (1 - blink, 0 - constant light)
int blankMask = B00000000; //bit mask for digits (1 - off, 0 - on)
byte fadeFlickerMask = B00000000;

byte dotPattern = B00000000; //bit mask for separeting dots (1 - on, 0 - off)
//B10000000 - upper dots
//B01000000 - lower dots

#define DS1307_ADDRESS 0x68
byte zero = 0x00; //workaround for issue #527
int RTC_hours, RTC_minutes, RTC_seconds, RTC_day, RTC_month, RTC_year, RTC_day_of_week;

#define TimeIndex        0
#define DateIndex        1
#define AlarmIndex       2
#define hModeIndex       3
#define TemperatureIndex 4
#define SleepModeIndex   5
#define TimeZoneIndex     123
#define TimeHoursIndex   6
#define TimeMinutesIndex 7
#define TimeSecondsIndex 8
#define DateFormatIndex  9
#define DateDayIndex     10
#define DateMonthIndex   11
#define DateYearIndex    12
#define AlarmHourIndex   13
#define AlarmMinuteIndex 14
#define AlarmSecondIndex 15
#define Alarm01          16
#define hModeValueIndex  17
#define DegreesFormatIndex 18
#define HoursOffsetIndex 19
#define SleepEnableIndex 20
#define OffHourIndex     21
#define OffMinuteIndex   22
#define OnHourIndex      23
#define OnMinuteIndex    24


#define FirstParent      TimeIndex
#define LastParent       SleepModeIndex
#define SettingsCount    (OnMinuteIndex+1)
#define NoParent         0
#define NoChild          0

//-------------------------------0--------1--------2-------3--------4--------5--------6--------7--------8--------9----------10-------11---------12---------13-------14-------15---------16---------17--------18----------19----------20----------21----------22----------23---------24
//                     names:  Time,    Date,   Alarm,  12/24  Temperature,Sleep,   hours,  mintues, seconds, DateFormat,   day,    month,     year,      hour,   minute,   second    alarm01  hour_format Deg.FormIndex HoursOffset Enable     OffHH      OffMM       OnHH        OnMM
byte parent[SettingsCount] = {NoParent, NoParent, NoParent, NoParent, NoParent, NoParent, 1,    1,       1,       2,         2,       2,         2,         3,       3,       3,       3,           4,        5,          6,           6,          6,          6,          6,          6};
byte firstChild[SettingsCount] = {6,       9,       13,     17,      18,      20,      0,       0,       0,    NoChild,      0,       0,         0,         0,       0,       0,       0,           0,        0,          0,           0,          0,          0,          0,          0};
byte lastChild[SettingsCount] = { 8,      12,       16,     17,      18,      24,      0,       0,       0,    NoChild,      0,       0,         0,         0,       0,       0,       0,           0,        0,          0,           0,          0,          0,          0,          0};
byte value[SettingsCount] = {     0,       0,       0,      0,       0,       0,       0,       0,       0,  EU_DateFormat,  0,       0,         0,         0,       0,       0,       0,           24,       0,          2,           1,          1,          0,          7,          0};
byte maxValue[SettingsCount] = {  0,       0,       0,      0,       0,       0,       23,      59,      59, US_DateFormat,  31,      12,        99,       23,      59,      59,       1,           24,   FAHRENHEIT,     14,           1,          23,         59,         23,         59};
byte minValue[SettingsCount] = {  0,       0,       0,      12,      0,       0,       00,      00,      00, EU_DateFormat,  1,       1,         00,       00,      00,      00,       0,           12,    CELSIUS,       -12,          0,           0,          0,         0,           0 };
byte blinkPattern[SettingsCount] = {
  B00000000, //0
  B00000000, //1
  B00000000, //2
  B00000000, //3
  B00000000, //4
  B00000000, //5
  B00000011, //6
  B00001100, //7
  B00110000, //8
  B00111111, //9
  B00000011, //10
  B00001100, //11
  B00110000, //12
  B00000011, //13
  B00001100, //14
  B00110000, //15
  B11000000, //16
  B00001100, //17
  B00111111, //18
  B00000011, //19
  B00001100, //20
  B00000011, //21
  B00001100, //22
  B00000011, //23
  B00001100, //24
};

bool TubesOn = true;
bool fadeIn, fadeOut;

bool editMode = false;

// Motion detection / auto sleep
#define noMotionSleepInterval 3600000 //3600000
unsigned long lastMotionMillis;

long downTime = 0;
long upTime = 0;
const long settingDelay = 250;
bool BlinkUp = false;
bool BlinkDown = false;
unsigned long lastMenuActionTime = 0;
unsigned long anyClickTime = 0;
bool RGBLedsOn = true;
#define RGBLEDsEEPROMAddress 0
#define HourFormatEEPROMAddress 1
#define AlarmTimeEEPROMAddress 2 //3,4,5
#define AlarmArmedEEPROMAddress 6
#define LEDsLockEEPROMAddress 7
#define LEDsRedValueEEPROMAddress 8
#define LEDsGreenValueEEPROMAddress 9
#define LEDsBlueValueEEPROMAddress 10
#define DegreesFormatEEPROMAddress 11
#define HoursOffsetEEPROMAddress 12
#define DateFormatEEPROMAddress 13
#define SleepModeEEPROMAddress 14 // 14-18

//buttons pins declarations
ClickButton setButton(pinSet, LOW, CLICKBTN_PULLUP);
ClickButton upButton(pinUp, LOW, CLICKBTN_PULLUP);
ClickButton downButton(pinDown, LOW, CLICKBTN_PULLUP);
bool upButtonHold, downButtonHold;
///////////////////

Tone tone1;
#define isdigit(n) (n >= '0' && n <= '9')
//char *song = "MissionImp:d=16,o=6,b=95:32d,32d#,32d,32d#,32d,32d#,32d,32d#,32d,32d,32d#,32e,32f,32f#,32g,g,8p,g,8p,a#,p,c7,p,g,8p,g,8p,f,p,f#,p,g,8p,g,8p,a#,p,c7,p,g,8p,g,8p,f,p,f#,p,a#,g,2d,32p,a#,g,2c#,32p,a#,g,2c,a#5,8c,2p,32p,a#5,g5,2f#,32p,a#5,g5,2f,32p,a#5,g5,2e,d#,8d";
//char *song = "PinkPanther:d=4,o=5,b=160:8d#,8e,2p,8f#,8g,2p,8d#,8e,16p,8f#,8g,16p,8c6,8b,16p,8d#,8e,16p,8b,2a#,2p,16a,16g,16e,16d,2e";
//char *song="VanessaMae:d=4,o=6,b=70:32c7,32b,16c7,32g,32p,32g,32p,32d#,32p,32d#,32p,32c,32p,32c,32p,32c7,32b,16c7,32g#,32p,32g#,32p,32f,32p,16f,32c,32p,32c,32p,32c7,32b,16c7,32g,32p,32g,32p,32d#,32p,32d#,32p,32c,32p,32c,32p,32g,32f,32d#,32d,32c,32d,32d#,32c,32d#,32f,16g,8p,16d7,32c7,32d7,32a#,32d7,32a,32d7,32g,32d7,32d7,32p,32d7,32p,32d7,32p,16d7,32c7,32d7,32a#,32d7,32a,32d7,32g,32d7,32d7,32p,32d7,32p,32d7,32p,32g,32f,32d#,32d,32c,32d,32d#,32c,32d#,32f,16c";
//char *song="DasBoot:d=4,o=5,b=100:d#.4,8d4,8c4,8d4,8d#4,8g4,a#.4,8a4,8g4,8a4,8a#4,8d,2f.,p,f.4,8e4,8d4,8e4,8f4,8a4,c.,8b4,8a4,8b4,8c,8e,2g.,2p";
//char *song="Scatman:d=4,o=5,b=200:8b,16b,32p,8b,16b,32p,8b,2d6,16p,16c#.6,16p.,8d6,16p,16c#6,8b,16p,8f#,2p.,16c#6,8p,16d.6,16p.,16c#6,16b,8p,8f#,2p,32p,2d6,16p,16c#6,8p,16d.6,16p.,16c#6,16a.,16p.,8e,2p.,16c#6,8p,16d.6,16p.,16c#6,16b,8p,8b,16b,32p,8b,16b,32p,8b,2d6,16p,16c#.6,16p.,8d6,16p,16c#6,8b,16p,8f#,2p.,16c#6,8p,16d.6,16p.,16c#6,16b,8p,8f#,2p,32p,2d6,16p,16c#6,8p,16d.6,16p.,16c#6,16a.,16p.,8e,2p.,16c#6,8p,16d.6,16p.,16c#6,16a,8p,8e,2p,32p,16f#.6,16p.,16b.,16p.";
//char *song="Popcorn:d=4,o=5,b=160:8c6,8a#,8c6,8g,8d#,8g,c,8c6,8a#,8c6,8g,8d#,8g,c,8c6,8d6,8d#6,16c6,8d#6,16c6,8d#6,8d6,16a#,8d6,16a#,8d6,8c6,8a#,8g,8a#,c6";
//char *song="WeWishYou:d=4,o=5,b=200:d,g,8g,8a,8g,8f#,e,e,e,a,8a,8b,8a,8g,f#,d,d,b,8b,8c6,8b,8a,g,e,d,e,a,f#,2g,d,g,8g,8a,8g,8f#,e,e,e,a,8a,8b,8a,8g,f#,d,d,b,8b,8c6,8b,8a,g,e,d,e,a,f#,1g,d,g,g,g,2f#,f#,g,f#,e,2d,a,b,8a,8a,8g,8g,d6,d,d,e,a,f#,2g";
char *song = "d=4";
#define OCTAVE_OFFSET 0
char *p;

int notes[] = { 0,
                NOTE_C4, NOTE_CS4, NOTE_D4, NOTE_DS4, NOTE_E4, NOTE_F4, NOTE_FS4, NOTE_G4, NOTE_GS4, NOTE_A4, NOTE_AS4, NOTE_B4,
                NOTE_C5, NOTE_CS5, NOTE_D5, NOTE_DS5, NOTE_E5, NOTE_F5, NOTE_FS5, NOTE_G5, NOTE_GS5, NOTE_A5, NOTE_AS5, NOTE_B5,
                NOTE_C6, NOTE_CS6, NOTE_D6, NOTE_DS6, NOTE_E6, NOTE_F6, NOTE_FS6, NOTE_G6, NOTE_GS6, NOTE_A6, NOTE_AS6, NOTE_B6,
                NOTE_C7, NOTE_CS7, NOTE_D7, NOTE_DS7, NOTE_E7, NOTE_F7, NOTE_FS7, NOTE_G7, NOTE_GS7, NOTE_A7, NOTE_AS7, NOTE_B7
              };

int fireforks[] = {0, 0, 1, //1
                   -1, 0, 0, //2
                   0, 1, 0, //3
                   0, 0, -1, //4
                   1, 0, 0, //5
                   0, -1, 0
                  }; //array with RGB rules (0 - do nothing, -1 - decrese, +1 - increse

void setRTCDateTime(byte h, byte m, byte s, byte d, byte mon, byte y, byte w = 1);

//int functionDownButton = 0;
//int functionUpButton = 0;
bool LEDsLock = false;

//antipoisoning transaction
bool modeChangedByUser = false;
bool transactionInProgress = false; //antipoisoning transaction
#define timeModePeriod 60000
#define dateModePeriod 5000
long modesChangePeriod = timeModePeriod;
//end of antipoisoning transaction



extern const int LEDsDelay;


/*******************************************************************************************************
  Init Programm
*******************************************************************************************************/
void setup()
{
  Wire.begin();
  //setRTCDateTime(23,40,00,25,7,15,1);

  Serial.begin(115200);
#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
  Serial1.begin(9600);
#endif


  if (EEPROM.read(HourFormatEEPROMAddress) != 12) value[hModeValueIndex] = 24; else value[hModeValueIndex] = 12;
  if (EEPROM.read(RGBLEDsEEPROMAddress) != 0) RGBLedsOn = true; else RGBLedsOn = false;
  if (EEPROM.read(AlarmTimeEEPROMAddress) == 255) value[AlarmHourIndex] = 0; else value[AlarmHourIndex] = EEPROM.read(AlarmTimeEEPROMAddress);
  if (EEPROM.read(AlarmTimeEEPROMAddress + 1) == 255) value[AlarmMinuteIndex] = 0; else value[AlarmMinuteIndex] = EEPROM.read(AlarmTimeEEPROMAddress + 1);
  if (EEPROM.read(AlarmTimeEEPROMAddress + 2) == 255) value[AlarmSecondIndex] = 0; else value[AlarmSecondIndex] = EEPROM.read(AlarmTimeEEPROMAddress + 2);
  if (EEPROM.read(AlarmArmedEEPROMAddress) == 255) value[Alarm01] = 0; else value[Alarm01] = EEPROM.read(AlarmArmedEEPROMAddress);
  if (EEPROM.read(LEDsLockEEPROMAddress) == 255) LEDsLock = false; else LEDsLock = EEPROM.read(LEDsLockEEPROMAddress);
  if (EEPROM.read(DegreesFormatEEPROMAddress) == 255) value[DegreesFormatIndex] = CELSIUS; else value[DegreesFormatIndex] = EEPROM.read(DegreesFormatEEPROMAddress);
  if (EEPROM.read(HoursOffsetEEPROMAddress) == 255) value[HoursOffsetIndex] = value[HoursOffsetIndex]; else value[HoursOffsetIndex] = EEPROM.read(HoursOffsetEEPROMAddress) + minValue[HoursOffsetIndex];
  if (EEPROM.read(DateFormatEEPROMAddress) == 255) value[DateFormatIndex] = value[DateFormatIndex]; else value[DateFormatIndex] = EEPROM.read(DateFormatEEPROMAddress);

  if (EEPROM.read(SleepModeEEPROMAddress) == 255)     value[SleepEnableIndex] = 0; else value[SleepEnableIndex] = EEPROM.read(SleepModeEEPROMAddress);
  if (EEPROM.read(SleepModeEEPROMAddress + 1) == 255) value[OffHourIndex] = 0; else value[OffHourIndex] = EEPROM.read(SleepModeEEPROMAddress + 1);
  if (EEPROM.read(SleepModeEEPROMAddress + 2) == 255) value[OffMinuteIndex] = 0; else value[OffMinuteIndex] = EEPROM.read(SleepModeEEPROMAddress + 2);
  if (EEPROM.read(SleepModeEEPROMAddress + 3) == 255) value[OnHourIndex] = 0; else value[OnHourIndex] = EEPROM.read(SleepModeEEPROMAddress + 3);
  if (EEPROM.read(SleepModeEEPROMAddress + 4) == 255) value[OnMinuteIndex] = 0; else value[OnMinuteIndex] = EEPROM.read(SleepModeEEPROMAddress + 4);


  //Serial.print(F("led lock="));
  //Serial.println(LEDsLock);

  /*  pinMode(RedLedPin, OUTPUT);
    pinMode(GreenLedPin, OUTPUT);
    pinMode(BlueLedPin, OUTPUT);*/

  tone1.begin(pinBuzzer);
  song = parseSong(song);

  pinMode(pinMotDet, INPUT);
  pinMode(LEpin, OUTPUT);
  pinMode(pinSHDN, OUTPUT);
  digitalWrite(pinSHDN, HIGH); //HIGH = ON


  // SPI setup
  SPISetup();
  LEDsSetup();


  //buttons pins inits
  pinMode(pinSet,  INPUT_PULLUP);
  pinMode(pinUp,  INPUT_PULLUP);
  pinMode(pinDown,  INPUT_PULLUP);
  ////////////////////////////
  pinMode(pinBuzzer, OUTPUT);

  //buttons objects inits
  setButton.debounceTime   = 20;   // Debounce timer in ms
  setButton.multiclickTime = 30;  // Time limit for multi clicks
  setButton.longClickTime  = 2000; // time until "held-down clicks" register

  upButton.debounceTime   = 20;   // Debounce timer in ms
  upButton.multiclickTime = 30;  // Time limit for multi clicks
  upButton.longClickTime  = 2000; // time until "held-down clicks" register

  downButton.debounceTime   = 20;   // Debounce timer in ms
  downButton.multiclickTime = 30;  // Time limit for multi clicks
  downButton.longClickTime  = 2000; // time until "held-down clicks" register

  //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  doTest();
  testDS3231TempSensor();
  //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  if (LEDsLock == 1)
  {
    setLEDsFromEEPROM();
  }
  getRTCTime();
  byte prevSeconds = RTC_seconds;
  unsigned long RTC_ReadingStartTime = millis();
  RTC_present = true;
  while (prevSeconds == RTC_seconds)
  {
    getRTCTime();
    //Serial.println(RTC_seconds);
    if ((millis() - RTC_ReadingStartTime) > 3000)
    {
      Serial.println(F("Warning! RTC DON'T RESPOND!"));
      RTC_present = false;
      break;
    }
  }
  setTime(RTC_hours, RTC_minutes, RTC_seconds, RTC_day, RTC_month, RTC_year);

#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
  irrecv.blink13(false);
  irrecv.enableIRIn(); // Start the receiver
#endif


  // Init no motion sleep
  lastMotionMillis = millis();

}

int rotator = 0; //index in array with RGB "rules" (increse by one on each 255 cycles)
int cycle = 0; //cycles counter

struct RGB {
  byte R = 255;
  byte G = 0;
  byte B = 0;
};
RGB rgbRotator;
RGB rgbFader;

unsigned long prevTime = 0; // time of lase tube was lit
unsigned long prevTime4FireWorks = 0; //time of last RGB changed
//int minuteL=0; //младшая цифра минут


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/***************************************************************************************************************
  MAIN Programm
***************************************************************************************************************/
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void loop() {

  // Synchronize with RTC every 10 seconds
  if (((millis() % 10000) == 0) && (RTC_present)) 
  {
    getRTCTime();
    setTime(RTC_hours, RTC_minutes, RTC_seconds, RTC_day, RTC_month, RTC_year);
    //Serial.println(F("Sync"));
  }

  // GPS
#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)

  if (gps.available( Serial1 )) fix = gps.read();
  else fix.valid.time = false;
  if (((millis()) - Last_Time_GPS_Sync) > GPS_Sync_Interval)
  {
    GPS_Sync_Interval = GPS_SYNC_INTERVAL;
    GPS_Sync_Flag = 0;
    //Serial.println(F("Attempt to sync with GPS"));
  }
  if (GPS_Sync_Flag == 0) GPSCheckValidity();

  /*
    IRresults.value = 0;
    if (irrecv.decode(&IRresults)) {
     Serial.println(IRresults.value, HEX);
     irrecv.resume(); // Receive the next value
    }

    ModeButtonState = IRModeButton.checkButtonState(IRresults.value);
    if (ModeButtonState == 1) Serial.println(F("Mode short"));
    if (ModeButtonState == -1) Serial.println(F("Mode long...."));

    UpButtonState = IRUpButton.checkButtonState(IRresults.value);
    if (UpButtonState == 1) Serial.println(F("Up short"));
    if (UpButtonState == -1) Serial.println(F("Up long...."));

    DownButtonState = IRDownButton.checkButtonState(IRresults.value);
    if (DownButtonState == 1) Serial.println(F("Down short"));
    if (DownButtonState == -1) Serial.println(F("Down long...."));
    #else
    ModeButtonState = 0;
    UpButtonState = 0;
    DownButtonState = 0;
  */
#endif


  // Jukebox from hell
  p = playmusic(p);


  // Cycle RGB lights
  if ((millis() - prevTime4FireWorks) > LEDsDelay)
  {
    rotateFireWorks(); //change color (by 1 step)
    prevTime4FireWorks = millis();
  }


  //////////////////////////////////////////////////////////////////////////////////////////
  // Not in menu. Display clock normally and take care of anti-poisoning
  //////////////////////////////////////////////////////////////////////////////////////////
  if ((menuPosition == TimeIndex) || (modeChangedByUser == false) ) antiPoisoning4();


  //////////////////////////////
  // Update tubes
  //////////////////////////////
  doIndication();


  //////////////////////////////
  // Update button statutes
  //////////////////////////////
  setButton.Update();
  upButton.Update();
  downButton.Update();
  if (downButton.clicks < 0) {
    downButtonHold = true;  
    tone1.play(1000, 100);
  } else if (!downButton.depressed)  downButtonHold = false;

  if (upButton.clicks < 0) {
    upButtonHold = true;
    tone1.play(1000, 100);
  }else if (!upButton.depressed)  upButtonHold = false;


  //////////////////////////////
  // Edit mode / other than time display timeout (revert to time display always automatically)
  //////////////////////////////  
  if ((editMode || (menuPosition != TimeIndex)) && (millis() - lastMenuActionTime) > 60000)  {
    editMode = false;
    menuPosition = TimeIndex;
    blinkMask = B00000000;
  }
  //  Disable blinks if not editing
  if (!editMode) blinkMask = B00000000;   


  //////////////////////////////
  // Mode button press
  //////////////////////////////
  if ((setButton.clicks > 0))  //short click
  {
    TubesOn = true;
    modeChangedByUser = true;
    p = 0; //shut off music 
    tone1.play(1000, 100);
    lastMenuActionTime = millis();
    anyClickTime = millis();
    /*if (value[DateFormatIndex] == US_DateFormat)
      {
      //if (menuPosition == )
      } else */
    menuPosition = menuPosition + 1;
#if defined (__AVR_ATmega328P__)

    if (menuPosition == TimeZoneIndex) menuPosition++;// skip TimeZone for Arduino Uno 
    if (menuPosition == AlarmIndex) menuPosition++;// skip Alarm menu too. We rather sleep.
#endif
    if (menuPosition == LastParent + 1) menuPosition = TimeIndex; // <-------------------------- END OF THE MENU HERE
    /*Serial.print(F("menuPosition="));
      Serial.println(menuPosition);
      Serial.print(F("value="));
      Serial.println(value[menuPosition]);*/

    blinkMask = blinkPattern[menuPosition];


    //////////////////////////////
    // Exit edit mode
    //////////////////////////////

    // we are in editMode: if not in the parent menu AND the previous menuposition was the last child within the sub menu, exit from edit mode
    if ((parent[menuPosition - 1] != 0) and (lastChild[parent[menuPosition - 1] - 1] == (menuPosition - 1)))
    {
      if ((parent[menuPosition - 1] - 1 == 1) && (!isValidDate()))
      {
        menuPosition = DateDayIndex;
        return;
      }

      // Exit edit mode
      editMode = false;

      // Set menu back to parent position
      menuPosition = parent[menuPosition - 1] - 1;

      // Update time if we were editing it
      if (menuPosition == TimeIndex) setTime(value[TimeHoursIndex], value[TimeMinutesIndex], value[TimeSecondsIndex], day(), month(), year());

      // Update date if we were editing it
      if (menuPosition == DateIndex)
      {
        Serial.print(F("Day:"));
        Serial.println(value[DateDayIndex]);
        Serial.print(F("Month:"));
        Serial.println(value[DateMonthIndex]);
        setTime(hour(), minute(), second(), value[DateDayIndex], value[DateMonthIndex], 2000 + value[DateYearIndex]);
        EEPROM.write(DateFormatEEPROMAddress, value[DateFormatIndex]);
      }

      // Update Alarm if we were editing it
      if (menuPosition == AlarmIndex) {
        EEPROM.write(AlarmTimeEEPROMAddress, value[AlarmHourIndex]);
        EEPROM.write(AlarmTimeEEPROMAddress + 1, value[AlarmMinuteIndex]);
        EEPROM.write(AlarmTimeEEPROMAddress + 2, value[AlarmSecondIndex]);
        EEPROM.write(AlarmArmedEEPROMAddress, value[Alarm01]);
      };

      // Update hour mode if we were editing it
      if (menuPosition == hModeIndex) EEPROM.write(HourFormatEEPROMAddress, value[hModeValueIndex]);

      // Update temperature if we were editing it
      if (menuPosition == TemperatureIndex)
      {
        EEPROM.write(DegreesFormatEEPROMAddress, value[DegreesFormatIndex]);
      }

      // Update sleep mode if we were editing it
      if (menuPosition == SleepModeIndex) {

        EEPROM.write(SleepModeEEPROMAddress, value[SleepEnableIndex]);
        EEPROM.write(SleepModeEEPROMAddress + 1, value[OffHourIndex]);
        EEPROM.write(SleepModeEEPROMAddress + 2, value[OffMinuteIndex]);
        EEPROM.write(SleepModeEEPROMAddress + 3, value[OnHourIndex]);
        EEPROM.write(SleepModeEEPROMAddress + 4, value[OnMinuteIndex]);
      }

      // Update time zone if we were editing it
      // if (menuPosition == TimeZoneIndex) EEPROM.write(HoursOffsetEEPROMAddress, value[HoursOffsetIndex] - minValue[HoursOffsetIndex]);

      // Update RTC time always (????)
      setRTCDateTime(hour(), minute(), second(), day(), month(), year() % 1000, 1);
      return;
    } //end exit from edit mode


    /*Serial.print("menu pos=");
      Serial.println(menuPosition);
      Serial.print("DateFormat");
      Serial.println(value[DateFormatIndex]);*/

    //????????????????????????????????????????????????????????
    // Some weird shit happening here???

    if ((menuPosition != HoursOffsetIndex) &&
        (menuPosition != DateFormatIndex) &&
        (menuPosition != DateDayIndex) &&
        (menuPosition != OnMinuteIndex)) {
      ;//  value[menuPosition] = extractDigits(blinkMask);
    }

  }



  ///////////////////////
  // Long mode button press
  ///////////////////////
  if ((setButton.clicks < 0))// || (ModeButtonState == -1)) //long click
  {
    tone1.play(1000, 100);
    anyClickTime = millis();

    TubesOn = true;

    if (!editMode)
    {
      lastMenuActionTime = millis();
      if (menuPosition == TimeIndex) {
        value[TimeHoursIndex] = hour();
        value[TimeMinutesIndex] = minute();
        value[TimeSecondsIndex] = second();
        stringToDisplay = PreZero(hour()) + PreZero(minute()) + PreZero(second()); //temporary enabled 24 hour format while settings
      }
    }

    if (menuPosition == DateIndex)
    {
      // Serial.println("DateEdit");
      value[DateDayIndex] =  day();
      value[DateMonthIndex] = month();
      value[DateYearIndex] = year() % 1000;
      if (value[DateFormatIndex] == EU_DateFormat) stringToDisplay = PreZero(int(value[DateDayIndex])) + PreZero(int(value[DateMonthIndex])) + PreZero(int(value[DateYearIndex]));
      else stringToDisplay = PreZero(int(value[DateMonthIndex])) + PreZero(int(value[DateDayIndex])) + PreZero(int(value[DateYearIndex]));
      //Serial.print("str=");
      // Serial.println(stringToDisplay);
    }
    menuPosition = firstChild[menuPosition];


    if (menuPosition == AlarmHourIndex) {
      value[Alarm01] = 1; /*digitalWrite(pinUpperDots, HIGH);*/dotPattern = B10000000;
    }

    editMode = !editMode;
    Serial.print(editMode);

    blinkMask = blinkPattern[menuPosition];
    if ((menuPosition != DegreesFormatIndex) &&
        (menuPosition != HoursOffsetIndex) &&
        (menuPosition != DateFormatIndex))
      value[menuPosition] = extractDigits(blinkMask);
    /*Serial.print(F("menuPosition="));
      Serial.println(menuPosition);
      Serial.print(F("value="));
      Serial.println(value[menuPosition]);  */
  }

  //if (upButton.clicks != 0) functionUpButton = upButton.clicks;


  ///////////////////////
  // Up button press
  ///////////////////////
  if (upButton.clicks > 0)
  {
    modeChangedByUser = true;
    p = 0; //shut off music )))
    tone1.play(1000, 100);
    anyClickTime = millis();

    TubesOn = true;
    incrementValue();
    if (!editMode)
    {
      LEDsLock = false;
      EEPROM.write(LEDsLockEEPROMAddress, 0);
    }
  }

  if (upButtonHold)
  {
    BlinkUp = false;
    if (editMode == true)
    {
      if ( (millis() - upTime) > settingDelay)
      {
        upTime = millis();
        incrementValue();
      }
    }
  } else BlinkUp = true;

  //if (downButton.clicks != 0) functionDownButton = downButton.clicks;



  ///////////////////////
  // Down button press
  ///////////////////////
  if (downButton.clicks > 0)
  {
    modeChangedByUser = true;
    p = 0; //shut off music )))
    tone1.play(1000, 100);
    anyClickTime = millis();

    TubesOn = true;
    dicrementValue();
    if (!editMode)
    {
      LEDsLock = true;
      EEPROM.write(LEDsLockEEPROMAddress, 1);
      EEPROM.write(LEDsRedValueEEPROMAddress, rgbRotator.R);
      EEPROM.write(LEDsGreenValueEEPROMAddress, rgbRotator.G);
      EEPROM.write(LEDsBlueValueEEPROMAddress, rgbRotator.B);
      /*Serial.println(F("Store to EEPROM:"));
        Serial.print(F("RED="));
        Serial.println(RedLight);
        Serial.print(F("GREEN="));
        Serial.println(GreenLight);
        Serial.print(F("Blue="));
        Serial.println(BlueLight);*/
    }
  }

  if (downButtonHold)
  {
    Serial.println(F("Long press"));
    BlinkDown = false;
    if (editMode)
    {
      if ( (millis() - downTime) > settingDelay)
      {
        downTime = millis();
        dicrementValue();
      }
    }
  } else BlinkDown = true;


  /////////////////////////////
  // RGB leds on / off
  /////////////////////////////
  if (!editMode)
  {
    // RGB on
    if ((upButton.clicks < 0))// || (UpButtonState == -1))
    {
      tone1.play(1000, 100);
      RGBLedsOn = true;
      EEPROM.write(RGBLEDsEEPROMAddress, 1);
      Serial.println(F("RGB=on"));
      setLEDsFromEEPROM();
    }
    // RGB off
    if ((downButton.clicks < 0))// || (DownButtonState == -1))
    {
      tone1.play(1000, 100);
      RGBLedsOn = false;
      EEPROM.write(RGBLEDsEEPROMAddress, 0);
      Serial.println(F("RGB=off"));
    }
  }


  static bool updateDateTime = false;
  switch (menuPosition)
  {
    case TimeIndex: //time mode
      if (!transactionInProgress) stringToDisplay = updateDisplayString();
      doDotBlink();
      checkAlarmTime();
      blankMask = B00000000;
      break;

    case DateIndex: //date mode
      if (!transactionInProgress) stringToDisplay = updateDateString();
      dotPattern = B01000000; //turn on lower dots
      checkAlarmTime();
      blankMask = B00000000;
      break;

    case AlarmIndex: //alarm mode
      stringToDisplay = PreZero(int(value[AlarmHourIndex])) + PreZero(int(value[AlarmMinuteIndex])) + PreZero(int(value[AlarmSecondIndex]));
      blankMask = B00000000;
      if (value[Alarm01] == 1) dotPattern = B11000000; //turn on upper dots
      else dotPattern = B00000000;

      checkAlarmTime();
      break;

    case hModeIndex: //12/24 hours mode
      stringToDisplay = "00" + String(value[hModeValueIndex]) + "00";
      blankMask = B00110011;
      dotPattern = B00000000; //turn off all dots
      checkAlarmTime();
      break;

    case TemperatureIndex: //missed break

    case DegreesFormatIndex:
      if (!transactionInProgress)
      {
        stringToDisplay = updateTemperatureString(getTemperature(value[DegreesFormatIndex]));
        if (value[DegreesFormatIndex] == CELSIUS)
        {
          blankMask = B00110001;
          dotPattern = B01000000;
        }
        else
        {
          blankMask = B00100011;
          dotPattern = B00000000;
        }
      }

      if (getTemperature(value[DegreesFormatIndex]) < 0) dotPattern |= B10000000;
      else dotPattern &= B01111111;
      break;

    //case TimeZoneIndex:

    // 6
    case SleepModeIndex:

      stringToDisplay = "00" + PreZero(int(value[SleepEnableIndex])) + "00";

      blankMask = B00000000;
      dotPattern = B11000000; //turn on all dots
      break;

    // 21
    case OffHourIndex:
      stringToDisplay = PreZero((int)value[OffHourIndex]) + PreZero((int)value[OffMinuteIndex]) + "00";
      blankMask = B00000000;
      dotPattern = B00000000; //turn off all dots
      break;

    // 23
    case OnHourIndex:
      stringToDisplay = PreZero((int)value[OnHourIndex]) + PreZero((int)value[OnMinuteIndex]) + "00";
      blankMask = B00000000;
      dotPattern = B11000000; //turn on all dots
      break;

    case HoursOffsetIndex:
      stringToDisplay = String(PreZero(int(value[HoursOffsetIndex]))) + "0000";
      blankMask = B00001111;
      if (value[HoursOffsetIndex] >= 0) dotPattern = B00000000; //turn off all dots
      else dotPattern = B10000000; //turn on upper dots
      break;

    case DateFormatIndex:
      if (value[DateFormatIndex] == EU_DateFormat)
      {
        stringToDisplay = "311299";
        blinkPattern[DateDayIndex] = B00000011;
        blinkPattern[DateMonthIndex] = B00001100;
      }
      else
      {
        stringToDisplay = "123199";
        blinkPattern[DateDayIndex] = B00001100;
        blinkPattern[DateMonthIndex] = B00000011;
      }
      break;

    case DateDayIndex:
    case DateMonthIndex:
    case DateYearIndex:
      if (value[DateFormatIndex] == EU_DateFormat) stringToDisplay = PreZero(int(value[DateDayIndex])) + PreZero(int(value[DateMonthIndex])) + PreZero(int(value[DateYearIndex]));
      else stringToDisplay = PreZero(int(value[DateMonthIndex])) + PreZero(int(value[DateDayIndex])) + PreZero(int(value[DateYearIndex]));
      break;

  }
  //  IRresults.value=0;


  SleepMode();
}

bool darknessIsHere;
void Darkness(bool letItFall) {

  if (letItFall && darknessIsHere == false) {

    //TubesOn = false;
    fadeOut = true;
    darknessIsHere = true;

  } else if (!letItFall && darknessIsHere == true) {

    //TubesOn = true;
    fadeIn = true;
    darknessIsHere = false;

  }
}




/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////
//////////    Sleep between defined hours
/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////

void SleepMode() {
  static bool NightMode = false;
 
  // Night mode, I should be sleeping?
  if (NowIsBetween((int)value[OffHourIndex], (int)value[OffMinuteIndex], (int)value[OnHourIndex], (int)value[OnMinuteIndex])) {

    if (!darknessIsHere && !NightMode && !editMode) {
      Darkness(true);
      NightMode = true;
    }

  } else {

    NightMode = false;

  }


  // Motion detection
  if (digitalRead(pinMotDet) && !NightMode) {
    lastMotionMillis = millis();
    Darkness(false);
  }

  if (TubesOn && !editMode) {
    if (millis() - lastMotionMillis > noMotionSleepInterval) {
      Darkness(true);
    }
  }


}


// Current time is between two given times (t_0 <= t_current < t_1)
bool NowIsBetween(int h0, int m0, int h1, int m1) {
  return TimeDiff(h0, m0, hour(), minute()) < TimeDiff(h0, m0, h1, m1);
}


// Calculate minutes between two times takin day change into account
int TimeDiff(int h0, int m0, int h1, int m1) {
  int dh = h1 - h0;
  if (dh < 0) dh += 24;
  int dm = m1 - m0;
  if (dm < 0)  {
    dm += 60;
    dh -= 1;
  }
  return dh * 60 + dm;
}



String PreZero(int digit)
{
  digit = abs(digit);
  if (digit < 10) return String("0") + String(digit);
  else return String(digit);
}

String updateDisplayString()
{
  static int prevS = -1;

  if (second() != prevS)
  {
    prevS = second();
    return getTimeNow();
  } else return stringToDisplay;
}

String getTimeNow()
{
  if (value[hModeValueIndex] == 24) return PreZero(hour()) + PreZero(minute()) + PreZero(second());
  else return PreZero(hourFormat12()) + PreZero(minute()) + PreZero(second());
}

void doTest()
{
  Serial.print(F("Firmware version: "));
  Serial.println(FirmwareVersion.substring(1, 2) + "." + FirmwareVersion.substring(2, 5));
  Serial.println(HardwareVersion);
  Serial.println(F("Start Test"));

  // VMa: Removed song blimblom
  //p = song;
  p = 0;
  //parseSong(p);
  //p=0; //need to be deleted

  LEDsTest();

#ifdef tubes8
  String testStringArray[11] = {"00000000", "11111111", "22222222", "33333333", "44444444", "55555555", "66666666", "77777777", "88888888", "99999999", ""};
  testStringArray[10] = FirmwareVersion + "00";
#endif
#ifdef tubes6
  String testStringArray[11] = {"000000", "111111", "222222", "333333", "444444", "555555", "666666", "777777", "888888", "999999", ""};
  testStringArray[10] = FirmwareVersion;
#endif



  int dlay = 500;
  bool test = 1;
  byte strIndex = -1;
  unsigned long startOfTest = millis() + 1000; //disable delaying in first iteration
  bool digitsLock = false;
  while (test)
  {
    if (digitalRead(pinDown) == 0) digitsLock = true;
    if (digitalRead(pinUp) == 0) digitsLock = false;

    if ((millis() - startOfTest) > dlay)
    {
      startOfTest = millis();
      if (!digitsLock) strIndex = strIndex + 1;
      if (strIndex == 10) dlay = 2000;
      if (strIndex > 10) {
        test = false;
        strIndex = 10;
      }

      stringToDisplay = testStringArray[strIndex];
      Serial.println(stringToDisplay);
      doIndication();
    }
    delayMicroseconds(2000);
  };

  if ( !ds.search(addr))
  {
    Serial.println(F("Temp. sensor not found."));
  } else TempPresent = true;

  Serial.println(F("Stop Test"));
  // while(1);
}

void doDotBlink()
{
  if (second() % 2 == 0) dotPattern = B11000000;
  else dotPattern = B00000000;
}

void setRTCDateTime(byte h, byte m, byte s, byte d, byte mon, byte y, byte w)
{
  Wire.beginTransmission(DS1307_ADDRESS);
  Wire.write(zero); //stop Oscillator

  Wire.write(decToBcd(s));
  Wire.write(decToBcd(m));
  Wire.write(decToBcd(h));
  Wire.write(decToBcd(w));
  Wire.write(decToBcd(d));
  Wire.write(decToBcd(mon));
  Wire.write(decToBcd(y));

  Wire.write(zero); //start

  Wire.endTransmission();

}

byte decToBcd(byte val) {
  // Convert normal decimal numbers to binary coded decimal
  return ( (val / 10 * 16) + (val % 10) );
}

byte bcdToDec(byte val)  {
  // Convert binary coded decimal to normal decimal numbers
  return ( (val / 16 * 10) + (val % 16) );
}

void getRTCTime()
{
  Wire.beginTransmission(DS1307_ADDRESS);
  Wire.write(zero);
  Wire.endTransmission();

  Wire.requestFrom(DS1307_ADDRESS, 7);

  RTC_seconds = bcdToDec(Wire.read());
  RTC_minutes = bcdToDec(Wire.read());
  RTC_hours = bcdToDec(Wire.read() & 0b111111); //24 hour time
  RTC_day_of_week = bcdToDec(Wire.read()); //0-6 -> sunday - Saturday
  RTC_day = bcdToDec(Wire.read());
  RTC_month = bcdToDec(Wire.read());
  RTC_year = bcdToDec(Wire.read());
}

int extractDigits(byte b)
{
  String tmp = "1";

  if (b == B00000011)
  {
    tmp = stringToDisplay.substring(0, 2);
  }
  if (b == B00001100)
  {
    tmp = stringToDisplay.substring(2, 4);
  }
  if (b == B00110000)
  {
    tmp = stringToDisplay.substring(4);
  }
  return tmp.toInt();
}

void injectDigits(byte b, int value)
{
  if (b == B00000011) stringToDisplay = PreZero(value) + stringToDisplay.substring(2);
  if (b == B00001100) stringToDisplay = stringToDisplay.substring(0, 2) + PreZero(value) + stringToDisplay.substring(4);
  if (b == B00110000) stringToDisplay = stringToDisplay.substring(0, 4) + PreZero(value);
}

bool isValidDate()
{
  int days[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
  if (value[DateYearIndex] % 4 == 0) days[1] = 29;
  if (value[DateDayIndex] > days[value[DateMonthIndex] - 1]) return false;
  else return true;

}

byte default_dur = 4;
byte default_oct = 6;
int bpm = 63;
int num;
long wholenote;
long duration;
byte note;
byte scale;
char* parseSong(char *p)
{
  // Absolutely no error checking in here
  // format: d=N,o=N,b=NNN:
  // find the start (skip name, etc)

  while (*p != ':') p++;   // ignore name
  p++;                     // skip ':'

  // get default duration
  if (*p == 'd')
  {
    p++; p++;              // skip "d="
    num = 0;
    while (isdigit(*p))
    {
      num = (num * 10) + (*p++ - '0');
    }
    if (num > 0) default_dur = num;
    p++;                   // skip comma
  }

  // get default octave
  if (*p == 'o')
  {
    p++; p++;              // skip "o="
    num = *p++ - '0';
    if (num >= 3 && num <= 7) default_oct = num;
    p++;                   // skip comma
  }

  // get BPM
  if (*p == 'b')
  {
    p++; p++;              // skip "b="
    num = 0;
    while (isdigit(*p))
    {
      num = (num * 10) + (*p++ - '0');
    }
    bpm = num;
    p++;                   // skip colon
  }

  // BPM usually expresses the number of quarter notes per minute
  wholenote = (60 * 1000L / bpm) * 4;  // this is the time for whole note (in milliseconds)
  return p;
}

// now begin note loop
static unsigned long lastTimeNotePlaying = 0;
char* playmusic(char *p)
{
  if (*p == 0)
  {
    return p;
  }
  if (millis() - lastTimeNotePlaying > duration)
    lastTimeNotePlaying = millis();
  else return p;
  // first, get note duration, if available
  num = 0;
  while (isdigit(*p))
  {
    num = (num * 10) + (*p++ - '0');
  }

  if (num) duration = wholenote / num;
  else duration = wholenote / default_dur;  // we will need to check if we are a dotted note after

  // now get the note
  note = 0;

  switch (*p)
  {
    case 'c':
      note = 1;
      break;
    case 'd':
      note = 3;
      break;
    case 'e':
      note = 5;
      break;
    case 'f':
      note = 6;
      break;
    case 'g':
      note = 8;
      break;
    case 'a':
      note = 10;
      break;
    case 'b':
      note = 12;
      break;
    case 'p':
    default:
      note = 0;
  }
  p++;

  // now, get optional '#' sharp
  if (*p == '#')
  {
    note++;
    p++;
  }

  // now, get optional '.' dotted note
  if (*p == '.')
  {
    duration += duration / 2;
    p++;
  }

  // now, get scale
  if (isdigit(*p))
  {
    scale = *p - '0';
    p++;
  }
  else
  {
    scale = default_oct;
  }

  scale += OCTAVE_OFFSET;

  if (*p == ',')
    p++;       // skip comma for next note (or we may be at the end)

  // now play the note

  if (note)
  {
    tone1.play(notes[(scale - 4) * 12 + note], duration);
    if (millis() - lastTimeNotePlaying > duration)
      lastTimeNotePlaying = millis();
    else return p;
    tone1.stop();
  }
  else
  {
    return p;
  }
  Serial.println(F("Incorrect Song Format!"));
  return 0; //error
}


void incrementValue()
{
  lastMenuActionTime = millis();
  if (editMode == true)
  {
    if (menuPosition != hModeValueIndex) // 12/24 hour mode menu position
      value[menuPosition] = value[menuPosition] + 1; else value[menuPosition] = value[menuPosition] + 12;
    if (value[menuPosition] > maxValue[menuPosition])  value[menuPosition] = minValue[menuPosition];
    if (menuPosition == Alarm01)
    {
      if (value[menuPosition] == 1) dotPattern = B11000000; //turn on upper dots
      else dotPattern = B00000000; //turn off all dots
    }
    if (menuPosition != DateFormatIndex) injectDigits(blinkMask, (int)value[menuPosition]);
    /*Serial.print("value=");
      Serial.println(value[menuPosition]);*/
  }
}


void dicrementValue()
{
  lastMenuActionTime = millis();
  if (editMode == true)
  {
    if (menuPosition != hModeValueIndex) value[menuPosition] = value[menuPosition] - 1; else value[menuPosition] = value[menuPosition] - 12;
    if (value[menuPosition] < minValue[menuPosition]) value[menuPosition] = maxValue[menuPosition];
    if (value[menuPosition] == 255) value[menuPosition] = maxValue[menuPosition]; // VMa: Added byte underflow handling
    if (menuPosition == Alarm01)
    {
      if (value[menuPosition] == 1) dotPattern = B11000000; //turn on upper dots
      else dotPattern = B00000000; //turn off all dots
    }
    if (menuPosition != DateFormatIndex) injectDigits(blinkMask, (int)value[menuPosition]);
    /*Serial.print("value=");
      Serial.println(value[menuPosition]);*/
  }
}


bool Alarm1SecondBlock = false;
unsigned long lastTimeAlarmTriggired = 0;
void checkAlarmTime()
{
  if (value[Alarm01] == 0) return;
  if ((Alarm1SecondBlock == true) && ((millis() - lastTimeAlarmTriggired) > 1000)) Alarm1SecondBlock = false;
  if (Alarm1SecondBlock == true) return;
  if ((hour() == value[AlarmHourIndex]) && (minute() == value[AlarmMinuteIndex]) && (second() == value[AlarmSecondIndex]))
  {
    lastTimeAlarmTriggired = millis();
    Alarm1SecondBlock = true;
    Serial.println(F("Wake up, Neo!"));
    p = song;
  }
}


void antiPoisoning4() {
  /*
     Do anti-poisoning slot machine animation hourly and a shorter version every minute.
     Run slot machine animation for each tube from current time digit incrementally, for minimum of a specified time and let only one tube finish per iteration.
     Update cycle is (roughly) synced with tube FPS limit for smooth animation at high speeds.
  */
  if (editMode == true) return;

  static unsigned long antiPoisoningDuration = 10000; // how long to run antipoisoning
  //const long antiPoisoningSpeed = 50; // how fast to scroll the "slot machine"
  static unsigned long lastAntiPoisoningTime = millis();
  //static unsigned long lastSlotMachineIteration = millis();
  static unsigned long prevHour = hour(), prevMinute = minute();

  static boolean active = false;
  static byte currentDigits[6];
  static bool doneDigits[6];
  static boolean allTubesDone = false;
  bool minTimeNotReached;

  // Init antipoisioning
  //if ((millis() - lastAntiPoisoningTime) > antiPoisoningInterval) {
  if ((hour() != prevHour) || (minute() != prevMinute)) {
    if (hour() != prevHour) {
      antiPoisoningDuration = 15000;
    } else {
      antiPoisoningDuration = 2000;
    }
     prevHour = hour();  
     prevMinute = minute();
    
    lastAntiPoisoningTime = millis();
    allTubesDone = false;

    for (int i = 0; i < 6; i++)
    {
      currentDigits[i] = getTimeNow().substring(i, i + 1).toInt();
      doneDigits[i] = false;
    }
  }

  minTimeNotReached = ((millis() - lastAntiPoisoningTime) < antiPoisoningDuration);

  if (!allTubesDone) {
    active = true;

    //if (millis() - lastSlotMachineIteration > antiPoisoningSpeed) {
    if (isUpdatingCycle()) {

      //lastSlotMachineIteration = millis();
      allTubesDone = true;
      bool loopLock = false;

      for (int i = 0; i < 6; i++)
      {
        if ((loopLock || minTimeNotReached || currentDigits[i] != getTimeNow().substring(i, i + 1).toInt()) && doneDigits[i] == false) {
          allTubesDone = false;
          currentDigits[i]++;
          if (currentDigits[i] == 10) currentDigits[i] = 0;

        } else {

          currentDigits[i] = getTimeNow().substring(i, i + 1).toInt();

          if (doneDigits[i] == false) {

            //lastAntiPoisoningTime += 500;
            doneDigits[i] = true;
            loopLock = true;
          }
        }
      }
    }

    String tmpStr;
    for (int i = 0; i < 6; i++) tmpStr += currentDigits[i];
    stringToDisplay = tmpStr;

  } else if (active) {

    active = false;
    stringToDisplay = getTimeNow();
  }

  //  Serial.println(stringToDisplay);

}


String updateDateString()
{
  static unsigned long lastTimeDateUpdate = millis() + 1001;
  static String DateString = PreZero(day()) + PreZero(month()) + PreZero(year() % 1000);
  static byte prevoiusDateFormatWas = value[DateFormatIndex];
  if (((millis() - lastTimeDateUpdate) > 1000) || (prevoiusDateFormatWas != value[DateFormatIndex]))
  {
    lastTimeDateUpdate = millis();
    if (value[DateFormatIndex] == EU_DateFormat) DateString = PreZero(day()) + PreZero(month()) + PreZero(year() % 1000);
    else DateString = PreZero(month()) + PreZero(day()) + PreZero(year() % 1000);
  }
  return DateString;
}


float getTemperature (boolean bTempFormat)
{
  byte TempRawData[2];
  ds.reset();
  ds.write(0xCC); //skip ROM command
  ds.write(0x44); //send make convert to all devices
  ds.reset();
  ds.write(0xCC); //skip ROM command
  ds.write(0xBE); //send request to all devices

  TempRawData[0] = ds.read();
  TempRawData[1] = ds.read();
  int16_t raw = (TempRawData[1] << 8) | TempRawData[0];
  if (raw == -1) raw = 0;
  float celsius = (float)raw / 16.0;
  float fDegrees;
  if (!bTempFormat) fDegrees = celsius * 10;
  else fDegrees = (celsius * 1.8 + 32.0) * 10;
  //Serial.println(fDegrees);
  return fDegrees;
}


String updateTemperatureString(float fDegrees)
{
  static  unsigned long lastTimeTemperatureString = millis() + 1100;
  static String strTemp = "000000";
  /*int delayTempUpdate;
    if (displayNow) delayTempUpdate=0;
    else delayTempUpdate = 1000;*/
  if ((millis() - lastTimeTemperatureString) > 1000)
  {
    //Serial.println("F(Updating temp. str.)");
    lastTimeTemperatureString = millis();
    int iDegrees = round(fDegrees);
    if (value[DegreesFormatIndex] == CELSIUS)
    {
      strTemp = "0" + String(abs(iDegrees)) + "0";
      if (abs(iDegrees) < 1000) strTemp = "00" + String(abs(iDegrees)) + "0";
      if (abs(iDegrees) < 100) strTemp = "000" + String(abs(iDegrees)) + "0";
      if (abs(iDegrees) < 10) strTemp = "0000" + String(abs(iDegrees)) + "0";
    } else
    {
      strTemp = "0" + String(abs(iDegrees)) + "0";
      if (abs(iDegrees) < 1000) strTemp = "00" + String(abs(iDegrees) / 10) + "00";
      if (abs(iDegrees) < 100) strTemp = "000" + String(abs(iDegrees) / 10) + "00";
      if (abs(iDegrees) < 10) strTemp = "0000" + String(abs(iDegrees) / 10) + "00";
    }

#ifdef tubes8
    strTemp = "" + strTemp + "00";
#endif
    return strTemp;
  }
  return strTemp;
}


void testDS3231TempSensor()
{
  int8_t DS3231InternalTemperature = 0;
  Wire.beginTransmission(DS1307_ADDRESS);
  Wire.write(0x11);
  Wire.endTransmission();

  Wire.requestFrom(DS1307_ADDRESS, 2);
  DS3231InternalTemperature = Wire.read();
  Serial.print(F("DS3231_T="));
  Serial.println(DS3231InternalTemperature);
  if ((DS3231InternalTemperature < 5) || (DS3231InternalTemperature > 65))
  {
    Serial.println(F("Faulty DS3231!"));
    for (int i = 0; i < 5; i++)
    {
      //tone(pinBuzzer, 1000);
      tone1.play(1000, 1000);
      delay(2000);
    }
  }
}


#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
void SyncWithGPS()
{
  //int offset=int((gps.UTCms()/1000.0)+0.5f);
  /*Serial.println(F("Updating time..."));
    Serial.println(fix.dateTime.hours);
    Serial.println(fix.dateTime.minutes);
    Serial.println(fix.dateTime.seconds);

    Serial.println(fix.dateTime.date);
    Serial.println(fix.dateTime.month);
    Serial.println(fix.dateTime.year);*/
  //Serial.print("offset=");
  //Serial.print(offset);

  setTime(fix.dateTime.hours, fix.dateTime.minutes, fix.dateTime.seconds, fix.dateTime.date, fix.dateTime.month, fix.dateTime.year);
  //if (gps.UTCms()>=500) adjustTime(1);
  //adjustTime(offset);
  adjustTime(1);
  adjustTime((long)value[HoursOffsetIndex] * 3600);

  /*Serial.print(F("Arduino Time="));
    Serial.print(hour());
    Serial.print(":");
    Serial.print(minute());
    Serial.print(":");
    Serial.println(second());*/

  setRTCDateTime(hour(), minute(), second(), day(), month(), year() % 1000, 1);
  GPS_Sync_Flag = 1;
  Last_Time_GPS_Sync = millis();


}


void GPSCheckValidity()
{
  if ((fix.dateTime.hours < 0)
      || (fix.dateTime.hours > 23)) return;

  if ((fix.dateTime.minutes < 0)
      || (fix.dateTime.minutes > 59)) return;

  if ((fix.dateTime.seconds < 0)
      || (fix.dateTime.seconds > 59)) return;

  if ((fix.dateTime.date < 0)
      || (fix.dateTime.date > 31)) return;

  if ((fix.dateTime.month < 0)
      || (fix.dateTime.month > 12)) return;

  if ((fix.dateTime.full_year() < 2020)
      || (fix.dateTime.full_year() > 2030)) return;

  fix.valid.time = false;
  SyncWithGPS();
}

#endif
