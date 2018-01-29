/*
 This example reads audio data from an Invensense's ICS43432 I2S microphone
 breakout board, and prints out the samples to the Serial console. The
 Serial Plotter built into the Arduino IDE can be used to plot the audio
 data (Tools -> Serial Plotter)

 Circuit:
 * Arduino/Genuino Zero, MKRZero or MKR1000 board
 * ICS43432:
   * GND connected GND
   * 3.3V (3V) connected 3.3V (Zero) or VCC (MKR1000, MKRZero)
   * WS (LRCL) connected to pin 0 (Zero) or pin 3 (MKR1000, MKRZero)
   * CLK (BCLK) connected to pin 1 (Zero) or pin 2 (MKR1000, MKRZero)
   * SD (DOUT) connected to pin 9 (Zero) or pin A6 (MKR1000, MKRZero)

 created 17 November 2016
 by Sandeep Mistry

 Modified and added audio avarage non filered volume data sending via MQTT
 by Aki Salminen 2018
 */

#include <I2S.h>
#include <WiFi101.h>
#include <MQTTClient.h>
#include "settings.h"

WiFiClient wific;
MQTTClient mqttc;

unsigned long time=0;
uint8_t vals=0;
uint64_t avgval=0;
uint8_t first=1;

#define SAMPLES 512 // make it a power of two for best DMA performance

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  // start I2S at 16 kHz with 32-bits per sample
  if (!I2S.begin(I2S_PHILIPS_MODE, 16000, 32)) {
    Serial.println("Failed to initialize I2S!");
    while (1); // do nothing
  }
  // MQTT
  mqttc.begin(MQTT_SERVER, wific);
  mqttc.onMessage(messageReceived);
	mqttconnect();
}

void mqttconnect() {
  Serial.println("Waiting wifi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }
  Serial.println("Connecting MQTT");
  //(name, username, password)
  while (!mqttc.connect("arduVUmeter", MQTT_USER, MQTT_PASSWORD)) {
    Serial.print(".");
    delay(1000);
  }
  Serial.println("\nconnected!");
  // For incoming control messages
  // mqttc.subscribe(MQTT_TOPIC);
  // mqttc.unsubscribe(MQTT_TOPIC);
}

void loop() {
  char buffer[8];
  //Serial.print("Time: ");
  // read a bunch of samples:
  int32_t samples[SAMPLES];

  for (uint16_t i=0; i<SAMPLES; i++) {
    int32_t sample = 0;
    while ((sample == 0) || (sample == -1) ) {
      sample = I2S.read();
    }
    // convert to 18 bit signed
    sample >>= 14;
    samples[i] = sample;
  }
  // find the 'peak to peak' max
  int32_t maxsample=0, minsample=0;
  for (uint16_t i=0; i<SAMPLES; i++) {
    minsample = min(minsample, samples[i]);
    maxsample = max(maxsample, samples[i]);
  }
  Serial.println((long)(maxsample - minsample));
  avgval+=(maxsample - minsample);
  vals++;
  if( millis()-time>1000 ) {
    if(first) {
      first=0;
      Serial.println("Skip first data (unstable data).");
    }
    else{
      Serial.print("Sending data to server: ");
      itoa((avgval/vals),buffer,10);
      mqttc.publish(MQTT_TOPIC, buffer );
      Serial.println(buffer);
    }
    avgval=0;
    vals=0;
    time=millis();
  }
}

void messageReceived(String &topic, String &payload) {
  Serial.println("incoming: " + topic + " - " + payload);
}
