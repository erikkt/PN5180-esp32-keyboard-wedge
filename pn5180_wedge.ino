
//Library used: https://github.com/tueddy/PN5180-Library

//
// ESP-32    <--> PN5180 pin mapping:
// 3.3V      <--> 3.3V
// 5V      <--> 5V
// GND       <--> GND
// SCLK, 18   --> SCLK
// MISO, 19  <--  MISO
// MOSI, 23   --> MOSI
// SS, 2     --> NSS (=Not SS -> active LOW)
// BUSY, 5   <--  BUSY
// Reset, 4  --> RST
// button = 27; 
// buzzer = 25; 
// redLed = 22;
// yellowLed = 21;
// greenLed = 17;
// buttonLed = 16;

#include <PN5180.h>
#include <PN5180ISO14443.h>
#include <BleKeyboard.h>


BleKeyboard bleKeyboard("ESP32 Wedge");

#define PN5180_NSS  2 
#define PN5180_BUSY 5 
#define PN5180_RST  4 


PN5180ISO14443 nfc(PN5180_NSS, PN5180_BUSY, PN5180_RST);

const int button = 27; //buzzer pin
const int buzzer = 25; //button pin 
const int redLed = 22;
const int yellowLed = 21;
const int greenLed = 17;
const int buttonLed = 16;
int loopCounter = 0;

char password[20];
// Password 1 header: 1XYZ, password 2 header: 2XYZ, and so on.
uint8_t header[] = "1XYZ2XYZ3XYZ";
uint8_t terminator = '$';
int headerIndex;
int passNo = 1;

void setup() {
  pinMode(button, INPUT_PULLUP); 
  pinMode(redLed, OUTPUT);
  pinMode(yellowLed, OUTPUT);
  pinMode(greenLed, OUTPUT);
  pinMode(buttonLed, OUTPUT);
  ledcSetup(0,1E5,12);
  ledcAttachPin(buzzer,0);
  Serial.begin(115200);
  Serial.println(F("=================================="));
  Serial.println(F("Uploaded: " __DATE__ " " __TIME__));
  Serial.println(F("PN5180 ISO14443 Keyboard Wedge"));
  bleKeyboard.begin();
  rapidBlink();
  nfc.begin();
  delay(100);
  Serial.println(F("----------------------------------"));
  Serial.println(F("PN5180 Hard-Reset..."));
  nfc.reset();
  delay(100);
  Serial.println(F("----------------------------------"));
  
  uint8_t productVersion[2];
  nfc.readEEprom(PRODUCT_VERSION, productVersion, sizeof(productVersion));
  Serial.print(F("Product version="));
  Serial.print(productVersion[1]);
  Serial.print(".");
  Serial.println(productVersion[0]);
  delay(100);
  if (0xff == productVersion[1]) { // if product version 255, the initialization failed
    Serial.println(F("Initialization failed!?"));
    Serial.println(F("Press reset to restart..."));
    Serial.flush();
    exit(-1); // halt
  }
  delay(100);
  //Serial.println(F("----------------------------------"));
  
  uint8_t firmwareVersion[2];
  nfc.readEEprom(FIRMWARE_VERSION, firmwareVersion, sizeof(firmwareVersion));
  Serial.print(F("Firmware version="));
  Serial.print(firmwareVersion[1]);
  Serial.print(".");
  Serial.println(firmwareVersion[0]);
  delay(100);
  //Serial.println(F("----------------------------------"));
  
  uint8_t eepromVersion[2];
  nfc.readEEprom(EEPROM_VERSION, eepromVersion, sizeof(eepromVersion));
  Serial.print(F("EEPROM version="));
  Serial.print(eepromVersion[1]);
  Serial.print(".");
  Serial.println(eepromVersion[0]);
  delay(100);
  Serial.println(F("----------------------------------"));
  Serial.println(F("Enable RF field..."));
  nfc.setupRF();
  delay(100);
}
void buttonLedBlink(){
  for (int u=0; u<8; u++){
    digitalWrite(buttonLed, HIGH);
    delay(80);
    digitalWrite(buttonLed, LOW);
    delay(80);
  }
}
void rapidBlink(){
  digitalWrite(greenLed, LOW);
  digitalWrite(yellowLed, LOW);
  digitalWrite(redLed, LOW);
  for (int j=0; j<4; j++){
    digitalWrite(greenLed, HIGH);
    delay(80);
    digitalWrite(greenLed, LOW);
    digitalWrite(yellowLed, HIGH);
    delay(80);
    digitalWrite(yellowLed, LOW);
    digitalWrite(redLed, HIGH);
    delay(80);
    digitalWrite(redLed, LOW);
    digitalWrite(yellowLed, HIGH);
    delay(80);
    digitalWrite(yellowLed, LOW);
    
    
    
  }
  digitalWrite(greenLed, HIGH);
  delay(80);
  digitalWrite(greenLed, LOW);
    
  
}

void beep(){
  ledcWriteTone(0,4000);
  delay(200);
  ledcWriteTone(0,0);
}

void multiBeep(int times){
  for (int m=0; m<times; m++){
    ledcWriteTone(0,5000);
    delay(100);
    ledcWriteTone(0,0);
    delay(200);
  }
}

void passwordFound(){
  rapidBlink();
  Serial.println("---------------------");
  Serial.println("The password is: ");
  for (int q=0; q<20; q++){
    Serial.print(password[q]);
    bleKeyboard.write(password[q]);
    delay(10);
  }
  bleKeyboard.write(KEY_RETURN);
  delay(10);
  Serial.println();
  Serial.println("---------------------");
  delay(1000);  
  return;
}
void frontLight(int frontColor){
  switch (frontColor) {
    case 1:
      digitalWrite(greenLed, HIGH);
      digitalWrite(yellowLed, LOW);
      digitalWrite(redLed, LOW);
      break;
    case 2:
      digitalWrite(greenLed, LOW);
      digitalWrite(yellowLed, HIGH);
      digitalWrite(redLed, LOW);
      break;
    case 3:      
      digitalWrite(greenLed, LOW);
      digitalWrite(yellowLed, LOW);
      digitalWrite(redLed, HIGH);
      break;
     }
}

void loop() {
  frontLight(passNo);
  if(bleKeyboard.isConnected()) {
    digitalWrite(buttonLed, HIGH);
  }
  else 
  {
    digitalWrite(buttonLed, LOW); 
  }
  int pageStart = 5;
  int pageEnd = 50;
  int count = 0;
  int bufferSize = 16;
  bool headerFound = false;
  uint8_t buffer[bufferSize];
  int btnPress = digitalRead(button);
  
  if (!btnPress){   
    passNo++;
    if (passNo == 4){
      passNo = 1;
    }
    Serial.print("Selected password number ");
    Serial.println(passNo);
    switch (passNo) {
      case 1:
        headerIndex = 0;
        frontLight(passNo);
        multiBeep(passNo);
        //buttonLedBlink();
        break;
      case 2:
        headerIndex = 4;
        frontLight(passNo);
        multiBeep(passNo);
        //buttonLedBlink();
        break;
      case 3:
        headerIndex = 8;
        frontLight(passNo);
        multiBeep(passNo);
        //buttonLedBlink();
        break;
     
     }
  }
  
  if (!nfc.isCardPresent()) {
    return;
  }
         
  for (int b = pageStart; b < pageEnd ; b+=4) {
    //Serial.println();
    
    if (!nfc.mifareBlockRead(b, buffer)){
      return;
    }
    for (int n=0; n<bufferSize; n++) {
       //Serial.write(buffer[n]);
       
       if (buffer[n] == header[headerIndex] && buffer[n+1] == header[headerIndex + 1] && buffer[n+2] == header[headerIndex + 2] && buffer[n+3] == header[headerIndex + 3]){
         headerFound = true; 
         n+=4;
         Serial.println();
         Serial.println("Header found!");
       }
       else if (headerFound && buffer[n] == terminator){
         Serial.println();
         Serial.println("Terminator found!");
         headerFound = false;
         beep();
         passwordFound();
       }
       if (headerFound){
        password[count] = buffer[n];
        count++;
       }
    }
     
  }
  
  
}


