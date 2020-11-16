# Dehumidifier-2.0
Осушитель подвала ver.2.0 stm32, доступ через сеть <br>
Дальнейшее развитие проекта https://github.com/pav2000/Podval. 
Принцип работы остался прежним - Идея контроля влажности подвала была подсмотрена на http://geektimes.ru (http://geektimes.ru/post/255298/) Вся идея состоит в том чтобы измерить температуру и относительную влажность в подвале и на улице, на основании температуры и относительной влажности рассчитать абсолютную влажность и принять решение о включении вытяжного вентилятора в подвале. Теория для расчета изложена здесь - https://carnotcycle.wordpress.com/2012/08/04/how-to-convert-relative-humidity-to-absolute-humidity/

Аппаратная часть полностью переделана. теперь используется STM32 и сетевой чип wiznet w5500. На самом чипе поднят простейший веб сервре, через который проводится настройка устройсва и его контоль. разрабатывая этот блок я планировал сделать универсальный модуль для домашней автоматизации, и заложил избыточность в схеме для реализации будующих устройств.


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



  


