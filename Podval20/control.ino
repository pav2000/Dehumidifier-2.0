
void reset_sum()  // Сброс расчета скользящего среднего
{
uint16_t i=0;
sensors.first=true;
sensors.sum_tOut=0;
sensors.sum_tIn=0;
sensors.sum_relHOut=0;
sensors.sum_relHIn=0;

for(i=0;i<NUM_SAMPLES;i++) // Массивы для усреднения
    {
      sensors.dat_tOut[i]=0;
      sensors.dat_tIn[i]=0;
      sensors.dat_relHOut[i]=0;
      sensors.dat_relHIn[1]=0;
    }  
}
// Измерение и обработка данных чтение датчиков --------------------------------------------------------------------------------
void measurement()
{ 
  float temperature = 0;
  float humidity = 0;
  int32_t temp; 
 
  // Проверка работоспособности вентилятора если он включен (если он работает)
  if(FLAG_FAN_CHECK) {if(sensors.CurrentACS758<setting.CurMin) FLAG_TEST_ERR; else FLAG_TEST_OK;}

  #ifdef  DEMO1  // Внутренний датчик
   sensors.inErr = 0;
   sensors.tIn=sensors.tIn+random(-60,95); 
   if (sensors.tIn>2000) sensors.tIn=1900;
   if (sensors.tIn<-1000) sensors.tIn=-900;
   sensors.relHIn=sensors.relHIn+(float)random(-60,90);
   if (sensors.relHIn>9600) sensors.relHIn=9000;
   if (sensors.relHIn<500) sensors.relHIn=1000;
  #else 
  if ((sensors.inErr = inDHT.read2(&temperature, &humidity, NULL)) != SimpleDHTErrSuccess) {
     sensors.numErrIn++; 
     #ifdef DEBUG
     Serial.print("Read inDHT failed, err="); Serial.println(uint8ToHex(sensors.inErr));
     #endif
  } else
   {
   temp=temperature*100+setting.eTIN;  // Добавим ошибку
    if (sensors.tIn==-5000) sensors.tIn=temp; else {
    sensors.tIn=constrain(temp, sensors.tIn-GAAP_TEMP, sensors.tIn+GAAP_TEMP); } // Убираем резкие скачки
     
   temp=humidity*100+setting.eHIN; 
    if (sensors.relHIn==5000) sensors.relHIn=temp; else {
    sensors.relHIn=constrain(temp, sensors.relHIn-GAAP_REALH, sensors.relHIn+GAAP_REALH); } 
     
  #ifdef DEBUG
 //  Serial.print("Temperature IN: "); Serial.println(sensors.tIn);
 //  Serial.print("Humidity IN...: "); Serial.println(sensors.relHIn); 
   #endif 
   }
  #endif 

  #ifdef  DEMO1  // Внешний датчик
   sensors.outErr = 0;
   sensors.tOut=sensors.tOut+random(-60,95); 
   if (sensors.tOut>2000) sensors.tOut=1900;
   if (sensors.tOut<-1000) sensors.tOut=-900;
   sensors.relHOut=sensors.relHOut+(float)random(-70,90);
   if (sensors.relHOut>9600) sensors.relHOut=9000;
   if (sensors.relHOut<500) sensors.relHOut=1000;
  #else  
    _delay(10); // Пауза между чтениями датиков (иначе часто ошибки вылезают при чтении второго датчика)
    if ((sensors.outErr = outDHT.read2(&temperature, &humidity, NULL)) != SimpleDHTErrSuccess) {
    sensors.numErrOut++;
    #ifdef DEBUG
    Serial.print("Read outDHT failed, err="); Serial.println(uint8ToHex(sensors.outErr)); 
    #endif
  } else
  {
    temp=temperature*100+setting.eTOUT; 
    if (sensors.tOut==-5000) sensors.tOut=temp; else {
    sensors.tOut=constrain(temp, sensors.tOut-GAAP_TEMP, sensors.tOut+GAAP_TEMP); } // Убираем резкие скачки
 
    temp=humidity*100+setting.eHOUT; 
    if (sensors.relHOut==5000) sensors.relHOut=temp; else {
    sensors.relHOut=constrain(temp, sensors.relHOut-GAAP_REALH, sensors.relHOut+GAAP_REALH); } 
    
    #ifdef DEBUG
 //   Serial.print("Temperature OUT: "); Serial.println(sensors.tOut);
 //   Serial.print("Humidity OUT...: "); Serial.println(sensors.relHOut); 
    #endif  
  }
  #endif
  // Скользящее среднее 
  if ((sensors.outErr==SimpleDHTErrSuccess)&&(sensors.inErr==SimpleDHTErrSuccess))// Если чтение без ошибок у ДВУХ датчиков обновляем данные
  {
  // 1. Удаляем самое старое значение из суммы
  sensors.sum_tOut=sensors.sum_tOut-sensors.dat_tOut[sensors.index];
  sensors.sum_tIn=sensors.sum_tIn-sensors.dat_tIn[sensors.index];
  sensors.sum_relHOut=sensors.sum_relHOut-sensors.dat_relHOut[sensors.index];
  sensors.sum_relHIn=sensors.sum_relHIn-sensors.dat_relHIn[sensors.index];
  // 2. Запоминаем новое значение
  sensors.dat_tOut[sensors.index]=sensors.tOut;
  sensors.dat_tIn[sensors.index]=sensors.tIn;
  sensors.dat_relHOut[sensors.index]=sensors.relHOut;
  sensors.dat_relHIn[sensors.index]=sensors.relHIn;
  //3. Добавляем к суммме новое значение
  sensors.sum_tOut=sensors.sum_tOut+sensors.tOut;
  sensors.sum_tIn=sensors.sum_tIn+sensors.tIn;
  sensors.sum_relHOut=sensors.sum_relHOut+sensors.relHOut;
  sensors.sum_relHIn=sensors.sum_relHIn+sensors.relHIn;  
  // 4. Вычисляем среднее значение
  if (sensors.first){ // первый проход - нет полного массива
       sensors.av_tIn=sensors.sum_tIn/(sensors.index+1);
       sensors.av_relHIn=sensors.sum_relHIn/(sensors.index+1);
       sensors.av_tOut=sensors.sum_tOut/(sensors.index+1);
       sensors.av_relHOut=sensors.sum_relHOut/(sensors.index+1);   
  }else{ 
       sensors.av_tIn=sensors.sum_tIn/NUM_SAMPLES;
       sensors.av_relHIn=sensors.sum_relHIn/NUM_SAMPLES;
       sensors.av_tOut=sensors.sum_tOut/NUM_SAMPLES;
       sensors.av_relHOut=sensors.sum_relHOut/NUM_SAMPLES;
  }
   // вычисление абсолютной влажности
   sensors.av_absHIn=(int)(calculationAbsH((float)(sensors.av_tIn/100.0),(float)(sensors.av_relHIn/100.0))*100.0);
   sensors.av_absHOut=(int)(calculationAbsH((float)(sensors.av_tOut/100.0),(float)(sensors.av_relHOut/100.0))*100.0);  
   // 5. Изменяем индекс 
   sensors.index++;
   if (sensors.index==NUM_SAMPLES) {sensors.index=0;sensors.first=false;} // Первая итерация
    #ifdef  DEBUG  
       Serial.println("Average value");
       Serial.print("IN T=");Serial.print(sensors.av_tIn);Serial.print(" H="); Serial.print(sensors.av_relHIn); Serial.print(" abs H="); Serial.println(sensors.av_absHIn);
       Serial.print("OUT T=");Serial.print(sensors.av_tOut);Serial.print(" H="); Serial.print(sensors.av_relHOut); Serial.print(" abs H="); Serial.println(sensors.av_absHOut);
 //      Serial.print("setting.T_min=");Serial.print(setting.T_min); Serial.print(" setting.dH_min=");Serial.println(setting.dH_min);
     #endif   
     // Данные обновились, надо оценить что включать
     CheckON();   // Проверка статуса вентилятора
   }   
}

// Калибровка датчика ACS758 --------------------------------------------------------------------------------
// Возвращает напряжение смещения 0 датчика
uint16_t CalibrACS758(void)
{
uint16_t i;
uint32_t sumOffset=0;
  for (i=0;i<CALIBR_SAMPLES;i++) // Копим отсчеты
  {
    sumOffset=sumOffset+(analogRead(PIN_ACS758)*UREF_VCC*10)/(4096-1);  // напряжение в ДЕСЯТЫХ мВ   
  // _delay(1);
  delayMicroseconds(100);
  }
   #ifdef  DEBUG  
     Serial.print("ACS758 '0' (mV)=");Serial.println(sumOffset/CALIBR_SAMPLES/10);
   #endif  
  return sumOffset/CALIBR_SAMPLES; // усредненный отсчет в мвольтах  
}

// установка АЦП для чтения температуры инапряжения питания ----------------------------------------------
// https://www.stm32duino.com/viewtopic.php?t=598   https://sparklogic.ru/arduino-for-stm32/reading-internal-temperature-sensor.html
#define ADC_CR2_TSVREFE_BIT 23
#define ADC_CR2_TSVREFE (1U << ADC_CR2_TSVREFE_BIT)
void setup_vdd_tempr_sensor() {
    adc_reg_map *regs = ADC1->regs;
    regs->CR2 |= ADC_CR2_TSVREFE;    // enable VREFINT and Temperature sensor
    // sample rate for TSVREFE ADC channel and for Temperature sensor
    regs->SMPR1 |=  (0b111 << 18);  // sample rate temperature
    regs->SMPR1 |=  (0b111 << 21);  // sample rate vrefint
    adc_calibrate(ADC1);
}

// Проверка статуса вытяжки, не пора ли переключится ----------------------------------------------------------
void CheckON(void)
{
#ifdef USE_HEAT // Если определен калорифер
// 0.  Проверить замораживание подвала КАЛОРИФЕР -- РАБОТАЕТ ВСЕГДА НЕЛЬЗЯ ОТКЛЮЧИТЬ
if (sensors.av_tIn<=TEMP_LOW) { FAN_OFF; HEAT_ON; return;}   // Контроль от промораживания подвала по идеи здесь надо включать калорифер
if ((FLAG_HEAT_CHECK)&&(sensors.av_tIn>TEMP_LOW+dT_OFF)) HEAT_OFF;    // Выключить калорифер
#endif

// 1. Режимы не зависящие от влажности и температуры ВЫСШИЙ приоритет
if ((setting.mode==BLOCK_OFF)&&(!FLAG_FAN_CHECK))  return;
if ((setting.mode==BLOCK_OFF)&&(FLAG_FAN_CHECK))  { FAN_OFF ; return;}
if ((setting.mode==HOOD_ON )&&(FLAG_FAN_CHECK))   return;
if ((setting.mode==HOOD_ON )&&(!FLAG_FAN_CHECK))  { FAN_ON  ; return;}

// 2. Режим охлаждения (второй приоритет) температура внутри больше 10 градусов темература снаружи меньше на 2 градуса чем внутри, на влажность не смотрим
if (setting.mode==COOLING)          // Режим охлаждение
  {
    if ((!FLAG_FAN_CHECK)&&(sensors.av_tIn>setting.T_min)&&((sensors.av_tIn-sensors.av_tOut)>setting.dH_min)) // dH_min используется не штатно для температуры
       {FAN_ON; return;}            // мотор выключен, температура выше установленной и снаружи температура ниже на 2 градуса  то ВКЛЮЧЕНИЕ мотора
    if ((FLAG_FAN_CHECK)&&(sensors.av_tIn<=sensors.av_tOut))   
       {FAN_OFF; return;}          // мотор включен и темература внутри ниже наружней то ВЫКЛЮЧЕННИЕ мотора
   return;                         // изменений нет выходим    
  } 
  
// 3. В режиме осушения (мотор рабатает) - проверка на достижение минимальной температуры помещения в режиме осушения - СРОЧНО ВЫКЛЮЧИТЬ  третий приоритет
if ((sensors.av_tIn<=setting.T_min)&&(FLAG_FAN_CHECK)){FAN_OFF;return;}         // выключить и выйти

if ((sensors.av_absHIn<=sensors.av_absHOut)&&(FLAG_FAN_CHECK)){FAN_OFF;return;} // влажность внутри меньше чем снаружи выключить и выйти

// 4. Режимы зависящие от температуры и влажности низший приоритет (что осталось)
if(FLAG_FAN_CHECK){  // Мотор включен
if ((sensors.av_tIn<=(setting.T_min))||(sensors.av_absHIn<(sensors.av_absHOut+dH_OFF))){FAN_OFF;return;}     // мотор включен и темература ниже критической ИЛИ абс влажность внутри ниже то ВЫКЛЮЧЕННИЕ мотора  
}else{               // Мотор выключен
if((sensors.av_tIn>setting.T_min)&&(sensors.av_absHIn>(sensors.av_absHOut+setting.dH_min))){FAN_ON;return;}// мотор выключен, темература выше критической, И абс влажность с наружи меньше то ВКЛЮЧЕНИЕ мотора
}

} 

// Тестирование мотора и определение 0 датчика ACS758 --------------------------------------------------------------------------------
// Возвращает true если тест пройден и false если не пройден
boolean testMotorAndACS758(void)
{
 boolean state=FLAG_FAN_CHECK; // запомнить состяние мотора до тестирования
 if(GETBIT(setting.flag,fAUTO)){ // Если требуется автокалиброва датчика тока
 #ifdef DEBUG
 Serial.println("Аutocalibration ACS758 . . .");
 #endif
 if(state){FAN_OFF; _delay(4000);}      // если надо то выключить мотор для калибровки ACS758
 vTaskSuspendAll(); 
 sensors.autoACS758=CalibrACS758();   // Автокалибровка датчика тока,
 xTaskResumeAll();
 if(GETBIT(setting.flag,fAUTO))sensors.offsetACS758=sensors.autoACS758;else sensors.offsetACS758=setting.constACS758*10; //Установить смещение в зависимости от настроек
 }
 #ifdef DEBUG
 Serial.print("Offset ACS758 (mV): "); Serial.println(sensors.offsetACS758/10);
 Serial.println("Test fan . . .");
 #endif
 if(!state){ FAN_ON; _delay(5000);} // Если мотор не включен, включить мотор и пауза для разгона
 if(sensors.CurrentACS758<setting.CurMin)FLAG_TEST_ERR;else FLAG_TEST_OK;
 #ifdef DEBUG
 if(FLAG_TEST_CHECK)  Serial.println("Fan OK"); else Serial.println("Fan not work!!"); 
 #endif 
 if(!state){_delay(2000);FAN_OFF;} // если надо выключить потор - если перед тестом он был выключен (возвращаем в состояние которые было)
 
}
