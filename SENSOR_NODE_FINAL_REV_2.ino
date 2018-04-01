#include <OneWire.h>
#include <DallasTemperature.h>
#include <Battery.h>
#include <SoftwareSerial.h>
#include <avr/sleep.h>
#include "DHT.h"

//#define WAKE_PIN 2
#define BATT_ACT_PIN 5
#define CO2_PWM_PIN 6
//#define ESP_RST_PIN 7 
#define HUMID_DHT22_PIN 8
#define TEMP_DSB_PIN 10
#define SENSORS_ACT_PIN 12
#define BATT_SENSE_PIN A0
#define AMP_SENSE_PIN A1
#define DHTTYPE DHT22
#define BATT_MIN_VOLT 7400
#define BATT_MAX_VOLT 8400
#define LOOP_CURRENT_NO 6 

OneWire oneWire(TEMP_DSB_PIN);
DallasTemperature tempSensor(&oneWire);
DHT humidSensor(HUMID_DHT22_PIN, DHTTYPE);
Battery battery(BATT_MIN_VOLT, BATT_MAX_VOLT, BATT_SENSE_PIN, BATT_ACT_PIN);
SoftwareSerial espSerial(3,4);

float setupCurrent, loopCurrent;
unsigned long setupTime, loopTime, totalReadTime;
String sleepTime, sleepTimeData;
int co2ValueInt;
long currentVcc;

void setup() {
  // put your setup code here, to run once:
  setupCode();
}

void loop() {
  // put your main code here, to run repeatedly:
  unsigned long loopStartTime = millis();
  Serial.println("Requesting current time from the rpi3 server..");
  int currentTime = espRequestTime();
  if (currentTime >= 8 && currentTime <= 18){
    sleepTime = "600";
    sleepTimeData = "10";
    Serial.println("Setting sleepTime as 300, which is 5 minutes");
  }
  else{
    sleepTime = "3600";
    sleepTimeData = "60";
    Serial.println("Setting sleepTime as 3600, which is 1 hour");
  }
  loopCurrent = getAvgAmps();
  //Serial.println("Loop current measured. Loop current is: " + String(loopCurrent, 4)+"A");
  //long currentVcc = readVcc();
  loopCurrent += getAvgAmps();
  //Serial.println("Current nano operating voltage is: " + String(currentVcc)+"V");
  int co2Value = getCo2();
  loopCurrent += getAvgAmps();
  //Serial.println("Measured co2 level is: " + String(co2Value)+ "ppm");
  float tempValue = getTemp();
  loopCurrent += getAvgAmps();
  //Serial.println("Measured temp level is: " + String(tempValue,2)+"°C");
  float humidValue = getHumid();
  loopCurrent += getAvgAmps();
  //Serial.println("Measured humid level is: " + String(humidValue,2)+"%");
  int battVoltValue = getBattVolt();
  //Serial.println("Measured battery voltage level is: " + String((battVoltValue/1000))+"V");
  int battLvlValue = getBattLvl(battVoltValue);
  //Serial.println("Measured battery level (in %) is: " + String(battLvlValue)+"%");
  loopCurrent += getAvgAmps();
  Serial.println("Shuting down the sensors..");
  sensorsOFF();
  Serial.println("Sensors switched off");
  unsigned long loopEndTime = millis();
  loopTime = loopEndTime - loopStartTime;
  //Serial.println("Loop time is: " + String(loopTime)+"ms");
  //Serial.println("Average loop current is: " + String((loopCurrent/LOOP_CURRENT_NO), 4)+"A");
  Serial.println("Calculating total average consumption...");
  float AvgCurrent = (setupCurrent*setupTime + (loopCurrent/LOOP_CURRENT_NO)*loopTime)/(setupTime + loopTime);
  totalReadTime = setupTime + loopTime;
  //Serial.println("Computed Total Average Current Consumption: "+ String(AvgCurrent,4)+"A");
  Serial.println("Combining the data into string for transmission..");
  String dataComplete = buildDataString(co2Value, tempValue, humidValue, battVoltValue, battLvlValue,
              AvgCurrent, currentVcc, totalReadTime,sleepTimeData);
  Serial.println("Done combining string");
  Serial.println("Combined String: " + dataComplete);
  Serial.println("Sending data to the client esp");
  espSendData(dataComplete);
  Serial.println("Data Sent");
  delay(60000);
  //espSendSleep(sleepTime);
  //selfDeepSleep();
  //digitalWrite(ESP_RST_PIN, LOW);
  //digitalWrite(ESP_RST_PIN, HIGH);
  setupCode();
}


void setupCode(){
  unsigned long setupStartTime = millis();
  Serial.begin(9600);
  espSerial.begin(9600);
  Serial.println("Setting up sensor node...");
  Serial.println("Setting up pin mode...");
  pinMode(CO2_PWM_PIN, INPUT);
  pinMode(SENSORS_ACT_PIN, OUTPUT);
  //pinMode(WAKE_PIN, INPUT);
  //pinMode(ESP_RST_PIN, OUTPUT);
  Serial.println("Initialize sensors...");
  sensorsON();
  currentVcc = 0;
  for(int i = 0 ; i <50;i++){
    currentVcc += readVcc();
  }
  currentVcc = currentVcc / 50L;
  tempSensor.begin();
  humidSensor.begin();
  battery.begin(currentVcc, 1.68);
  Serial.println("Sensors started");
  setupCurrent = getAvgAmps();
  unsigned long setupEndTime = millis();
  setupTime = setupEndTime - setupStartTime;
  Serial.println("Setup current is: " + String(setupCurrent,4));
  Serial.println("Setup time is: " + String(setupTime));
}

float getAvgAmps(){
  unsigned int analogVolt=0;
  float voltage = 0;
  float ampsValue = 0;
  for (int i = 0 ; i < 100 ; i ++ ){
    analogVolt = analogVolt + analogRead(AMP_SENSE_PIN);
    delay(1);
    }
  voltage = ((analogVolt / 100)/1024.0) *currentVcc ;
  ampsValue = (voltage -((currentVcc/2)-115)) / 1850 ; //-2.644357/0.18;
  while ( ampsValue < 0 ){
    analogVolt=0;
    for (int i = 0 ; i < 100 ; i ++ ){
      analogVolt = analogVolt + analogRead(AMP_SENSE_PIN);
      delay(1);
    }
    voltage = ((analogVolt / 100)/1024.0) * currentVcc ;//readVcc()
    ampsValue = (voltage - ((currentVcc/2)-115)) / 1850 ; //-2.644357/0.185//1850 (readVcc()/2)
  }
  Serial.print("Analog pin: ");
  Serial.println(analogVolt/100);
  Serial.print("Voltage :");
  Serial.println(voltage);
  Serial.print("Measured current :");
  Serial.println(String(ampsValue,4));
  return ampsValue;
}

int getCo2(){
  //unsigned long co2StartTime = millis();
  
  unsigned long riseTime = pulseIn(CO2_PWM_PIN, HIGH);
  while (riseTime < 2000){
    unsigned long correctedRiseTime = pulseIn(CO2_PWM_PIN, HIGH);
    riseTime = correctedRiseTime;
  }
  Serial.print(riseTime/1000); Serial.println(" ms ");
  long co2Value = (5 * ((riseTime/1000) - 2)) -650;//- 800 ;
  co2ValueInt = (int) co2Value;
  Serial.print("co2Value is: ");
  Serial.println(co2Value);
  delay(2000);
  while(co2ValueInt < 0){
    unsigned long riseTime = pulseIn(CO2_PWM_PIN, HIGH);
    while (riseTime < 2000){
    unsigned long correctedRiseTime = pulseIn(CO2_PWM_PIN, HIGH);
    riseTime = correctedRiseTime;
    }
    Serial.print(riseTime/1000); Serial.println(" ms ");
    long co2Value = (5 * ((riseTime/1000) - 2)) -650;//- 800 ;
    co2ValueInt = (int) co2Value;
    Serial.print("co2Value is: ");
    Serial.println(co2Value);
    delay(2000);
  }
  //Serial.print("co2Value is: ");
  //Serial.println(co2Value);
  //co2Current = getAvgAmps();
  //unsigned long co2EndTime = millis();
  //co2Time = co2EndTime - co2StartTime;
  return co2ValueInt;
}

float getTemp(){
  //unsigned long tempStartTime = millis(); 
  Serial.println("Measuring Temperature..");
  tempSensor.requestTemperatures();
  float tempValue = tempSensor.getTempCByIndex(0);
  //tempCurrent = getAvgAmps();
  //unsigned long tempEndTime = millis();
  //tempTime = tempEndTime - tempStartTime;
  //Serial.println("Temp current is: " + String(tempCurrent, 4)+"A");
  //Serial.println("Temp time is: " + String(tempTime)+"ms");
  return tempValue;
}

float getHumid(){
  //unsigned long humidStartTime = millis();
  Serial.println("Measuring Humidity..");
  float humidValue = (humidSensor.readHumidity() - 1.20);
  //humidCurrent = getAvgAmps();
  //unsigned long humidEndTime = millis();
  //humidTime = humidEndTime - humidStartTime;
  //Serial.println("Humid current is: " + String(humidCurrent, 4)+"A");
  //Serial.println("Humid time is: " + String(humidTime)+"ms");
  return humidValue;
}



int getBattVolt(){
  //unsigned long battVoltStartTime = millis();
  Serial.println("Measuring Battery Voltage and battery level..");
 
  int battVoltValue = battery.voltage();
  //139mV reduction in connectivity
  //battVoltCurrent = getAvgAmps();
  //unsigned long battVoltEndTime = millis();
  //battVoltTime = battVoltEndTime - battVoltStartTime;
  //Serial.println("BattVolt current is: " + String(battVoltCurrent, 4)+"A");
  //Serial.println("BattVolt time is: " + String(battVoltTime)+"ms"); 
  return battVoltValue;
}

int getBattLvl(int battVoltValue){
  int battLvlValue = battery.level(battVoltValue);
  return battLvlValue;
}

int espRequestTime(){
  Serial.println("Sending getTime to the esp client..");
  espSerial.print("getTime\n");
  while(!espSerial.available()){
    Serial.println("Waiting time response from the rpi3 server..");
    delay(500);
  };
  String currentTimeStr = espSerial.readStringUntil('\n');
  currentTimeStr.trim();
  Serial.println("Current time is: " + currentTimeStr+"hr");
  int currentTimeInt = currentTimeStr.toInt();
  return currentTimeInt;
}


void espSendData(String a){
  Serial.println("Sending sendData to the esp client..");
  espSerial.print("sendData\n");
  while(!espSerial.available()){};
  String r = espSerial.readStringUntil('\n');
  r.trim();
  Serial.println("Received response from esp: " + r);
  if (r == "sendDataOK"){
  Serial.println("Printing combined string to esp client..");
  espSerial.print(a+'\n');
  }
  while(!espSerial.available()){};
  String b = espSerial.readStringUntil('\n');
  b.trim();
  if(b=="RECEIVED"){
  Serial.println("Received second response from esp: " + b);
  }
}

String buildDataString(int a, float b, float c, int d, int e, float f, long g, long h,String i){
  String sensorData = String(a)+"%"+String(b,2)+"%"+String(c,2)+"%"+String(d)+
            "%"+String(e)+"%"+String(f,4)+"%"+String(g)+"%"+String(h)+"%"+i;
  return sensorData;
}

void wakeUpNow(){}

void selfDeepSleep(){

  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  sleep_enable();
  attachInterrupt(0, wakeUpNow, LOW);
  sleep_mode();
  sleep_disable();
  detachInterrupt(0);
  
}

void espSendSleep(String a){

  espSerial.print("sleepNow\n");
  while(!espSerial.available()){};
  String r = espSerial.readStringUntil('\n');
  r.trim();
  if ( r == "sleepNowOK" ){
    espSerial.print(a+'\n');
  }

}

void sensorsON(){
  digitalWrite(SENSORS_ACT_PIN, HIGH);
  delay(200);
}

void sensorsOFF(){
  digitalWrite(SENSORS_ACT_PIN, LOW);
  delay(200);
}

long readVcc() {
  // Read 1.1V reference against AVcc
  // set the reference to Vcc and the measurement to the internal 1.1V reference
  #if defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
    ADMUX = _BV(REFS0) | _BV(MUX4) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  #elif defined (__AVR_ATtiny24__) || defined(__AVR_ATtiny44__) || defined(__AVR_ATtiny84__)
    ADMUX = _BV(MUX5) | _BV(MUX0);
  #elif defined (__AVR_ATtiny25__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__)
    ADMUX = _BV(MUX3) | _BV(MUX2);
  #else
    ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  #endif  

  delay(2); // Wait for Vref to settle
  ADCSRA |= _BV(ADSC); // Start conversion
  while (bit_is_set(ADCSRA,ADSC)); // measuring

  uint8_t low  = ADCL; // must read ADCL first - it then locks ADCH  
  uint8_t high = ADCH; // unlocks both

  long result = (high<<8) | low;


  result=1124642.36L / result; // Calculate Vcc (in mV); 1126400 = 1.1*1024*1000//1119729.272//1142586.644//1138231.933//1118841.905//1163742.053//1230333.407//1140260.793//1103057.294//1134639.08//1139138.797
  return result; // Vcc in millivolts
}
