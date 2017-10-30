/****************************************************************
apds9960.ino
APDS-9960 RGB and Gesture Sensor
Shawn Hymel @ SparkFun Electronics
October 29, 2014
https://github.com/sparkfun/APDS-9960_RGB_and_Gesture_Sensor

Shows how to control the MicroView using the APDS-9960 Gesture
Sensor. While idling, the MicroView displays graphs of the
current ambient, red, green, and blue light. When a gesture (UP,
DOWN, LEFT, RIGHT, NEAR, or FAR) is detected, a graphic is drawn
on the MicroView depicting that gesture.

Hardware Connections:

IMPORTANT: The APDS-9960 can only accept 3.3V!
 
 MicroView    APDS-9960 Board  FTDI Breakout  Notes
 
 VIN          VCC              3V3            Power
 GND          GND              GND            Ground
 A4           SDA              -              I2C Data
 A5           SCL              -              I2C Clock
 2            INT              -              Interrupt
 RST          -                DTR            0.1uF in series
 RX           -                RXI            Programming (Rx)
 TX           -                TXO            Programming (Tx)
 
Resources:
Include Wire.h, SFE_APDS-9960.h, and MicroView.h

Development environment specifics:
Written in Arduino 1.0.5
Tested with MicroView

This code is beerware; if you see me (or any other SparkFun 
employee) at the local, and you've found our code helpful, please
buy us a round!

Distributed as-is; no warranty is given.
****************************************************************/

#include <Wire.h>
#include <SparkFun_APDS9960.h>

// Pins
#define APDS9960_INT    D5 // Needs to be an interrupt pin

// Constants
#define READ_LIGHT      1    // 1 to read light, 0 to not
#define LIGHT_MAX       2000 // Max value of light readings

// Global Variables
SparkFun_APDS9960 apds = SparkFun_APDS9960();
uint16_t ambient_light = 0;
uint16_t red_light = 0;
uint16_t green_light = 0;
uint16_t blue_light = 0;
int isr_flag = 0;

#define SDA     D2
#define SCL     D1


void setup() {
  
  // Initialize Serial port
  Serial.begin(115200);
  Wire.begin(SDA, SCL);

  Serial.println();
  Serial.println("MicroView Gesture Demo");
  
  
  // Initialize interrupt service routine
  attachInterrupt(APDS9960_INT, interruptRoutine, FALLING);
  
  
  // Initialize APDS-9960 (configure I2C and initial values)
  if ( apds.init() ) {
    Serial.println(F("APDS-9960 initialization complete"));
  } else {
    Serial.println(F("Something went wrong during APDS-9960 init!"));
  }
  
#if READ_LIGHT
  // Set light gain value
  if ( !apds.setAmbientLightGain(AGAIN_1X) ) {
    Serial.println(F("Something went wrong trying to set gain!"));
  }
  
  // Start running the APDS-9960 light sensor (no interrupts)
  if ( apds.enableLightSensor(false) ) {
    Serial.println(F("Light sensor is now running"));
  } else {
    Serial.println(F("Something went wrong during light sensor init!"));
  }
#endif
  
  // Start running the gesture sensor engine (with interrupts)
  if ( apds.enableGestureSensor(true) ) {
    Serial.println(F("Gesture sensor is now running"));
  } else {
    Serial.println(F("Something went wrong during gesture sensor init!"));
  }
  
  // Wait for initialization and calibration to finish
  delay(500);
}

void loop() {
  
  // If interrupt, display gesture on MicroView
  if ( isr_flag == 1 ) {
    handleGesture();
    isr_flag = 0;
    delay(500);
  }
  
  // Draw title and C, R, G, B labels
#if READ_LIGHT
  // Read the light levels (ambient, red, green, blue)
  if (  !apds.readAmbientLight(ambient_light) ||
        !apds.readRedLight(red_light) ||
        !apds.readGreenLight(green_light) ||
        !apds.readBlueLight(blue_light) ) {
    Serial.println("Error reading light values");
  } else {
    if ( ambient_light > LIGHT_MAX ) ambient_light = LIGHT_MAX;
    if ( red_light > LIGHT_MAX ) red_light = LIGHT_MAX;
    if ( green_light > LIGHT_MAX ) green_light = LIGHT_MAX;
    if ( blue_light > LIGHT_MAX ) blue_light = LIGHT_MAX;
    /*
    Serial.println(ambient_light);
    Serial.println(red_light);
    Serial.println(green_light);
    Serial.println(blue_light);
    */
  }
#endif
  
  
  // Wait before next reading
  delay(100);
}

void interruptRoutine() {
  isr_flag = 1;
}

void handleGesture() {
  bool do_clear = true;
  
  // Draw symbol based on gesture
  if ( apds.isGestureAvailable() ) {
    switch ( apds.readGesture() ) {
      case DIR_UP:
        Serial.println("UP");
        break;
      case DIR_DOWN:
        Serial.println("DOWN");
        break;
      case DIR_LEFT:
        Serial.println("LEFT");
        break;
      case DIR_RIGHT:
        Serial.println("RIGHT");
        break;
      case DIR_NEAR:
        Serial.println("NEAR");
        break;
      case DIR_FAR:
        Serial.println("FAR");
        break;
      default:
        Serial.println("NONE");
    }
  }
}


