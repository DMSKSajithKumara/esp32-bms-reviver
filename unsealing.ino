#include <Wire.h>

// Define the I2C pins for the ESP32
#define I2C_SDA 21
#define I2C_SCL 22

// The I2C address found by the scanner
#define BMS_ADDRESS 0x0B

// The command register for sending keys
#define MANU_ACCESS 0x00

void setup() {
  Wire.begin(I2C_SDA, I2C_SCL);
  Serial.begin(115200);
  while (!Serial) { ; }

  Serial.println("Attempting to UNSEAL the Smart Battery Gas Gauge...");

  // Try to unseal using the most common default TI key
  unsealBMS(0x0414, 0x3672);

  // If that doesn't work, you can comment out the line above
  // and try this other common key below.
  // unsealBMS(0x1234, 0xABCD);
}

void loop() {
  // After unsealing, you would typically send commands to clear
  // permanent failure flags or reset the device. For now, we'll stop here.
  delay(10000);
}

void unsealBMS(uint16_t key1, uint16_t key2) {
  Serial.print("Sending Unseal Key: 0x");
  Serial.print(key1, HEX);
  Serial.print(" followed by 0x");
  Serial.println(key2, HEX);

  // Send the first word of the key
  Wire.beginTransmission(BMS_ADDRESS);
  Wire.write(MANU_ACCESS);    // ManufacturerAccess register
  Wire.write(key1 & 0xFF);     // Send low byte first
  Wire.write((key1 >> 8) & 0xFF); // Send high byte
  byte error1 = Wire.endTransmission();

  if (error1 == 0) {
    Serial.println("SUCCESS: First part of the key sent.");
  } else {
    Serial.print("ERROR: Failed to send first part of key. Code: ");
    Serial.println(error1);
    return; // Stop if the first part fails
  }

  delay(100); // A small delay is good practice

  // Send the second word of the key
  Wire.beginTransmission(BMS_ADDRESS);
  Wire.write(MANU_ACCESS);    // ManufacturerAccess register again
  Wire.write(key2 & 0xFF);     // Send low byte first
  Wire.write((key2 >> 8) & 0xFF); // Send high byte
  byte error2 = Wire.endTransmission();

  if (error2 == 0) {
    Serial.println("SUCCESS: Second part of the key sent.");
    Serial.println("If the key is correct, the BMS is now UNSEALED.");
    Serial.println("You can now send commands to clear failure flags or reset.");
  } else {
    Serial.print("ERROR: Failed to send second part of key. Code: ");
    Serial.println(error2);
  }
}