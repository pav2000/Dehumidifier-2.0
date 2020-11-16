# Dehumidifier-2.0
Осушитель подвала ver.2.0 stm32, доступ через сеть <br>
Дальнейшее развитие проекта https://github.com/pav2000/Podval. 
Принцип работы остался прежним - Идея контроля влажности подвала была подсмотрена на http://geektimes.ru (http://geektimes.ru/post/255298/) Вся идея состоит в том чтобы измерить температуру и относительную влажность в подвале и на улице, на основании температуры и относительной влажности рассчитать абсолютную влажность и принять решение о включении вытяжного вентилятора в подвале. Теория для расчета изложена здесь - https://carnotcycle.wordpress.com/2012/08/04/how-to-convert-relative-humidity-to-absolute-humidity/

Аппаратная часть полностью переделана. Теперь используется STM32 и сетевой чип wiznet w5500. На самом чипе поднят простейший веб сервер, через который проводится настройка устройства и его контоль. Разрабатывая этот блок я планировал сделать универсальный модуль для домашней автоматизации, и заложил избыточность в схеме для реализации будующих устройств.
Сам блок реализован на плате maple mini (STM32F103CBT6). 
Характеристи микроконтроллера:
Тактовая частота: 72 MHz
Входное напряжение (рекомендуемое): 3.0V-12V
34 цифровых I/O-пинов (GPIO)
9 аналоговых входных пинов
12-битный АЦП-преобразователь
12 PWM-выводов, 16-битное разрешение (PWM)
USB-порт для программирования и коммуникации
120 Кб Flash и 20 Кб SRAM
Интегрированный SPI (2 выхода) и I2C (2 выхода)
7 каналов Direct Memory Access (DMA)
3 USART-устройства
Четыре 4-канальных таймера
Поддержка режимов низкого напряжения и сна (менее 500 мА)
На blue pill (самая распространненая плата stm32) 64 кб флеша - по этому я испольщовал maple mini c 128 кб.

Интеграция stm32f103 в среду arduino описана вот здесь:
https://habr.com/ru/post/395577/
https://istarik.ru/blog/arduino/102.html

github с доработками для ардуино https://github.com/rogerclarkmelbourne/Arduino_STM32

Схема платы:
![Схема осушителя](https://github.com/pav2000/Dehumidifier-2.0/blob/master/Picture/Schematic%20ver%202.0.jpg)

Печатная плата
![Плата1](https://github.com/pav2000/Dehumidifier-2.0/blob/master/Picture/20200928_090243.jpg)
![Плата2](https://github.com/pav2000/Dehumidifier-2.0/blob/master/Picture/20200928_090301.jpg)

Сборка 
![Сборка1](https://github.com/pav2000/Dehumidifier-2.0/blob/master/Picture/20200929_081615.jpg)
![Сборка2](https://github.com/pav2000/Dehumidifier-2.0/blob/master/Picture/20200929_081629.jpg)

Работа
![Работа1](https://github.com/pav2000/Dehumidifier-2.0/blob/master/Picture/20201101_104746.jpg)
![Работа2](https://github.com/pav2000/Dehumidifier-2.0/blob/master/Picture/20201101_104822.jpg)



  


