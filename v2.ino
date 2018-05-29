#include <UbidotsESP8266.h>
#include <TinyGPS.h>
#include <NewPing.h>
#include <SoftwareSerial.h>
#include <SparkFun_ADXL345.h>


#define TOKEN  "A1E-MEo1jXWG2exAIbnLHPagV1TrNfBGBN"  /* Put here your Ubidots TOKEN */
#define ID_1 "5b006c77c03f97528bbaf5fb" /* Put your variable ID here */
#define WIFISSID "Luzsanti" /* Put here your Wi-Fi SSID */
#define PASSWORD "$F4M1L14c4Rv$" /* Put here your Wi-Fi password */

//Bad practice constants
const byte tiltBp = 1;
const byte proximityBp = 2;
const byte speedBp = 3;
const byte zigZagBp = 4;
const byte helmetBp = 5;
//Sensors pins
const int tiltSensor = 2;
const int ultraSonicSenorEcho1 = 6;
const int ultraSonicSenorEcho2 = 8;
const int ultraSonicSenorTrigger1 = 5;
const int ultraSonicSenorTrigger2 = 7;
//Umbral variables
const int distanceUmbral = 6; // cm
const int zigZagUmbral = 15;
const int speedUmbral = 18;
//Time
unsigned long time;
unsigned long waitTime = 3000;
unsigned long lastTiltOcurrence = -waitTime ;
unsigned long lastProximityOcurrence = -waitTime ;
unsigned long lastZigZagOcurrence = - waitTime;
unsigned long lastSpeedOcurrence = - waitTime;
//Link to NewPing tutorial https://bitbucket.org/teckel12/arduino-new-ping/wiki/Home#!event-timer-sketch
int maxDistance = 200;
NewPing sonar(ultraSonicSenorTrigger1, ultraSonicSenorEcho1, maxDistance);
NewPing sonar2(ultraSonicSenorTrigger2, ultraSonicSenorEcho2, maxDistance);
unsigned int pingSpeed = 50; // How frequently are we going to send out a ping (in milliseconds). 50ms would be 20 times a second.
unsigned long pingTimer1;
unsigned long pingTimer2;

//GPS variables
TinyGPS gps;
SoftwareSerial softSerial(4, 3);
float lat =  4.638367, lon = -74.084162;
//Acc Variables
ADXL345 adxl = ADXL345();
int accX, accY, accZ;

//wifi

Ubidots client(TOKEN);
bool dataSent = false;

//button or something  can be toggled
const int buttonPin = 3;


struct badPractice
{
  byte bp;
  float value;
  float latitude;
  float longitude;
};
const byte bpLength = 10;
badPractice bpReport[bpLength];
byte bpCounter = 0;

void checkTilt(){
  if(time > lastTiltOcurrence + waitTime ){
    if(digitalRead(tiltSensor)){
      readGps();
      lastTiltOcurrence = time;
      bpReport[bpCounter].bp = tiltBp;
      bpReport[bpCounter].value = 45;
      bpReport[bpCounter].latitude = lat;
      bpReport[bpCounter].longitude = lon;
      bpCounter++;
    }
  }
}

void checkZigZag(){

  if(time > lastZigZagOcurrence + waitTime ){
    adxl.readAccel(&accX, &accY, &accZ);
    if(accY > zigZagUmbral){
      readGps();
      lastZigZagOcurrence = time;
      bpReport[bpCounter].bp = zigZagBp;
      bpReport[bpCounter].value = accY;
      bpReport[bpCounter].latitude = lat;
      bpReport[bpCounter].longitude = lon;
      bpCounter++;
    }
  }
}

void checkSpeed(){

  if(time > lastSpeedOcurrence + waitTime ){
    adxl.readAccel(&accX, &accY, &accZ);
    if(accX > speedUmbral){
      readGps();
      lastSpeedOcurrence = time;
      bpReport[bpCounter].bp = speedBp;
      bpReport[bpCounter].value = accX;
      bpReport[bpCounter].latitude = lat;
      bpReport[bpCounter].longitude = lon;
      bpCounter++;
    }
  }
}


void checkProximity(){
  if(time > lastProximityOcurrence + waitTime ){
      if (millis() >= pingTimer1) {
        pingTimer1 += pingSpeed;
        sonar.ping_timer(echoCheck);
      }
      if (millis() >= pingTimer2) {
        pingTimer2 += pingSpeed;
        sonar2.ping_timer(echoCheck2);
      }

    }
}

void echoCheck() {
  if (sonar.check_timer()) {
    if((sonar.ping_result / US_ROUNDTRIP_CM) < distanceUmbral){
      readGps();
      lastProximityOcurrence = time;
      bpReport[bpCounter].bp = proximityBp;
      bpReport[bpCounter].value = sonar.ping_result / US_ROUNDTRIP_CM;
      bpReport[bpCounter].latitude = lat;
      bpReport[bpCounter].longitude = lon;
      bpCounter++;
    }

  }
  // Don't do anything here!
}

void echoCheck2() {
  if (sonar2.check_timer()) {
    if((sonar2.ping_result / US_ROUNDTRIP_CM) < distanceUmbral){
      readGps();
      lastProximityOcurrence = time;
      bpReport[bpCounter].bp = proximityBp;
      bpReport[bpCounter].value = sonar2.ping_result / US_ROUNDTRIP_CM;
      bpReport[bpCounter].latitude = lat;
      bpReport[bpCounter].longitude = lon;
      bpCounter++;
    }

  }
  // Don't do anything here!
}

void printBpReport(){
  Serial.println("Yes");
 // if(time % 2000 == 0){
    for(int i = 0; i < bpCounter; i++){
      Serial.print(i);
      Serial.print(" ");
      Serial.print("MALA PRACTICA!! ");
      Serial.print(bpReport[i].bp );
      Serial.print(" ");

      Serial.print(bpReport[i].value);
      Serial.print(" ");
      Serial.print(bpReport[i].latitude, 6);
      Serial.print(" ");
      Serial.print(bpReport[i].longitude, 6);
      Serial.println();
   }
   Serial.println();
   Serial.println();
   //delay(2000);
  //}
}


void readGps(){
   bool newData = false;
   unsigned long chars;
   unsigned short sentences, failed;

   // Intentar recibir secuencia durante un segundo

    while (softSerial.available())
    {

       char c = softSerial.read();
       //Serial.println(c);
       if (gps.encode(c)){ // Nueva secuencia recibida
          newData = true;
        }
    }

   if (newData)
   {A1E-MEo1jXWG2exAIbnLHPagV1TrNfBGBN
      gps.f_get_position(&lat, &lon);

   }
}


void setup() {
  Serial.begin(4800);
  pinMode(tiltSensor, INPUT);
  pinMode(ultraSonicSenorEcho1, INPUT);
  pinMode(ultraSonicSenorEcho2, INPUT);
  pinMode(ultraSonicSenorTrigger1, OUTPUT);
  pinMode(ultraSonicSenorTrigger2, OUTPUT);
  pinMode(buttonPin, INPUT);
  pingTimer1 = millis();
  pingTimer2 = millis();
  softSerial.begin(19200);
  adxl.powerOn();            
  adxl.setRangeSetting(8);


  // put your setup code here, to run once:

}

void loop() {

  // read the state of the pushbutton value:
  buttonState = digitalRead(buttonPin);


  // check if the pushbutton is pressed.
  // if it is, the buttonState is HIGH:
  if (buttonState == HIGH) {
    //wifi
    Serial.print("Conectando a red Wi-Fi " + WIFISSID);
    client.wifiConnection(WIFISSID, PASSWORD);
    delay(10000);
    Serial.print("Red conectada: " + WIFISSID);
    for(int i = 0; i < bpCounter; i++){
      char context[25];
      sprintf(context, "lat=%f$lng=%f$sensed=%f", bpReport[i].latitude, bpReport[i].longitude, bpReport[i].value);
      client.add(ID_1, bpReport[i].bp, context);
      client.sendAll(false);
    }

  }
  else {
    //Serial.print()
    if(datasent){
      Serial.print("Datos enviados, ejecución terminada");
      delay(60000);
    }else {
      if(bpCounter < bpLength){
        time = millis();
        checkTilt();
        checkProximity();
        checkZigZag();
        checkSpeed();
        Serial.println(bpCounter);
    
        }else{
          printBpReport();
        }
  //reset();
  //delay(500);
}
    }
  }
}
