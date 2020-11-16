// Цвета которые используются, подбор цвета https://trolsoft.ru/ru/articles/rgb565-color-picker
#define CHART_FON       0x0883  // цвет фона графика если вентилятор не работает
#define CHART_FON_FAN   0x39EA  // цвет фона графика если вентилятор работает
#define CHART_GRID      0xD6EE  // цвет сетки графика
#define CHART_AXIS      0x6338  // цвет осей графика
#define CHART_LABEL     0xDEC4  // цвет надписей графика
#define ILI9341_GREY    0x6B6D  // серый
#ifdef DEMO
  const char TIME_LABEL[] = "0m   2m   4m   6m   8m";  // метки времени на оси х для демо
#else
  const char TIME_LABEL[] = "0h        5h        10h"; // метки времени на оси х для work
#endif
#define hRow 18 // высота в пикселях ряда
#define maxString 64
static char OutputBuf[maxString + 1] = "";

// Переключиться на вывод на дисплей true  если удалось
__attribute__((always_inline)) inline boolean switchTFT()  
{
   if(xSemaphoreTake(xTFTSemaphore, 5000) == pdFALSE) {  // захватываем семафор
            #ifdef DEBUG
            Serial.println("Error take semaphore");
            #endif 
           return false;
        } 
  vTaskSuspendAll();       
  SPI.setModule(2);
  SPI.setClockDivider(SPI_CLOCK_DIV2);
  return true;
}

 // Переключиться на сеть, обычный режим
__attribute__((always_inline)) inline void  switchNET()
{
  SPI.setModule(1);
  SPI.setClockDivider(SPI_CLOCK_DIV2);
  xSemaphoreGive(xTFTSemaphore);
  xTaskResumeAll();
}

void reset_ili9341(void)
{
  pinMode(pinTFT_RST, OUTPUT);                    // Сброс дисплея сигнал активным является LOW
  digitalWrite(pinTFT_RST, LOW);  
  _delay(50);
  digitalWrite(pinTFT_RST, HIGH);  
}
void print_static()  // Печать статической картинки 
{
  uint8_t i;
  switchTFT(); 
  if (GETBIT(setting.flag,fTFT_180)) tft.setRotation(3); else tft.setRotation(1); // Установить орентацию дисплея
  tft.fillScreen(ILI9341_BLACK);
  tft.fillRect(0, 0, 320, 20, ILI9341_BLUE);
  tft.setTextColor(ILI9341_WHITE); 
  tft.drawString(utf8rus((char*)"Ток (мА)"),2,2,2);
  #ifdef DEMO
     tft.setTextColor(ILI9341_RED); 
     tft.drawString(utf8rus((char*)"DEMO"),2+105,2,2);
  #endif

 // Таблица для данных
  for(i=0;i<5;i++) {tft.drawLine(0, (1+i)*hRow+2, 320, (1+i)*hRow+2, ILI9341_GREEN);} // Горизонтальные линии
   tft.drawLine(210-2, 1*hRow+2, 210-2, 5*hRow+2, ILI9341_GREEN);
   tft.drawLine(270-2, 1*hRow+2, 270-2, 5*hRow+2, ILI9341_GREEN);
   
   tft.setTextColor(ILI9341_YELLOW);  // Заголовки
   tft.drawString(utf8rus((char*)"Подпол"),0+210,3+hRow*1,2);  
   tft.drawString(utf8rus((char*)"Улица"),0+270,3+hRow*1,2);
   tft.drawString(utf8rus((char*)"Температура градусы C\xB0"),0,3+hRow*2,2);  
   tft.drawString(utf8rus((char*)"Относительная влаж. %"),0,3+hRow*3,2);
   tft.drawString(utf8rus((char*)"Абсолют. влаж. г/м*3" ),0,3+hRow*4,2);  

  // Графики
   tft.setTextColor(CHART_LABEL);  // Заголовки
   tft.drawString(utf8rus((char*)"Температура"),0+20,3+hRow*5,2);  
   tft.drawString(utf8rus((char*)"Абс. влажность"),0+170,3+hRow*5,2);  
 
   tft.drawLine(1, 240-5-hRow, 130, 240-5-hRow, CHART_AXIS);
   tft.drawLine(320-130-25, 240-5-hRow, 320-1-25, 240-5-hRow, CHART_AXIS);
   
   tft.drawLine(1, 240-5-hRow, 1, 240-5-105-hRow, CHART_AXIS);                    // гор
   tft.drawLine(320-130-25, 240-5-hRow,320-130-25, 240-1-5-105-hRow, CHART_AXIS); // вер
 
   // надписи на графиках
   tft.setTextColor(CHART_LABEL);
   tft.setCursor(0+130,123); tft.print("+20.0");
   tft.setCursor(0+130,162); tft.print("0.0");
   tft.setCursor(0+130,202); tft.print("-20.0");

   tft.setCursor(0+295,112); tft.print("20.0");
   tft.setCursor(0+295,137); tft.print("15.0");
   tft.setCursor(0+295,162); tft.print("10.0");
   tft.setCursor(0+295,187); tft.print("5.0");
   tft.setCursor(0+295,212); tft.print("0.0");
   // ось времени
   tft.setTextColor(CHART_LABEL); 
   tft.setCursor(2,219);
   tft.print((char*)TIME_LABEL); tft.setCursor(162,220); tft.print((char*)TIME_LABEL); // Вывод шкалы времени

   
// Нижня строка статуса
   tft.setTextColor(ILI9341_WHITE); 
   tft.setCursor(2,229);
   tft.print("ver. ");  tft.print(VERSION);  tft.print("   IP:");tft.print(IPAddress2String(realIP));  tft.print(" mac:");
   for(i=0;i<6;i++){tft.print(mac[i],HEX);if(i!=5) tft.print(":");}
 
 switchNET();
}

//  вывод на экран данных (то что меняется)
void print_data(void)
{ 
char buf[10];  
// Статистика по моточасам, время ведется в интервалах вывода данных = NUM_SAMPLES*TIME_SCAN_SENSOR а потом пересчитывается в часы при выводе.
setting.hour_unit++;
if (FLAG_FAN_CHECK) setting.hour_motor++;  // если мотор включен
#ifdef USE_HEAT
if (FLAG_HEAT_CHECK) setting.hour_heat++;    // если нагреватель включен
#endif
buf[0]=0;
switchTFT();  
 // Печать тока потребления
   tft.fillRect(65,2,42,hRow-1, ILI9341_BLUE);
   if(!FLAG_TEST_CHECK) tft.setTextColor(ILI9341_RED); else tft.setTextColor(ILI9341_WHITE); // цвет в зависимости то теста
   tft.drawString(_itoa(sensors.CurrentACS758,buf),65,2,2);  
 // Печать значений для дома
   tft.setTextColor(ILI9341_RED); 
   tft.fillRect(0+212,3+hRow*2,55,hRow-1, ILI9341_BLACK); tft.drawString(ftoa(buf,(float)(sensors.av_tIn)/100.0,2),0+212,3+hRow*2,2);  
   tft.fillRect(0+212,3+hRow*3,55,hRow-1, ILI9341_BLACK); tft.drawString(ftoa(buf,(float)(sensors.av_relHIn)/100.0,2),0+212,3+hRow*3,2);  
   tft.fillRect(0+212,3+hRow*4,55,hRow-1, ILI9341_BLACK); tft.drawString(ftoa(buf,(float)(sensors.av_absHIn)/100.0,2),0+212,3+hRow*4,2);  
 
 // Печать значений для улицы
   tft.setTextColor(ILI9341_GREEN); 
   tft.fillRect(0+272,3+hRow*2,55,hRow-1, ILI9341_BLACK); tft.drawString(ftoa(buf,(float)(sensors.av_tOut)/100.0,2),0+272,3+hRow*2,2); 
   tft.fillRect(0+272,3+hRow*3,55,hRow-1, ILI9341_BLACK); tft.drawString(ftoa(buf,(float)(sensors.av_relHOut)/100.0,2),0+272,3+hRow*3,2); 
   tft.fillRect(0+272,3+hRow*4,55,hRow-1, ILI9341_BLACK); tft.drawString(ftoa(buf,(float)(sensors.av_absHOut)/100.0,2),0+272,3+hRow*4,2); 
       
 switchNET();  
} 

// Печать графика на экране, добавляется одна точка и график сдвигается 
void printChart() 
{
byte i,x=0,y;
int16_t tmp;
// Работаем через кольцевой буфер
// Добавить новую точку в кольцевой буфер
     chartTemp.dataIn[posChart]=sensors.av_tIn; // Температура в доме. диапазон -25 . . . +25 растягиваем на 100 точек  
     chartTemp.dataOut[posChart]=sensors.av_tOut; // Температура на улице. диапазон -25 . . . +25 растягиваем на 100 точек 
     chartAbsH.dataIn[posChart]=sensors.av_absHIn; // Абсолютная влажность в доме диапазон от 0 до 20 грамм на кубометр, растягиваем на 100 точек 
     if (ChartMotor==true) chartAbsH.dataIn[posChart]=chartAbsH.dataIn[posChart]+0x4000;  // Признак включения мотора- 14 бит в 1 - цвет фона на графике меняется
     ChartMotor=false;
     chartAbsH.dataOut[posChart]=sensors.av_absHOut; // Абсолютная влажность на улицу диапазон от 0 до 20 грамм на кубометр, растягиваем на 100 точек  

 //  digitalWrite(PIN_LED2,LOW);  
   switchTFT();
   for(i=0;i<CHART_POINT;i++)    // График слева на право
     { 
     // Вычислить координаты текущей точки x в кольцевом буфере. Изменяются от 0 до 120-1
     if (posChart<i) x=120+posChart-i; else x=posChart-i;
     // нарисовать фон в зависимости от статуса мотора
     if  (chartAbsH.dataIn[x]&0x4000){tft.drawLine(3+i,240-6-hRow,3+i,240-6-105-hRow,CHART_FON_FAN);tft.drawLine(320-130-23+i,240-6-hRow,320-130-23+i,240-6-105-hRow,CHART_FON_FAN);}// Мотор был ключен
     else                            {tft.drawLine(3+i,240-6-hRow,3+i,240-6-105-hRow,CHART_FON); tft.drawLine(320-130-23+i,240-6-hRow,320-130-23+i,240-6-105-hRow,CHART_FON);}// Мотор был выключен

     if (x%5==0) // Пунктирные линии графика
     {
       tft.drawPixel(3+i,240-6-hRow-10,CHART_GRID);
       tft.drawPixel(3+i,240-6-hRow-50,CHART_GRID);
       tft.drawPixel(3+i,240-6-hRow-90,CHART_GRID);
       tft.drawPixel(320-130-23+i,240-6-hRow-25,CHART_GRID);
       tft.drawPixel(320-130-23+i,240-6-hRow-50,CHART_GRID);
       tft.drawPixel(320-130-23+i,240-6-hRow-75,CHART_GRID);
       tft.drawPixel(320-130-23+i,240-6-hRow-100,CHART_GRID);
     }

     // Вывести новую точку
      if (chartTemp.dataIn[x]<=-2500) y=0;          // Если температура меньше -25 то округляем до -25
      else  if (chartTemp.dataIn[x]>=2500) y=100;   // Если температура больше 25  то округляем до 25
      else y=((2*chartTemp.dataIn[x])+5000)/100;    // внутри -25...+25 растягиваем в два раза
      if ((y==0)||(y==100)) tft.drawPixel(3+i,240-6-hRow-y,ILI9341_WHITE); else  tft.drawPixel(3+i,240-6-hRow-y,ILI9341_RED);
      
      if (chartTemp.dataOut[x]<=-2500) y=0;          // Если температура меньше -25 то округляем до -25
      else  if (chartTemp.dataOut[x]>=2500) y=100;   // Если температура больше 25  то округляем до 25
      else y=((2*chartTemp.dataOut[x])+5000)/100;    // внутри -25...+25 растягиваем в два раза
      if ((y==0)||(y==100)) tft.drawPixel(3+i,240-6-hRow-y,ILI9341_WHITE); else tft.drawPixel(3+i,240-6-hRow-y,ILI9341_GREEN);
   
      tmp=chartAbsH.dataIn[x]&0xbfff;             // Обнулить 14 разряд - признак включения мотора
      if (tmp>=2000) y=100;
      else y=(5*tmp)/100;                         // внутри 0...20 растягиваем в пять  раз     
      if (y==100) tft.drawPixel(320-130-23+i,240-6-hRow-y,ILI9341_WHITE);else tft.drawPixel(320-130-23+i,240-6-hRow-y,ILI9341_RED);
       
      if (chartAbsH.dataOut[x]>=2000) y=100;
      else y=(5*chartAbsH.dataOut[x])/100;   
      if (y==100) tft.drawPixel(320-130-23+i,240-6-hRow-y,ILI9341_WHITE);else tft.drawPixel(320-130-23+i,240-6-hRow-y,ILI9341_GREEN);    
  //    digitalWrite(PIN_LED2,HIGH);  
        }
 if (posChart<120-1) posChart++; else posChart=0;            // Изменили положение в буфере и Замкнули буфер
 switchNET();
} 

// Индикатор загрузки контроллера
#define hLoad 1  // Толщина индикатора
static uint16_t cpu_old=0;
void print_LoadCPU()
{
 uint16_t cpu; 
 cpu = 320 - 320*lastPeriodIdleValue/(1 << periodLen);
 if (cpu==cpu_old) return; // рисовать нечего выходим
 switchTFT();
 tft.fillRect(0, 239-hLoad , 320, hLoad, ILI9341_BLACK); // Стирание
 tft.fillRect(0, 239-hLoad, cpu, hLoad, ILI9341_WHITE); // Загрузка
 switchNET();
 cpu_old=cpu;
}

const unsigned char ventLogo []  = {  // перевод картинки http://javl.github.io/image2cpp/
  0x00, 0x00, 0x00, 0x01, 0xf0, 0x00, 0x01, 0xf8, 0x00, 0x03, 0xf8, 0x00, 0x03, 0xf0, 0x00, 0x01, 
  0xe0, 0x00, 0x71, 0xc0, 0x00, 0xf9, 0x2f, 0x80, 0xfe, 0x1f, 0x80, 0xfe, 0xdf, 0xc0, 0x7e, 0x0f, 
  0xc0, 0x38, 0xc7, 0x80, 0x00, 0xe3, 0x00, 0x01, 0xe0, 0x00, 0x07, 0xf0, 0x00, 0x07, 0xe0, 0x00, 
  0x03, 0xe0, 0x00, 0x01, 0x80, 0x00
};
const unsigned char heatLogo []  = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x80, 0x00, 0x01, 0xc0, 0x00, 0x01, 0xe0, 0x00, 0x11, 
  0xf0, 0x00, 0x1b, 0xf6, 0x00, 0x1f, 0xf6, 0x00, 0x1f, 0xfe, 0x00, 0x1f, 0xbf, 0x00, 0x3f, 0xbf, 
  0x00, 0x3f, 0x9f, 0x00, 0x37, 0x19, 0x00, 0x36, 0x03, 0x00, 0x30, 0x03, 0x00, 0x18, 0x06, 0x00, 
  0x08, 0x04, 0x00, 0x00, 0x00, 0x00
};
const unsigned char netLogo [] = { 
  0x00, 0x00, 0x00, 0x01, 0xe0, 0x00, 0x02, 0x10, 0x00, 0x02, 0x10, 0x00, 0x02, 0x10, 0x00, 0x03, 
  0x30, 0x00, 0x00, 0xc0, 0x00, 0x00, 0xc0, 0x00, 0x1f, 0xfe, 0x00, 0x3f, 0xff, 0x00, 0x20, 0xc1, 
  0x00, 0x30, 0xc3, 0x00, 0x8b, 0x34, 0x40, 0x8a, 0x14, 0x40, 0x8a, 0x14, 0x40, 0x8a, 0x14, 0x40, 
  0xf9, 0xe7, 0xc0, 0x00, 0x00, 0x00
};
#define icoX 210-65 // Положение иконок на панеле управления
static boolean blinkFan=false;
void print_status(boolean net) // Печать панели статуса Значки на статус панели
{
 switchTFT();
 if (!FLAG_TEST_CHECK) // Тест не пройден
 {if(blinkFan) tft.drawBitmap(icoX, 1, ventLogo, 18, 18, ILI9341_RED); else tft.drawBitmap(icoX, 1, ventLogo, 18, 18, ILI9341_GREY);  blinkFan=!blinkFan;} // мигание иконки при отказе вентилятора
 else{ 
 if (FLAG_FAN_CHECK) tft.drawBitmap(icoX, 1, ventLogo, 18, 18, ILI9341_WHITE); else tft.drawBitmap(icoX, 1, ventLogo, 18, 18, ILI9341_GREY);} // 1. Признак включения мотора
#ifdef USE_HEAT
 if (FLAG_HEAT_CHECK) tft.drawBitmap(icoX+20, 1, heatLogo, 18, 18, ILI9341_WHITE); else tft.drawBitmap(icoX+20, 1, heatLogo, 18, 18, ILI9341_GREY); // 2. Признак включения калорифера
#else
 tft.drawBitmap(icoX+20, 1, heatLogo, 18, 18, ILI9341_GREY);
#endif 
 if (net) tft.drawBitmap(icoX+40, 1, netLogo, 18, 18, ILI9341_WHITE); else tft.drawBitmap(icoX+40, 1, netLogo, 18, 18, ILI9341_GREY);// 1. Признак линка сети
 switchNET();
}

#define errX 280-68    // координата Х для кода ошибки
void print_error_AHT(void) // Печать ошибки чтения датчиков выводится 1 раз за TIME_UPDATE_TFT мсек
{
  uint16_t err=sensors.outErr+(sensors.inErr<<8);  // Суммараная ошибка
  if (err!=last_error)     // если статус ошибки поменялся то надо вывести если нет то не выводим - экономия время и нет мерцания
  {
      last_error=err; 
      switchTFT();
      tft.fillRect(errX,2,65-1,hRow,ILI9341_BLUE);  // Сначала стереть
      tft.setTextColor(ILI9341_WHITE);  
      if (err>0) tft.drawString(uint16ToHex(err),errX,2,2);
      else       tft.drawString((char*)"   Ok",errX,2,2);
      switchNET();  
   }   
}   
#define timeX 276    // координата Х для времени
void print_time(void) // Печать ошибки чтения датчиков выводится 1 раз за TIME_UPDATE_TFT мсек
{
  
rt.breakTime(rt.now(),tm);
OutputBuf[0]=0;
if(tm.hour<10)strcpy(OutputBuf,"0");_itoa(tm.hour,OutputBuf);strcat(OutputBuf," ");if(tm.minute<10)strcat(OutputBuf,"0");_itoa(tm.minute,OutputBuf);
 switchTFT();
 tft.fillRect(timeX,2,42,hRow,ILI9341_BLUE);  // Сначала стереть
 if(FLAG_NTP_CHECK) tft.setTextColor(ILI9341_WHITE); else tft.setTextColor(ILI9341_PINK); // Разный цвет в зависимости от обновления по NTP
 tft.drawString(OutputBuf,timeX,2,2);
 if(FLAG_NET_CHECK)  tft.drawString((char*)":",timeX+19,2,2); // Мигающее двоеточие - признак работы веб серевера
 switchNET();  
}   

// Вывод информации о настройках и сохрание индекса настроек в eeprom ---------------------------------
void print_setting()
{
  switchTFT();
 // Настройка
  tft.fillRect(0,3+hRow*1,208-1,hRow-1, ILI9341_BLACK);// Стереть
  tft.setTextColor(ILI9341_WHITE);  // Заголовки
  switch (setting.mode)
        {
        case  BLOCK_OFF:          tft.drawString(utf8rus((char*)strMode[BLOCK_OFF]),0,3+hRow*1,2);  setting.dH_min=255;setting.T_min=255; break; 
        case  HOOD_ON:            tft.drawString(utf8rus((char*)strMode[HOOD_ON]),0,3+hRow*1,2);  setting.dH_min=0;  setting.T_min=0;   break; 
        case  COOLING:            tft.drawString(utf8rus((char*)strMode[COOLING]),0,3+hRow*1,2);  setting.dH_min=500;setting.T_min=1000;break;   // dH_min используется не штатно для температуры     
        case  DEHUMIDIFIER_T3_H3: tft.drawString(utf8rus((char*)strMode[DEHUMIDIFIER_T3_H3]),0,3+hRow*1,2);  setting.dH_min=30; setting.T_min=300; break;
        case  DEHUMIDIFIER_T3_H6: tft.drawString(utf8rus((char*)strMode[DEHUMIDIFIER_T3_H6]),0,3+hRow*1,2);  setting.dH_min=60; setting.T_min=300; break;
        case  DEHUMIDIFIER_T4_H4: tft.drawString(utf8rus((char*)strMode[DEHUMIDIFIER_T4_H4]),0,3+hRow*1,2);  setting.dH_min=40; setting.T_min=400; break;
        case  DEHUMIDIFIER_T4_H8: tft.drawString(utf8rus((char*)strMode[DEHUMIDIFIER_T4_H8]),0,3+hRow*1,2);  setting.dH_min=80; setting.T_min=400; break;
        case  DEHUMIDIFIER_T5_H8: tft.drawString(utf8rus((char*)strMode[DEHUMIDIFIER_T5_H8]),0,3+hRow*1,2);  setting.dH_min=80; setting.T_min=500; break;
        default:                  tft.drawString(utf8rus((char*)strMode[BLOCK_OFF]),0,3+hRow*1,2);  setting.dH_min=255;setting.T_min=255; setting.mode=BLOCK_OFF;break; 
        } 
 switchNET();
}
