#include <Adafruit_NeoPixel.h>
#include <ESP8266WiFi.h>
#include <MFRC522.h>
// #include <WiFi.h>
byte mac[6];

#define RST_PIN 5   //RFID Reset Pin
#define SDA_PIN  15  //RFID SDA Pin
MFRC522 mfrc522(SDA_PIN, RST_PIN);

#define LED_PIN 2 //LEDs pin
#define NUMPIXELS 32
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, LED_PIN, NEO_GRB + NEO_KHZ800);

#define RELAY_PIN  0   //Relay Signal Pin

const char* ssid     = "NETGEAR";
const char* password = "";
String macString  = "";

void connect();
WiFiClient client;

void setup() {
  Serial.begin(115200);    // Initialize serial communications
  while (!Serial);
  SPI.begin();
  pinMode(LED_PIN, OUTPUT);
  pinMode(RELAY_PIN, OUTPUT);
  mfrc522.PCD_Init();
  mfrc522.PCD_DumpVersionToSerial();
  
  //pixels.begin();
  //pixels.show();
  
  WiFi.begin(ssid, password);
  int j=0;
  while (WiFi.status() != WL_CONNECTED) {
    theaterChase(pixels.Color(0, 0, 127), 50); // Blue
    delay(500);
    Serial.print(".");
    pixels.setPixelColor(j, pixels.Color(0,150,0)); // Moderately bright green color.
    pixels.show();
    j++;
    delay(250);
  }

  WiFi.macAddress(mac);
  Serial.print("MAC: ");
  
  macString.concat(String(mac[5],HEX));
  macString.concat(":");
  macString.concat(String(mac[4],HEX));
  macString.concat(":");
  macString.concat(String(mac[3],HEX));
  macString.concat(":");
  macString.concat(String(mac[2],HEX));
  macString.concat(":");
  macString.concat(String(mac[1],HEX));
  macString.concat(":");
  macString.concat(String(mac[0],HEX));
  macString.toUpperCase();
  Serial.print(macString);
  
  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
/*  
  for(int i=0;i<8;i++){
    pixels.setPixelColor(i, pixels.Color(0,150,0)); // Moderately bright green color.
    pixels.show();
    delay(250);
  }
*/
  delay(2000);
  for(int i=0;i<8;i++){
    pixels.setPixelColor(i, pixels.Color(0,0,0)); // Moderately bright green color.
    pixels.show();
  }  
}
bool occoupied = 0;
int noCardCount = 7;
const int NO_CARD_DETECTIONS_BEFORE_NEW_READ = 2;

void loop() { 
  Serial.println(noCardCount);
  if (mfrc522.PICC_IsNewCardPresent()) {
    if(noCardCount > NO_CARD_DETECTIONS_BEFORE_NEW_READ){
      /*
       * Do card look up here
       */

       
      Serial.println("Card present!");
      mfrc522.PICC_ReadCardSerial();
      Serial.print("UID tag :");
      String content= "";
      byte letter;
      for (byte i = 0; i < mfrc522.uid.size; i++) 
      {
         // Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
         // Serial.print(mfrc522.uid.uidByte[i], HEX);
         Serial.print(mfrc522.uid.uidByte[i], DEC);
         // content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
         // content.concat(String(mfrc522.uid.uidByte[i], HEX));
         content.concat(String(mfrc522.uid.uidByte[i], DEC));
      }
      Serial.println();
      Serial.print("MAC: ");
      Serial.println(macString); 

      for(int i=0;i<32;i++){
        pixels.setPixelColor(i, pixels.Color(100,100,0)); // Moderately bright yellow color.
        pixels.show();
      }
      
      relayON();
    }
    noCardCount = 0;
  }else{ // not present
    noCardCount++;
    
    if (noCardCount >2){
      relayOFF();
    }
    pixels.setPixelColor(noCardCount, pixels.Color(150,150,150));
    pixels.setPixelColor(noCardCount-1, pixels.Color(0,0,0));
    pixels.show();
     
  }

/*
  if (noCardCount > 1){
    for(int i=0;i<32;i++){
        pixels.setPixelColor(i, pixels.Color(0,0,0));
        pixels.show();
    }
*/    
    if (noCardCount > 32){
      noCardCount = 7;
    }
  
/*  
  if ( mfrc522.PICC_IsNewCardPresent()) {
    Serial.print("X");
    for(int i=0;i<65;i++){
        pixels.setPixelColor(i, pixels.Color(255,255,0)); // Moderately bright yellow color.
        pixels.show();
        occoupied = 1;
      }
  } else {
    theaterChase(pixels.Color(0, 0, 127), 50); // Blue
    occoupied = 0;
    return;
  }

  if ( mfrc522.PICC_ReadCardSerial()) {
    
  } else{
    return;
  }
    
  //Show UID on serial monitor
  Serial.print("UID tag :");
  String content= "";
  byte letter;
  for (byte i = 0; i < mfrc522.uid.size; i++) 
  {
     //Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
     //Serial.print(mfrc522.uid.uidByte[i], DEC);
     content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
     content.concat(String(mfrc522.uid.uidByte[i], DEC));
  }
  content.toUpperCase();
  Serial.println(content);
  Serial.print("Message : ");
  

  if (content.substring(1) == "72 1C 42 F2" && !occoupied) //change here the UID of the card/cards that you want to give access
  {
    Serial.println("Authorized access");
    Serial.println();
    for(int i=0;i<65;i++){
      pixels.setPixelColor(i, pixels.Color(0,150,0)); // Moderately bright green color.
      pixels.show();
    }
    delay(3000);
    for(int i=0;i<65;i++){
      pixels.setPixelColor(i, pixels.Color(0,0,0)); // Moderately bright green color.
      pixels.show();
    }
  }
 
 else   {
    Serial.println(" Access denied");
    for(int i=0;i<65;i++){
      pixels.setPixelColor(i, pixels.Color(150,0,0)); // Moderately bright green color.
      pixels.show();
    }
    delay(3000);
    for(int i=0;i<65;i++){
      pixels.setPixelColor(i, pixels.Color(0,0,0)); // Moderately bright green color.
      pixels.show();
    }
  }
*/  
}

void relayON(){
  digitalWrite(RELAY_PIN,1);
}

void relayOFF(){
  digitalWrite(RELAY_PIN,0);
}

// Helper routine to dump a byte array as hex values to Serial
void dump_byte_array(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], DEC);
  }
}

//Theatre-style crawling lights.
void theaterChase(uint32_t c, uint8_t wait) {
  for (int j=0; j<10; j++) {  //do 10 cycles of chasing
    for (int q=0; q < 3; q++) {
      for (uint16_t i=0; i < pixels.numPixels(); i=i+3) {
        pixels.setPixelColor(i+q, c);    //turn every third pixel on
      }
      pixels.show();

      delay(wait);

      for (uint16_t i=0; i < pixels.numPixels(); i=i+3) {
        pixels.setPixelColor(i+q, 0);        //turn every third pixel off
      }
    }
  }
}

