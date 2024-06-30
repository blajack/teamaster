
#include <IRremote.h>
#include <OneWire.h> // ,for temp -sensor

#include <Wire.h> // for LCD display
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27,16,2);   /* Задаем адрес и размерность дисплея. 
При использовании LCD I2C модуля с дисплеем 20х04 ничего в коде изменять не требуется, cледует только задать правильную размерность */
/* Задаем адрес и размерность дисплея. 
При использовании LCD I2C модуля с дисплеем 20х04 ничего в коде изменять не требуется, cледует только задать правильную размерность */
/* Задаем адрес и размерность дисплея. 
При использовании LCD I2C модуля с дисплеем 20х04 ничего в коде изменять не требуется, cледует только задать правильную размерность */



OneWire  ds(10);  // on pin 10 (a 4.7K resistor is necessary on plus)

int i = 0; //shag motora
int speed_fan = 0;
int motorPin = 6;
int temp_off = 70;
int set_power = 0;

//ir
int RECV_PIN = 12;
IRrecv irrecv(RECV_PIN); //Создаем объект получения сигнала с определнного порта
decode_results results; //Переменная, хранящая результат


// for Music settings
#define  c3    7634
#define  d3    6803
#define  e3    6061
#define  f3    5714
#define  g3    5102
#define  a3    4545
#define  b3    4049
#define  c4    3816    // 261 Hz 
#define  d4    3401    // 294 Hz 
#define  e4    3030    // 329 Hz 
#define  f4    2865    // 349 Hz 
#define  g4    2551    // 392 Hz 
#define  a4    2272    // 440 Hz 
#define  a4s   2146
#define  b4    2028    // 493 Hz 
#define  c5    1912    // 523 Hz
#define  d5    1706
#define  d5s   1608
#define  e5    1517    // 659 Hz
#define  f5    1433    // 698 Hz
#define  g5    1276
#define  a5    1136
#define  a5s   1073
#define  b5    1012
#define  c6    955
#define  R     0      //rest
int speakerOut = 9; 
int melody[] = {  f4,  f4, f4,  a4s,   f5,  d5s,  d5,  c5, a5s, f5, d5s,  d5,  c5, a5s, f5, d5s, d5, d5s,   c5};
int beats[]  = {  21,  21, 21,  128,  128,   21,  21,  21, 128, 64,  21,  21,  21, 128, 64,  21, 21,  21, 128 }; 
int MAX_COUNT = sizeof(melody) / 2;
long tempo = 10000; 
int pause = 1000;
int rest_count = 50;//
int toneM = 0;
int beat = 0;
long duration  = 0;



void setup(void) {
  Serial.begin(9600);
  
  irrecv.enableIRIn(); // Начинаем прием

  pinMode (motorPin, OUTPUT);
  pinMode (motorPin, speed_fan);
  pinMode(speakerOut, OUTPUT);
  
  lcd.init();                       // Инициализация lcd             
  lcd.backlight();                  // Включаем подсветку
  lcd.setCursor(0, 0);              // Устанавливаем курсор в начало 1 строки
  lcd.print("Hello");       // Выводим текст
  lcd.setCursor(0, 1);              // Устанавливаем курсор в начало 2 строки
  lcd.print("www.chernov.pro");         // Выводим текст
  delay(2000);
  lcd.clear(); 
  lcd.setCursor(0, 1);
  lcd.print("Press ON");
  
  
  
}

// Music Function
void playTone() {
  long elapsed_time = 0;
  if (toneM > 0) {
    while (elapsed_time < duration) {
      digitalWrite(speakerOut,HIGH);
      delayMicroseconds(toneM / 2);
      digitalWrite(speakerOut, LOW);
      delayMicroseconds(toneM / 2);
      elapsed_time += (toneM);
    }
  }
  else {
    for (int j = 0; j < rest_count; j++) {
      delayMicroseconds(duration);
    }
  }
}

//Scan Pult
int scanPult ()
{
  if (irrecv.decode(&results)) //При получении сигнала...
  { 
    Serial.println(results.value); //…выводим его значение в последовательный порт
    
    switch (results.value) {
        // case 0xFFE21D: // клавиша звук 
        
        case 0xFF906F: //клавиша +
        temp_off=temp_off+5; 
        break;
        
        case 0xFFA857: //клавиша -
        temp_off=temp_off-5; 
        break;
        
        case 0xFFA25D: //клавиша ON/OFF
        if (set_power) 
          {set_power=0;
          lcd.clear(); 
          lcd.setCursor(0, 1);
          lcd.print("Press ON");}
          
        else
          {set_power=1;
          lcd.clear(); 
          lcd.setCursor(0, 1);
          lcd.print("I'm working!!!");}
        break;
    
    }
    
    irrecv.resume(); // Получаем следующее значение
  }
}

void loop(void) {
  
  byte i;
  byte present = 0;
  byte type_s;
  byte data[12];
  byte addr[8];
  float celsius;
  
  scanPult ();
  
  
  if ( !ds.search(addr)) {
    //Serial.println("No more addresses.");
    Serial.println();
    ds.reset_search();
    delay(150);
    return;
  }
  


  if (OneWire::crc8(addr, 7) != addr[7]) {
      Serial.println("CRC is not valid!");
      return;
  }
  Serial.println();
 
  // the first ROM byte indicates which chip
  switch (addr[0]) {
    case 0x10:
      Serial.println("  Chip = DS18S20");  // or old DS1820
      type_s = 1;
      break;
    case 0x28:
      Serial.println("  Chip = DS18B20");
      type_s = 0;
      break;
    case 0x22:
      Serial.println("  Chip = DS1822");
      type_s = 0;
      break;
    default:
      Serial.println("Device is not a DS18x20 family device.");
      return;
  } 

  ds.reset();
  ds.select(addr);
  ds.write(0x44, 1);        // start conversion, with parasite power on at the end
  
  //delay(800);     // maybe 750ms is enough, maybe not
  // we might do a ds.depower() here, but the reset will take care of it.
  
  present = ds.reset();
  ds.select(addr);    
  ds.write(0xBE);         // Read Scratchpad

 
  Serial.print(" ");
  for ( i = 0; i < 9; i++) {           // we need 9 bytes
    data[i] = ds.read();
  }
 
  // Convert the data to actual temperature
  // because the result is a 16 bit signed integer, it should
  // be stored to an "int16_t" type, which is always 16 bits
  // even when compiled on a 32 bit processor.
  int16_t raw = (data[1] << 8) | data[0];
  if (type_s) {
    raw = raw << 3; // 9 bit resolution default
    if (data[7] == 0x10) {
      // "count remain" gives full 12 bit resolution
      raw = (raw & 0xFFF0) + 12 - data[6];
    }
  } else {
    byte cfg = (data[4] & 0x60);
    // at lower res, the low bits are undefined, so let's zero them
    if (cfg == 0x00) raw = raw & ~7;  // 9 bit resolution, 93.75 ms
    else if (cfg == 0x20) raw = raw & ~3; // 10 bit res, 187.5 ms
    else if (cfg == 0x40) raw = raw & ~1; // 11 bit res, 375 ms
    //// default is 12 bit resolution, 750 ms conversion time
  }
  celsius = (float)raw / 16.0;
  
  Serial.print("  Temperature = ");
  Serial.print(celsius);
  Serial.println(" Celsius, ");
  
  Serial.print("  Fun-speed = ");
  Serial.print(speed_fan);
  Serial.println(" ");  
////////////////////  
  if (celsius > temp_off && set_power)
   {speed_fan=250;}
  else if (celsius <= temp_off && set_power)
   {speed_fan=0; //stop fun & play music
      
     lcd.setCursor(0, 1);
     lcd.print("Finish            ");
    
      
    
   }
   else {speed_fan=0;}
  
  pinMode (motorPin, speed_fan);
 
 //  Отдельное условие для музыки иначе вентилятор остановится не сразу
 if (celsius <= temp_off && set_power)
 {
     for (int i=0; i<MAX_COUNT; i++) {
      toneM = melody[i];
      beat = beats[i];
      duration = beat * tempo;
      playTone();
      delayMicroseconds(pause);
      scanPult (); // Если во время мелодии нажали на кнопку ON/OFF, то принудидельно выходимЕсли во время мелодии нажали на кнопку, то принудидельно выходим
      if (!set_power) {break;}
      if (irrecv.decode(&results)) {break;}
     }
 }

  // Printing information on LCD
 //lcd.clear();                      // Очистка дисплея, (удаление всех данных) установка курсора в ноль
 lcd.setCursor(0, 0);              // Устанавливаем курсор (номер ячейки, строка) 
 lcd.print("t:"); 
 //lcd.setCursor(7, 0);
 lcd.print(celsius);

 //lcd.setCursor(0, 1);
 lcd.print(" Off:");
 lcd.print(temp_off);
 //lcd.print("Val:");
 //lcd.print(val);
}

//Functions


