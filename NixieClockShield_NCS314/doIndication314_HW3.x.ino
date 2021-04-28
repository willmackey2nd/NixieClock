//driver for NCS314 HW3.x (registers HV57708)
//driver version 1.1
//1.1 SPI mode changed to MODE3
//0 on register's input will turn on a digit (when pol=high)

#include "doIndication314_HW3.x.h"

#define UpperDotsMask 0x80000000
#define LowerDotsMask 0x40000000

void SPISetup()
{
  SPI.begin(); //
  SPI.beginTransaction(SPISettings(1000000, MSBFIRST /*LSBFIRST*/, SPI_MODE3)); // по датащиту получается вроде как режим 0, но мне кажется что он не подходит: выводит одновременно несколько цифр
  // режим 1 делает вид что работает выводит 3 <- эти два режима работют наиболее адекватно
  // режим 2 делает вид что работает выводит 7
  // режим 3 делает вид что работает выводит 3 <- эти два режима работют наиболее адекватно
}


bool isUpdatingCycle() {
  static unsigned long lastTimeInterval1Started;
  if ((micros() - lastTimeInterval1Started) < fpsLimit) {
    return false;
  } else {
    lastTimeInterval1Started = micros();
    return true;
  }

}

void doIndication()
{

  static unsigned long lastTimeInterval1Started;
  if ((micros() - lastTimeInterval1Started) < fpsLimit) {
    return;
  }
  //if (menuPosition==TimeIndex) doDotBlink();
  lastTimeInterval1Started = micros();

  unsigned long Var32 = 0, Var32B = 0;
  unsigned long New32_L = 0;
  unsigned long New32_H = 0;

  long digits = stringToDisplay.toInt();


  /* Bit-char matrix. 
   *  SR = Shift register
   *  Chars = Hour1, Hour2, Minute1,...
   *  Pins = Pin names, not pin numbers of the chip
   *  LD = Left dot, RD = Right dot
+----------+------+------+------+------+------+------+------+------+------+------+------+------+------+----+----+-----+
| Bit      |    0 |    1 |    2 |    3 |    4 |    5 |    6 |    7 |    8 |    9 |   10 |   11 |   12 | 13 | 14 |  15 |
+----------+------+------+------+------+------+------+------+------+------+------+------+------+------+----+----+-----+
| SR1      |    0 |    1 |    2 |    3 |    4 |    5 |    6 |    7 |    8 |    9 |   10 |   11 |   12 | 13 | 14 |  15 |
+----------+------+------+------+------+------+------+------+------+------+------+------+------+------+----+----+-----+
| Char     | H1 0 | H1 4 | H1 8 | H2 2 | H2 6 | M1 0 | M1 4 | M1 8 | M2 0 | M2 4 | M2 8 | S1 2 | S1 6 |    |    |     |
+----------+------+------+------+------+------+------+------+------+------+------+------+------+------+----+----+-----+
| Pin(HVx) |    1 |    5 |    9 |   13 |   17 |   21 |   25 |   29 |   33 |   37 |   41 |   45 |   49 | 53 | 57 |  61 |
+----------+------+------+------+------+------+------+------+------+------+------+------+------+------+----+----+-----+
| Bit      |   16 |   17 |   18 |   19 |   20 |   21 |   22 |   23 |   24 |   25 |   26 |   27 |   28 | 29 | 30 |  31 |
+----------+------+------+------+------+------+------+------+------+------+------+------+------+------+----+----+-----+
| SR2      |    0 |    1 |    2 |    3 |    4 |    5 |    6 |    7 |    8 |    9 |   10 |   11 |   12 | 13 | 14 |  15 |
+----------+------+------+------+------+------+------+------+------+------+------+------+------+------+----+----+-----+
| Char     | H1 1 | H1 5 | H1 9 | H2 3 | H2 7 | M1 1 | M1 5 | M1 9 | M2 1 | M2 5 | M2 9 | S1 3 | S1 7 |    |    |     |
+----------+------+------+------+------+------+------+------+------+------+------+------+------+------+----+----+-----+
| Pin(HVx) |    2 |    6 |   10 |   14 |   18 |   22 |   26 |   30 |   34 |   38 |   42 |   46 |   50 | 54 | 58 |  62 |
+----------+------+------+------+------+------+------+------+------+------+------+------+------+------+----+----+-----+
| Bit      |   32 |   33 |   34 |   35 |   36 |   37 |   38 |   39 |   40 |   41 |   42 |   43 |   44 | 45 | 46 |  47 |
+----------+------+------+------+------+------+------+------+------+------+------+------+------+------+----+----+-----+
| SR3      |    0 |    1 |    2 |    3 |    4 |    5 |    6 |    7 |    8 |    9 |   10 |   11 |   12 | 13 | 14 |  15 |
+----------+------+------+------+------+------+------+------+------+------+------+------+------+------+----+----+-----+
| Char     | H1 2 | H1 6 | H2 0 | H2 4 | H2 8 | M1 2 | M1 6 | LD1  | M2 2 | M2 6 | S1 0 | S1 4 | S1 8 |    |    | RD1 |
+----------+------+------+------+------+------+------+------+------+------+------+------+------+------+----+----+-----+
| Pin(HVx) |    3 |    7 |   11 |   15 |   19 |   23 |   27 |   31 |   35 |   39 |   43 |   47 |   51 | 55 | 59 |  63 |
+----------+------+------+------+------+------+------+------+------+------+------+------+------+------+----+----+-----+
| Bit      |   48 |   49 |   50 |   51 |   52 |   53 |   54 |   55 |   56 |   57 |   58 |   59 |   60 | 61 | 62 |  63 |
+----------+------+------+------+------+------+------+------+------+------+------+------+------+------+----+----+-----+
| SR4      |    0 |    1 |    2 |    3 |    4 |    5 |    6 |    7 |    8 |    9 |   10 |   11 |   12 | 13 | 14 |  15 |
+----------+------+------+------+------+------+------+------+------+------+------+------+------+------+----+----+-----+
| Char     | H1 3 | H1 7 | H2 1 | H2 5 | H2 9 | M1 3 | M1 7 | LD2  | M2 3 | M2 7 | S1 1 | S1 5 | S1 9 |    |    | RD2 |
+----------+------+------+------+------+------+------+------+------+------+------+------+------+------+----+----+-----+
| Pin(HVx) |    4 |    8 |   12 |   16 |   20 |   24 |   28 |   32 |   36 |   40 |   44 |   48 |   52 | 56 | 60 |  64 |
+----------+------+------+------+------+------+------+------+------+------+------+------+------+------+----+----+-----+

   */

  /**********************************************************
     Incoming data in chars: [H1][H2}[M1][M2][S1][Y1][Y2]
     Converted to 2x 32-bit binary where each tube has 10 bits. Bit0 = '0'.
     This 2x 32-bit format is again reordered to another 2x 32-bit format according to the matrix above.
   *********************************************************/

  if (TubesOn) {
    //-------- REG 1 -----------------------------------------------
    Var32 = 0;

    Var32 |= (unsigned long)(SymbolArray[digits % 10] & FlickerMask(5) & doEditBlink(5)) << 20; // s2
    digits /= 10;

    Var32 |= (unsigned long)(SymbolArray[digits % 10] & FlickerMask(4) & doEditBlink(4)) << 10; //s1
    digits /= 10;

    Var32 |= (unsigned long)(SymbolArray[digits % 10] & FlickerMask(3) & doEditBlink(3)); //m2
    digits /= 10;

    if (LD) Var32 |= LowerDotsMask;
    else  Var32 &= ~LowerDotsMask;

    if (UD) Var32 |= UpperDotsMask;
    else Var32 &= ~UpperDotsMask;


    for (int i = 0; i < 32; i++)
    {
      i = i + 32;
      int newindex = 16 * (i % 4) + floor((float)i / 4);
      i = i - 32;
      if (newindex < 32) {
        bitWrite(New32_L, newindex, bitRead(Var32, i)) ;
      }
      else {
        bitWrite(New32_H, newindex - 32, bitRead(Var32, i));
      }
    }

    //-------- REG 0 -----------------------------------------------
    Var32B = 0;

    Var32B |= (unsigned long)(SymbolArray[digits % 10] & FlickerMask(2) & doEditBlink(2)) << 20; // m1
    digits /= 10;

    Var32B |= (unsigned long)(SymbolArray[digits % 10] & FlickerMask(1) & doEditBlink(1)) << 10; //h2
    digits /= 10;

    Var32B |= (unsigned long)SymbolArray[digits % 10] & FlickerMask(0) & doEditBlink(0); //h1
    digits /= 10;

    if (LD) Var32B |= LowerDotsMask;
    else  Var32B &= ~LowerDotsMask;

    if (UD) Var32B |= UpperDotsMask;
    else Var32B &= ~UpperDotsMask;

    for (int i = 0; i < 32; i++)
    {
      int newindex = 16 * (i % 4) + floor((float)i / 4);
      if (newindex < 32) {
        bitWrite(New32_L, newindex, bitRead(Var32B, i)); // 0-0,16-1,...,....,1-4,17-5,
      }
      else {
        bitWrite(New32_H, newindex - 32, bitRead(Var32B, i));          //  ...,....,0-2,26-3,...,....,
      }
    }

/*
    New32_H = 0;
    New32_L = 0;
    for (int i = 0; i <= 63; i++) {
      int newindex = 16 * (i % 4) + floor((float)i / 4);
      int srcindex = i;
     
      unsigned long* tmp32;
      if (srcindex < 32) {     
        tmp32 = &Var32B;
         Serial.print("Write High32: ");
      } else {
        tmp32 = &Var32;
        srcindex -= 32;
         Serial.print("Write Low32: ");
      }
      
      Serial.print(srcindex );
      Serial.print(" -> ");
      Serial.println(newindex );

      if (newindex < 32) {
        bitWrite(New32_L, newindex, bitRead(*tmp32, srcindex)); 
      }
      else {
        bitWrite(New32_H, newindex - 32, bitRead(*tmp32, srcindex)); 
      }
    }
    */

  } else {
    New32_H = 0;
    New32_L = 0;
  }

  SPI.transfer((New32_H) >> 24);
  SPI.transfer((New32_H) >> 16);
  SPI.transfer((New32_H) >> 8);
  SPI.transfer(New32_H);

  SPI.transfer((New32_L) >> 24);
  SPI.transfer((New32_L) >> 16);
  SPI.transfer((New32_L) >> 8);
  SPI.transfer(New32_L);

  digitalWrite(LEpin, HIGH); //<<-- это правильно H -> L // translates to "it is correct" ???
  digitalWrite(LEpin, LOW); // <<-- это правильно H -> L // translates to "it is correct" ???
  //-------------------------------------------------------------------------
}


word FlickerMask(int pos) {
  if (fadeFlickerMask >> pos & B00000001) {
    return 0x0000;
  } else return 0xFFFF;
}


word doEditBlink(int pos)

{

  // Disable mask if button is held
  if (!BlinkUp) return 0xFFFF;
  if (!BlinkDown) return 0xFFFF;

  bool blinkActive = B00000001 & blinkMask >> pos;

  static unsigned long lastTimeEditBlink = millis();
  static bool blinkState = false;
  word mask = 0xFFFF;
  static int tmp = 0; //blinkMask;
  if ((millis() - lastTimeEditBlink) > 300)
  {
    lastTimeEditBlink = millis();
    blinkState = !blinkState;
    if (blinkState) tmp = 0;
    else tmp = blinkMask;
  }
  if (((dotPattern & ~tmp) >> 6) & 1 == 1) LD = true; //digitalWrite(pinLowerDots, HIGH);
  else LD = false; //digitalWrite(pinLowerDots, LOW);
  if (((dotPattern & ~tmp) >> 7) & 1 == 1) UD = true; //digitalWrite(pinUpperDots, HIGH);
  else UD = false; //digitalWrite(pinUpperDots, LOW);

  if (blinkState && blinkActive) mask = 0x0000;

  return mask;
}

word blankDigit(int pos)
{
  int lowBit = blankMask >> pos;
  lowBit = lowBit & B00000001;
  word mask = 0;
  if (lowBit == 1) mask = 0xFFFF;
  return mask;
}
