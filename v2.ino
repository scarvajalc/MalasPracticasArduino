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


void loop() {
  //Serial.print()
  if(bpCounter < bpLength){
    time = millis();
    checkTilt();
    //printBpReport();
  }

  
}
