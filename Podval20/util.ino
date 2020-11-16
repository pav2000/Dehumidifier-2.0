#include <itoa.h>  

// ------------------------------ Различные преобразования ------------------------------------------
// Float to String   - экономим место 
char *ftoa(char *a, double f, int precision)
{
 long p[] = {0,10,100,1000,10000,100000,1000000,10000000,100000000};
 char *ret = a;
 long heiltal = (long)f;
 itoa(heiltal, a, 10);
 while (*a != '\0') a++;
 *a++ = '.';
 long desimal = abs((long)((f - heiltal) * p[precision]));
 itoa(desimal, a, 10);
 return ret;
}

//int в *char в строку ДОБАВЛЯЕТ!! к строке 
char* _itoa( int value, char *string)
{
  char *ret = string;
 //   string[0]=0; // стереть строку
    while(*string) string++;

  char *pbuffer = string;
  unsigned char negative = 0;

  if (value < 0) {
    negative = 1;
    value = -value;
  }

  /* This builds the string back to front ... */
  do {
    *(pbuffer++) = '0' + value % 10;
    value /= 10;
  } while (value > 0);

  if (negative)
    *(pbuffer++) = '-';

  *(pbuffer) = '\0';

  /* ... now we reverse it (could do it recursively but will
   * conserve the stack space) */
  uint32_t len = (pbuffer - string);
  for (uint32_t i = 0; i < len / 2; i++) {
    char j = string[i];
    string[i] = string[len-i-1];
    string[len-i-1] = j;
  }
  return ret; 
}
// Разбор строки в IP адрес, если удачно то возвращает true, если не удачно то возвращает false и адрес не меняет
 boolean parseIPAddress(const char* str, char sep, IPAddress &ip)
{   int i,x;
    char y;
    byte tmp[4];
    for (i = 0; i < 4; i++) 
        {
        y=str[0];  
        x= strtoul(str, NULL, 10);             // Convert byte
        if (x>255) return false;               // Значение байта не верно
        if ((x==0)&&(y!='0')) return false;    // Значение байта не верно
        tmp[i]=x;
        str = strchr(str, sep);               // Find next separator
        if (str == NULL || *str == '\0') {
            break;                            // No more separators, exit
        }
        str++;                                // Point to next character after separator
    //    Serial.println(tmp[i]);
    }
 if (i<4-1) return false; else { ip=tmp;return true; }  
}
// IP адрес в строку
char _bufTemp[18];
char *IPAddress2String(IPAddress & ip) 
{
  _bufTemp[0] = '\0';
  for(uint8_t i = 0; i < 4; i++) {
    _itoa(ip[i], _bufTemp);
    if(i < 3) strcat(_bufTemp, ".");
  }
  return _bufTemp;
}
const char codeHex[]={"0123456789ABCDEF"};
// uint32_t в текстовую строку вида 0xabcdf50e
char* uint32ToHex(uint32_t f)
{ 
_bufTemp[0]='0';
_bufTemp[1]='x';
_bufTemp[2]=codeHex[0x0f & (f>>28)];
_bufTemp[3]=codeHex[0x0f & (f>>24)];
_bufTemp[4]=codeHex[0x0f & (f>>20)];
_bufTemp[5]=codeHex[0x0f & (f>>16)];
_bufTemp[6]=codeHex[0x0f & (f>>12)];
_bufTemp[7]=codeHex[0x0f & (f>>8)];
_bufTemp[8]=codeHex[0x0f & (f>>4)];
_bufTemp[9]=codeHex[0x0f & (f)];
_bufTemp[10]=0; // Конец строки
return _bufTemp;
}
// uint16_t в текстовую строку вида 0xf50e
char* uint16ToHex(uint16_t f)
{ 
_bufTemp[0]='0';
_bufTemp[1]='x';
_bufTemp[2]=codeHex[0x0f & (unsigned char)(highByte(f)>>4)];
_bufTemp[3]=codeHex[0x0f & (unsigned char)(highByte(f))];
_bufTemp[4]=codeHex[0x0f & (unsigned char)(lowByte(f)>>4)];
_bufTemp[5]=codeHex[0x0f & (unsigned char)(lowByte(f))];
_bufTemp[6]=0; // Конец строки
return _bufTemp;
}
// uint8_t в текстовую строку вида 0xf5
char* uint8ToHex(uint8_t f)
{ 
_bufTemp[0]='0';
_bufTemp[1]='x';
_bufTemp[2]=codeHex[0x0f & (unsigned char)(f>>4)];
_bufTemp[3]=codeHex[0x0f & (unsigned char)(f)];
_bufTemp[4]=0; // Конец строки
return _bufTemp;
}

void beep(int x)  // Пищать х мсек
{ digitalWrite(PIN_BEEP, HIGH);  
    _delay(x);
    digitalWrite(PIN_BEEP, LOW);   
} 

char *utf8rus(char *source)
{
  int i,j,k;
  unsigned char n;
  char m[2] = { '0', '\0' };

  strcpy(OutputBuf, ""); k = strlen(source); i = j = 0;

  while (i < k) {
    n = source[i]; i++;

    if (n >= 0xC0) {
      switch (n) {
        case 0xD0: {
          n = source[i]; i++;
          if (n == 0x81) { n = 0xA8; break; }
          if (n >= 0x90 && n <= 0xBF) n = n + 0x30;
          break;
        }
        case 0xD1: {
          n = source[i]; i++;
          if (n == 0x91) { n = 0xB8; break; }
          if (n >= 0x80 && n <= 0x8F) n = n + 0x70;
          break;
        }
      }
    }

    m[0] = n; strcat(OutputBuf, m);
    j++; if (j >= maxString) break;
  }
  return OutputBuf;
}

// Функция переводит относительную влажность в абсолютную 
// t-температура в градусах Цельсия h-относительная влажность в процентах
float calculationAbsH(float t, float h)
{
 float temp;
 temp=pow(2.718281828,(17.67*t)/(t+243.5));
 return (6.112*temp*h*2.1674)/(273.15+t);
}

// определение причины сброса stm32, на входе значение регистра RCC_CSR на выходе строка с описанием причины сброса
char bRes[32];
char* whatReset(uint32_t reg){
bRes[0]=0;
if (bitRead(reg,RCC_CSR_LPWRRSTF_BIT)) strcat(bRes,"Low power<br>");
if (bitRead(reg,RCC_CSR_WWDGRSTF_BIT)) strcat(bRes,"Window watchdog<br>");
if (bitRead(reg,RCC_CSR_IWDGRSTF_BIT)) strcat(bRes,"Independent watchdog<br>");
if (bitRead(reg,RCC_CSR_SFTRSTF_BIT))  strcat(bRes,"Software reset<br>");
if (bitRead(reg,RCC_CSR_PORRSTF_BIT))  strcat(bRes,"Power up<br>");
if (bitRead(reg,RCC_CSR_PINRSTF_BIT)) if(strlen(bRes)<1) strcat(bRes,"Reset key<br>"); // Если только нажата клавиша ресет (отальных источников нет)
return bRes;
}



// ------------------------------ EEPROM ------------------------------------------
// Сохранение настроек в EEPROM
uint16_t saveEEPROM()
{
    uint16_t *ptr,  i, count, Status;
    ptr=(uint16_t*)&setting;
    for (i=0;i<sizeof(setting)/2;i++) {Status=EEPROM.write(i,ptr[i]);if (Status!=0) break; }
#ifdef DEBUG  
    EEPROM.count(&count);
    Serial.print("Save setting, status=");Serial.print(Status);Serial.print(" number of variable:"); Serial.println(count);
#endif  
return Status;    
}

// Восстановление настроек из EEPROM
uint16_t loadEEPROM()
{
uint16_t *ptr,i,Status,count;  
ptr=(uint16_t*)&setting;
  for (i=0;i<sizeof(setting)/2; i+= 1) {Status=EEPROM.read(i,&ptr[i]); if (Status!=0) break;}
#ifdef DEBUG  
    EEPROM.count(&count);
    Serial.print("Load setting, status=");Serial.print(Status);Serial.print(" number of variable:"); Serial.println(count);
#endif  
return Status;    
}

// Инициализация EEPROM
uint16_t initEEPROM()
{
uint16_t Status;  
    // Установка области сохраненя
    EEPROM.PageBase0 = 0x801F000;
    EEPROM.PageBase1 = 0x801F800;
    EEPROM.PageSize  = 0x400;
    Status = EEPROM.init();// Инициализация
#ifdef DEBUG  
    Serial.print("EEPROM.init() : ");Serial.println(Status, HEX);
#endif      
return Status;     
}

// форматирование EEPROM
uint16_t formatEEPROM()
{
uint16_t Status;  
    Status = EEPROM.format();// Форматирование области
#ifdef DEBUG  
    Serial.print("EEPROM.format() : ");Serial.println(Status, HEX);
#endif      
return Status;     
}
