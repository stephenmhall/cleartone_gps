#include <Arduino.h>
#include <SoftwareSerial.h>

SoftwareSerial mySerial(D5, D6); // RX, TX
SoftwareSerial radio(D7, D8);

const float ktomph = 1.15077944802354;
const String target_issi = "6698705";

String GPRMC = "";
String GPGGA = "";
unsigned long updateTime;
int sendUpdate = 10000;// 60 seconds
String Hexstring0, Hexstring1;
int Hexstringlength;

//Split string on seperator
String getValue(String data, char separator, int index)
{
  int found = 0;
  int strIndex[] = {0, -1};
  int maxIndex = data.length()-1;

  for(int i=0; i<=maxIndex && found<=index; i++){
    if(data.charAt(i)==separator || i==maxIndex){
        found++;
        strIndex[0] = strIndex[1]+1;
        strIndex[1] = (i == maxIndex) ? i+1 : i;
    }
  }
  return found>index ? data.substring(strIndex[0], strIndex[1]) : "";
}

// Convert Text to Hex Routine
void convertstring(String hex){
  Hexstring0  = String(hex);
  Hexstring1  = String();
  Hexstringlength = (Hexstring0.length() + 1);
for(int x = 0; x < Hexstringlength; x++){
    Hexstring1 = Hexstring1 + (String(Hexstring0.charAt(x),HEX));
    }
  }

void Send_GPS(){
  convertstring(GPRMC);
  radio.print("AT+CTSD=12,0,0,0,0,0,0,0,0,0\n\r");
  Serial.print("AT+CTSD=12,0,0,0,0,0,0,0,0,0\n\r");
  radio.print("AT+CMGS=");//6789946
  radio.print(target_issi);
  radio.print(",");
  Serial.print("AT+CMGS=");//6789946
  Serial.print(target_issi);
  Serial.print(",");
  Hexstring1 = "82040101" + Hexstring1;
  Hexstringlength = (Hexstringlength * 8 + 32);
  radio.print(Hexstringlength);
  Serial.print(Hexstringlength);
  radio.print("\n\r");
  Serial.print("\n\r");
  radio.print(Hexstring1);
  Serial.println(Hexstring1);
  Hexstring0 = "";
  Hexstring1 = "";
}

void update(String str){
  Serial.print("Satelites: ");
  Serial.println(getValue(GPGGA, ',', 7));
  Serial.print("HDOP :");
  Serial.print(getValue(GPGGA, ',', 8));
  Serial.println("M");
  Serial.println(str);
  Serial.print("Speed : ");
  Serial.print(getValue(str, ',', 7).toFloat() * ktomph);
  Serial.println(" mph");
  Serial.print("updating @");
  Serial.print(sendUpdate);
  Serial.println(" Seconds");
  String valid = getValue(str, ',', 2);
  if(valid == "A"){
    Send_GPS();
  }
}

void speedUpdate(){
  float speed = getValue(GPRMC, ',', 7).toFloat() * ktomph;

  if(speed < 10.0){
    sendUpdate = 60000;
  }else if(speed > 10 && speed < 30) {
    sendUpdate = 30000;
  }else if(speed >= 30 && speed < 60){
    sendUpdate = 20000;
  }else{
    sendUpdate = 10000;
  }
}

void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(115200);
  // set the data rate for the SoftwareSerial port
  mySerial.begin(9600);
  radio.begin (9600);
  delay(500);
  // always use this to "mount" the filesystem

  Serial.println("\n");
  Serial.println("GPS Engine starting");

  //Turn off most messages
  mySerial.println("$PUBX,40,GLL,0,0,0,0*5C");
  //mySerial.println("$PUBX,40,GLL,0,1,0,0*5D"); //on
  mySerial.println("$PUBX,40,GSV,0,0,0,0*59");
  //mySerial.println("$PUBX,40,GSV,1,0,0,0*58"); //on
  mySerial.println("$PUBX,40,GSA,0,0,0,0*4E");
  mySerial.println("$PUBX,40,VTG,0,0,0,0*5E");
}

void loop() { // run over and over
  unsigned long loopTime = millis();
  String content = "";
  char character;

  if(mySerial.available()){
    content = mySerial.readStringUntil('\n');
  }

  if (content != "") {
    //Serial.println(content);
    if(content.startsWith("RMC", 3)) {
      GPRMC = content;
      //Serial.println(GPRMC);
    }
    if(content.startsWith("GGA", 3)) {
      GPGGA = content;
      //Serial.println(GPGGA);
    }
  }

  speedUpdate();

  if(loopTime - updateTime > sendUpdate){
    update(GPRMC);
    updateTime = loopTime;
  }

  if (Serial.available()) {
    mySerial.write(Serial.read());
  }
}
