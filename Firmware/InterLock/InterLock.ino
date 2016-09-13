#include <Adafruit_NeoPixel.h>
#include <ESP8266WiFi.h>
#include <MFRC522.h>
#include <ArduinoJson.h>

byte mac[6];

#define RST_PIN 5   //RFID Reset Pin
#define SDA_PIN  15  //RFID SDA Pin
MFRC522 mfrc522(SDA_PIN, RST_PIN);

#define LED_PIN 0 //LEDs pin
#define NUMPIXELS 32
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, LED_PIN, NEO_GRB + NEO_KHZ800);



#define RELAY_PIN  2   //Relay Signal Pin

const char* server = "www.strangeproject.com"; 
const char* cookieResource = "/api/card/generate_auth_cookie/";
const char* machineResource = "/api/card/machine_access/";

const char* ssid     = "NETGEAR";
const char* password = "";
String macString  = "";
String macStringNoColons  = "";
const unsigned long HTTP_TIMEOUT = 10000;  // max respone time from server
const size_t MAX_CONTENT_SIZE = 512;       // max size of the HTTP response

struct CookieData {
  char returnStatus[2];
  char cookie[150];
};

struct MachineData {
  char returnStatus[2];
  char machineAccess[1];
};

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
  
  pixels.begin();
  pixels.show();
  
  WiFi.begin(ssid, password);
  int j=0;
  
  while (WiFi.status() != WL_CONNECTED) {
    theaterChase(pixels.Color(0, 0, 127), 50); // Blue
    Serial.print(".");
    pixels.setPixelColor(j, pixels.Color(0,150,0)); // Moderately bright green color.
    pixels.show();
    j++;
  }

  WiFi.macAddress(mac);
  Serial.print("MAC: ");
  
  macString.concat(String(mac[5],HEX));
  macStringNoColons.concat(String(mac[5],HEX));
  macString.concat(":");
  macString.concat(String(mac[4],HEX));
  macStringNoColons.concat(String(mac[4],HEX));
  macString.concat(":");
  macString.concat(String(mac[3],HEX));
  macStringNoColons.concat(String(mac[3],HEX));
  macString.concat(":");
  macString.concat(String(mac[2],HEX));
  macStringNoColons.concat(String(mac[2],HEX));
  macString.concat(":");
  macString.concat(String(mac[1],HEX));
  macStringNoColons.concat(String(mac[1],HEX));
  macString.concat(":");
  macString.concat(String(mac[0],HEX));
  macStringNoColons.concat(String(mac[0],HEX));
  macString.toUpperCase();
  macStringNoColons.toUpperCase();
  Serial.print(macString);
  
  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  String cookieResource = "/api/card/generate_auth_cookie/";
  cookieResource += "?insecure=cool";
  cookieResource += "&username=";
  cookieResource +=  macStringNoColons;
  cookieResource += "&password=";
  cookieResource += macString; 
  
  if (connect(server)) {
    if (sendRequest(server, cookieResource) && skipResponseHeaders()) {
      char response[MAX_CONTENT_SIZE];
      readReponseContent(response, sizeof(response));

      CookieData cookieData;
      if (parseCookieData(response, &cookieData)) {
        printCookieData(&cookieData);
      }
    }
    disconnect();
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
      
      
      for(int i=0;i<32;i++){
        pixels.setPixelColor(i, pixels.Color(0,0,0)); // Bright off color.
        pixels.show();
      } 
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

      // build string to send
      String machineResource = "/api/card/machine_access/";
      machineResource += "?insecure=cool";
      machineResource += "&machine_number=";
      machineResource +=  macString;
      machineResource += "&card_number=";
      machineResource += content;
  
       Serial.println(machineResource);
       delay(3000);
       
      if (connect(server)) {
        if (sendRequest(server, machineResource) && skipResponseHeaders()) {
          char response[MAX_CONTENT_SIZE];
          readReponseContent(response, sizeof(response));
    
          MachineData machineResource;
          if (parseMachineData(response, &machineResource)) {
            printMachineData(&machineResource);
          }
        }
        disconnect();
      }
      
      if (machineAccess(content)){
        for(int i=0;i<32;i++){
          pixels.setPixelColor(i, pixels.Color(0,255,0)); // Moderately bright green color.
          pixels.show();
        }
        relayON();
      } else {
        for(int i=0;i<32;i++){
          pixels.setPixelColor(i, pixels.Color(255,0,0)); // Moderately bright red color.
          pixels.show();
        }
      }
      
    }
    noCardCount = 0;
  }else{ // not present
    noCardCount++;
    if (noCardCount >2){
        for(int i=0;i<32;i++){
          pixels.setPixelColor(i, pixels.Color(0,0,0)); // Bright off color.
          pixels.show();
        }
      relayOFF();
    }
    pixels.setPixelColor(noCardCount, pixels.Color(51,153,255));
    pixels.setPixelColor(noCardCount-1, pixels.Color(0,0,0));
    pixels.show();
    if (noCardCount > 32){
      noCardCount = 7;
    }
  }    
}

void relayON(){
  digitalWrite(RELAY_PIN,1);
}

void relayOFF(){
  digitalWrite(RELAY_PIN,0);
}

// Skip HTTP headers so that we are at the beginning of the response's body
bool skipResponseHeaders() {
  // HTTP headers end with an empty line
  char endOfHeaders[] = "\r\n\r\n";

  client.setTimeout(HTTP_TIMEOUT);
  bool ok = client.find(endOfHeaders);

  if (!ok) {
    for(int i=0;i<32;i++){
      pixels.setPixelColor(i, pixels.Color(255,0,0)); // Bright red color.
      pixels.show();
    }
    delay(250);
    Serial.println("No response or invalid response!");
  }
  for(int i=0;i<32;i++){
    pixels.setPixelColor(i, pixels.Color(0,255,0)); // Bright green color.
    pixels.show();
  }
  delay(250);
  return ok;
}

// Read the body of the response from the HTTP server
void readReponseContent(char* content, size_t maxSize) {
  size_t length = client.readBytes(content, maxSize);
  content[length] = 0;
  Serial.println(content);
}

bool parseCookieData(char* content, struct CookieData* cookieData) {
  // Compute optimal size of the JSON buffer according to what we need to parse.
  // This is only required if you use StaticJsonBuffer.
  // const size_t BUFFER_SIZE = JSON_OBJECT_SIZE(2);     // the root object has 2 elements

  // Allocate a temporary memory pool on the stack
  // StaticJsonBuffer<BUFFER_SIZE> jsonBuffer;
  // If the memory pool is too big for the stack, use this instead:
  DynamicJsonBuffer jsonBuffer;

  JsonObject& root = jsonBuffer.parseObject(content);

  if (!root.success()) {
    Serial.println("JSON parsing failed!");
    for(int i=0;i<32;i++){
      pixels.setPixelColor(i, pixels.Color(255,0,0)); // Bright red color.
      pixels.show();
    }
    delay(250);
    return false;
  }

  // Here were copy the strings we're interested in
  // strcpy(cookieData->returnStatus, root["status"]);
  strcpy(cookieData->cookie, root["cookie"]);
  // It's not mandatory to make a copy, you could just use the pointers
  // Since, they are pointing inside the "content" buffer, so you need to make
  // sure it's still in memory when you read the string
  for(int i=0;i<32;i++){
    pixels.setPixelColor(i, pixels.Color(0,255,0)); // Bright green color.
    pixels.show();
  }
  delay(250);
  return true;
}
bool parseMachineData(char* content, struct MachineData* machineData) {
  // Compute optimal size of the JSON buffer according to what we need to parse.
  // This is only required if you use StaticJsonBuffer.
  // const size_t BUFFER_SIZE = JSON_OBJECT_SIZE(2);     // the root object has 2 elements

  // Allocate a temporary memory pool on the stack
  // StaticJsonBuffer<BUFFER_SIZE> jsonBuffer;
  // If the memory pool is too big for the stack, use this instead:
  DynamicJsonBuffer jsonBuffer;

  JsonObject& root = jsonBuffer.parseObject(content);

  if (!root.success()) {
    Serial.println("JSON parsing failed!");
    for(int i=0;i<32;i++){
      pixels.setPixelColor(i, pixels.Color(255,0,0)); // Bright red color.
      pixels.show();
    }
    delay(250);
    return false;
  }

  // Here were copy the strings we're interested in
  // strcpy(cookieData->returnStatus, root["status"]);
  strcpy(machineData->machineAccess, root["machineaccess"]);
  // It's not mandatory to make a copy, you could just use the pointers
  // Since, they are pointing inside the "content" buffer, so you need to make
  // sure it's still in memory when you read the string
  for(int i=0;i<32;i++){
    pixels.setPixelColor(i, pixels.Color(0,255,0)); // Bright green color.
    pixels.show();
  }
  delay(250);
  return true;
}

// Send the HTTP GET request to the server
bool sendRequest(const char* host, String resource) {
  
  Serial.print("GET ");
  Serial.println(resource);
  
  client.print("GET ");
  client.print(resource);
  client.println(" HTTP/1.1");
  client.print("Host: ");
  client.println(server);
  client.println("Connection: close");
  client.println();

  return true;
}

// Open connection to the HTTP server
bool connect(const char* hostName) {
  Serial.print("Connect to ");
  Serial.println(hostName);
  for(int i=0;i<32;i++){
    pixels.setPixelColor(i, pixels.Color(204,0,204)); // Moderately bright purple color.
    pixels.show();
  }
  bool ok = client.connect(hostName, 80);

  Serial.println(ok ? "Connected" : "Connection Failed!");
  return ok;
}

// Print the data extracted from the JSON
void printCookieData(const struct CookieData* cookieData) {
  // Serial.print("Status = ");
  // Serial.println(cookieData->returnStatus);
  Serial.print("Cookie = ");
  Serial.println(cookieData->cookie);
}
// Print the data extracted from the JSON
void printMachineData(const struct MachineData* machineData) {
  // Serial.print("Status = ");
  // Serial.println(cookieData->returnStatus);
  Serial.print("MachineAccess = ");
  Serial.println(machineData->machineAccess);
}
// Close the connection with the HTTP server
void disconnect() {
  Serial.println("Disconnect");
  client.stop();
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

bool machineAccess(String cardNo){
    

     
  }
