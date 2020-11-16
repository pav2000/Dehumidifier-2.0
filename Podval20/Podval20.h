// Макросы работы с битами байта, используется для флагов
#define GETBIT(b,f)   ((b&(1<<(f)))?true:false)              // получить состяние бита
#define SETBIT1(b,f)  (b|=(1<<(f)))                          // установка бита в 1
#define SETBIT0(b,f)  (b&=~(1<<(f)))                         // установка бита в 0 
#define SWAPB(x)      ((unsigned short)x>>8) | (((unsigned short)x&0x00FF)<<8)

//  НОГИ к которым прицеплена переферия 
#define pinTFT_CS        D19   // SPI2 PB4        // CS
#define pinTFT_DC        D31   // SPI2 PB12       // DC
#define pinTFT_RST       D27   // SPI2 PA8       // сброс дисплея
#define PIN_BEEP         D18   // Ножка куда повешена пищалка
#define PIN_KEY          D24   // Ножка куда повешена кнопка (пока не поддерживается)
#define PIN_RELAY1       D8    // Ножка на которую повешено реле (SSR1) вентилятора
#define PIN_RELAY2       D9    // Ножка на которую повешено реле (SSR2) Калорифер
#define PIN_LED1         D20   // Ножка на которую повешен первый светодиод (LED1)
#define PIN_LED2         D17   // Ножка на которую повешен второй светодиод (LED2)
#define PIN_IN_DHT       PB6   // Ножка на которую повешен внутренний датчик IN 
#define PIN_OUT_DHT      PB7   // Ножка на которую повешен внешний датчик OUT
#define PIN_ACS758       PA0   // A0 Нога куда прицеплен токовый датчик ACS758
#define PIN_W5500        D3    // SPI1 сброс W5500
#define PIN_CS_W25Q8     D10   // SPI1 CS чипа памяти W25Q8ODVSSIG
#define PIN_CS_SDCARD    D23   // SPI1 CS карты памяти

// Мои макросы
#define FAN_BIT                   0            // бит мотора в sensors.flags
#define HEAT_BIT                  1            // бит калорифера в sensors.flags
#define TEST_BIT                  2            // бит отказа вентилятора (false-отказ)
#define NTP_BIT                   3            // бит удачного обновления времени по NTP 
#define NET_BIT                   4            // бит контроля здачи вебсервера 

#define FLAG_FAN_ON               (SETBIT1(sensors.flags,FAN_BIT))                   // бит мотора установить в 1
#define FLAG_FAN_OFF              (SETBIT0(sensors.flags,FAN_BIT))                   // бит мотора установить в 0
#define FLAG_FAN_CHECK            (GETBIT(sensors.flags,FAN_BIT))                    // бит мотора проверить на 1
#define FAN_ON                    {digitalWrite(PIN_RELAY1,HIGH); FLAG_FAN_ON;  }    // включить мотор
#define FAN_OFF                   {digitalWrite(PIN_RELAY1, LOW) ; FLAG_FAN_OFF; }    // выключить мотор

#ifdef USE_HEAT
#define FLAG_HEAT_ON              (SETBIT1(sensors.flags,HEAT_BIT))                  // бит калорифера установить в 1
#define FLAG_HEAT_OFF             (SETBIT0(sensors.flags,HEAT_BIT))                  // бит калорифера установить в 0
#define FLAG_HEAT_CHECK           (GETBIT(sensors.flags,HEAT_BIT))                   // бит калорифера проверить на 1
#define HEAT_ON                   {digitalWrite(PIN_RELAY2, HIGH); FLAG_HEAT_ON; }   // включить калорифер
#define HEAT_OFF                  {digitalWrite(PIN_RELAY2, LOW); FLAG_HEAT_OFF; }   // выключить калорифер
#endif

#define FLAG_TEST_OK             (SETBIT1(sensors.flags,TEST_BIT))                   // бит теста установить в 1 
#define FLAG_TEST_ERR            (SETBIT0(sensors.flags,TEST_BIT))                   // бит теста установить в 0 - ОТКАЗ ВЕНТИЛЯТОРА
#define FLAG_TEST_CHECK          (GETBIT(sensors.flags,TEST_BIT))                    // бит теста проверить на 1
 
#define FLAG_NTP_OK              (SETBIT1(sensors.flags,NTP_BIT))                    // бит NTP установить в 1 
#define FLAG_NTP_ERR             (SETBIT0(sensors.flags,NTP_BIT))                    // бит NTP установить в 0 - Время не обновилось по ntp
#define FLAG_NTP_CHECK           (GETBIT(sensors.flags,NTP_BIT))                     // бит NTP проверить на 1

#define FLAG_NET_FALSE           (SETBIT1(sensors.flags,NET_BIT))                    // бит NET установить в 1 
#define FLAG_NET_TRUE            (SETBIT0(sensors.flags,NET_BIT))                    // бит NET установить в 0 
#define FLAG_NET_CHECK           (GETBIT(sensors.flags,NET_BIT))                     // бит NET проверить на 1

// - КОНСТАНТЫ --------------------------------------
#define dH_OFF          20                       // Гистерезис абсолютной влажности в сотых грамма на куб (фактически сколько не доходит до уличной влажности и выключается)
#define dT_OFF          20                       // Гистерезис температуры в сотых градуса 
#define TEMP_LOW        200                      // Температура подвала критическая (в сотых градуса) - система выключается и включается калорифер
// Датчик тока ACS758
#define UREF_VCC        3330                      // опорное напряжение (Напряжение питания контроллера в данном случае) для ацп stm mV (для калибровки необходимо померить 3.3 вольта и занести сюда точное значение)
#define CALIBR_SAMPLES  800                       // ACS758 число усреднений при калибровке должно быть кратно 20
#define CURRENT_SAMPLES 400                       // ACS758 число усреднений при измернии должно быть кратно 20
#define miliVoltsPerAmp 40                        // ACS758 Чуствительность датчика см даташит
#define GAAP_TEMP  50                             // Максимальное изменение температуры между измерениями (в сотых градуса)
#define GAAP_REALH 100                            // Максимальное изменение температуры между измерениями (в сотых %)
// Границы графиков google 
#define CHART_POINT     120                       // Число точек графика 

// - ВРЕМЕНА ---------------------------------------
#ifdef DEMO                                     // Для демо все быстрее и случайным образом
    #define NUM_SAMPLES      2                  // Число усреднений измерений датчика
    #define TIME_SCAN_SENSOR 1000               // Время опроса датчиков мсек, для демки быстрее
    #define TIME_UPDATE_TFT  1000               // Время обновления дисплея   
    #define TIME_PRINT_CHART 4000               // Время вывода точки графика мсек, для демки быстрее
    #define TIME_HOUR        50000              // Число мсек в часе, для демки быстрее  
#else   
   #define NUM_SAMPLES      50                  // Число усреднений измерений датчика (скользящее среднее) NUM_SAMPLES*TIME_SCAN_SENSOR=Время полного обновления данных
   #define TIME_SCAN_SENSOR 6000                // Время опроса датчиков мсек
   #define TIME_UPDATE_TFT  2000                // Время обновления дисплея      
   #define TIME_PRINT_CHART 300000              // Время вывода точки графика мсек
   #define TIME_HOUR        3600000             // Число мсек в часе
#endif

// Настройки варианты 
#define NUM_SETTING    8                        // Число вариантов настроек 
const char *strMode[]= {"Выключено",
                        "Режим вытяжки",
                        "Охлаждение T>10 dT>5",
                        "Осушение T>+3 dH>0.3",
                        "Осушение T>+3 dH>0.6",
                        "Осушение T>+4 dH>0.4",
                        "Осушение T>+4 dH>0.8",
                        "Осушение T>+5 dH>0.8" 
                                   };
enum TYPE_MODE      //  Перечисляемый тип - режим работы блока
{
    BLOCK_OFF, 
    HOOD_ON, 
    COOLING,   // dH_min используется не штатно для температуры     
    DEHUMIDIFIER_T3_H3,
    DEHUMIDIFIER_T3_H6,
    DEHUMIDIFIER_T4_H4,
    DEHUMIDIFIER_T4_H8,
    DEHUMIDIFIER_T5_H8
};
              
