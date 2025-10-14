#include <Wire.h>

// I2C pins for the ESP32
#define I2C_SDA 21
#define I2C_SCL 22

// I2C address found by the scanner
#define BMS_ADDRESS 0x0B

// --- Standard Smart Battery Commands ---
// Registers
#define MANU_ACCESS 0x00
#define BATT_MODE   0x03
#define TEMP_REG    0x08
#define VOLT_REG    0x09
#define CURRENT_REG 0x0A
#define SOC_REG     0x0D

// Keys and Commands
#define UNSEAL_KEY1 0x0414
#define UNSEAL_KEY2 0x3672
#define CLEAR_PF_KEY1 0x4172
#define CLEAR_PF_KEY2 0x4436
#define RESET_CMD   0x0041

void setup() {
  Wire.begin(I2C_SDA, I2C_SCL);
  Serial.begin(115200);
  while (!Serial) { ; }
  Serial.println("\n--- Starting Full BMS Unlock Procedure ---");

  // Step 1: Unseal the device
  Serial.println("\n[Step 1] Sending Unseal Keys...");
  if (sendCommand(UNSEAL_KEY1) && sendCommand(UNSEAL_KEY2)) {
    Serial.println(">>> Unseal successful.");
  } else {
    Serial.println(">>> Unseal FAILED. Halting.");
    while(1); // Stop here if it fails
  }
  delay(500);

  // Step 2: Clear the Permanent Failure flags
  Serial.println("\n[Step 2] Sending Clear Permanent Failure Keys...");
  if (sendCommand(CLEAR_PF_KEY1) && sendCommand(CLEAR_PF_KEY2)) {
    Serial.println(">>> Clear PF successful.");
  } else {
    Serial.println(">>> Clear PF FAILED.");
  }
  delay(500);

  // Step 3: Issue a full reset to apply changes
  Serial.println("\n[Step 3] Sending Reset Command...");
  if (sendCommand(RESET_CMD)) {
    Serial.println(">>> Reset command sent. The BMS should now be unlocked and reset.");
  } else {
    Serial.println(">>> Reset FAILED.");
  }
  
  Serial.println("\n--- Starting Data Monitoring ---");
}

void loop() {
  // Read and display key battery parameters
  int voltage = readRegister(VOLT_REG);
  int current = readRegister(CURRENT_REG); // This will likely be 0
  int soc = readRegister(SOC_REG);
  
  Serial.print("State of Charge (SOC): "); Serial.print(soc); Serial.print("% | ");
  Serial.print("Voltage: "); Serial.print(voltage); Serial.print(" mV | ");
  Serial.print("Current: "); Serial.print((int16_t)current); Serial.println(" mA");

  delay(5000); // Wait 5 seconds between readings
}

// Function to send a 2-byte command to ManufacturerAccess (0x00)
bool sendCommand(uint16_t command) {
  Wire.beginTransmission(BMS_ADDRESS);
  Wire.write(MANU_ACCESS);      // Target register
  Wire.write(command & 0xFF);   // Low byte
  Wire.write((command >> 8) & 0xFF); // High byte
  byte error = Wire.endTransmission();
  return (error == 0); // Return true if successful, false if error
}

// Function to read a 2-byte (word) value from a register
uint16_t readRegister(byte reg) {
  Wire.beginTransmission(BMS_ADDRESS);
  Wire.write(reg);
  Wire.endTransmission(false); // Send restart
  
  Wire.requestFrom(BMS_ADDRESS, 2); // Request 2 bytes
  if (Wire.available() >= 2) {
    byte lowByte = Wire.read();
    byte highByte = Wire.read();
    return (highByte << 8) | lowByte;
  }
  return 0; // Return 0 if read fails
}