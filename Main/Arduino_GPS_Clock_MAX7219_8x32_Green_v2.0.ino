#include <TinyGPS++.h>
#include <SoftwareSerial.h>
#include <Timezone.h>
#include <Time.h>
#include <Ticker.h>
#include "MAX7219Driver.h"
#include <Arduino.h>

#define rotation180

// Количество сегментов 8x8, которые вы соединяете
const int LEDMATRIX_SEGMENTS   = 4;
const int LEDMATRIX_WIDTH      = LEDMATRIX_SEGMENTS * 8;

// экранные буферы
byte screen_buffer[LEDMATRIX_WIDTH];

// Подключить DIN->D11 CLK->D13
const uint8_t LEDMATRIX_CS_PIN = 15; //D9

// Объект отображения
LEDMatrixDriver lmd(LEDMATRIX_SEGMENTS, LEDMATRIX_CS_PIN);

// Текущее время
// DateTime CurTime;
int CurHours = -1;//не инициирован
int CurMins  = 0;
int CurSecs  = 0;

#define CHANGED_NOTHING 0
#define CHANGED_ALL     6
#define CHANGED_SEC     CHANGED_ALL
#define CHANGED_MIN1    1
#define CHANGED_MIN10   2
#define CHANGED_HOUR1   3
#define CHANGED_HOUR10  4
#define SCROLL_ALL      5
#define CHANGED_SEC1    7
#define CHANGED_SEC10   8

//позиции цифр часов


#define POS_DIGIT1 1                //0
#define POS_DIGIT2 (POS_DIGIT1 + 7) //7
#define POS_COLON  (POS_DIGIT2 + 7) //14
#define POS_DIGIT3 (POS_COLON  + 3) //17
#define POS_DIGIT4 (POS_DIGIT3 + 7) //24

// цифровой 6x8 моно
#define BIG_COLON   0xa
#define BIG_COLON1  0xb
const byte font_digit_6x8[] PROGMEM = 
{
  0x7e, 0xff, 0x81, 0x81, 0xff, 0x7e,  //'0' 0
  0x00, 0x82, 0xff, 0xff, 0x80, 0x00,  //'1' 1 
  0x82, 0xc1, 0xa1, 0x91, 0xcf, 0xc6,  //'2' 2 
  0x42, 0xc1, 0x89, 0x89, 0xff, 0x76,  //'3' 3
  0x38, 0x24, 0xa2, 0xff, 0xff, 0xa0,  //'4' 4
  0x4f, 0xcf, 0x89, 0x89, 0xf9, 0x71,  //'5' 5
  0x7c, 0xfe, 0x8b, 0x89, 0xf9, 0x70,  //'6' 6
  0x01, 0x81, 0xf1, 0xf9, 0x8f, 0x07,  //'7' 7
  0x76, 0xff, 0x89, 0x89, 0xff, 0x76,  //'8' 8
  0x0e, 0x9f, 0x91, 0xd1, 0x7f, 0x3e,  //'9' 9
  0x62, 0x62, 0x00, 0x00, 0x00, 0x00,  //':' 0xa
  0x46, 0x46, 0x00, 0x00, 0x00, 0x00   //':' 0xb
};

// цифровой 6x6 мини
const byte font_digit_6x6_mini[] PROGMEM = 
{
        126, 126, 66, 66, 126, 126, 	// 0
        0, 4, 4, 126, 126, 0, 	// 1
        122, 122, 74, 82, 94, 94, 	// 2
        66, 74, 74, 74, 126, 118, 	// 3
        48, 56, 36, 34, 126, 126, 	// 4
        94, 94, 82, 74, 122, 122, 	// 5
        126, 126, 82, 74, 122, 122, 	// 6
        2, 2, 114, 122, 14, 6, 	// 7
        // 126, 126, 74, 74, 126, 126, 	// 8
        118, 126, 74, 74, 126, 118, //8
        94, 94, 82, 74, 126, 126, 	// 9
        36, 36, 0, 0, 0, 0, // Двоеточие
        0, 0, 0, 0, 0, 0 // Пробел
};

// цифровой 6x7 мини
const byte font_digit_6x7[] PROGMEM = 
{
  127, 127, 65, 65, 127, 127, 	// 0
	0, 2, 2, 127, 127, 0, 	// 1
	121, 121, 73, 65, 79, 79, 	// 2
	65, 65, 73, 73, 127, 119, 	// 3
	48, 56, 36, 34, 127, 127, 	// 4
	79, 79, 65, 73, 121, 121, 	// 5
	127, 127, 65, 73, 121, 121, 	// 6
	1, 1, 121, 125, 7, 3, 	// 7
	119, 127, 73, 73, 127, 119, 	// 8
	79, 79, 73, 65, 127, 127, 	// 9
	36, 36, 0, 0, 0, 0, 	// Двоеточие
  0, 0, 0, 0, 0, 0 // Пробел
};

// цифровой 6x8 
const byte font_digit_6x8m[] PROGMEM = 
{
  255, 255, 129, 129, 255, 255, 	// 0
	0, 4, 6, 255, 255, 0, 	// 1
	249, 249, 137, 145, 159, 159, 	// 2
	129, 137, 137, 137, 255, 247, 	// 3
	48, 56, 36, 34, 255, 255, 	// 4
	159, 159, 129, 137, 249, 249, 	// 5
	255, 255, 129, 137, 249, 249, 	// 6
	1, 1, 249, 253, 7, 3, 	// 7
	247, 255, 137, 137, 255, 247, 	// 8
	159, 159, 145, 129, 255, 255, 	// 9
	36, 36, 0, 0, 0, 0, 	// Двоеточие
  0, 0, 0, 0, 0, 0 // Пробел
};

// Коды меню часов
enum {
  //Показать меню
  MODE_SHOW_BEGIN,
    MODE_SHOW_CLOCK,
    MODE_SHOW_TEMP,
    MODE_SHOW_PRESSURE,
    MODE_SHOW_DATE,
    MODE_SHOW_ALARM,
  MODE_SHOW_END,

  //Настройки
  MODE_SET_BEGIN,
    MODE_SET_TIME,
    MODE_SET_ALARM,
    MODE_SHOW_VERSION,
  MODE_SET_END,

  //изменить меню времени/даты
  MODE_CH_BEGIN,
    MODE_CH_HOUR,
    MODE_CH_MIN,
    MODE_CH_SEC,
    MODE_CH_DAY,
    MODE_CH_MONTH,
    MODE_CH_YEAR,
  MODE_CH_END,

  //изменить меню будильника
  MODE_CH_ALARM_BEGIN,
    MODE_CH_ALARM_HOUR,
    MODE_CH_ALARM_MIN,
    MODE_CH_ALARM_DAY1,
    MODE_CH_ALARM_DAY2,
    MODE_CH_ALARM_DAY3,
    MODE_CH_ALARM_DAY4,
    MODE_CH_ALARM_DAY5,
    MODE_CH_ALARM_DAY6,
    MODE_CH_ALARM_DAY7,
    MODE_CH_ALARM_MELODY,
  MODE_CH_ALARM_END,
  
  MODE_UNUSED
  //переместить сюда неиспользуемый идентификатор меню
};

void ShowBuffer(byte* buffer = NULL);

Ticker flipper;

static const uint8_t RXPin = 4, TXPin = 5; //D2 - RX, D1 - TX

static const uint32_t GPSBaud = 9600;

// The TinyGPS++ object
TinyGPSPlus GPS;

// The serial connection to the GPS device
SoftwareSerial SerialGPS(RXPin, TXPin);

TimeChangeRule myDST = {"CEST", Last, Sun, Mar, 2, 180}; // Central European Summer Time; CEST (Центральноевропейское Летнее Время) - одно из общеизвестных названий для UTC+2 часового пояса
TimeChangeRule mySTD = {"CET", Last, Sun, Oct, 2, 120};   // Central European Time(UTC+1)
Timezone myTZ(myDST, mySTD);//DST - правило для начала летнего времени или летнего времени для любого года;
                            //STD - правило начала поясного времени на любой год.
TimeChangeRule *tcr;


// #define DIN_PIN D7//15 //D8
// #define CS_PIN  D8//12 //D6
// #define CLK_PIN D5//13 //D7
// //SPI D8 - CS, D7 - MOSI, D6 - MISO, D5 - CLK

// #define NUM_MAX 4

// //#define rotation180

// #include "max7219_hr.h"
// #include "fonts.h"

// #define MAX_DIGITS byte(16)
// byte dig[MAX_DIGITS] = {0};
// byte digold[MAX_DIGITS] = {0};
// byte digtrans[MAX_DIGITS] = {0};

// int dx = 0;
// int dy = 0;
// int del = 0;
byte dots = 0;
// byte h, m, s, satelit;
//----------------------------------------------------------------------------------------------------------------------------------


//////////////////////////////////////////////////////////////////////////////
void ShowBuffer(byte* buffer)
{
  if(buffer && screen_buffer != buffer)
    memcpy(screen_buffer, buffer, LEDMATRIX_WIDTH);

  for(byte row = 0; row < LEDMATRIX_WIDTH; row++)
    lmd.setColumn(row, screen_buffer[row]);
  lmd.display();
}

//////////////////////////////////////////////////////////////////////////////////////////////
int GetCurTime()
{
  static uint32_t lastTime = 0;

  int h = CurHours;
  int m = CurMins;
  int s = CurSecs;

  if (millis() - lastTime > 250)
     {
        lastTime = millis();
        readGPS();
     }

  // CurTime  = rtc.now();
  // CurHours = CurTime.hour();
  // CurMins  = CurTime.minute();
  // CurSecs  = CurTime.second();

  if(h == -1)
  {
    // первый звонок
    return CHANGED_ALL;
  }
  // проверить измененные цифры
  int mod = CHANGED_NOTHING;
  
  //  if(h % 10 == CurHours % 10)  {}
  //    else  mod = CHANGED_HOUR1;
  //       if(h / 10 == CurHours / 10) {}
  //         else  mod = CHANGED_HOUR10;
  //            if(m % 10 == CurMins % 10) {}
  //              else  mod = CHANGED_MIN1;
  //                  if(m / 10 == CurMins / 10)  {}
  //                    else  mod = CHANGED_MIN10;
                    //    if(s % 10 == CurSecs % 10)  {}
                    //       else  mod = CHANGED_SEC1;
                    //           if(s / 10 == CurSecs / 10)  {}
                    //             else  mod = CHANGED_SEC10;
  if(h / 10 != CurHours / 10)       mod = CHANGED_HOUR10;
  else if(h % 10 != CurHours % 10)  mod = CHANGED_HOUR1;
  else if(m / 10 != CurMins / 10)   mod = CHANGED_MIN10;
  else if(m % 10 != CurMins % 10)   mod = CHANGED_MIN1;
  else //if(s != CurSecs)//(dots)
                                    mod = CHANGED_SEC;

  return mod;
}

//////////////////////////////////////////////////////////////////////////////
void ScrollVertical(byte* buffer, byte from, byte to, boolean fUp)
{
  for(byte i = 0; i < 8; i++)
  {
    ScrollVerticalOneRow(buffer, from, to, fUp);
    ShowBuffer();
    delay(50);//скорость прокручивания цифры
  }
}

//////////////////////////////////////////////////////////////////////////////
void ScrollVerticalOneRow(byte* buffer, byte from, byte to, boolean fUp)
{
  for(byte n = from; n <= to; ++n)
  {
    if(fUp)
    {
      screen_buffer[n] >>= 1;
      bitWrite(screen_buffer[n], 7, bitRead(buffer[n], 0));
      buffer[n] >>= 1;
    }
    else
    {
      screen_buffer[n] <<= 1;
      bitWrite(screen_buffer[n], 0, bitRead(buffer[n], 7));
      buffer[n] <<= 1;
    }
  }
}

//////////////////////////////////////////////////////////////////////////////
byte GetColumnMask(int size)
{
  switch(size)
  {
    case 1:
      return B10000000;
    case 2:
      return B11000000;
    case 3:
      return B11100000;
    case 4:
      return B11110000;
    case 5:
      return B11111000;
    case 6:
      return B11111100;
    case 7:
      return B11111110;
    case 8:
      return B11111111;
    //  case 1:
    //   return B11111111;
    // case 2:
    //   return B01111111;
    // case 3:
    //   return B00111111;
    // case 4:
    //   return B00011111;
    // case 5:
    //   return B00001111;
    // case 6:
    //   return B00000111;
    // case 7:
    //   return B00000011;
    // case 8:
    //   return B00000001;
  }

  return B11111111;
}

/////////////////////////////////////////////////////////////////////////////////////
byte reverse_bits(byte num)
{
    byte rez = 0;
    int n = sizeof(num) * 8 - 1;
    while(num)
    {
        rez|= (num & 0x01) << n;
        num >>= 1;
        --n;
    }
    return rez;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CopySymbol(byte* pBuffer, const byte* pFont, int Symbol, int x, int y, int fontx, int fonty, int sizex = 0)
{
  byte mask = GetColumnMask(fonty);

  if(sizex == 0)
    sizex = fontx;// fontx = 6

#ifdef rotation180
  byte j = 0;
  for(int i = sizex; i > 0; --i)
  {// fontx = 6, y = 0, x = 1
    
    byte column = pgm_read_byte(pFont + Symbol * fontx + i - 1);
    column = reverse_bits(column);
    pBuffer[x + j] = (pBuffer[x + j] & ~(mask << y)) | (column << y);
    j++;
  }
#else
  for(int i = 0; i < sizex; ++i)
  {
    byte column = pgm_read_byte(pFont + Symbol * fontx + i);
    pBuffer[x + i] = (pBuffer[x + i] & ~(mask << y)) | (column << y);
  }
#endif

}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void DisplayTime(int hours, int mins, int secs, byte scroll_mode, boolean fUp)
{
  byte sprite_buffer[LEDMATRIX_WIDTH];
  memset(sprite_buffer, 0, LEDMATRIX_WIDTH);
  #ifdef rotation180
        // #define All_Little6x6_Font
                #ifdef All_Little6x6_Font                                                                                                                       
                  CopySymbol(sprite_buffer, font_digit_6x6_mini, hours / 10, POS_DIGIT4, 0, 6, 8);
                  CopySymbol(sprite_buffer, font_digit_6x6_mini, hours % 10, POS_DIGIT3, 0, 6, 8);
                  CopySymbol(sprite_buffer, font_digit_6x6_mini, BIG_COLON + (dots & 1), POS_COLON, 0, 6, 8, 2);
                  // CopySymbol(sprite_buffer, font_digit_6x6_mini, BIG_COLON + (secs & 1), POS_COLON, 0, 6, 8, 2);
                  CopySymbol(sprite_buffer, font_digit_6x6_mini, mins / 10, POS_DIGIT2, 0, 6, 8);
                  CopySymbol(sprite_buffer, font_digit_6x6_mini, mins % 10, POS_DIGIT1, 0, 6, 8);  
                  // CopySymbol(sprite_buffer, font_digit_6x6_mini, secs / 10, POS_DIGIT5, 0, 6, 8);
                  // CopySymbol(sprite_buffer, font_digit_6x6_mini, secs % 10, POS_DIGIT6, 0, 6, 8);
                
                #else
                //  if(hours / 10)
                  CopySymbol(sprite_buffer, font_digit_6x7, hours / 10, POS_DIGIT4, 0, 6, 8);
                  CopySymbol(sprite_buffer, font_digit_6x7, hours % 10, POS_DIGIT3, 0, 6, 8);
                  CopySymbol(sprite_buffer, font_digit_6x7, BIG_COLON + (dots & 1), POS_COLON, 0, 6, 8, 2);
                  CopySymbol(sprite_buffer, font_digit_6x7, mins / 10, POS_DIGIT2, 0, 6, 8);
                  CopySymbol(sprite_buffer, font_digit_6x7, mins % 10, POS_DIGIT1, 0, 6, 8);
                  // CopySymbol(sprite_buffer, font_digit_6x8, secs / 10, POS_DIGIT3, 0, 6, 8);
                  // CopySymbol(sprite_buffer, font_digit_6x8, secs % 10, POS_DIGIT4, 0, 6, 8);
                  // if(alarm)
                  // {
                  //   sprite_buffer[LEDMATRIX_WIDTH - 1] = 0b10000000;
                  // }
                #endif

  #else
        #define All_Little6x6_Font
                #ifdef All_Little6x6_Font                                                                                                                       
                  CopySymbol(sprite_buffer, font_digit_6x6_mini, hours / 10, POS_DIGIT1, 0, 6, 8);
                  CopySymbol(sprite_buffer, font_digit_6x6_mini, hours % 10, POS_DIGIT2, 0, 6, 8);
                  CopySymbol(sprite_buffer, font_digit_6x6_mini, BIG_COLON + (dots & 1), POS_COLON, 0, 6, 8, 2);
                  // CopySymbol(sprite_buffer, font_digit_6x6_mini, BIG_COLON + (secs & 1), POS_COLON, 0, 6, 8, 2);
                  CopySymbol(sprite_buffer, font_digit_6x6_mini, mins / 10, POS_DIGIT3, 0, 6, 8);
                  CopySymbol(sprite_buffer, font_digit_6x6_mini, mins % 10, POS_DIGIT4, 0, 6, 8);  
                  // CopySymbol(sprite_buffer, font_digit_6x6_mini, secs / 10, POS_DIGIT5, 0, 6, 8);
                  // CopySymbol(sprite_buffer, font_digit_6x6_mini, secs % 10, POS_DIGIT6, 0, 6, 8);
                
                #else
                //  if(hours / 10)
                  CopySymbol(sprite_buffer, font_digit_6x8, hours / 10, POS_DIGIT1, 0, 6, 8);
                  CopySymbol(sprite_buffer, font_digit_6x8, hours % 10, POS_DIGIT2, 0, 6, 8);
                  CopySymbol(sprite_buffer, font_digit_6x8, BIG_COLON + (secs & 1), POS_COLON, 0, 6, 8, 2);
                  CopySymbol(sprite_buffer, font_digit_6x8, mins / 10, POS_DIGIT3, 0, 6, 8);
                  CopySymbol(sprite_buffer, font_digit_6x8, mins % 10, POS_DIGIT4, 0, 6, 8);
                  // CopySymbol(sprite_buffer, font_digit_6x8, secs / 10, POS_DIGIT3, 0, 6, 8);
                  // CopySymbol(sprite_buffer, font_digit_6x8, secs % 10, POS_DIGIT4, 0, 6, 8);
                  // if(alarm)
                  // {
                  //   sprite_buffer[LEDMATRIX_WIDTH - 1] = 0b10000000;
                  // }
                #endif
  #endif

  switch(scroll_mode)
  {
    case CHANGED_ALL:
      // default:
      ShowBuffer(sprite_buffer);
      break;
      #ifdef rotation180
                    // case CHANGED_SEC1:
                    //   ScrollVertical(sprite_buffer, POS_DIGIT4, POS_DIGIT4+5, fUp);
                    //   break;
                    // case CHANGED_SEC10:
                    //   ScrollVertical(sprite_buffer, POS_DIGIT4, POS_DIGIT4+5, fUp);
                    //   ScrollVertical(sprite_buffer, POS_DIGIT3, POS_DIGIT3+5, fUp);
                    //   break;
                    case CHANGED_MIN1:
                      ScrollVertical(sprite_buffer, POS_DIGIT1, POS_DIGIT1+5, fUp);
                      break;
                    case CHANGED_MIN10:
                      ScrollVertical(sprite_buffer, POS_DIGIT1, POS_DIGIT1+5, fUp);
                      ScrollVertical(sprite_buffer, POS_DIGIT2, POS_DIGIT2+5, fUp);
                      break;
                    case CHANGED_HOUR1:
                      ScrollVertical(sprite_buffer, POS_DIGIT1, POS_DIGIT1+5, fUp);
                      ScrollVertical(sprite_buffer, POS_DIGIT2, POS_DIGIT2+5, fUp);
                      ScrollVertical(sprite_buffer, POS_DIGIT3, POS_DIGIT3+5, fUp);
                      break;
                    case CHANGED_HOUR10:
                      ScrollVertical(sprite_buffer, POS_DIGIT1, POS_DIGIT1+5, fUp);
                      ScrollVertical(sprite_buffer, POS_DIGIT2, POS_DIGIT2+5, fUp);
                      ScrollVertical(sprite_buffer, POS_DIGIT3, POS_DIGIT3+5, fUp);
                      ScrollVertical(sprite_buffer, POS_DIGIT4, POS_DIGIT4+5, fUp);
                      break;
                    case SCROLL_ALL:
                      ScrollVertical(sprite_buffer, 0, 31, fUp);
                      break;

    #else

                    // case CHANGED_SEC1:
                    //   ScrollVertical(sprite_buffer, POS_DIGIT4, POS_DIGIT4+5, fUp);
                    //   break;
                    // case CHANGED_SEC10:
                    //   ScrollVertical(sprite_buffer, POS_DIGIT4, POS_DIGIT4+5, fUp);
                    //   ScrollVertical(sprite_buffer, POS_DIGIT3, POS_DIGIT3+5, fUp);
                    //   break;
                    case CHANGED_MIN1:
                      ScrollVertical(sprite_buffer, POS_DIGIT4, POS_DIGIT4+5, fUp);
                      break;
                    case CHANGED_MIN10:
                      ScrollVertical(sprite_buffer, POS_DIGIT4, POS_DIGIT4+5, fUp);
                      ScrollVertical(sprite_buffer, POS_DIGIT3, POS_DIGIT3+5, fUp);
                      break;
                    case CHANGED_HOUR1:
                      ScrollVertical(sprite_buffer, POS_DIGIT4, POS_DIGIT4+5, fUp);
                      ScrollVertical(sprite_buffer, POS_DIGIT3, POS_DIGIT3+5, fUp);
                      ScrollVertical(sprite_buffer, POS_DIGIT2, POS_DIGIT2+5, fUp);
                      break;
                    case CHANGED_HOUR10:
                      ScrollVertical(sprite_buffer, POS_DIGIT4, POS_DIGIT4+5, fUp);
                      ScrollVertical(sprite_buffer, POS_DIGIT3, POS_DIGIT3+5, fUp);
                      ScrollVertical(sprite_buffer, POS_DIGIT2, POS_DIGIT2+5, fUp);
                      ScrollVertical(sprite_buffer, POS_DIGIT1, POS_DIGIT1+5, fUp);
                      break;
                    case SCROLL_ALL:
                      ScrollVertical(sprite_buffer, 0, 31, fUp);
                      break;
    #endif
  }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void readGPS()
{
    while (SerialGPS.available())
    {
      GPS.encode(SerialGPS.read());
      if (GPS.time.isValid() && GPS.date.isValid())
      {
        setTime(GPS.time.hour(), GPS.time.minute(), GPS.time.second(), GPS.date.day(), GPS.date.month(), GPS.date.year());
        CurMins = minute();
        CurHours = hour(myTZ.toLocal(now()));
        CurSecs = second();
      }
    }
      
//      else     printStringConnect();
      
     
//      time_t utc = now();
//      time_t local = CE.toLocal(utc, &tcr);
  
//      h = GPS.time.hour();
//      m = GPS.time.minute();
        
//        h = hour();
//        h = hour(myTZ.toLocal(now(), &tcr));
          
          
//      satelit = GPS.satellites.value();
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



void setup()
{
  SerialGPS.begin(GPSBaud);
  // initMAX7219();
// //  flipper.attach(0.25, readGPS);
  flipper.attach_scheduled(0.5, tochka);
// //  clr();
// //  pinMode(LED_BUILTIN, OUTPUT);
//   sendCmdAll(CMD_SHUTDOWN, 1);
//   sendCmdAll(CMD_INTENSITY, 8);
//   clr();
// //  xPos=0;
//     printStringConnect();

//       while (h == 0 && m == 0)
//     {
//         while (SerialGPS.available())
//               {
//                 GPS.encode(SerialGPS.read());

//                 if (GPS.time.isValid() && GPS.date.isValid())
//                    {
//                       h = GPS.time.hour();
//                       m = GPS.time.minute();
//                    }
//               }
      
//     }
lmd.setIntensity(9);

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
void tochka()
{
     dots = !dots; 
    // dots++;
    // if(dots > 59)  dots = 0;
     
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
void loop()
{
// //  static byte j,i = 0;
//     static uint32_t lastTime = 0;

// //  if (i++ > 240)
// //  {
// //    i = 0;
// //    dots = !dots;
// //  }
// //    if (j++ > 253)
// //        {
// //          j = 0;
// //          readGPS();
// //        }
                                              //     if (millis() - lastTime > 250)
                                              //       {
                                              //         lastTime = millis();
                                              //         readGPS();
                                              //       }

                                              // showAnimClock();
//   //  showSimpleClock();
//   //  printStringConnect();
//   //  clr();

//текущий режим
  static int  Mode = MODE_SHOW_CLOCK;
    bool fBlink = false;
  //Обновить параметры
  int TimeChanged = GetCurTime();


// Применить режим
  if(Mode == MODE_SHOW_CLOCK)
  {
    fBlink = false;
    if(TimeChanged)
    {
      // uint8_t h, m, d, n;
      // alarm.GetAlarm(d, h, m, n);
      // bool alarm = (Alarm::DayOfTheWeek(CurTime.dayOfTheWeek()) & d) != 0;
      #ifdef rotation180
                DisplayTime(CurHours, CurMins, CurSecs, TimeChanged, true);
      #else
                DisplayTime(CurHours, CurMins, CurSecs, TimeChanged, false);
      #endif
    }
  }
}
//---------------------------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------------------------
// void showSimpleClock()
// {
//   clr();
//   dx = dy = 0;
//   #ifdef rotation180
//   showDigit(h/10,  0, dig6x8);
//   showDigit(h%10,  8, dig6x8);
//   showDigit(m/10, 17, dig6x8);
//   showDigit(m%10, 25, dig6x8);
// //  showDigit(m/10,  9, dig6x8);
// //  showDigit(m%10,  1, dig6x8);
// //  showDigit(h/10, 25, dig6x8);
// //  showDigit(h%10, 16, dig6x8);
//   #else
//   showDigit(h/10,  0, dig6x8);
//   showDigit(h%10,  8, dig6x8);
//   showDigit(m/10, 17, dig6x8);
//   showDigit(m%10, 25, dig6x8);
//   #endif
// //  showDigit(s/10, 38, dig6x8);
// //  showDigit(s%10, 46, dig6x8);
//   #ifdef rotation180
//   setCol(15, dots ? B00100100 : 0);
// //  setCol(23, dots ? B00100100 : 0);
//   #else
//   setCol(15, dots ? B00100100 : 0);
//   #endif
// //  setCol(32,dots ? B00100100 : 0);
// //  refreshAll();
//   #ifdef rotation180
//   refreshAllRot270();
//   #else
//   refreshAllRot90();
//   #endif
// }
// //-------------------------------------------------------------------------------------------------------------
// void showAnimClock()
// {
// //  byte digPos[6]={4,12,21,29,38,46};
//   byte digPos[6] = {0, 8, 18, 26, 34, 42};//Положение цифр - часы, минуты, секунды (по 2 цифры)
//   byte digHt = 12;
//   byte num = 6; 
//   byte i;
//   if(!del) 
//   {
//     del = digHt;
//     for(i = 0; i < num; i++) digold[i] = dig[i];
//     dig[0] = h/10 ? h/10 : 10;//Без нуля в начале - часы
// //    dig[0] = h/10;
//     dig[1] = h%10;
//     dig[2] = m/10;//satelit/10;
//     dig[3] = m%10;//satelit%10;
// //    dig[4] = s/10;
// //    dig[5] = s%10;
//     for(i = 0; i < num; i++)  digtrans[i] = (dig[i] == digold[i]) ? 0 : digHt;
//   } 
//   else
//     del--;
  
//   clr();
//   for(i = 0; i < num; i++) 
//   {
//     if(digtrans[i] == 0) 
//     {
//       dy = 0;
//       showDigit(dig[i], digPos[i], dig6x8);
//     } 
//     else 
//     {
//       dy = digHt-digtrans[i];
//       showDigit(digold[i], digPos[i], dig6x8);
//       dy = -digtrans[i];
//       showDigit(dig[i], digPos[i], dig6x8);
//       digtrans[i]--;
//       delay(30);
//     }
//   }
//   dy = 0;
//   setCol(14, dots ? B00000000 : B00000000);//мигание двоеточия  
//   setCol(15, dots ? B01000100 : B00100010);//мигание двоеточия
//   setCol(16, dots ? B01000100 : B00100010);//мигание двоеточия
//   setCol(17, dots ? B00000000 : B00000000);//мигание двоеточия

// //  setCol(14, dots ? B00000000 : B00000000);//мигание двоеточия  
// //  setCol(15, dots ? B00100100 : B00000000);//мигание двоеточия
// //  setCol(16, dots ? B00000000 : B00100010);//мигание двоеточия
// //  setCol(17, dots ? B00000000 : B00000000);//мигание двоеточия

// //  setCol(14, dots ? B00000000 : B00000000);//мигание двоеточия  
// //  setCol(15, dots ? B01111100 : B00100000);//мигание двоеточия
// //  setCol(16, dots ? B00000100 : B00111110);//мигание двоеточия
// //  setCol(17, dots ? B00000000 : B00000000);//мигание двоеточия

// //  setCol(14, dots ? B00000000 : B00000000);//мигание двоеточия  
// //  setCol(15, dots ? B01101100 : B00000000);//мигание двоеточия
// //  setCol(16, dots ? B00000000 : B00110110);//мигание двоеточия
// //  setCol(17, dots ? B00000000 : B00000000);//мигание двоеточия

// //  setCol(14, dots ? B00000000 : B00000000);//мигание двоеточия  
// //  setCol(15, dots ? B01100000 : B00000110);//мигание двоеточия
// //  setCol(16, dots ? B01100000 : B00000110);//мигание двоеточия
// //  setCol(17, dots ? B00000000 : B00000000);//мигание двоеточия

// //  setCol(14, dots ? B00000000 : B00000000);//мигание двоеточия  
// //  setCol(15, dots ? B01000100 : B00100100);//мигание двоеточия
// //  setCol(16, dots ? B00101000 : B01000010);//мигание двоеточия
// //  setCol(17, dots ? B00000000 : B00000000);//мигание двоеточия
  
// //  setCol(32,dots ? B00100100 : 0);
// //  refreshAll();

//   #ifdef rotation180
//   refreshAllRot270();
//   #else
//   refreshAllRot90();
//   #endif
// }
// //---------------------------------------------------------------------------------------------------------------------
// void printStringConnect()
// {
//     printStringWithShift("to GPS", 100);
// }
// //----------------------------------------------------------------------------------------------------
// void printStringWithShift(const char* s, int shiftDelay)
// {
//   while (*s) 
//   {
//     printCharWithShift(*s, shiftDelay);
//     s++;
//   }
// }
// //--------------------------------------------------------------------------------------------------------------
// void printCharWithShift(byte c, int shiftDelay) 
// {
//   if (c < ' ' || c > '~' + 25) return;
//   c -= 32;
//   byte w = showChar(c, font);
//   for (byte i = 0; i < (w + 1); i++) 
// //  for (byte i = (w+1); i > 0; i--)
//   {
//     delay(shiftDelay);

//   #ifdef rotation180
// //  scrollRight();
//   scrollLeft();
//   refreshAllRot270();
//   #else
//   scrollLeft();
//   refreshAllRot90();
//   #endif
//   }
// }
// //--------------------------------------------------------------------------------------------------------------
// void showDigit(char _char, byte column, const uint8_t *font)
// {
//   if(dy < -8 | dy > 8) return;
//   byte len = pgm_read_byte(font);
//   byte w = pgm_read_byte(font + 1 + _char * len);
//   column += dx;
//   for (byte i = 0; i < w; i++)
//     if(column + i >= 0 && column + i < 8 * NUM_MAX) 
//     {
//       byte _value = pgm_read_byte(font + 1 + _char * len + 1 + i);
//       if(!dy) scr[column + i] = _value;
//       else scr[column + i] |= dy > 0 ? _value >> dy : _value <<- dy;
// //        if(!dy) scr[column + i] = _value; else scr[column + i] |= dy > 0 ? _value <<- dy : _value >> dy;
//     }
// }
// //--------------------------------------------------------------------------------------------------------------
// void setCol(byte column, byte _value)
// {
//   if(dy <- 8 | dy > 8) return;
//   column += dx;
//   if(column >= 0 && column < 8 * NUM_MAX)
//     if(!dy) scr[column] = _value;
//     else scr[column] |= dy > 0 ? _value >> dy : _value <<- dy;
// }
// //------------------------------------------------------------------------------------------------------------
// byte showChar(char _char, const uint8_t *font)
// {
//   byte len = pgm_read_byte(font);
//   byte i,w = pgm_read_byte(font + 1 + _char * len);
//   for (i = 0; i < w; i++)
//     scr[NUM_MAX * 8 + i] = pgm_read_byte(font + 1 + _char * len + 1 + i);
//     scr[NUM_MAX * 8 + i] = 0;
//   return w;
// }
// //-------------------------------------------------------------------------------------------------------------*/
