#ifndef webserver_h
#define webserver_h
// Редактор https://be1.ru/html-redaktor-online/
// http://translit-online.ru/image-base64-converter.html
// https://texttools.ru/add-line-breaks
// https://www.buildmystring.com/default.php

// Константы для веб страницы --------------------------------------------------------------------------
const char  indexMain0[] = 
"<!DOCTYPE html>"
"<html>"
"<head>"
"<title>Осушитель 2.0</title>"
// https://base64.guru/converter/encode/image/ico
"<link href=\"data:image/x-icon;base64,/9j/4AAQSkZJRgABAQEAYABgAAD/2wBDAAMCAgMCAgMDAwMEAwMEBQgFBQQEBQoHBwYIDAoMDAsKCwsNDhIQDQ4RDgsLEBYQERMUFRUVDA8XGBYUGBIUFRT/2wBDAQMEBAUEBQkFBQkUDQs"
"NFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBT/wAARCAAgACADASIAAhEBAxEB/8QAHwAAAQUBAQEBAQEAAAAAAAAAAAECAwQFBgcICQoL/8QAtRAAAgEDAwIEAwUFBAQAAAF9AQIDAAQRBRIhMUEGE1FhByJxFDKBkaEII0K"
"xwRVS0fAkM2JyggkKFhcYGRolJicoKSo0NTY3ODk6Q0RFRkdISUpTVFVWV1hZWmNkZWZnaGlqc3R1dnd4eXqDhIWGh4iJipKTlJWWl5iZmqKjpKWmp6ipqrKztLW2t7i5usLDxMXGx8jJytLT1NXW19jZ2uHi4+Tl5ufo6erx8vP09fb3+Pn6/8QAHwEAAwEBAQEBAQE"
"BAQAAAAAAAAECAwQFBgcICQoL/8QAtREAAgECBAQDBAcFBAQAAQJ3AAECAxEEBSExBhJBUQdhcRMiMoEIFEKRobHBCSMzUvAVYnLRChYkNOEl8RcYGRomJygpKjU2Nzg5OkNERUZHSElKU1RVVldYWVpjZGVmZ2hpanN0dXZ3eHl6goOEhYaHiImKkpOUlZaXmJmaoqO"
"kpaanqKmqsrO0tba3uLm6wsPExcbHyMnK0tPU1dbX2Nna4uPk5ebn6Onq8vP09fb3+Pn6/9oADAMBAAIRAxEAPwD7T+Nnx21Hwjr9j4J8EaUviHx5qCeYkDsTDZRn/lpKQRnpnqOOScYDeSatpPi1fEFpp3iz9oSbSPEt5DJOmnaRbEQRhCgdfkZAQC68FR39Can+Ed9"
"Pq1x8WPGxkCa9qniM6LFckfNbQKyBQPQBXUf8AX0qP9oHxFonwZ+L/wAHL2HS3kTTxqBuFVkR7uCdYbd1TcQZpg8kcnljkhT0zX0lWp/Z8lh6EVzJatpNt2u907Jbab7s4P4nvSehtW/xY+InwHksLrxtf23xB+H11IsJ8SadHtubMk4DSBRyM8dyem7OFP1BY31tqlj"
"b3lnNHc2lxGssM0RDJIjDKsMdQQQcduteT+IfB+mza/c+HJIF/sPxRYXEd1ZqBsSVAv71R0UkN27gGsX9ifXLrV/gRYWt05lOl3txYRyFiSyKwdeT0A37R7KBXNXUMVhniVFRnFpOysmneztsndNO2jNabcZ8j2PPdQW0+B3xd8XeGfE0jaf4G8dTnU9N1c8RWd2W3Mr"
"N0ADEc9AFjJwCxHu3ir4e6V8QtP8ADJ1O5muG0XUbXWbS7tnVS88OSrE4IKtk5A6g9a6vxf4M0Tx9oU+jeINNh1TTZuWgmQ8MOjLjBVh7EEdc14ef2MdL0t3i8O+PPF/h3THOW0+01H90MnkKQBx7nPNOVXDY1KdabhNJJ6XTtpfTVO2/R7goypt8quif48fEXT/h3cT"
"SWl7/AGh411S1Gm6PpKEEwNIcGYgchc4OT1xge179nnS4vhXpNh4QL7o5YxK0mP8Al4YbiSP9oY47fL6VZ8P/ALMPhP4c6bd3ui2VzrPiFyrPqOqTeddSAMC6qcAKSMggDOOpNI2k32s+L/NsY5vnnWXzHiZPIGcjdkcY6fhxXq4eOEnhZ0Kcrrq3pqttOiWtu+p4mNq"
"16VenKK+S633P/9k=\"  rel=\"icon\" "
"type=\"image/x-icon\" />"
"</head>"
"<body>"

"<table border=0><tr><td>" // Не видная таблица из двух ячеек картинка и надпись

"<img alt=\"\" src=\"data:image/x-icon;base64,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" 
"Aq1f8BMr8ZSHtbbjwAAAAASUVORK5CYII=\" />"
//"</body>\r\n"

"</td><td>"
"<span style=\"font-size:28px\"><strong>&nbsp;&nbsp;&nbsp;&nbsp;Осушитель 2.0</strong></span>";

//"</td></tr></table></body><hr/>"

const char  indexMain1[] = 
"<hr/>"
"<form action=\"/get\"enctype=\"text/plain\"id=\"network\"method=\"get\"name=\"Настройки\">"
" <table rules=\"none\"cellspacing=\"0\"cellpadding=\"2\"border=\"0\"width=\"640\">"
   "<caption>Настройки</caption>"
   "<tr>"
    "<th align=\"left\">Наименование параметра</th>"
    "<th align=\"left\">Значение</th>"
    "</tr>";


const char  indexMain2[] = 
"</table>"
//"<p>&nbsp;</p>\r\n"
"<p><input name=\"send\" type=\"submit\"value=\"Применить\" /></p>"
"</form>"
"<hr/><p></p>";

const char  indexMain3[] =
"<p>&nbsp;</p>"
"</body>"
"</html>";

// ------------------------------------- ГРАФИКИ ------------------------------------------------------------
// Google Chart  константы для вывода графиков
const char  ChartHeader[] =  // заголовок страницы
      "<html>"
      "<head>"
      "<script type=\"text/javascript\" src=\"https://www.gstatic.com/charts/loader.js\"></script>"
      "<script type=\"text/javascript\">"
      "google.charts.load('current', {'packages':['corechart']});"
      "google.charts.setOnLoadCallback(drawTemp);"
      "google.charts.setOnLoadCallback(drawAbsH);";

const char  startTemp[] = // заголовок графика температуры 
      "function drawTemp(){"
      "var data=google.visualization.arrayToDataTable(["
      "['Время','Подпол','Улица'],";
const char  endTemp[] = // окончание графика температуры
      "]);"
      "var options={"
      "title:'Температура C°',"
      "titleTextStyle:{fontSize:16},"
      "curveType:'none',"//Сглаживать(function) или нет(none) линию
      "legend:{position:'bottom'},"
      "colors:['red','green'],"  // Цвет линий
      "chartArea:{left:40,top:20,width:'99%',height:'80%',backgroundColor:'#FAFAF5'}"
      "};"
      "var chart=new google.visualization.LineChart(document.getElementById('Temp_chart'));"
      "chart.draw(data,options);"
      "}";

const char  startAbsH[] = // заголовок графика влажности
      "function drawAbsH() {"
      "var data=google.visualization.arrayToDataTable(["
      "['Время','Подпол','Улица'],";
const char  endAbsH[] = // Окончание графика влажности
      "]);"
      "var options={"
      "title:'Абсолютная влажность г/м*3',"
      "titleTextStyle:{fontSize:16},"
      "curveType:'none',"//Сглаживать(function) или нет(none) линию
      "legend:{position:'bottom'},"
      "colors:['red','green']," // Цвет линий
      "chartArea:{left:40,top:20,width:'99%',height:'80%',backgroundColor:'#FAFAF5'}" // Положение и размеры области графика цвет области рисования
      "};"
      "var chart=new google.visualization.LineChart(document.getElementById('AbsH_chart'));"
      "chart.draw(data,options);"
      "}";
      
const char  ChartBotton[] = // окончание страницы графиков
      "</script>"
      "</head>"
      "<body>"
      "<table class=\"columns\">"
      "<tr>"
      "<td><div id=\"Temp_chart\" style=\"width:500px;height:300px\" \"border: 1px solid #ccc\"></div></td>"  // размеры графиков
      "<td><div id=\"AbsH_chart\" style=\"width:500px;height:300px\" \"border: 1px solid #ccc\"></div></td>"
      "</tr>"
      "</table>"
      "</body>"
      "</html>";
#endif      
