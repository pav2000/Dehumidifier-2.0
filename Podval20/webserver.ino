#include "webserver.h" 
const char IP_FORMAT[]="\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}" ;        // формат ввода ип адреса
const char INT3_FORMAT[]="-?[0-9]{1,3}";                                    // формат ввода целого 3 разряда
const char INT4_FORMAT[]="\\d{1,4}" ;                                       // формат ввода положительного целого 4 разряда
const char RED_FONT[]="<font color=\"red\">";                               // установка красного шрифта
const char GREEN_FONT[]="<font color=\"green\">";                           // установка зеленого шрифта
const char END_FONT[]="</font>" ;                                           // завершения тега фонт

char inBufGet[2048];                                                        // входной буффер ответа GET (web server)
uint32_t connectTime[MAX_SOCK_NUM];                                         // времена для чистки сокетов

// Проверка подключения сетевого провода ------------------------------------------------------------
#define W5500_LINK        0x01  // МАСКА регистра PHYCFGR  (W5500 PHY Configuration Register) [R/W] [0x002E] при котором считается что связь есть
#define W5500_SPEED       0x02  // МАСКА регистра PHYCFGR  (W5500 PHY Configuration Register) [R/W] [0x002E] определяется скорость Speed Status
#define W5500_DUPLEX      0x04  // МАСКА регистра PHYCFGR  (W5500 PHY Configuration Register) [R/W] [0x002E] определяется дуплекс Duplex Status
boolean checkNetLink()
{
 uint8_t st,i;
 for(i=0;i<20;i++){  // Пробуем линк прочитать до 20 раз
     st= W5100.readPHYCFGR();
     if(st & W5500_LINK) break;
     _delay(50);
 }
 if(st & W5500_LINK) return true;  else return false; 
}
// Аппаратный сброс (через ножку reset) чипа w5500 
void reset_w5500(void)
{
  pinMode(PIN_W5500, OUTPUT);   // активным является LOW
  digitalWrite(PIN_W5500, LOW);  
  _delay(50);
  digitalWrite(PIN_W5500, HIGH); 
}

// Инициализация чипа w5500, запомнить полученный IP адрес
void init_w5500(void){
  #ifdef DEBUG
  Serial.println("Init w5500 . . .");
  #endif    
Ethernet.begin(mac, setting.ip, setting.dns, setting.gateway,setting.mask);  // Пишем адрес статик вначале иначе дальше проблема при работе с сетью при отсутсвии линка, 
if (GETBIT(setting.flag,fBEEP)) beep(200); else _delay(200); // Пищалка на старте и пауза перед возможным обращением к серверу DHCP
if(checkNetLink()) 
{ 
   if (GETBIT(setting.flag,fDHCP)){digitalWrite(PIN_LED1, LOW); Ethernet.begin(mac); digitalWrite(PIN_LED1, HIGH); // Если есть линк то пытаемся получить адрес DHCP, мигнуть LED1 в момент получения адреса
   #ifdef DEBUG
   Serial.print("Local IP:");Serial.println(Ethernet.localIP());
   Serial.print("Gateway IP:");Serial.println(Ethernet.gatewayIP());
   Serial.print("DNS server IP:");Serial.println(Ethernet.dnsServerIP());
   Serial.print("Subnet mask:");Serial.println(Ethernet.subnetMask());
   #endif
    }
  }

  server.begin();
  realIP=Ethernet.localIP(); // Запомнить полученный адрес РЕАЛЬНЫЙ
  #ifdef DEBUG
  Serial.print("Web server is at "); Serial.println(realIP);
  #endif  
}

// Сброс зависших сокетов ------------------------------------------------------------
void checkSockStatus()
{
 for (uint8_t i = 0; i < MAX_SOCK_NUM; i++) {        // По всем сокетам!!
     uint8_t s = W5100.readSnSR(i);                                          // Прочитать статус сокета
//     Serial.print("Socket ");Serial.print(i); Serial.print(" state:");Serial.println(s,HEX); 
     if((s == SnSR::ESTABLISHED) || (s == SnSR::CLOSE_WAIT) /*|| (s == 0x22)*/ ) { // если он "кандидат"
        if(abs(millis() - connectTime[i]) > 5000) {        // Время пришло millis() может переполняться
          #ifdef DEBUG
          Serial.print(" Socket frozen:");Serial.println(i);
          #endif        
          W5100.execCmdSn(i, Sock_CLOSE);
          W5100.writeSnIR(i, 0xFF);
        }
    } // if((s == 0x17) || (s == 0x1C))
    else connectTime[i] = millis();                                         // Обновить время если статус не кандидат
  } // for
}

EthernetClient client; 
// В данной задаче нельзя использовать вывод на дисплей
// Веб сервер для настройки моста -------------------------------------------------------------
void webserver()
{
  boolean modeParser; // Флаг начала работы парсера Referer:GET /get?IP=192.168.1.102
  uint8_t getCount=0,i;
  uint16_t num=0;
  char arg[6];
  client = server.available();
  if (client) {
 //   Serial.println("new client");
    modeParser=false;
    getCount=0;
    num=0;
    inBufGet[num]=0;
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        if (getCount!=9)
        {
        if (c=='G') getCount=1; else 
        if((c=='E')&&(getCount==1))getCount=2; else
        if((c=='T')&&(getCount==2))getCount=3; else 
        if((c==' ')&&(getCount==3))getCount=4; else 
        if((c=='/')&&(getCount==4))getCount=5; else 
        if((c=='g')&&(getCount==5))getCount=6; else 
        if((c=='e')&&(getCount==6))getCount=7; else
        if((c=='t')&&(getCount==7))getCount=8; else 
        if((c=='?')&&(getCount==8)){ modeParser=true;getCount=9; }else  getCount=0;
         }
  //     else  if ((c=='?')&&(getCount==3)) modeParser=true;
        if (modeParser==true){
        if (c != '\n') { inBufGet[num]=c; num++;}
        else { modeParser=false; parserGET(inBufGet);}
        }
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank) {
          // send a standard http response header

          vTaskSuspendAll();    
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html; charset=utf-8");
          client.println("Connection: close");  // the connection will be closed after completion of the response
          if (GETBIT(setting.flag,fUPDATE)) client.println("Refresh:60");  //авто обновление страницы раз в 60 секунд, по настройкам
          client.println();
          client.println(indexMain0);  // Секция заголовок
          client.println("<form action=\"/get\"enctype=\"text/plain\"id=\"mode\"method=\"get\">");   // Маленькая форма что бы поменять режим работы блока
          oneInputMode(client, (char*)"Режим работы: ",setting.mode);
          // чекбокс показа настроек 
          strcpy(inBufGet,(char*)"&nbsp;&nbsp;Показать настройки");strcat(inBufGet,"&nbsp;<input name=\"");strcat(inBufGet,(char*)"QUICK");strcat(inBufGet,"\" type=\"checkbox\"");
          if (GETBIT(setting.flag,fFULL_WEB)) strcat(inBufGet,"checked");

    //      strcat(inBufGet,(char*)"&nbsp;&nbsp;Сброс контроллера");strcat(inBufGet,"&nbsp;<input name=\"");strcat(inBufGet,(char*)"RST");strcat(inBufGet,"\" type=\"checkbox\"");
    //      if (GETBIT(setting.flag,fFULL_WEB)) strcat(inBufGet,"checked");
          
          strcat(inBufGet,"/>");
          client.println(inBufGet);    
          client.println("<input name=\"send\" type=\"submit\"value=\">>\"/></form>"); // Надпись на кнопке ">>" не менять, она используется для определения что вводится
          client.println("</td></tr></table></body><hr/>");
          xTaskResumeAll();
          updateData(client);          // Секция данные
          if (GETBIT(setting.flag,fFULL_WEB)){
           //       vTaskSuspendAll();  
              client.println(indexMain1);  // Секция настройки
    //          xTaskResumeAll();
              client.println("<tr><td><i>&nbsp;&nbsp;&nbsp;&nbsp;1. Сетевые настройки W5500</i></td><td></td></tr>");
              oneInputCheckbox(client, (char*)"Получение сетевых настроек по DHCP",(char*)"DHCP", GETBIT(setting.flag,fDHCP)); 
              oneInputText(client, (char*)"IP адрес",                 (char*)"IP",     (char*)IPAddress2String(setting.ip),      16,(char*)IP_FORMAT);
              oneInputText(client, (char*)"DNS",                      (char*)"DNS",    (char*)IPAddress2String(setting.dns),     16,(char*)IP_FORMAT);
              oneInputText(client, (char*)"Шлюз",                     (char*)"gateway",(char*)IPAddress2String(setting.gateway), 16,(char*)IP_FORMAT);
              oneInputText(client, (char*)"Маска подсети",            (char*)"mask",   (char*)IPAddress2String(setting.mask),    16,(char*)IP_FORMAT);
              client.println("<tr><td><i>&nbsp;&nbsp;&nbsp;&nbsp;2. Опции</i></td><td></td></tr>");
              oneInputCheckbox(client, (char*)"Обновление времени раз в сутки по NTP",(char*)"NTP", GETBIT(setting.flag,fNTP)); 
              oneInputCheckbox(client, (char*)"Авто обновление web страницы раз в 60 сек.",(char*)"UPDATE", GETBIT(setting.flag,fUPDATE)); 
              oneInputCheckbox(client, (char*)"Повернуть изображение на дисплее на 180 градусов",(char*)"TFT_180", GETBIT(setting.flag,fTFT_180));           
              oneInputCheckbox(client, (char*)"Выключить дисплей",(char*)"TFT_OFF", GETBIT(setting.flag,fTFT_OFF)); 
              oneInputCheckbox(client, (char*)"Каждый час инициализировать дисплей + SPI 18 Мгц",(char*)"TFT_RST", GETBIT(setting.flag,fTFT_RST)); 
              oneInputCheckbox(client, (char*)"Включить биппер",(char*)"BEEP", GETBIT(setting.flag,fBEEP)); 
              oneInputCheckbox(client, (char*)"<FONT color=red>Програмный сброс осушителя</FONT>",(char*)"RST", false);  // Сброс контроллера
              client.println("<tr><td><i>&nbsp;&nbsp;&nbsp;&nbsp;3. Датчик тока ACS758</i></td><td></td></tr>");
              oneInputCheckbox(client, (char*)"Ежедневное тестирование вентилятора",(char*)"TEST", GETBIT(setting.flag,fTEST));
              oneInputCheckbox(client, (char*)"Автоматическое определение смещения \"0\" ACS758",(char*)"AUTO", GETBIT(setting.flag,fAUTO));
              arg[0]=0; _itoa(setting.constACS758,arg); oneInputText(client, (char*)"Смещение \"0\" ACS758, если нет автоматического определения (мВ)", (char*)"CONST",arg,4,(char*)INT4_FORMAT); 
              arg[0]=0; _itoa(setting.CurMin,arg); oneInputText(client, (char*)"Минимальный ток потребления вентилятора при тесте (мА)", (char*)"CurMin",arg,4,(char*)INT4_FORMAT); 
              client.println("<tr><td><i>&nbsp;&nbsp;&nbsp;&nbsp;4. Ошибки датчиков DHT22 (добавляются)</i></td><td></td></tr>");
              arg[0]=0; _itoa(setting.eTIN,arg); oneInputText(client, (char*)"Ошибка внутреннего датчика температуры (сотые доли C°)", (char*)"eTIN",arg,4,(char*)INT3_FORMAT); 
              arg[0]=0; _itoa(setting.eTOUT,arg); oneInputText(client, (char*)"Ошибка внешнего датчика температуры (сотые доли C°)", (char*)"eTOUT",arg,4,(char*)INT3_FORMAT); 
              arg[0]=0; _itoa(setting.eHIN,arg); oneInputText(client, (char*)"Ошибка внутреннего датчика влажности (сотые доли %)", (char*)"eHIN",arg,4,(char*)INT3_FORMAT); 
              arg[0]=0; _itoa(setting.eHOUT,arg); oneInputText(client, (char*)"Ошибка внешнего датчика влажности (сотые доли %)", (char*)"eHOUT",arg,4,(char*)INT3_FORMAT); 
              client.println(indexMain2); // инфо блок
          }
          vTaskSuspendAll();    
          client.print("<i>Версия прошивки: "); client.print(VERSION); 
          client.print("&nbsp;&nbsp;&nbsp;&nbsp;Текуший IP адрес: "); client.print(IPAddress2String(realIP));          
          client.print("&nbsp;&nbsp; mac:"); for(i=0;i<6;i++){client.print(mac[i],HEX);if(i!=5) client.print(":");} client.print("</i><br>"); 

          client.println(indexMain3);
          xTaskResumeAll();
          break;
        }
        if (c == '\n') {  // you're starting a new line
          currentLineIsBlank = true; 
          modeParser=false;
        } 
        else if (c != '\r') {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }
  }
   vTaskDelay(5);    // give the web browser time to receive the data
   iwdg_feed();      // Вачдог сброс
   client.stop();    // close the connection:
 //  Serial.println("client disonnected");
}

// Парсер ответа get - сохранение настроек в рабочей структуре без сохранения во флеше (для обоих форм!!)
// вход - буфер который надо разобрать
// возврат true - надо настройки сохранить, false настройки не изменились
void parserGET(char *buf)
{ 
uint8_t code;  
char *str, *strGet;  
boolean update=false, tft_off=false, tft_180=false, ChangeTFT=false, upTFT=false, dhcp=false, ntp=false, _update=false, _beep=false, test=false, keyForm=false, quick=false, _auto=false, tft_rst=false, rst=false ;
IPAddress temp;
str = strstr(buf,"HTTP");  str[0]=0;     //Обрезать хвост

while ((str = strtok_r(buf, "&", &buf)) != NULL) // разбор отдельных комманд
  {
//  Serial.println(str);

  if (strstr(str,"IP")!=NULL) // Команда ip найдена
   {
    strGet=strchr(str,'=');
    if (strGet!=NULL){parseIPAddress(strGet+1,'.',temp); if(setting.ip!=temp){setting.ip=temp; update=true;}}
   }
  else if (strstr(str,"DNS")!=NULL) // Команда DNS найдена
   {
    strGet=strchr(str,'=');
    if (strGet!=NULL){parseIPAddress(strGet+1,'.',temp); if(setting.dns!=temp){setting.dns=temp; update=true;}}
   }   
  else  if (strstr(str,"gateway")!=NULL) // Команда gateway найдена
   {
    strGet=strchr(str,'=');
    if (strGet!=NULL){parseIPAddress(strGet+1,'.',temp); if(setting.gateway!=temp){setting.gateway=temp; update=true;}}
   }
  else  if (strstr(str,"mask")!=NULL) // Команда mask найдена
   {
    strGet=strchr(str,'=');
    if (strGet!=NULL){parseIPAddress(strGet+1,'.',temp);if(setting.mask!=temp){setting.mask=temp; update=true;}}
   }
 else  if (strstr(str,"mode")!=NULL) // Команда mode найдена
   {
    strGet=strchr(str,'='); // содержит = и код символа от 0 до 7
    code=((uint8_t)strGet[1])-0x30;// преобразовать код символа в число (грязная техника)  
    if (strGet!=NULL){ if((setting.mode!=(TYPE_MODE)code)&&(code>=0)&&(code<=7)){setting.mode=(TYPE_MODE)code; update=true;ChangeTFT=true;}}   // Обновление на дисплее режима работы
 //   Serial.println(setting.mode);
   } 
 else  if (strstr(str,"QUICK")!=NULL) // Команда QUICK найдена
   {
    strGet=strchr(str,'=');
    if (strGet!=NULL) 
        if(strcmp(strGet+1,"on")==0) quick=true;
   }       
 else if (strstr(str,"TFT_OFF")!=NULL) // Команда TFT_OFF найдена
   {
    strGet=strchr(str,'=');
    if (strGet!=NULL) 
        if(strcmp(strGet+1,"on")==0) tft_off=true;
   }    
 else  if (strstr(str,"TFT_180")!=NULL) // Команда TFT_180 найдена
   {
    strGet=strchr(str,'=');
    if (strGet!=NULL) 
        if(strcmp(strGet+1,"on")==0) tft_180=true;
   }    
 else if (strstr(str,"DHCP")!=NULL) // Команда DHCP найдена
   {
    strGet=strchr(str,'=');
    if (strGet!=NULL) 
        if(strcmp(strGet+1,"on")==0) dhcp=true;
   }             
 else if (strstr(str,"NTP")!=NULL) // Команда NTP найдена
   {
    strGet=strchr(str,'=');
    if (strGet!=NULL) 
        if(strcmp(strGet+1,"on")==0) ntp=true;
   }             
 else if (strstr(str,"UPDATE")!=NULL) // Команда UPDATE найдена
   {
    strGet=strchr(str,'=');
    if (strGet!=NULL) 
        if(strcmp(strGet+1,"on")==0) _update=true;
   }             
 else if (strstr(str,"BEEP")!=NULL) // Команда BEEP найдена
   {
    strGet=strchr(str,'=');
    if (strGet!=NULL) 
        if(strcmp(strGet+1,"on")==0) _beep=true;
   } 
 else if (strstr(str,"TFT_RST")!=NULL) // Команда TFT_RST найдена
   {
    strGet=strchr(str,'=');
    if (strGet!=NULL) 
        if(strcmp(strGet+1,"on")==0) tft_rst=true;
   }  
 else if (strstr(str,"RST")!=NULL) // Команда RST найдена (сброс контроллера) должна быть после команды TFT_RST
   {
    strGet=strchr(str,'=');
    if (strGet!=NULL) 
        if(strcmp(strGet+1,"on")==0) rst=true;
   }       
 else if (strstr(str,"TEST")!=NULL) // Команда TEST найдена
   {
    strGet=strchr(str,'=');
    if (strGet!=NULL) 
        if(strcmp(strGet+1,"on")==0) test=true;
   }    
 else if (strstr(str,"AUTO")!=NULL) // Команда TEST найдена
   {
    strGet=strchr(str,'=');
    if (strGet!=NULL) 
        if(strcmp(strGet+1,"on")==0) _auto=true;
   } 
   
 else  if (strstr(str,"CONST")!=NULL) // Команда CONST найдена
   {
    strGet=strchr(str,'=');
    if (strGet!=NULL){ if(setting.constACS758!=atoi(strGet+1)) {setting.constACS758=atoi(strGet+1);update=true;}}
   }
     
 else if (strstr(str,"CurMin")!=NULL) // Команда CurMin найдена
   {
    strGet=strchr(str,'=');
    if (strGet!=NULL){ if(setting.CurMin!=atoi(strGet+1)) {setting.CurMin=atoi(strGet+1);update=true;}}
   }
   
 else if (strstr(str,"eTIN")!=NULL) // Команда eTIN найдена
   {
    strGet=strchr(str,'=');
    if (strGet!=NULL){ if(setting.eTIN!=atoi(strGet+1)) {setting.eTIN=atoi(strGet+1);update=true;}}
   }
 else if (strstr(str,"eTOUT")!=NULL) // Команда eTOUT найдена
   {
    strGet=strchr(str,'=');
    if (strGet!=NULL){ if(setting.eTOUT!=atoi(strGet+1)) {setting.eTOUT=atoi(strGet+1);update=true;}}
   }
 else if (strstr(str,"eHIN")!=NULL) // Команда eHIN найдена
   {
    strGet=strchr(str,'=');
    if (strGet!=NULL){ if(setting.eHIN!=atoi(strGet+1)) {setting.eHIN=atoi(strGet+1);update=true;}}
   }
 else if (strstr(str,"eHOUT")!=NULL) // Команда eHOUT найдена
   {
    strGet=strchr(str,'=');
    if (strGet!=NULL){ if(setting.eHOUT!=atoi(strGet+1)) {setting.eHOUT=atoi(strGet+1);update=true;}}
   }
// определяем какая кнопка обрабатывается (есть две кнопки ">>" и "Примнить") ДВЕ ФОРМЫ!!!
 if (strstr(str,"send")!=NULL) // определяем кнопку
   {
    strGet=strchr(str,'=');
    if (strstr(strGet+1,"%3E%3E")!=NULL)keyForm=false;else keyForm=true; //Кодировка >> в %3E%3E
   } 
   
  } //while ((str = strtok_r(buf, "&", &buf)) != NULL)
// Отдельный анализ чекбоксов
if (keyForm) { // если используется кнопка "применить" нижняя форма

if ((!tft_off)&&(GETBIT(setting.flag,fTFT_OFF ))){SETBIT0(setting.flag,fTFT_OFF );update=true;ChangeTFT=true;}
else if ((tft_off)&&(!GETBIT(setting.flag,fTFT_OFF ))){SETBIT1(setting.flag,fTFT_OFF );update=true;ChangeTFT=true;}

if ((!tft_180)&&(GETBIT(setting.flag,fTFT_180))){SETBIT0(setting.flag,fTFT_180);update=true;ChangeTFT=true;}
else if ((tft_180)&&(!GETBIT(setting.flag,fTFT_180))){SETBIT1(setting.flag,fTFT_180);update=true;ChangeTFT=true;}

if ((!tft_rst)&&(GETBIT(setting.flag,fTFT_RST))){SETBIT0(setting.flag,fTFT_RST);update=true;}
else if ((tft_rst)&&(!GETBIT(setting.flag,fTFT_RST))){SETBIT1(setting.flag,fTFT_RST);update=true;}

if ((!dhcp)&&(GETBIT(setting.flag,fDHCP))){SETBIT0(setting.flag,fDHCP);update=true;}
else if ((dhcp)&&(!GETBIT(setting.flag,fDHCP))){SETBIT1(setting.flag,fDHCP);update=true;}

if ((!ntp)&&(GETBIT(setting.flag,fNTP))){SETBIT0(setting.flag,fNTP);update=true;}
else if ((ntp)&&(!GETBIT(setting.flag,fNTP))){SETBIT1(setting.flag,fNTP);update=true;}

if ((!_update)&&(GETBIT(setting.flag,fUPDATE))){SETBIT0(setting.flag,fUPDATE);update=true;}
else if ((_update)&&(!GETBIT(setting.flag,fUPDATE))){SETBIT1(setting.flag,fUPDATE);update=true;}

if ((!_beep)&&(GETBIT(setting.flag,fBEEP))){SETBIT0(setting.flag,fBEEP);update=true;}
else if ((_beep)&&(!GETBIT(setting.flag,fBEEP))){SETBIT1(setting.flag,fBEEP);update=true;}

if ((!test)&&(GETBIT(setting.flag,fTEST))){SETBIT0(setting.flag,fTEST);update=true;}
else if ((test)&&(!GETBIT(setting.flag,fTEST))){SETBIT1(setting.flag,fTEST);update=true;}

if ((!_auto)&&(GETBIT(setting.flag,fAUTO))){SETBIT0(setting.flag,fAUTO);update=true;}
else if ((_auto)&&(!GETBIT(setting.flag,fAUTO))){SETBIT1(setting.flag,fAUTO);update=true;}

}
else // Верхняя форма  кнопка ">>"
{
if ((!quick)&&(GETBIT(setting.flag,fFULL_WEB))){SETBIT0(setting.flag,fFULL_WEB);update=true;quick=true;}
else if ((quick)&&(!GETBIT(setting.flag,fFULL_WEB))){SETBIT1(setting.flag,fFULL_WEB);update=true;quick=true;}
}
  
if (update) {
   if(ChangeTFT)            // Настройки дисплея были изменены
        fullTftUpdate=true; // Установить признак включения дисплея
   if(GETBIT(setting.flag,fAUTO))sensors.offsetACS758=sensors.autoACS758;else sensors.offsetACS758=setting.constACS758*10; //Установить смещение в зависимости от настроек
   saveEEPROM();
   }
if (rst) nvic_sys_reset(); // Сброс контроллера   
}

// Вывести одно поле ввода параметра Text - функция универсальна ----------------------------------------------------------------
const char constInputText[]="<tr><td>%s&nbsp;</td><td valign=\"top\"><input maxlength=\"16\" name=\"%s\" required=\"required\" size=\"%d\" type=\"text\" value=\"%s\" pattern=\"%s\" /></td></tr>";
void oneInputText(EthernetClient client, char *label, char *name, char *val, int len, char *pattern)
{
char tmp[8];  
    tmp[0]=0;
    strcpy(inBufGet,"<tr><td>");
    strcat(inBufGet,label);
    strcat(inBufGet,"&nbsp;</td><td valign=\"top\"><input maxlength=\"16\" name=\"");
    strcat(inBufGet,name);
    strcat(inBufGet,"\" required=\"required\" size=\"");
    strcat(inBufGet,_itoa(len,tmp));
    strcat(inBufGet,"\" type=\"text\" value=\"");
    strcat(inBufGet,val);
    strcat(inBufGet,"\" pattern=\"");
    strcat(inBufGet,pattern);
    strcat(inBufGet,"\" /></td></tr>\r\n");
vTaskSuspendAll();
client.println(inBufGet);  
xTaskResumeAll();
}

// выпадающий список - режим работы блока  --------------------------------------------------------------------------------------
//const char constInputOrder1234[]="<tr><td>%s&nbsp;</td><td valign=\"top\"><select name=\"order\" size=\"1\"><option selected=\"selected\" value=\"o1234\">1234</option><option value=\"o4321\">4321</option><option value=\"o2143\">2143</option><option value=\"o3412\">3412</option></select></td></tr>\r\n";
void oneInputMode(EthernetClient client, char *label, TYPE_MODE mode)
{
uint8_t i; 
char tmp[8]; 
inBufGet[0]=0; // стереть буфер     
//  strcpy(inBufGet,"<tr><td>");
  strcpy(inBufGet,"<br><br>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;");
  strcat(inBufGet,label);
//  strcat(inBufGet,"&nbsp;</td><td valign=\"top\"><select name=\"mode\" size=\"1\">");
  strcat(inBufGet,"<select name=\"mode\" size=\"1\">");
  for(i=0;i<NUM_SETTING;i++)
  {
    if (i==mode) strcat(inBufGet,"<option selected=\"selected\" value=\""); 
    else strcat(inBufGet,"<option value=\"");  
    tmp[0]=0;
    strcat(inBufGet,_itoa(i,tmp)); // Кодировка - просто по порядку цифры от 0 до 7
    strcat(inBufGet,"\">");
    strcat(inBufGet,strMode[i]);
    strcat(inBufGet,"</option>");
  }
//  strcat(inBufGet,"</select></td></tr>");
  strcat(inBufGet,"</select>");
vTaskSuspendAll();    
client.println(inBufGet);  
xTaskResumeAll();
}

// Чек бокс в форме - - функция универсальна  --------------------------------------------------------------------------------------------------------------
// <form name="1">Переставлять байты в передаче текстовых данных<input name="orText" type="checkbox" />&nbsp;</form>
//const char constInputCheckboxOn[]= "<tr><td>%s&nbsp;</td><td valign=\"top\"><input name=\"%s\" type=\"checkbox\" checked /></td></tr>\r\n";
//const char constInputCheckboxOff[]="<tr><td>%s&nbsp;</td><td valign=\"top\"><input name=\"%s\" type=\"checkbox\" /></td></tr>\r\n";
void oneInputCheckbox(EthernetClient client, char *label, char *name, boolean b)
{
  strcpy(inBufGet,"<tr><td>");strcat(inBufGet,label);strcat(inBufGet,"&nbsp;</td><td valign=\"top\"><input name=\"");strcat(inBufGet,name);strcat(inBufGet,"\" type=\"checkbox\"");
  if (b) strcat(inBufGet,"checked");
  strcat(inBufGet,"/></td></tr>");
vTaskSuspendAll();
client.println(inBufGet);    
xTaskResumeAll();
}

// Вывод данных ----------------------------------------------------------------------------------------
void updateData(EthernetClient cl)
{
int i,x; 
char b[8];  
vTaskSuspendAll();
cl.println("<span style=\"font-size:18px\"><strong>Состояние вентилятора:"); // Показ состояния вентилятора
if(FLAG_FAN_CHECK)   cl.print(" ВКЛЮЧЕН "); else cl.print(" ВЫКЛЮЧЕН ");
if(FLAG_TEST_CHECK)  cl.println("(исправен)."); else cl.println("<font color=\"red\">(отказ мотора!).</font>"); 
cl.println("</strong></span>"); 
xTaskResumeAll();

Table_Show(cl);   // вывод таблицы
inBufGet[0]=0; // стереть буфер
vTaskSuspendAll();
cl.print(ChartHeader);
cl.print(startTemp);   // График температур
xTaskResumeAll();
        x=posChart;
        inBufGet[0]=0; // стереть буфер
        for(i=0;i<CHART_POINT;i++)  // первый график
         { 
           strcat(inBufGet,"[");_itoa(CHART_POINT-i,inBufGet);strcat(inBufGet,",");strcat(inBufGet,ftoa(b,(float)(chartTemp.dataIn[x])/100.0,2));strcat(inBufGet,",");strcat(inBufGet,ftoa(b,(float)(chartTemp.dataOut[x])/100.0,2));strcat(inBufGet,"]"); 
           if (x<CHART_POINT-1) x++; else x=0; 
           if (i<CHART_POINT-1) strcat(inBufGet,","); 
         }
vTaskSuspendAll();
cl.print(inBufGet);
cl.print(endTemp);
xTaskResumeAll();

inBufGet[0]=0; // стереть буфер
vTaskSuspendAll();
cl.print(startAbsH);  // График Абсолютных влажносей
xTaskResumeAll();
        x=posChart;
        inBufGet[0]=0; // стереть буфер
        for(i=0;i<CHART_POINT;i++)  // первый график
         { 
          strcat(inBufGet,"[");_itoa(CHART_POINT-i,inBufGet);strcat(inBufGet,",");strcat(inBufGet,ftoa(b,(float)((chartAbsH.dataIn[x]&0xbfff))/100.0,2));strcat(inBufGet,",");strcat(inBufGet,ftoa(b,(float)(chartAbsH.dataOut[x])/100.0,2));strcat(inBufGet,"]"); // Обнулить 14 разряд - признак включения мотора
           if (x<CHART_POINT-1) x++; else x=0; 
           if (i<CHART_POINT-1) strcat(inBufGet,","); 
         }
vTaskSuspendAll();
cl.print(inBufGet);
cl.print(endAbsH);
cl.print(ChartBotton);
xTaskResumeAll();
} 
// --------------------------------------------------------------------
// Вывод отдельной таблицы с данными на экран
#define END_TD_TR "</td></tr>"
#define TD_TD "</td><td>"
void Table_Show(EthernetClient cl)
{
float tempr;
int vdd;
char buf[8];  
vTaskSuspendAll();
// Внутрення опора 1.2 вольта, я поставил 1208 после калибровки по питанию
vdd = 1208*4096.0/adc_read(ADC1,17); // (мВ) following 1.43 and 0.0043 parameters come from F103 datasheet - ch. 5.9.13  and need to be calibrated for every chip (large fab parameters variance)
tempr = (1.43 - (vdd/1000.0/4096.0*adc_read(ADC1,16)))/0.0043 + 25.0;
xTaskResumeAll();
strcpy(inBufGet,"<table border=0>");
strcat(inBufGet,"<tr><td>");
strcat(inBufGet,"<table border=1>");
strcat(inBufGet,"<tr><td> Параметр </td><td>Подпол</td><td>Улица</td></tr>");
strcat(inBufGet,"<tr><td>Температура (C°)</td> <td>");strcat(inBufGet,RED_FONT);strcat(inBufGet,ftoa(buf,(float)(sensors.av_tIn)/100.0,2));strcat(inBufGet,END_FONT);strcat(inBufGet,TD_TD);strcat(inBufGet,GREEN_FONT);strcat(inBufGet,ftoa(buf,(float)(sensors.av_tOut)/100.0,2));strcat(inBufGet,END_FONT);strcat(inBufGet,END_TD_TR);
strcat(inBufGet,"<tr><td>Относительная влажность (%)</td> <td>");strcat(inBufGet,RED_FONT);strcat(inBufGet,ftoa(buf,(float)(sensors.av_relHIn)/100.0,2));strcat(inBufGet,END_FONT);strcat(inBufGet,TD_TD);strcat(inBufGet,GREEN_FONT);strcat(inBufGet,ftoa(buf,(float)(sensors.av_relHOut)/100.0,2));strcat(inBufGet,END_FONT);strcat(inBufGet,END_TD_TR);
strcat(inBufGet,"<tr><td>Абсолютная влажность (г/м<sup>3</sup>)</td><td>");strcat(inBufGet,RED_FONT);strcat(inBufGet,ftoa(buf,(float)(sensors.av_absHIn)/100.0,2));strcat(inBufGet,END_FONT);strcat(inBufGet,TD_TD);strcat(inBufGet,GREEN_FONT);strcat(inBufGet,ftoa(buf,(float)(sensors.av_absHOut)/100.0,2));strcat(inBufGet,END_FONT);strcat(inBufGet,END_TD_TR);
if (GETBIT(setting.flag,fFULL_WEB)){
    strcat(inBufGet,"<tr><td>Код последней ошибки чтения датчика DHT22 </td><td>"); strcat(inBufGet,uint8ToHex(sensors.inErr));strcat(inBufGet,TD_TD);strcat(inBufGet,uint8ToHex(sensors.outErr));;strcat(inBufGet,END_TD_TR);
    strcat(inBufGet,"<tr><td>Число ошибок чтения датчика DHT22</td><td>");_itoa(sensors.numErrIn,inBufGet);strcat(inBufGet,TD_TD);_itoa(sensors.numErrOut,inBufGet);strcat(inBufGet,END_TD_TR);
    strcat(inBufGet,"<tr><td>Текущий ток потребления (мА)</td><td colspan=\"2\" align=\"center\">");_itoa(sensors.CurrentACS758,inBufGet);strcat(inBufGet,END_TD_TR);
    strcat(inBufGet,"<tr><td>Напряжение \"0\" ACS758 (мВ)</td><td colspan=\"2\" align=\"center\">");_itoa(sensors.offsetACS758/10,inBufGet);strcat(inBufGet,END_TD_TR);
    strcat(inBufGet,"<tr><td>Время последнего сброса</td><td colspan=\"2\" align=\"center\">");
    _itoa(rTime.year+1970,inBufGet);strcat(inBufGet,"/");_itoa(rTime.month,inBufGet);strcat(inBufGet,"/");_itoa(rTime.day,inBufGet);strcat(inBufGet," ");
    _itoa(rTime.hour,inBufGet);strcat(inBufGet,":");_itoa(rTime.minute,inBufGet);strcat(inBufGet,":");_itoa(rTime.second,inBufGet);strcat(inBufGet,END_TD_TR);
    strcat(inBufGet,"<tr><td>Причина последнего сброса</td><td colspan=\"2\" align=\"center\">");strcat(inBufGet,whatReset(reg_RCC_CSR));strcat(inBufGet,END_TD_TR);
    strcat(inBufGet,"<tr><td>Напряжение питания stm32 (мВ)</td><td colspan=\"2\" align=\"center\">");_itoa(vdd,inBufGet);strcat(inBufGet,END_TD_TR);
    strcat(inBufGet,"<tr><td>Температура stm32 (C°)</td><td colspan=\"2\" align=\"center\">");strcat(inBufGet,ftoa(buf,tempr,2));strcat(inBufGet,END_TD_TR);
   }
strcat(inBufGet,"</table>");
vTaskSuspendAll();
cl.print(inBufGet);
xTaskResumeAll();
if (GETBIT(setting.flag,fFULL_WEB)){
    strcpy(inBufGet,"</td><td>");
    strcat(inBufGet,"<p style=\"margin-left: 50px;\"><small><i>");
    strcat(inBufGet,"Коды ошибок чтения датчиков DHT22<br>");
    strcat(inBufGet,"0x10 - Error to wait for start low signal<br>");
    strcat(inBufGet,"0x11 - Error to wait for start high signal<br>");
    strcat(inBufGet,"0x12 - Error to wait for data start low signal<br>");
    strcat(inBufGet,"0x13 - Error to wait for data read signal<br>");
    strcat(inBufGet,"0x14 - Error to wait for data EOF signal<br>");
    strcat(inBufGet,"0x15 - Error to validate the checksum<br>");
    strcat(inBufGet,"0x16 - Error when temperature and humidity are zero, it shouldn't happen<br>");
    strcat(inBufGet,"0x17 - Error when pin is not initialized<br>");
    strcat(inBufGet,"</i></small></p></td></tr>");
    strcat(inBufGet,"</table>");
    vTaskSuspendAll();
    cl.print(inBufGet);
    xTaskResumeAll();
  }
}  

// Часы по NTP -------------------------------------------
// В момент обновления горит LED2
const int NTP_PACKET_SIZE= 48;             // временная отметка NTP находится в первых 48 байтах сообщения
byte packetBuffer[NTP_PACKET_SIZE];        // буфер, в котором будут храниться входящие и исходящие пакеты 
 IPAddress timeServer(88,147,254,235);     // NTP-сервер ntp3.stratum2.ru
// IPAddress timeServer(132, 163, 4, 102); // NTP-сервер time-b.timefreq.bldrdoc.gov
// IPAddress timeServer(132, 163, 4, 103); // NTP-сервер time-c.timefreq.bldrdoc.gov NTP
time_t getNtpTime()
{
  #ifdef DEBUG
  Serial.println("Update time NTP");
  #endif 
  
  if(!checkNetLink()) { //Если нет линка то выходим
  #ifdef DEBUG
  Serial.println("No link ethernet");
  #endif
  FLAG_NTP_ERR; // время не обновлено
  return 0;  
  }
  digitalWrite(PIN_LED2, LOW); // В процессе обновления горит LED2
  while (Udp.parsePacket() > 0) ; // discard any previously received packets
  #ifdef DEBUG
  Serial.println("Transmit NTP Request");
  #endif
  sendNTPpacket(timeServer);
  uint32_t beginWait = millis();
  while (millis() - beginWait < 1500) {
    int size = Udp.parsePacket();
    if (size >= NTP_PACKET_SIZE) {
      #ifdef DEBUG
      Serial.println("Receive NTP Response");
      #endif
      Udp.read(packetBuffer, NTP_PACKET_SIZE);  // read packet into the buffer
       // Временная отметка начинается с 40 байта полученного пакета
       // и его длина составляет четыре байта или два слова.
       // Для начала извлекаем два этих слова:
       unsigned long highWord = packetBuffer[40]<<8 | packetBuffer[41];
       unsigned long lowWord =  packetBuffer[42]<<8 | packetBuffer[43];
       // Совмещаем четыре байта (два слова) в длинное целое.
       // Это и будет NTP-временем (секунды начиная с 1 января 1990 года):
        unsigned long secsSince1900 = highWord << 16 | lowWord;
        // Время Unix стартует с 1 января 1970 года. В секундах это 2208988800:
        const unsigned long seventyYears = 2208988800UL;
        uint32 epoch = secsSince1900-seventyYears+TZ;  // Вычитаем 70 лет и делаем поправку на часовой пояс
         #ifdef DEBUG  
         Serial.print("NTP ok, unix time = ");Serial.println(epoch);
         #endif
         rt.setTime(epoch);
         FLAG_NTP_OK; // время обновлено
         digitalWrite(PIN_LED2, HIGH); 
         return epoch;
    }
  }
  #ifdef DEBUG
  Serial.println("No NTP Response :-(");
  #endif
  FLAG_NTP_ERR; // время не обновлено
  digitalWrite(PIN_LED2, HIGH); 
  return 0; // return 0 if unable to get the time
}

// send an NTP request to the time server at the given address
void sendNTPpacket(IPAddress &address)
{
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;
  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:                 
  Udp.beginPacket(address, 123); //NTP requests are to port 123
  Udp.write(packetBuffer, NTP_PACKET_SIZE);
  Udp.endPacket();
}
