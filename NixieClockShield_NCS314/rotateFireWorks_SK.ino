#include <Adafruit_NeoPixel.h>

#define PIN            6
#define NUMPIXELS      8
#define LEDsSpeed      3
const int LEDsDelay = 40;

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);


void LEDsSetup()
{
  pixels.begin(); // This initializes the NeoPixel library.
  pixels.setBrightness(50);
}


void LEDsOFF()
{
  for (int i = 0; i < NUMPIXELS; i++)
  {
    pixels.setPixelColor(i, pixels.Color(0, 0, 0));
  }
  pixels.show();
}


void LEDsTest()
{
  for (int i = 0; i < NUMPIXELS; i++)
  {
    pixels.setPixelColor(i, pixels.Color(255, 0, 0));
  }
  pixels.show(); // This sends the updated pixel color to the hardware.
  delay(1000);
  for (int i = 0; i < NUMPIXELS; i++)
  {
    pixels.setPixelColor(i, pixels.Color(0, 255, 0));
  }
  pixels.show(); // This sends the updated pixel color to the hardware.
  delay(1000);
  for (int i = 0; i < NUMPIXELS; i++)
  {
    pixels.setPixelColor(i, pixels.Color(0, 0, 255));
  }
  pixels.show(); // This sends the updated pixel color to the hardware.
  delay(1000);
  LEDsOFF();
}


void rotateFireWorks()
{
  if (tone1.isPlaying()) return;
  //int LEDspeed=10;

  // Smooth on/off
  if (fadeIn || fadeOut) {
    fadeInOut();
    return;
  }

  if (!RGBLedsOn)
  {
    LEDsOFF();
    // analogWrite(RedLedPin, 0 );
    // analogWrite(GreenLedPin, 0);
    // analogWrite(BlueLedPin, 0);
    return;
  }
  if (LEDsLock) return;

  rgbRotator.R = rgbRotator.R + LEDsSpeed * fireforks[rotator * 3];
  rgbRotator.G =  rgbRotator.G + LEDsSpeed * fireforks[rotator * 3 + 1];
  rgbRotator.B =  rgbRotator.B + LEDsSpeed * fireforks[rotator * 3 + 2];
  rgbFader.R = rgbRotator.R;
  rgbFader.G = rgbRotator.G;
  rgbFader.B = rgbRotator.B;

  //  Serial.print( rgbRotator.R); Serial.print("  ");
  // Serial.print( rgbRotator.G);  Serial.print("  ");
  // Serial.println( rgbRotator.B);


  for (int i = 0; i < NUMPIXELS; i++)
  {
    //pixels.setPixelColor(i, pixels.Color(rgbRotator.R, rgbRotator.G, rgbRotator.B));
    pixels.setPixelColor(i, pixels.Color( rgbRotator.R,  rgbRotator.G,  rgbRotator.B));
  }

  pixels.show(); // This sends the updated pixel color to the hardware.

  cycle = cycle + 1;
  if (cycle == 255 / LEDsSpeed)
  {
    rotator = rotator + 1;
    cycle = 0;
  }
  if (rotator > 5) rotator = 0;
}


float fadeR, fadeG, fadeB;
float fadeRs, fadeGs, fadeBs;
float fadeRt, fadeGt, fadeBt;
byte fadeRm, fadeGm, fadeBm;
byte fadeState;



void fadeInOut() {
  const float fadeSpeed = 0.01;

  switch (fadeState) {

    case 0:

      if (fadeOut) {

        fadeRm = rgbFader.R;
        fadeGm = rgbFader.G;
        fadeBm = rgbFader.B;
        fadeR  = (float)rgbFader.R;
        fadeG  = (float)rgbFader.G;
        fadeB  = (float)rgbFader.B;
        fadeRs = fadeSpeed * rgbFader.R;
        fadeGs = fadeSpeed * rgbFader.G;
        fadeBs = fadeSpeed * rgbFader.B;
        Serial.println("fade out started");
        Serial.println(fadeRs);
        fadeState = 10;

      } else if (fadeIn) {

        TubesOn = true;
        fadeR = 0;
        fadeG = 0;
        fadeB = 0;

        fadeRt = (float)fadeRm;
        fadeGt = (float)fadeGm;
        fadeBt = (float)fadeBm;
        fadeRs = fadeSpeed * rgbFader.R;
        fadeGs = fadeSpeed * rgbFader.G;
        fadeBs = fadeSpeed * rgbFader.B;
        Serial.println("fade in finished");
        Serial.println(fadeRs);
        fadeState = 20;

      }
      break;


    case 1: // Cancel fadeout and fade in to where fadeout began

      fadeRt = (float)fadeRm;
      fadeGt = (float)fadeGm;
      fadeBt = (float)fadeBm;
      fadeRs = fadeSpeed * rgbFader.R;
      fadeGs = fadeSpeed * rgbFader.G;
      fadeBs = fadeSpeed * rgbFader.B;
      fadeState = 20;

      break;


    case 10:

      fadeR = max(fadeR - fadeRs, 0);
      fadeG = max(fadeG - fadeGs, 0);
      fadeB = max(fadeB - fadeBs, 0);

      /*
            Serial.print(fadeR); Serial.print("  ");
            Serial.print(fadeG);  Serial.print("  ");
            Serial.println(fadeB);
      */

      // Update flicker
      GenFlickerMask(max(max(fadeR, fadeG), fadeB));
     

      for (int i = 0; i < NUMPIXELS; i++)
      {
        pixels.setPixelColor(i, pixels.Color((byte)fadeR, (byte)fadeG, (byte)fadeB));
      }
      pixels.show();

      if (fadeR == 0 && fadeG == 0 && fadeB == 0) {
        RGBLedsOn = false;
        fadeOut = false;
        fadeState = 0;
        TubesOn = false;
        Serial.println("fade out finished");
      } else if (fadeIn) {
        fadeOut = false;
        fadeState = 1;
        Serial.println("fade out canceled");
      }
      break;


    case 20:

      fadeR = min(fadeR + fadeRs, fadeRt);
      fadeG = min(fadeG + fadeGs, fadeGt);
      fadeB = min(fadeB + fadeBs, fadeBt);

      /*
            Serial.print(fadeR); Serial.print("  ");
            Serial.print(fadeG);  Serial.print("  ");
            Serial.println(fadeB);
      */

       // Update flicker
      GenFlickerMask(max(max(fadeR, fadeG), fadeB));

      for (int i = 0; i < NUMPIXELS; i++)
      {
        pixels.setPixelColor(i, pixels.Color((byte)fadeR, (byte)fadeG, (byte)fadeB));
      }
      pixels.show();

      if (fadeR == fadeRt && fadeG == fadeGt && fadeB == fadeBt) {
        RGBLedsOn = true;
        fadeIn = false;
        fadeState = 0;
        Serial.println("fade in finished");
      } else if (fadeOut) {
        fadeIn = false;
        fadeState = 0;
        Serial.println("fade in canceled");
      }
      break;

  }

}


void GenFlickerMask(byte threshold) {
  fadeFlickerMask = 0;
  byte rnd;
  for (byte i = 0; i < 8; i++) {
    fadeFlickerMask |= (random(0, 255) > threshold ? B1 << i : B0);
  }
}


void setLEDsFromEEPROM()
{

  rgbFader.R = EEPROM.read(LEDsRedValueEEPROMAddress);
  rgbFader.G = EEPROM.read(LEDsGreenValueEEPROMAddress);
  rgbFader.B = EEPROM.read(LEDsBlueValueEEPROMAddress);

  for (int i = 0; i < NUMPIXELS; i++)
  {
    pixels.setPixelColor(i, pixels.Color(rgbFader.R, rgbFader.G, rgbFader.B));
  }

  pixels.show();
  /*analogWrite(RedLedPin, EEPROM.read(LEDsRedValueEEPROMAddress));
    analogWrite(GreenLedPin, EEPROM.read(LEDsGreenValueEEPROMAddress));
    analogWrite(BlueLedPin, EEPROM.read(LEDsBlueValueEEPROMAddress));*/
  // ********
  Serial.println(F("Read from EEPROM"));
  Serial.print(F("RED="));
  Serial.println(rgbFader.R);
  Serial.print(F("GREEN="));
  Serial.println(rgbFader.G);
  Serial.print(F("Blue="));
  Serial.println(rgbFader.B);
  // ********
}
