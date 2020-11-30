//  SPI - 1 использует сетевая карта w5500 (библиотека настроена на использование ТОЛЬКО SPI1), подключение
//  D4   PA7   1_MOSI  
//  D5   PA6   1_MISO  
//  D6   PA5   1_SCK   
//  D7   PA4   1_NSS  CSN_PIN
//  D3   PB0   1_RST  // Сброс W5500

//  SPI - 2 использует дисплей (библиотеке надо указать какой SPI исрользовать), подключение
//  D28   PB15  2_MOSI  Yes+
//  D29   PB14  2_MISO  Yes+
//  D30   PB13  2_SCK   Yes+
//  D19   PB12  2_NSS   Yes+    ИЛИ  PB11 
//  D31   PB11  2_DC    TFT_CS  или PB4   
//  D27   PB10  2_RST   TFT_RST или PA8

#include <MapleFreeRTOS821.h>
#include <SPI.h>
#include <EEPROM.h>
#include "Adafruit_GFX_AS.h"
#include "Adafruit_ILI9341_STM.h" //
#include <Ethernet_STM.h>  // Сеть прицецеплена на 1 spi
#include <Wire.h>
#include "utility/w5100.h"
#include <libmaple/iwdg.h>
#include <RTClock.h>

#define DEBUG          // Выводить отладочную информацию в консоль
//#define DEMO           // Режим демо 
#define VERSION    45  // Версия программы
//#define USE_HEAT     // Использовать нагреватель (калорифер) используется SSR2

#include "Podval20.h"  
#include "webserver.h" 
#include "DHT.h"

// Флаги настроек
#define fTFT_180          0                     // Флаг поворота дисплея на 180 градусов
#define fTFT_OFF          1                     // Флаг выключения дисплея 1 выключен
#define fDHCP             2                     // Флаг использования DHCP
#define fNTP              3                     // Флаг обновления времени по NTP
#define fUPDATE           4                     // Флаг обновления страницы каждые 60 сек
#define fBEEP             5                     // Флаг разрешения пищалки
#define fTEST             6                     // Флаг ежедневного тестирования и определениея "0" датчика тока
#define fFULL_WEB         7                     // Флаг показа настроек
#define fAUTO             8                     // Флаг автокалибровки датчика ACS758 
static struct type_setting_eeprom               // Структура для сохранения данных в eeprom
 {
     char name[24]="Dehumidifier pav2000";      // Название прибора  
     TYPE_MODE mode = BLOCK_OFF;                // Какой набор настроек используется
     uint16_t dH_min,T_min;                     // Расшифровка настроек срабатывания см mode  (в сотых грамма на м*3)(в сотых градуса)
     uint16_t flag=0;                           // флаги (бинарные настройки)
     IPAddress ip;                              // ИП адрес
     IPAddress dns;                             // DNS
     IPAddress gateway;                         // Шлюз
     IPAddress mask;                            // Маска подсети
     unsigned long hour_unit;                   // мото часы блок измеряется в интервалах вывода данных = NUM_SAMPLES*TIME_SCAN_SENSOR
     unsigned long hour_motor;                  // мото часы мотора измеряется в интервалах вывода данных = NUM_SAMPLES*TIME_SCAN_SENSOR
     unsigned long hour_heat;                   // мото часы нагревателя измеряется в интервалах вывода данных = NUM_SAMPLES*TIME_SCAN_SENSOR
     uint16_t CurMin=100;                       // минимальный ток потребления вентилятора, если меньше то проблема
     uint16_t CurMax=2000;                      // максимальный ток потребления вентилятора, если больше то проблема
     uint16_t constACS758=2519;                 // Смещене 0 датчика заданное через веб морду
     int16_t eTIN=0;                            // ошибка внутреннего датчика температуры (сотые градуса) ОШИБКА ДОБАВЛЯЕТСЯ!
     int16_t eTOUT=0;                           // ошибка внешнего датчика температуры (сотые градуса) ОШИБКА ДОБАВЛЯЕТСЯ!
     int16_t eHIN=0;                            // ошибка внутреннего датчика влажности (сотые %) ОШИБКА ДОБАВЛЯЕТСЯ!
     int16_t eHOUT=0;                           // ошибка внешнего датчика влажности (сотые %) ОШИБКА ДОБАВЛЯЕТСЯ!
     uint32_t reserve1;                         // Резерв 
     uint32_t reserve2; 
     uint32_t reserve3; 
  } setting;                                    // Рабочая копия настроек в памяти

static struct type_sensors                          // структура датчиков
{
       uint8_t  flags=0x00;                         // байт флагов состояния  0 бит - мотор включен/выключен 1 бит - калорифер включен/выключен 2 бит ошибка вентилятора
       // Ошибки
       uint8_t  inErr,outErr;                       // Последние ошибки чтения датчиков
       uint32_t  numErrIn=0,numErrOut=0;            // Число ошибок чтения датчика
       // Усреднение, с  чем работаем (реализовано скользящее среднее)
       int16_t  av_tOut=-5000,av_tIn=-5000;         // Текущие температуры в сотых градуса ЕСТЬ УСРЕДНЕНИЕ
       int16_t  av_relHOut=5555,av_relHIn=5555;     // Относительные влажности сотых процента ЕСТЬ УСРЕДНЕНИЕ
       int16_t  av_absHOut=5555,av_absHIn=5555;     // Абсолютные влажности в сотых грамма на м*3 ЕСТЬ УСРЕДНЕНИЕ
       // Данные  для усреднения и массив накопленных данных для реализации скользящего среднего
       int16_t  index=0;                            // Текущий индекс в массиве усреднений
       boolean  first=true;                         // признак первого прохода (нет полного массива данных)
       int16_t  tOut=-5000,tIn=-5000;               // Текущие температуры в сотых градуса НЕТ УСРЕДНЕНИЯ    // С датчиков (не обработано)
       int16_t  relHOut=5000,relHIn=5000;           // Относительные влажности сотых процента НЕТ УСРЕДНЕНИЯ  // С датчиков (не обработано)
       int16_t  dat_tOut[NUM_SAMPLES],dat_tIn[NUM_SAMPLES]; // Массивы для усреднения температур
       int16_t  dat_relHOut[NUM_SAMPLES],dat_relHIn[NUM_SAMPLES]; // Массивы для усреднения относительных влажностей
       int32_t  sum_tOut=0,sum_tIn=0;               // Сумма для усреднения Текущие температуры в сотых градуса !!! место экономим
       int32_t  sum_relHOut=0,sum_relHIn=0;         // Сумма для усреднения Относительные влажности сотых процента !!! место экономим
       // Датчик тока
        uint16_t CurrentACS758=0;                   // Текущий ток потребляения mA
        uint16_t offsetACS758=0;                    // ТЕКУЩЕЕ Смещене 0 датчика ДЕСЯТЫЕ мВ!! (в зависимости от настроек)
        uint16_t autoACS758=0;                      // Смещене 0 датчика по автокалибровке ДЕСЯТЫЕ мВ!!
} sensors;

struct type_gchart // Структура для вывода графиков 
{
 int16_t dataIn[CHART_POINT];    // Массив данных подвал температура хранится в сотых градуса, влажность в сотых грамма (дополнительно 14 бит данных влажности используется для хрананеия признака работы вентилятора)
 int16_t dataOut[CHART_POINT];   // Массив данных улица
} chartTemp,chartAbsH;           // Графики температуры и влажности
    
static uint8_t  posChart=0;             // Позиция в массиве графиков - начало вывода от 0 до 120-1
static bool     ChartMotor=false;       // Признак работы мотора во время интервала графика если мотор был включен на любое время то на графике фон меняется в этом месте
static uint16_t last_error=0;           // Предыдущая ошибка чтения датчиков

// Handle задач
TaskHandle_t hReadSensor;
TaskHandle_t hUpdateTFT;
TaskHandle_t hWebTask;
SemaphoreHandle_t xTFTSemaphore;             // Семафор для дисплея

// RTC NTP ----------------------------
#define TZ  3*60*60                     // Часовой пояс в секундах !!!
RTClock rt(RTCSEL_LSE);                 // используется внешний часовой кварц
tm_t tm;                                // хранение текущего времени
tm_t rTime;                             // время последнего сброса
EthernetUDP Udp;                        // Создаем экземпляр класса EthernetUDP, который позволит нам отправлять и получать пакеты через UDP:
unsigned int localPort = 8888;          // локальный порт, который будет прослушиваться на предмет UDP-пакетов
#define _rcc_csr *(uint32_t*)0x40021024 // Адрес Регистра управления/статуса RCC_CSR
uint32_t reg_RCC_CSR;                   // Значение регистр сброса при старте


Adafruit_ILI9341_STM tft = Adafruit_ILI9341_STM(pinTFT_CS, pinTFT_DC,pinTFT_RST);  // дисплей
static boolean fullTftUpdate=false;      // Признак необходимости полного обновления дисплея
SimpleDHT22 inDHT(PIN_IN_DHT);
SimpleDHT22 outDHT(PIN_OUT_DHT);

EthernetServer server(80);
//char inBufGet[2048];                         // входной буффер ответа GET (web server)
//uint32_t connectTime[MAX_SOCK_NUM];          // времена для чистки сокетов
IPAddress realIP;                              // текущий IP адрес (прочитали из сетевого чипа)
uint8_t mac[] = { 0x90, 0xA2, 0xDA, 0x00, 0x51, 0x00 };
  
// Для работы необходимо добавить строчку #define INCLUDE_xTaskGetSchedulerState    1 в файл FreeRTOSConfig.h
inline void _delay(uint t) // Функция задержки (мсек) в зависимости от шедулера задач FreeRtos
{ if(xTaskGetSchedulerState() == taskSCHEDULER_RUNNING) vTaskDelay(t/portTICK_PERIOD_MS); else delay(t);}
///////////////////////////////////////////////////////////////////////////////////////////////////////////
// ПРОГРАММА
///////////////////////////////////////////////////////////////////////////////////////////////////////////
void setup(){
  uint8_t ip[]  = { 192, 168, 1, 175};
  uint8_t i;
  float temperature = 0;
  float humidity = 0;

#ifdef  DEBUG  
   Serial.begin(115200); 
   Serial.println("DEBUG MODE"); 
#endif
   
   // Причина сброса
   reg_RCC_CSR=(uint32_t)_rcc_csr;    // Получить и запомнить причину последнего сброса
   bitSet(_rcc_csr,RCC_CSR_RMVF_BIT);  // Сброс состояния (Очистка флагов сброса)

   // Запретить неиспользуемые устройства на шине SPI1
   pinMode(PIN_CS_W25Q8, OUTPUT);      //  Настройка ноги для CS чипа памяти W25Q8ODVSSIG
   digitalWrite(PIN_CS_W25Q8, HIGH);   //  запретить 
   pinMode(PIN_CS_SDCARD, OUTPUT);     //  Настройка ноги для CS карты памяти
   digitalWrite(PIN_CS_SDCARD, HIGH);  //  запретить 
   // Сброс SPI устройств
   reset_ili9341();                    // Сброс дисплей через ножку
   reset_w5500();                      // Сброс W5500 через ножку
   // Настройка периферии
   pinMode(PIN_RELAY1, OUTPUT);        //  Настройка ноги для вентилятора (SSR1)
   digitalWrite(PIN_RELAY1, LOW);      //  выкл SSR1
   pinMode(PIN_RELAY2, OUTPUT);        //  Настройка ноги для калорифера (SSR2)
   digitalWrite(PIN_RELAY2, LOW);      //  выкл SSR2
   pinMode(PIN_KEY, INPUT);            //  Настройка ноги кнопки ВХОД
   pinMode(PIN_ACS758 , INPUT_ANALOG); //  Датчик тока
   pinMode(PIN_BEEP, OUTPUT);          //  Настройка ноги для динамика
   digitalWrite(PIN_BEEP, LOW); 
   pinMode(PIN_LED1, OUTPUT);          //  Настройка ноги для LED1
   pinMode(PIN_LED2, OUTPUT);          //  Настройка ноги для LED2
   digitalWrite(PIN_LED1, LOW);   
   digitalWrite(PIN_LED2, LOW);   
//   _delay(200);
   sensors.autoACS758=CalibrACS758(); // Автокалибровка датчика тока, используется как пауза для включения светодиодов (делаем всегда)
   digitalWrite(PIN_LED1, HIGH);   
   digitalWrite(PIN_LED2, HIGH);   
  
   // Подготовка переменных и графиков
   reset_sum();
   for (i=0;i<CHART_POINT;i++){chartTemp.dataOut[i]=-3000;chartTemp.dataIn[i]=-3000;chartAbsH.dataOut[i]=0;chartAbsH.dataIn[i]=0;}// Обнуление графиков
   // Установки по умолчанию
   setting.mode = BLOCK_OFF;    
   setting.flag = 0x00;
   SETBIT1(setting.flag,fBEEP); // Разрешить звук по умолчанию
   SETBIT1(setting.flag,fDHCP); // Разрешить DHCP умолчанию (на экране можно посмотреть полученный адрес)
   SETBIT1(setting.flag,fAUTO); // Автоматическое определение 0 датчика тока
   setting.hour_unit = 0;
   setting.hour_motor = 0;
   setting.hour_heat = 0;
   setting.CurMin=400;
   setting.CurMax=1000;
   parseIPAddress("192.168.1.176",'.',setting.ip);
   parseIPAddress("192.168.1.1",'.',setting.dns);
   parseIPAddress("192.168.1.1",'.',setting.gateway);
   parseIPAddress("255.255.255.0",'.',setting.mask);
   sensors.CurrentACS758=0;
   sensors.flags=0x00;  
   FLAG_FAN_OFF;
   #ifdef USE_HEAT
      FLAG_HEAT_OFF;
   #endif
   FLAG_TEST_OK; // По умолчанию мотор исправен
   FLAG_NTP_ERR; // По умолчанию время не обновлено

    #ifdef  DEBUG  
      Serial.print(setting.name);Serial.println("  start . . ."); 
    #endif    
   // Попытка прочитать сохраненные настройки
   if(initEEPROM()!=0) {formatEEPROM(); saveEEPROM();} // Запись настроек по умолчанию
   else loadEEPROM();                                  // восстановление настроек 
   if(GETBIT(setting.flag,fAUTO))sensors.offsetACS758=sensors.autoACS758;else sensors.offsetACS758=setting.constACS758*10; //Установить смещение в зависимости от настроек
   
  //  SETBIT1(setting.flag,fDHCP);    
  
   init_w5500();  // Инициализация сети перед дисплеем что бы показать текущий IP
  
   // разбираемся с часами и обновляем время по сети если возможно     
   rtc_set_prescaler_load(0x7fff);  // установка делителя для часов  
       Udp.begin(localPort);
       getNtpTime();    
       #ifdef DEBUG
       rt.breakTime(rt.now(),tm);
       Serial.print("RTC timestamp:");Serial.print(tm.year+1970);Serial.print("/");Serial.print(tm.month);Serial.print("/");Serial.print(tm.day);Serial.print(" ");Serial.print(tm.hour);Serial.print(":");Serial.print(tm.minute);Serial.print(":");Serial.println(tm.second);
       #endif

   rt.breakTime(rt.now(),rTime);  // Запомнить время последнего сброса для вывода на вебморде
   #ifdef  DEBUG  
      Serial.print("Register RCC_CSR:"); Serial.println(uint32ToHex(reg_RCC_CSR));  
   #endif 
   
     // Тестирование датчиков и если удачно то полученными значенми заполнить массивы графиков
     if ((sensors.inErr = inDHT.read2(&temperature, &humidity, NULL)) != SimpleDHTErrSuccess){// Внутренний датчик
         #ifdef DEBUG
         Serial.print("Read inDHT failed, err="); Serial.println(uint8ToHex(sensors.inErr)); 
         #endif
         }
         else {
          #ifdef DEBUG
          Serial.println("Read inDHT Ok");
          #endif
          sensors.av_tIn    = temperature*100;
          sensors.av_relHIn = humidity*100;
          sensors.av_absHIn = calculationAbsH(temperature, humidity)*100; 
          for (i=0;i<CHART_POINT;i++){chartTemp.dataIn[i]=sensors.av_tIn; chartAbsH.dataIn[i]=sensors.av_absHIn;}
          }
    if ((sensors.outErr = outDHT.read2(&temperature, &humidity, NULL)) != SimpleDHTErrSuccess) { // Наружний датчик
         #ifdef DEBUG
         Serial.print("Read outDHT failed, err="); Serial.println(uint8ToHex(sensors.outErr)); 
           #endif
         }
         else {
          #ifdef DEBUG
          Serial.println("Read outDHT Ok");
          #endif
          sensors.av_tOut    = temperature*100; 
          sensors.av_relHOut = humidity*100;
          sensors.av_absHOut = calculationAbsH(temperature, humidity)*100; 
          for (i=0;i<CHART_POINT;i++){chartTemp.dataOut[i]=sensors.av_tOut; chartAbsH.dataOut[i]=sensors.av_absHOut;}
          }
     
   // Вывод на дисплей
   SPI.setModule(2);
   SPI.setClockDivider(SPI_CLOCK_DIV2);
   tft.begin();
   fullTftUpdate=true;  // Нужна полная прорисовка дисплея при старте ОС
   
   // Создание задач  
   xTaskCreate(vReadSensor, "ReadSensor",150,NULL,4,&hReadSensor); // было 100 пробуем 150
   xTaskCreate(vUpdateTFT,  "UpdateTFT",150,NULL,2,&hUpdateTFT);   // стек минимум 150  пробуем 200
   xTaskCreate(vWebTask,    "Web" ,350,NULL,1,&hWebTask);          // стек минимум 200?  250 падает  последний вариант 350
   vSemaphoreCreateBinary(xTFTSemaphore); // Создание семафора
     
   if (xTFTSemaphore==NULL){
    #ifdef DEBUG
    Serial.println("Error create semaphore (low memory?)");
    #endif 
    exit;
   }

   iwdg_init(IWDG_PRE_256, 2*1250);      // Сторожевой таймер 2*8 секунд. 12 бит Максимальное значение 4096 !!!
   setup_vdd_tempr_sensor();             // Подготовить 16 17 каналы ацп
   #ifdef DEBUG
    Serial.println(" . . . Start free RTOS . . .");
   #endif 
   vTaskStartScheduler();
   #ifdef DEBUG
   Serial.println("vTaskStartScheduler() NOT RUNNING!!!"); 
   #endif 
}

void loop()
{
}
// Расчет загрузки контроллера 
static const uint8 periodLen = 9; // 2^periodLen ticks - 512 x 1ms ticks 
volatile TickType_t curIdleTicks = 0;
volatile TickType_t lastCountedTick = 0;
volatile TickType_t lastCountedPeriod = 0;
static TickType_t lastPeriodIdleValue = 0;  // Простой контроллера за последний период
volatile TickType_t minIdleValue = 1 << periodLen; // минимальный простой контроллера с момента пуска
extern "C" void vApplicationIdleHook( void ) //В конфиге freertos  должно быть #define configUSE_IDLE_HOOK   1 в файл FreeRTOSConfig.h
{
    // Process idle tick counter
    volatile TickType_t curTick = xTaskGetTickCount();
    if(curTick != lastCountedTick)
    {
          curIdleTicks++;
          lastCountedTick = curTick;
    }
    // Store idle metrics each ~0.5 seconds (512 ticks)
    curTick >>= periodLen;
    if(curTick >  lastCountedPeriod)
    {
        lastPeriodIdleValue = curIdleTicks;
        curIdleTicks = 0;
        lastCountedPeriod = curTick;
          // Store the max value
          if(lastPeriodIdleValue < minIdleValue)
          minIdleValue = lastPeriodIdleValue;
    }  
   
}

// Задача по чтению датчиков ------------------------------------------------------------
static void vReadSensor(void *pvParameters) {
uint16_t i;
int32_t sum,rawVolt;
static TickType_t DHT22Tick = 0;     // счетчик тиков для суточных отсчетов (обновление времени и тестирование мотора)
static TickType_t curTick = 0;
static TickType_t sockTick = 0;     // сброс зависших сокетов
// При первом пуске автокалибровка датчика тока
//sensors.offsetACS758=CalibrACS758(); // Автокалибровка датчика тока, используется как пауза для включения светодиодов
 
  for (;;) {  
      iwdg_feed();  // Вачдог сброс
      SPI.setModule(1);
      SPI.setClockDivider(SPI_CLOCK_DIV2);
      xSemaphoreGive(xTFTSemaphore);

      if(curTick < sockTick) {DHT22Tick=0;sockTick=0;}// если счетик тиков xTaskGetTickCount() переполнился  
      // 1. Измерение тока (мА) с датчика ACS758  
      sum=0;
      for (i=0;i<CURRENT_SAMPLES;i++) // Копим отсчеты
      {
      rawVolt=(analogRead(PIN_ACS758)*UREF_VCC*10)/(4096-1); 
      if(rawVolt>sensors.offsetACS758) sum=sum+rawVolt-sensors.offsetACS758;else sum=sum+sensors.offsetACS758-rawVolt; // суммирование + выпремление тока и вычетание смещения (не забываем что у нас ток переменный) надо суммировать обе полуволны и убирать смещение vcc/2
     // _delay(1);
      delayMicroseconds(100); // Нужна строкая переодичность
      }
      sensors.CurrentACS758 = (sum/CURRENT_SAMPLES)*100/miliVoltsPerAmp;  // ток в мА
      curTick = xTaskGetTickCount();
      if(curTick < sockTick) {DHT22Tick=0;sockTick=0;}// если счетик тиков xTaskGetTickCount() переполнился  
    
      // 2. Пора чистить сокеты  
      if (curTick-sockTick>10*60*1000){  // Раз в 10 минут
      sockTick=curTick;
      SPI.setModule(1);
      SPI.setClockDivider(SPI_CLOCK_DIV2);
      xSemaphoreGive(xTFTSemaphore);
      checkSockStatus();  
      }
      
     // 3. Пора читать датчики DHT22 
    if (curTick-DHT22Tick>TIME_SCAN_SENSOR-CURRENT_SAMPLES/10){ // Пора читать датчики DHT22 период корректируется на время измерения тока CURRENT_SAMPLES
      DHT22Tick=curTick;
      digitalWrite(PIN_LED1,LOW);  
      measurement(); 
      digitalWrite(PIN_LED1,HIGH);
     }  
    //  vTaskDelay(TIME_SCAN_SENSOR-CURRENT_SAMPLES/10);    // период корректируется на время измерения тока CURRENT_SAMPLES
    vTaskDelay(500);
   }     
}

// Задача обновлению дисплея с частотой усреднения ------------------------------------------------------------
static void vUpdateTFT(void *pvParameters) {
static TickType_t UpdateDataTick = 0;    // счетчик тиков обновления данных
static TickType_t UpdateChartTick = 0;   // счетчик тиков обновления графика
static TickType_t UpdateDayTick = 0;     // счетчик тиков для суточных отсчетов (обновление времени и тестирование мотора)
static TickType_t curTick = 0;
  for (;;) { 
   iwdg_feed();      // Вачдог сброс 
   curTick = xTaskGetTickCount();
   if(curTick<UpdateDataTick) {UpdateDataTick=0;UpdateChartTick=0;UpdateDayTick=0;}// если счетик тиков xTaskGetTickCount() переполнился  
   if((GETBIT(setting.flag,fTFT_OFF))&&(fullTftUpdate)) // выключение дисплея 
   {
     fullTftUpdate=false;                    // Сбросить признак полного обновления дисплей
     if(switchTFT()) tft.fillScreen(ILI9341_BLACK); // Стереть экран 
     switchNET();
   }
   if (curTick-UpdateDayTick>24*TIME_HOUR){ // Время пришло раз в сутки
       UpdateDayTick=curTick;
       if(!checkNetLink()) {reset_w5500();_delay(100); init_w5500();}  // Сбросить контроллер если сети (нет линка) может он завис (для увеличения надежности)
       if (GETBIT(setting.flag,fNTP))// Если надо обновлять время по NTP
       {
          Udp.begin(localPort);
          getNtpTime();
          #ifdef DEBUG
          rt.breakTime(rt.now(),tm);
          Serial.print("RTC update NTP:");Serial.print(tm.year+1970);Serial.print("/");Serial.print(tm.month);Serial.print("/");Serial.print(tm.day);Serial.print(" ");Serial.print(tm.hour);Serial.print(":");Serial.print(tm.minute);Serial.print(":");Serial.println(tm.second);
          #endif  
       }
       if (GETBIT(setting.flag,fTEST)) testMotorAndACS758(); // Если надо тестировать 

     }
   
   if (!GETBIT(setting.flag,fTFT_OFF)) // Дисплей не выключен
     {
         if (fullTftUpdate){ // если надо полностью обновить дисплей
              fullTftUpdate=false;                  // Сбросить признак полного обновления дисплей
              print_static();                       // распечатать таблицу
              print_setting();                      // показать настройки   и обновить переменные границ
              print_data();                         // вывод усредненных значений 
              print_status(checkNetLink());         // панель состояния
              printChart();
              last_error=100;                       // Показать ошибку                    
             }
              print_LoadCPU();                      // Показ графика загрузки
              print_time();
          if (curTick-UpdateDataTick>TIME_SCAN_SENSOR){ //  Вывод усредненных значений
              UpdateDataTick=curTick;
              print_error_AHT();                    // Вывод ошибки чтения датчика при каждом чтении контроль за качеством связи с датчиком
              print_data();                          // вывод усредненных значений 
              print_status(checkNetLink());          // панель состояния
              if (FLAG_FAN_CHECK) ChartMotor=true;   // Признак того что надо показывать включение мотора на графике
             }
        if (curTick-UpdateChartTick>TIME_PRINT_CHART){ //  Вывод Графиков  
               UpdateChartTick=curTick;   
               printChart(); // Сдвиг графика и вывод новой точки
               #ifdef  DEBUG  
               Serial.println("Point add chart");
               #endif  
               if (GETBIT(setting.flag,fBEEP)) beep(30);  // Звук добавления точки
            } 
      } // if (!GETBIT(setting.flag,fTFT_OFF)) 
      vTaskDelay(TIME_UPDATE_TFT);
  }   // for  
}

// Задача работы веб сервера  ------------------------------------------------------------
static void vWebTask(void *pvParameters) {
static TickType_t NetTick = 0;     // счетчик тиков для проверки работы сети (мигание точки часов)
static TickType_t curTick = 0;  
for (;;) {
  curTick = xTaskGetTickCount();
  if(curTick<NetTick) {NetTick=0;}// если счетик тиков xTaskGetTickCount() переполнился  
  if(curTick-NetTick>TIME_UPDATE_TFT){NetTick=curTick; if(FLAG_NET_CHECK) FLAG_NET_TRUE; else FLAG_NET_FALSE;} // Мигание двоеточие часов - веб сервер работает
  webserver();
  vTaskDelay(5);    // было 10
  }
}
