#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>

// Hardware SPI (faster, but must use certain hardware pins):
// SCK is LCD serial clock (SCLK) - this is pin 13 on Arduino Uno
// MOSI is LCD DIN - this is pin 11 on an Arduino Uno
// pin 5 - Data/Command select (D/C)
// pin 4 - LCD chip select (CS)
// pin 3 - LCD reset (RST)
Adafruit_PCD8544 display = Adafruit_PCD8544(5, 4, 3);
// Note with hardware SPI MISO and SS pins aren't used but will still be read
// and written to during SPI transfer.  Be careful sharing these pins!

//#define framerate 200

static const unsigned char PROGMEM ball[] =
{ 
  B01110000,
  B11011000,
  B10001000,
  B11011000,
  B01110000
};
#define ballw 5
#define ballh 5
#define boomR 2
uint8_t rocketw = 15;
uint8_t rocketh = 2;

volatile int8_t x = LCDWIDTH/2;
volatile int8_t x0 = 1;
volatile int8_t y = LCDHEIGHT/2; 
volatile int8_t y0 = -1;
volatile int potValue = 0; //значение с потенциометра
uint32_t globalcounter = 0;
uint16_t framerate = 50;
bool endflag = false;
uint8_t width = 83; //ширина дисплея
uint8_t height = 47; //высота дисплея
String title = "GAMEOVER";
//-----decition-------------
int8_t xarr[]={0,-1,-2,-1,0,1,2,1}; //x coordinate
int8_t yarr[]={-2,-1,0,1,2,1,0,-1}; //y coordinate
uint8_t xyvector[8]; //vector bool/direction
//---------------------------------------------------
void drawbackground();
void drawrocket();
void drawball(const uint8_t *bitmap, uint8_t w, uint8_t h);
void clearent(const uint8_t *bitmap1, uint8_t w1, uint8_t h1);
void wintest();
void ADC_init();
//void testdrawbitmap(const uint8_t *bitmap, uint8_t w, uint8_t h);
//=====================================================
void setup()   
{
  display.begin();
  display.clearDisplay();
  randomSeed(analogRead(A0));
  ADC_init();
  // for(uint8_t i = 0; i<5;i++) //prepyatstvia
  // {
  //   uint8_t xr = random(width);
  //   uint8_t yr = random(height);
  //   display.writeFillRect(xr,yr,5,5,BLACK);
  // }
  display.writeFillRect(5,5,width -10,10,BLACK);
}

void loop() 
{
  if(endflag == false)
  {
    drawbackground();
    drawrocket();
    drawball(ball, ballw, ballh);
    display.display();
    delay(framerate);
    clearent(ball, ballw, ballh);
    x += x0;
    y += y0;
    // globalcounter++;
    // if(globalcounter%50 == 0 && framerate > 20)
    //   framerate--;
    if(y>height) //endgame
    {
      y = 10;
      rocketw--;
      if(rocketw < 2)
        endflag = true;
    }
    wintest();
  }else{
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(BLACK);
    display.setCursor(20,20);
    display.println(title);
    display.display();
    delay(50000);
  }
}

void ADC_init()
{
  ADCSRA = 0;             // Сбрасываем регистр ADCSRA
  ADCSRB = 0;             // Сбрасываем регистр ADCSRB
  ADMUX |= (1 << REFS0);  // Задаем ИОН
  ADMUX |= (1 << ADLAR);  // Меняем порядок записи бит, чтобы можно было читать только 8 бит регистра ADCH
  ADMUX |= (0 & 0x07);    // Выбираем пин A0 для преобразования
  // Устанавливаем предделитель - 64 (ADPS[2:0]=111) частота работы АЦП - 125 кГц
  ADCSRA |= (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);      //Битам ADPS2 и ADPS1 и ADPS0 присваиваем единицу
  ADCSRA |= (1 << ADATE); // Включаем автоматическое преобразование
  //ADCSRA |= (1 << ADIE);  // Разрешаем прерывания по завершении преобразования
  ADCSRA |= (1 << ADEN);  // Включаем АЦП
  ADCSRA |= (1 << ADSC);  // Запускаем преобразование
}

void drawball(const uint8_t *bitmap, uint8_t w, uint8_t h)
{
  int8_t xoffset = x + 2;
  int8_t yoffset = y + 2;
  for(uint8_t i = 0; i < 8; i++)
  {
    xyvector[i] = display.getPixel(xoffset + xarr[i],yoffset + yarr[i]);
  }
  //not need to second for
  for(uint8_t i = 0; i < 8; i++)
  {
    if(xyvector[i] == BLACK)
    {
      if(i == 0 || i == 4) // bottom or top
      {
        y0 = -y0;
        if(y == height - 5) //bottom\rocket
        {
          int8_t xrocket = xoffset - potValue;
          x0 = map(xrocket,0,rocketw,1,-2);
          framerate = random(20,51);
        }
        else if(y != 1) //top
          display.fillCircle(xoffset + xarr[i],yoffset + yarr[i],boomR,WHITE);
        else //bottom
          display.fillCircle(xoffset + xarr[i],yoffset + yarr[i],boomR,WHITE);
      }
      else if(i == 2 || i == 6) // left or right
      {
        x0 = -x0;
        if(i == 6 && x != width-1)
          display.fillCircle(xoffset + xarr[i],yoffset + yarr[i],boomR,WHITE);
        else if(x != 1)
          display.fillCircle(xoffset + xarr[i],yoffset + yarr[i],boomR,WHITE);
      }
      else if(i == 1 || i == 3 || i == 5 || i == 7)
      {
        x0 = -x0;
        y0 = -y0;
        display.fillCircle(xoffset + xarr[i],yoffset + yarr[i],boomR,WHITE);
      }
      break;
    }
  }
  display.drawBitmap(x, y, bitmap, w, h, BLACK);
}

void drawrocket()
{
  //potValue = ADCH;  // ADLAR=1, Получаем 8-битный результат, остальными битами пренебрегаем
  potValue = map(ADCH,0,255,width-rocketw,0); 
  display.drawRect(potValue,height-1,rocketw,rocketh,BLACK);
}

void clearent(const uint8_t *bitmap1, uint8_t w1, uint8_t h1)
{
  display.drawBitmap(x, y, bitmap1, w1, h1, WHITE);
  display.drawRect(potValue,height-1,rocketw,rocketh,WHITE);
}

void drawbackground()
{
  display.drawLine(0,0,0,height,BLACK); // left line
  display.drawLine(83,0,83,height,BLACK); //right line
  display.drawLine(0,0,width,0,BLACK); //top line
}
void wintest()
{
  uint16_t summ = 0;
  for(uint8_t i = 5; i < width - 5; i++)
  {
    for(uint8_t j = 5; j < 10; j++)
    {
      summ += display.getPixel(i,j);
    }
  }
  if(summ == 0)
  {
    endflag = true;
    title = "YOU WIN!";
  }
}
// void testdrawbitmap(const uint8_t *bitmap, uint8_t w, uint8_t h) {
//   uint8_t icons[NUMFLAKES][3];
//   randomSeed(666);     // whatever seed
 
//   // initialize
//   for (uint8_t f=0; f< NUMFLAKES; f++) {
//     icons[f][XPOS] = random(display.width());
//     icons[f][YPOS] = 0;
//     icons[f][DELTAY] = random(5) + 1;
    
//     Serial.print("x: ");
//     Serial.print(icons[f][XPOS], DEC);
//     Serial.print(" y: ");
//     Serial.print(icons[f][YPOS], DEC);
//     Serial.print(" dy: ");
//     Serial.println(icons[f][DELTAY], DEC);
//   }

//   while (1) {
//     // draw each icon
//     for (uint8_t f=0; f< NUMFLAKES; f++) {
//       display.drawBitmap(icons[f][XPOS], icons[f][YPOS], bitmap, w, h, BLACK);
//     }
//     display.display();
//     delay(200);
    
//     // then erase it + move it
//     for (uint8_t f=0; f< NUMFLAKES; f++) {
//       display.drawBitmap(icons[f][XPOS], icons[f][YPOS], bitmap, w, h, WHITE);
//       // move it
//       icons[f][YPOS] += icons[f][DELTAY];
//       // if its gone, reinit
//       if (icons[f][YPOS] > display.height()) {
//         icons[f][XPOS] = random(display.width());
//         icons[f][YPOS] = 0;
//         icons[f][DELTAY] = random(5) + 1;
//       }
//     }
//   }
// }
//add something