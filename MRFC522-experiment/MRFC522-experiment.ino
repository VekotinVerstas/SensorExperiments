/* 
 *  Wiring the MFRC522 to ESP8266
 *  For Wemos D1 Mini (pro) see:
 *  https://wiki.wemos.cc/products:retired:d1_mini_pro_v1.1.0

RST     = GPIO5
SDA(SS) = GPIO4 
MOSI    = GPIO13
MISO    = GPIO12
SCK     = GPIO14
GND     = GND
3.3V    = 3.3V
*/

#include "settings.h"
#include <ESP8266WiFi.h>
#include <DNSServer.h>            // Local DNS Server used for redirecting all requests to the configuration portal
#include <ESP8266WebServer.h>     // Local WebServer used to serve the configuration portal
#include <WiFiManager.h>          // https://github.com/tzapu/WiFiManager WiFi Configuration Magic
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>
#include <SPI.h>
#include "MFRC522.h"

#define RST_PIN  5  // RST-PIN für RC522 - RFID - SPI - Modul GPIO5 
#define SS_PIN  4  // SDA-PIN für RC522 - RFID - SPI - Modul GPIO4 

// Define and set up all variables / objects
String mac_str;
byte mac[6];
char mac_addr[13]; 

WiFiClient wifiClient;
WiFiManager wifiManager;
MFRC522 mfrc522(SS_PIN, RST_PIN); // Create MFRC522 instance


void setup() {
  Serial.begin(115200);    // Initialize serial communications
  delay(250);
  Serial.println(F("Booting...."));
  mac_str = WiFi.macAddress();
  WiFi.macAddress(mac);
  byteArrayToString(mac_addr, mac, 6);  
  char ap_name[30];
  sprintf(ap_name, "RFID_READER_%d", ESP.getChipId());
  Serial.print("AP name would be: ");
  Serial.println(ap_name);
  wifiManager.setConfigPortalTimeout(180);
  wifiManager.autoConnect(ap_name);  
  SPI.begin();           // Init SPI bus
  mfrc522.PCD_Init();    // Init MFRC522
  Serial.println(F("Ready!"));
  Serial.println(F("======================================================")); 
  Serial.println(F("Scan for Card and print UID:"));
}


void loop() {
  readTag();
}

void readTag() { 
  // Look for new cards
  if ( ! mfrc522.PICC_IsNewCardPresent()) {
    delay(50);
    return;
  }
  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial()) {
    delay(50);
    return;
  }
  // Show some details of the PICC (that is: the tag/card)
  Serial.print(F("Card UID:"));
  Serial.println();
  char tagId[2*mfrc522.uid.size+1]; 
  byteArrayToString(tagId, mfrc522.uid.uidByte, mfrc522.uid.size);
  getpr24h(tagId);
  delay(5000);

}


// Helper routine to dump a byte array as hex values to Serial
void byteArrayToString(char *dest, byte *buffer, byte bufferSize) {
  // char charArr[2*bufferSize + 1];  // Note there needs to be 1 extra space for this to work as snprintf null terminates.
  char* myPtr = &dest[0];       // or just myPtr=charArr; but the former described it better.
   for (byte i = 0; i < bufferSize; i++){
     snprintf(myPtr,3,"%02x",buffer[i]); //convert a byte to character string, and save 2 characters (+null) to charArr;
     myPtr += 2; //increment the pointer by two characters in charArr so that next time the null from the previous go is overwritten.
   }  
   Serial.println(dest);
   for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
   Serial.println();
}


void getpr24h(char *tagId) {
  std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);
  client->setInsecure();
  HTTPClient https;
  char buffer [200];
  int cx;
  cx = snprintf ( buffer, 200, "%s?devid=%s&tagid=%s", BASE_URL, mac_addr, tagId );
  Serial.println(buffer);

  if (https.begin(*client, buffer)) {  // HTTPS
    Serial.println("[HTTPS] GET...");
    int httpCode = https.GET();

    // httpCode will be negative on error
    if (httpCode > 0) {
      // HTTP header has been send and Server response header has been handled
      Serial.printf("[HTTPS] GET... code: %d\n", httpCode);
      // file found at server?
      if (httpCode == HTTP_CODE_OK) {
        String payload = https.getString();
        Serial.println(String("[HTTPS] Received payload: ") + payload);
        Serial.println(payload);
      }
    } else {
      Serial.printf("[HTTPS] GET... failed, error: %s\n\r", https.errorToString(httpCode).c_str());
    }

    https.end();
  } else {
    Serial.printf("[HTTPS] Unable to connect\n\r");
  }
}
