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

volatile uint8_t boomR = 2;
volatile uint8_t rocketw = 19;
uint8_t rocketh = 2;

volatile float x = LCDWIDTH/2;
volatile float x0 = 1;
volatile float y = LCDHEIGHT/2+10; 
volatile float y0 = -1;
volatile uint8_t potValue = 0; //значение с потенциометра
uint32_t globalcounter = 0;
uint16_t framerate = 50;
bool endflag = false;
bool levelflag = false;
uint8_t width = 83; //ширина дисплея
uint8_t height = 47; //высота дисплея
String title = "GAMEOVER";
//-----decition-------------
int8_t xarr[]={ 0,-1,-2,-2,-2,-1,0,1,2,2, 2,-1}; //x coordinate
int8_t yarr[]={-2,-2,-1, 0, 1, 2,2,2,1,0,-1,-2}; //y coordinate
uint8_t xyvector[12]; //vector bool/direction
// float xvec[] = {-1.306562964876377,-1.0,-0.541196100146197, 0, 0.541196100146197, 1, 1.306562964876377};
// float yvec[] = {-0.541196100146197,-1.0,-1.306562964876377,-2,-1.306562964876377,-1,-0.541196100146197};
float xvec[] = {-0.92387953251128675612818318939679,-0.70710678118654752440084436210485,-0.3826834323650897717284599840304,  0.0,                                0.3826834323650897717284599840304,  0.70710678118654752440084436210485, 0.92387953251128675612818318939679};
float yvec[] = {-0.3826834323650897717284599840304, -0.70710678118654752440084436210485,-0.92387953251128675612818318939679,-0.70710678118654752440084436210485,-0.92387953251128675612818318939679,-0.70710678118654752440084436210485,-0.3826834323650897717284599840304};
//---------------------------------------------------
void drawbackground();
void drawrocket();
void drawball();
void clearent();
void wintest();
void ADC_init();
void leveldots(uint8_t i);
//=====================================================
void setup()   
{
  display.begin();
  display.clearDisplay();
  randomSeed(analogRead(A0));
  ADC_init();
  display.setTextSize(1);
  display.setTextColor(BLACK);
  potValue = map(ADCH,0,255,width - rocketw,0); 
}

void loop() 
{
  for(uint8_t i = 1; i < 11; i++)
  {
    display.clearDisplay();
    display.setCursor(20,20);
    display.print("Level ");display.println(i);
    display.display();
    delay(2000);
    display.clearDisplay();
    x = potValue;
    x0 = 0;
    y = LCDHEIGHT/2+10; 
    y0 = -1;
    //prepyatstvia
    leveldots(i);
    //-----------
    levelflag = false;
    while(levelflag == false)
    {
      if(endflag == false)
      {
        drawbackground();
        drawrocket();
        drawball();
        display.display();
        delay(framerate);
        clearent();
        x += x0;
        y += y0;
        if(y > height || x > width) // - life
        {
          y = 20;
          x = potValue;
          y0 = -1;
          rocketw--;
          if(rocketw < 2)
            endflag = true;
        }
        wintest();
      }else{
        display.clearDisplay();
        display.setCursor(20,20);
        display.println(title);
        display.display();
        delay(50000);
      }
    }
    //level completed
    display.clearDisplay();
    display.setCursor(20,20);
    display.print("Level ");display.println(i);
    display.setCursor(20,30);
    display.println("Completed");
    display.display();
    delay(1000);
    endflag = false;
    levelflag = true;
  }
  //win
  display.clearDisplay();
  display.setCursor(20,20);
  display.println("You WIN!");
  display.display();
  delay(50000);
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

void drawball()
{
  for(uint8_t i = 0; i < 12; i++)
  {
    xyvector[i] = display.getPixel(x + xarr[i],y + yarr[i]);
  }
  if(xyvector[1] == BLACK && xyvector[2] == BLACK)  //top left corner
  {
    x0 = -x0;
    y0 = -y0;
    display.fillCircle(x - 1,y - 1,boomR,WHITE);
  }
  else if(xyvector[7] == BLACK && xyvector[8] == BLACK) //bottom right corner
  {
    x0 = -x0;
    y0 = -y0;
    display.fillCircle(x + 1,y + 1,boomR,WHITE);
  }
  else if(xyvector[4] == BLACK && xyvector[5] == BLACK) //bottom left corner
  {
    x0 = -x0;
    y0 = -y0;
    display.fillCircle(x - 1,y + 1,boomR,WHITE);
  }
  else if(xyvector[10] == BLACK && xyvector[11] == BLACK) //top right corner
  {
    x0 = -x0;
    y0 = -y0;
    display.fillCircle(x + 1,y - 1,boomR,WHITE);
  }
  else if(xyvector[11] == BLACK || xyvector[0] == BLACK || xyvector[1] == BLACK) //top
  {
    y0 = -y0;
    if(y > 3) //top
      display.fillCircle(x + xarr[0],y + yarr[0],boomR,WHITE);
  }
  else if(xyvector[5] == BLACK || xyvector[6] == BLACK || xyvector[7] == BLACK) // bottom
  {
    y0 = -y0;
    if(y == height - 3) //bottom\rocket
    {
      int8_t xrocket = x - (potValue - 2);
      xrocket = map(xrocket,0,rocketw,0,6);
      x0 = xvec[xrocket];
      y0 = yvec[xrocket];
    }
    else 
      display.fillCircle(x + xarr[6],y + yarr[6],boomR,WHITE);
  }
  else if(xyvector[2] == BLACK || xyvector[3] == BLACK || xyvector[4] == BLACK ||
   xyvector[8] == BLACK || xyvector[9] == BLACK || xyvector[10] == BLACK) // left or right
  {
    x0 = -x0;
    if((xyvector[8] == BLACK || xyvector[9] == BLACK || xyvector[10] == BLACK) && x != width-3)
      display.fillCircle(x + xarr[9],y + yarr[9],boomR,WHITE);
    else if(x > 3)
      display.fillCircle(x + xarr[3],y + yarr[3],boomR,WHITE);
  }
  display.drawCircle(x, y, 2, BLACK);
}

void drawrocket()
{
  //potValue = ADCH;  // ADLAR=1, Получаем 8-битный результат, остальными битами пренебрегаем
  potValue = map(ADCH,0,255,width - rocketw,0); 
  display.drawRect(potValue,height - 1,rocketw,rocketh,BLACK);
}

void clearent()
{
  display.fillCircle(x, y, 2, WHITE); //ball
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
    for(uint8_t j = 5; j <= 30; j++)
    {
      summ += display.getPixel(i,j);
    }
  }
  if(summ == 0)
    levelflag = true;
}
void leveldots(uint8_t i)
{
  switch (i)
  {
    case 1:
      display.writeFillRect(5,5,width - 10,10,BLACK);
      boomR = 5;
      break;
    case 2:
      for(uint8_t i = 0; i < 7; i++) //prepyatstvia
      {
        uint8_t xr = random(5,width - 10);
        uint8_t yr = random(5,25);
        display.writeFillRect(xr,yr,5,5,BLACK);
      }
      framerate = 45;
      break;
    case 3:
      display.fillTriangle(width/2,25,5,5,width-10,5,BLACK);
      boomR = 4;
      framerate = 40;
      break;
    case 4:
      for(uint8_t i = 10;i < width - 10; i+=10)
        display.drawCircle(i,10,5,BLACK);
      framerate = 35;
      break;
    case 5:
      display.fillCircle(10,10,5,BLACK);
      display.fillCircle(width - 10,10,5,BLACK);
      display.fillCircle(width/2,height/2,5,BLACK);
      boomR = 3;
      framerate = 30;
      break;
    case 6:
      display.setTextSize(2);
      display.setCursor(7,5);
      display.println("DO IT!");
      display.setCursor(7,20);
      display.println("DO IT!");
      display.setTextSize(1);
      framerate = 40;
      break;
    case 7:
      display.setCursor(5,5);
      display.fillCircle(width/2,1,25,BLACK);
      break;
    case 8://lines
      for(uint8_t i = 0; i < 10; i++) //prepyatstvia
      {
        uint8_t xr1 = random(5,width - 5);
        uint8_t yr1 = random(5,height - 15);
        uint8_t xr2 = random(5,width - 5);
        uint8_t yr2 = random(5,height - 15);
        display.drawLine(xr1,yr1,xr2,yr2,BLACK);
      }
      boomR = 2;
      framerate = 20;
      break;
    case 9:
      for(uint8_t i = 0; i < 5; i++) //prepyatstvia
      {
        uint8_t xr = random(10,width - 15);
        uint8_t yr = random(10,height - 15);
        display.fillCircle(xr,yr,5,BLACK);
      }
      break;
    case 10:
      display.drawRoundRect(5,5,width - 10,10,3,BLACK);
      break;
  }
}