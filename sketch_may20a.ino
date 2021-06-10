#include <SPI.h>
#include <SD.h>
#include <MFRC522.h>
#include <ThreeWire.h>  
#include <RtcDS1302.h>

ThreeWire myWire(7,8,6); // IO, SCLK, CE
RtcDS1302<ThreeWire> Rtc(myWire);

#define SS_PIN 53
#define RST_PIN 5

const int chipSelect = 3;
MFRC522 rfid(SS_PIN, RST_PIN); 
MFRC522::MIFARE_Key key; 
byte nuidPICC[4];
int speak = 9;
int melody = 440;

void setup() { 
  Serial.begin(9600);
  SPI.begin(); // Init SPI bus
  rfid.PCD_Init(); // Init MFRC522
  pinMode(speak,OUTPUT);

  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }
 
  printHex(key.keyByte, MFRC522::MF_KEY_SIZE);

  if(!SD.begin(chipSelect)){
    Serial.println("Card failed, or not present");
    while(1);
  }
  Serial.println("card initialized.");
}
 
void loop() {
  if ( ! rfid.PICC_IsNewCardPresent())
    return;
  if ( ! rfid.PICC_ReadCardSerial())
    return;
  MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);

  if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI &&  
    piccType != MFRC522::PICC_TYPE_MIFARE_1K &&
    piccType != MFRC522::PICC_TYPE_MIFARE_4K) {
    Serial.println(F("Your tag is not of type MIFARE Classic."));
    return;
  }

  if (rfid.uid.uidByte[0] != nuidPICC[0] || 
    rfid.uid.uidByte[1] != nuidPICC[1] || 
    rfid.uid.uidByte[2] != nuidPICC[2] || 
    rfid.uid.uidByte[3] != nuidPICC[3] ) {

    for (byte i = 0; i < 4; i++) {
      nuidPICC[i] = rfid.uid.uidByte[i];
    }
    tone(speak, melody, 100);
    
    Serial.print(F("In hex: "));
    printHex(rfid.uid.uidByte, rfid.uid.size);
    Serial.println();
  }
  else
  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
}

#define countof(a) (sizeof(a) / sizeof(a[0]))

void printHex(byte *buffer, byte bufferSize) {
  File dataFile = SD.open("data.txt",FILE_WRITE);

  if(dataFile){
  char datestring[20];
  RtcDateTime now = Rtc.GetDateTime();
  snprintf_P(datestring, 
    countof(datestring),
    PSTR("%04u/%02u/%02u %02u:%02u:%02u"),
    now.Year(),
    now.Month(),
    now.Day(),
    now.Hour(),
    now.Minute(),
    now.Second() );
    dataFile.print(datestring);
    dataFile.print(" : ");
    
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);

    dataFile.print(buffer[i] < 0x10 ? " 0" : " ");
    dataFile.print(buffer[i], HEX);
    }
    dataFile.println();
   } 
   else{
   }
   dataFile.close();
}

void printDec(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], DEC);
  }
}
