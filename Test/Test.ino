/**************************************************************************
 @FILE:         Test.cpp
 @AUTHOR:       Raimundo Alfonso
 @COMPANY:      Ray Ingeniería Electronica, S.L.
 @DESCRIPTION:  Test placa 220412 - v1.00
 @BOARD:        WiFi LoRa 32 (V2)
 @MICRO:        ESP32
  
 @VERSIONS:
  01-06-2022 - v1.00 :
  - primera versión
  
**************************************************************************/
#include <Wire.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "SHT2x.h"

#define SOIL1       36
#define SOIL2       38
#define SOIL3       39
#define EV1         2
#define EV2         23    // Cambiamos pin original 12 a pin 23 
#define EV1_AL      32
#define EV2_AL      33
#define DS18B20     13
#define LED         25


// DS18B20...
OneWire oneWire(DS18B20);
DallasTemperature ds18b20(&oneWire);

// SHT21...
SHT2xClass SHT2x;


void menu_inicio(void){ 
  Serial.println("Test placa 220412 - v1.00");
  Serial.println("Ray Ingenieria Electronica, S.L. - 2022 ");  
  Serial.println();
}

int test = 0;
float temperature = 0;
float humidity = 0;           
float dew_point = 0; 
float temperature2 = 0;

// Timer
hw_timer_t * timer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

void IRAM_ATTR onTimer(){
  char c = 0;
  String inString = "";
  
  // Increment the counter and set the time of ISR
  portENTER_CRITICAL_ISR(&timerMux);
  
  while (Serial.available() > 0) {
    c = Serial.read();   
    inString += (char)c;    
  }    
  if(c != 0){
    inString.trim();
    Serial.println(inString);
    if(inString == ""){
      test++;
    }
    if(inString == "r" || inString == "R"){
      test--;
      if(test < 1) test = 1;
    }
  }
  Serial.flush();
  
  portEXIT_CRITICAL_ISR(&timerMux);
}

void setup(){ 
  pinMode(EV1, OUTPUT);
  pinMode(EV2, OUTPUT);
  pinMode(EV1_AL, INPUT);
  pinMode(EV2_AL, INPUT);
  pinMode(LED, OUTPUT);

 
  // Configura puertos serie...
  Serial.begin(9600); 
  delay(100);

  analogReadResolution(12);
  //analogSetWidth(10);                           
  //analogSetAttenuation((adc_attenuation_t)0);   

  // Mensaje inicial...
  menu_inicio();

  // SHT21...
  Wire.begin();

  // DS18B20...
  Serial.println("Temperature sensor init...");
  ds18b20.begin();
  ds18b20.setWaitForConversion(true);
  ds18b20.setResolution(12);
  Serial.print("Found ");
  Serial.print(ds18b20.getDeviceCount(), DEC);
  Serial.println(" devices.");
  
  
  // Timer cada 100mS...
  // Use 1st timer of 4 (counted from zero).
  // Set 80 divider for prescaler (see ESP32 Technical Reference Manual for more
  // info).
  timer = timerBegin(0, 80, true);
  // Attach onTimer function to our timer.
  timerAttachInterrupt(timer, &onTimer, true);
  // Set alarm to call onTimer in microseconds
  // Repeat the alarm (third parameter)
  timerAlarmWrite(timer, 100000, true);
  // Start an alarm
  timerAlarmEnable(timer);
 

  Serial.println();
  Serial.println("Habilita NL y CR");
  Serial.println("Pulsa 'INTRO' para avanzar por el test y 'r' para retrodecer.");
  Serial.println();
} 



void loop(){ 

  switch(test){
    case 1:
      testSalidas();
      break;    
    case 2:
      testSensores();
      break;      
    case 3:
      testFin();
      break;               
  }
} 

void testFin(void){
  Serial.println();
  Serial.println("TEST FINALIZADO");
  while(1);  
}

void testSalidas(void){
  static boolean primeraVez = true;

  if(primeraVez){
    Serial.println("Test de salidas digitales...");
    primeraVez = false;
  }
  
  Serial.println("Salida EV1");
  digitalWrite(EV1, HIGH);
  delay(1000);
  Serial.print("Alarma EV1: ");
  if(digitalRead(EV1_AL))
    Serial.println("on");
  else
    Serial.println("off");
  digitalWrite(EV1, LOW);
  delay(1000);

  Serial.println("Salida EV2");
  digitalWrite(EV2, HIGH);
  delay(1000);
  Serial.print("Alarma EV2: ");
  if(digitalRead(EV2_AL))
    Serial.println("on");
  else
    Serial.println("off");  
  digitalWrite(EV2, LOW);
  delay(1000);

  Serial.println();

}

void testSensores(void){
  static boolean primeraVez = true;
  int mv;

  if(primeraVez){
    Serial.println("Test de sensores...");
    primeraVez = false;
  }

  // Activamos led de la placa
  digitalWrite(LED, HIGH);

  // Lectura de los sensores SOILWATCH...
  Serial.println("SOILWATCH...");
  mv = map(analogRead(SOIL1), 0, 4095, 0, 3300);
  Serial.print("SoilWatch1 : ");
  Serial.println(mv);

  mv = map(analogRead(SOIL2), 0, 4095, 0, 3300);
  Serial.print("SoilWatch2 : ");
  Serial.println(mv);

  mv = map(analogRead(SOIL3), 0, 4095, 0, 3300);
  Serial.print("SoilWatch3 : ");
  Serial.println(mv);
  Serial.println();

  // Lectura del sensor SHT21...
  leeSHT21();  
  Serial.println("SHT21...");
  Serial.print("Temperatura: ");
  Serial.print(temperature);
  Serial.println(" ºC");
  Serial.print("Humedad    : ");
  Serial.print(humidity);
  Serial.println(" %");
  Serial.print("Pto. rocio : ");
  Serial.print(dew_point);
  Serial.println(" ºC");
  Serial.println();


  // Lectura del sensor DS18B20...
  Serial.println("DS18D20...");
  ds18b20.requestTemperatures(); 
  temperature2 = ds18b20.getTempCByIndex(0);
  if((int)temperature2 >= 85 || (int)temperature2 <= -50){
    Serial.println(F("Temperatura: FALLO"));
    Serial.println();
  }else{
    Serial.print("Temperatura: ");
    Serial.print(temperature2,1);
    Serial.println(" ºC");
  }
  Serial.println();
  Serial.println();  

  // Desactivamos led de la placa
  digitalWrite(LED, LOW);

  delay(2000);
}

// Lectura del sensor SHT21 sin bloqueo...
void leeSHT21(void){
  unsigned long timeOutSensor = 0; 
  float t;
  float h;
  boolean temperature_ok = false;
  boolean humidity_ok = false;
 
  SHT2x.PrepareTemperatureNoHold();
  timeOutSensor = millis();  
  do{ 
    if(SHT2x.GetTemperatureNoHold(&t)){
      temperature_ok = true;     
    }
  }while(temperature_ok == false && (millis() - timeOutSensor) < 100L);
  
  SHT2x.PrepareHumidityNoHold();
  timeOutSensor = millis();  
  do{ 
    if(SHT2x.GetHumidityNoHold(&h)){
      humidity_ok = true;   
    }
  }while(humidity_ok == false && (millis() - timeOutSensor) < 100L);

  if(temperature_ok && humidity_ok){
    temperature = t;
    humidity = h;     
    dew_point = calcDewpoint(h,t); 
  }else{
    temperature = 0;
    humidity = 0;           
    dew_point = 0;   
  }
}

float calcDewpoint(float humi, float temp) {
  float k;
  k = log(humi/100) + (17.62 * temp) / (243.12 + temp);
  return 243.12 * k / (17.62 - k);
}
