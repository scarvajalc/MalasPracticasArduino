//Bad practice constants
const byte tiltBp = 1;
const byte proximityBp = 2;
const byte speedBp = 3;
const byte zigzagBp = 4;
const byte helmetBp = 5;
//Sensors pins
const int tiltSensor = 2;
const int ultraSonicSenorEcho1 = 6;
const int ultraSonicSenorTrigger1 = 5;
//Time
unsigned long time;
unsigned long waitTime = 3000;
unsigned long lastTiltOcurrence = -waitTime ;
//wifi
String ssid ="yourSSID";
String password="yourPassword";
SoftwareSerial esp(6, 7);// RX, TX
String data;
string apiToken;
string authToken;
String server = "things.ubidots.com";
String uri = "/api/v1.6/variables/5b006c77c03f97528bbaf5fb/values";

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

void setup() {
  Serial.begin(9600);
  pinMode(tiltSensor, INPUT);
  pinMode(ultraSonicSenorEcho1, INPUT);
  pinMode(ultraSonicSenorTrigger1, OUTPUT);

  //wifi
  esp.begin(9600);
  reset();
  connectWifi();
  // put your setup code here, to run once:

}

void checkTilt(){
  if(time > lastTiltOcurrence + waitTime ){
    if(digitalRead(tiltSensor)){
      lastTiltOcurrence = time;
      bpReport[bpCounter].bp = tiltBp;
      bpReport[bpCounter].value = 45;
      bpReport[bpCounter].latitude = 20.5;
      bpReport[bpCounter].longitude = 40.56;
      bpCounter++;
    }
  }
}

void printBpReport(){
  if(time % 2000 == 0){
    for(int i = 0; i < bpCounter; i++){
      Serial.print(bpReport[i].bp );
      Serial.print(" ");
      Serial.print(i);
      Serial.print(" ");
      Serial.print(bpReport[i].value);
      Serial.print(" ");
      Serial.print(bpReport[i].latitude);
      Serial.print(" ");
      Serial.print(bpReport[i].longitude);
      Serial.println();
   }
   //delay(2000);
  }
}

//WiFi Functions ===================================================================

//reset the esp8266 module
void reset() {
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


void loop() {
  //Serial.print()
  if(bpCounter < bpLength){
    time = millis();
    checkTilt();
    //printBpReport();
  }


}
