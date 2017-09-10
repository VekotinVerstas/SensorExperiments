/*
 * Read MLX90614 contactless IR thermometer I2C sensor.
 * Original code was found from here:
 * http://www.esp8266.com/viewtopic.php?f=29&t=2865
 */

#include <Wire.h>

#define MLX90614_TA 0x06
#define MLX90614_TOBJ1 0x07

void setup() {
  Serial.begin(115200);
  Wire.begin(D2,D1);  // Wire.begin([SDA], [SCL])
}


uint16_t read16(uint8_t addr, uint8_t i2c_addr){
  uint16_t ret;
  Wire.beginTransmission(i2c_addr); // start transmission to device 
  Wire.write(addr); // sends register address to read from
  Wire.endTransmission(false); // end transmission
  Wire.requestFrom(i2c_addr, (uint8_t)3);// send data n-bytes read
  ret = Wire.read(); // receive DATA
  ret |= Wire.read() << 8; // receive DATA
  uint8_t pec = Wire.read();
  return ret;
}


float readTemp(uint8_t reg, uint8_t i2c_addr) {
  float temp;
  temp = read16(reg, i2c_addr);
  temp *= .02;
  temp -= 273.15;
  return temp;
}

double readObjectTempC(uint8_t i2c_addr) {
  return readTemp(MLX90614_TOBJ1, i2c_addr);
}

double readAmbientTempC(uint8_t i2c_addr) {
  return readTemp(MLX90614_TA, i2c_addr);
}

void loop() {
  Serial.print("Ambient = "); Serial.print(readAmbientTempC(0x5A)); 
  Serial.print("*C\tObject = "); Serial.print(readObjectTempC(0x5A)); Serial.println("*C");
  Serial.println();
  delay(1000);
}
