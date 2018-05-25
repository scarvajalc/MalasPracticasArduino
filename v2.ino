#include <TinyGPS.h>
#include <NewPing.h>
#include <SoftwareSerial.h>
#include <SparkFun_ADXL345.h> 


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
const int distanceUmbral = 50; // cm
const int zigZagUmbral = 2;
const int speedUmbral = 3;
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
String ssid ="Luzsanti";
String password="$F4M1L14c4Rv$";
SoftwareSerial esp(10, 11);// RX, TX
String data;
String apiToken;
String authToken;
String server = "things.ubidots.com";
String uri = "/api/v1.6/variables/5b006c77c03f97528bbaf5fb/values";

struct badPractice
{
  byte bp;
  float value;
  float latitude;
  float longitude;
};
const byte bpLength = 20;
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
  if(time % 2000 == 0){
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
  }
}

//WiFi Functions ===================================================================

//reset the esp8266 module
void reset() {
   Serial.println("ALGOOOO");
  esp.println("AT+RST");
  delay(1000);
  if(esp.find("OK") ) Serial.println("Module Reset");
}

//connect to your wifi network
void connectWifi() {
  String cmd = "AT+CWJAP=\"" +ssid+"\",\"" + password + "\"";
  esp.println(cmd);
  delay(4000);
  if(esp.find("OK")) {
    Serial.println("Connected!");
  }
  else {
    connectWifi();
    Serial.println("Cannot connect to wifi");
  }
}

//send POST request
void httppost () {
  esp.println("AT+CIPSTART=\"TCP\",\"" + server + "\",80");//start a TCP connection.
  if( esp.find("OK"))
    Serial.println("TCP connection ready");
  delay(1000);
  String postRequest = "POST " + uri + " HTTP/1.1\r\n" +
                       "Host: " + server + "\r\n" +
                       "X-Auth-Token: " + authToken +"\r\n" +
                       "Accept: application/json\r\n" +
                       "Content-Type: application/json\r\n" +
                       "Cache-Control: no-cache\r\n"
                       "\r\n" + data;
  String sendCmd = "AT+CIPSEND=";//determine the number of caracters to be sent.
  esp.print(sendCmd);
  esp.println(postRequest.length() );
  delay(500);
  if(esp.find(">")) {
    Serial.println("Sending..");
    esp.print(postRequest);
    if( esp.find("SEND OK")) {
      Serial.println("Packet sent");
      while (esp.available()) {
        String tmpResp = esp.readString();
        Serial.println(tmpResp);
      }
      // close the connection
      esp.println("AT+CIPCLOSE");
    }
  }
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
   {
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
  pingTimer1 = millis();
  pingTimer2 = millis();
  softSerial.begin(19200);
  adxl.powerOn();            
  adxl.setRangeSetting(8);
  //wifi
  esp.begin(9600);
  //reset();
  //connectWifi();
  // put your setup code here, to run once:
  
}

void loop() {
  //Serial.print()
 /* if(bpCounter < bpLength){
    time = millis();
    checkTilt();
    checkProximity();
    checkZigZag();
    checkSpeed();
    printBpReport();
  }*/
  reset();
  delay(500);
}


