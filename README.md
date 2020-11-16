# Dehumidifier-2.0
Осушитель подвала ver.2.0 stm32, доступ через сеть <br>
Короткий ролик поясняющий работу и конструкцию. https://youtu.be/ywdXSmak6OI <br>

Дальнейшее развитие проекта https://github.com/pav2000/Podval. <br>
Принцип работы остался прежним - Идея контроля влажности подвала была подсмотрена на http://geektimes.ru (http://geektimes.ru/post/255298/) Вся идея состоит в том чтобы измерить температуру и относительную влажность в подвале и на улице, на основании температуры и относительной влажности рассчитать абсолютную влажность и принять решение о включении вытяжного вентилятора в подвале. Теория для расчета изложена здесь - https://carnotcycle.wordpress.com/2012/08/04/how-to-convert-relative-humidity-to-absolute-humidity/

Аппаратная часть полностью переделана. Теперь используется STM32 и сетевой чип wiznet w5500. На самом чипе поднят простейший веб сервер, через который проводится настройка устройства и его контоль. Разрабатывая этот блок я планировал сделать универсальный модуль для домашней автоматизации, и заложил избыточность в схеме для реализации будующих устройств.
Сам блок реализован на плате maple mini (STM32F103CBT6). 
Характеристи микроконтроллера:
- Тактовая частота: 72 MHz
- Входное напряжение (рекомендуемое): 3.0V-12V
- 34 цифровых I/O-пинов (GPIO)
- 9 аналоговых входных пинов
- 12-битный АЦП-преобразователь
- 12 PWM-выводов, 16-битное разрешение (PWM)
- USB-порт для программирования и коммуникации
- 120 Кб Flash и 20 Кб SRAM
- Интегрированный SPI (2 выхода) и I2C (2 выхода)
- 7 каналов Direct Memory Access (DMA)
- 3 USART-устройства
- Четыре 4-канальных таймера
- Поддержка режимов низкого напряжения и сна (менее 500 мА)
На blue pill (самая распространненая плата stm32) 64 кб флеша - по этому я испольщовал maple mini c 128 кб.

Интеграция stm32f103 в среду arduino описана вот здесь:<br>
https://habr.com/ru/post/395577/ <br>
https://istarik.ru/blog/arduino/102.html <br>
http://www.count-zero.ru/2017/stm32duino/ <br>

github с доработками для ардуино https://github.com/rogerclarkmelbourne/Arduino_STM32

Схема платы:
![Схема осушителя](https://github.com/pav2000/Dehumidifier-2.0/blob/master/Picture/Schematic%20ver%202.0.jpg)

На схеме (и плате) разведены следующие узлы:
- кварц для часов реального времени stm32 
- батарейка для часов
- два порта i2c
- флеш память spi (можно установить до 32 мбит)
- разъем sd карты (режим spi)
- дисплей 2.8 дюйма на контроллере ili9341 подключение spi
- модуль питания 220->5в
- два ssr реле со схемами гашения помех и варисторами
- датчик переменного тока (0-50A) для измерения токов нагрузки 220
- разъем для подключения отладчика
- разъем для подключения nrf24f01 - для работы с беспроводными датчиками
- два светодида на отдельных gpio
- мост i2c oneWire со схемой защиты для подключения OneWire датчиков
- пищалка
- разъем раширения (uart+gpio)

Под этот проект была разработана и изготовлена печатная плата. Плата предназначена для установки в корпус g212 
![Плата1](https://github.com/pav2000/Dehumidifier-2.0/blob/master/Picture/20200928_090243.jpg)
![Плата2](https://github.com/pav2000/Dehumidifier-2.0/blob/master/Picture/20200928_090301.jpg)

Сборка.<br>
Косяки разводки:<br>
- шелкография - на разъеме отладчика SWD перепутаны сигналы DIO CLK
- шелкография - на раъеме 220 и реле перепутаны надписи ssr1 ssr2
- разводка - надо бросить сигнальный провод (почему то не развелся) от датчика тока до maple mini
- дополнительно было распаяно (частично учтено на схеме)
RC фильтр на датчик тока, конденсатор на ножку питания, резистор 6.8к между землей и DO usb (без него и без подключения к usb МК переходил в режим загрузчика)
![Сборка1](https://github.com/pav2000/Dehumidifier-2.0/blob/master/Picture/20200929_081615.jpg)
![Сборка2](https://github.com/pav2000/Dehumidifier-2.0/blob/master/Picture/20200929_081629.jpg)

Работа.<br>
Первоначально планировалсь использовать датчики температуры и влажности AHT10 на шине i2c. Два датчика отказывались работfь на одной шине (хотя были разнесены по разным адресам), т.е. работают корректно 20-30 минут а потом отваливаются от шины. Один датчик рабоатет хорошо. При переходе на программный i2c (ногодрыг) оба датчика нормально заработали.<br>
НО при увеличении длины провода более 1.5 метра на программном i2c датчики отказались работать. По этому принято решение использовать провереннные временем DHT22 (шина типа 1-wire). Разъем i2c имеет 4 контакта (питание SDA SCL) и был перепрограммирован для подключения двух датчиков DHT22. 
Длина проводов: внутренний датчик 3 метра, внешний 7 метров, все работает с разумным количеством ошибок.<br>
Датчик тока ACS758 (диапазон 50A - применен для универсальности) позволяет измерять ток потребления устройстов. При этом при работающем вентиляторе вытяжки можно измерить общий ток и сделать вывод об работоспособности вентилятора. Обычно вентиляторе есть термопредохранитель, который срабатывает при их отказе. Так можно следить за работой вентилятора через инет. Едиственная проблема, что устройство потребляет достаточно мало и много шумов, но эту проблему удалось побороть усреднением.

Web морда устройства:<br>
Графики используют google chart api, по этому без инета они рабоать не будут.
![Работа1](https://github.com/pav2000/Dehumidifier-2.0/blob/master/Picture/Screenshot_2020-11-15-06-59-38.jpg)

Установка на объекте.<br>
![Установка1](https://github.com/pav2000/Dehumidifier-2.0/blob/master/Picture/20201101_104746.jpg)
![Установка2](https://github.com/pav2000/Dehumidifier-2.0/blob/master/Picture/20201101_104822.jpg)



  


