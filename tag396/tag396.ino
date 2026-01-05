// =======================================================
//  Tag V3.9.6 – Nano ESP32 (High-Rate Broadcast Version)
//  - Increase BLE broadcast frequency to ~10Hz
//  - Do NOT stop advertising (prevents BLE stack delay)
//  - Anchor can collect 10–20 samples per second
// =======================================================

#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_LSM6DSL.h>
#include <Adafruit_Sensor.h>
#include <ArduinoBLE.h>

// ===========================
// HARDWARE CONFIGURATION
// ===========================
#define BUTTON_PIN  D4
#define LED_PIN     D5
#define LSM_CS      D10
#define LSM_MOSI    D11
#define LSM_MISO    D12
#define LSM_SCK     D13

// ===========================
// VARIABLES
// ===========================
Adafruit_LSM6DSL lsm6dsl;
bool imuConnected = false;

const float MOVEMENT_THRESHOLD = 0.5;
const unsigned long MOTION_TIMEOUT = 5000;

bool forkliftOccupied = false;
bool isMoving = false;

unsigned long lastMotionTime = 0;
unsigned long lastDebounceTime = 0;
int buttonState;
int lastReading = HIGH;
const unsigned long debounceDelay = 50;

// BLE broadcast timing – now 100ms per update
unsigned long lastAdvUpdate = 0;
const unsigned long ADV_INTERVAL = 100;

// IMU delta memory
float prevX = 0, prevY = 0, prevZ = 0;

// BLE Data Buffer
uint8_t mfgData[6];
BLEService dummyService("180A");

// ===============================
// Function: Update Manufacturer Data Only
// ===============================
void updateBLEAdvertisement() {
  if (millis() - lastAdvUpdate < ADV_INTERVAL) return;
  lastAdvUpdate = millis();

  // Build Data Payload (Your protocol)
  mfgData[0] = 0x4B;
  mfgData[1] = 0x01;
  mfgData[2] = 0x02;
  mfgData[3] = forkliftOccupied ? 0x01 : 0x00;
  mfgData[4] = isMoving         ? 0x01 : 0x00;
  mfgData[5] = 0xFF;

  // IMPORTANT:
  // DO NOT stop+restart advertising.
  // Just update manufacturer data directly.
  BLE.setManufacturerData(mfgData, 6);

  Serial.print("[BLE] Updated: Occ=");
  Serial.print(forkliftOccupied);
  Serial.print(" | Mov=");
  Serial.println(isMoving);
}

// ===============================
// SETUP
// ===============================
void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("=== Tag V3.9.6 (High-Rate BLE) ===");

  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  buttonState = digitalRead(BUTTON_PIN);

  // IMU Init
  if (lsm6dsl.begin_SPI(LSM_CS, LSM_SCK, LSM_MISO, LSM_MOSI)) {
    Serial.println("✅ IMU Connected (SPI)!");
    imuConnected = true;
    lsm6dsl.setAccelRange(LSM6DS_ACCEL_RANGE_4_G);
    lsm6dsl.setAccelDataRate(LSM6DS_RATE_52_HZ);   // Faster sample rate
  } else {
    Serial.println("❌ IMU Failed! Check wiring.");
  }

  // BLE Init
  if (!BLE.begin()) {
    Serial.println("❌ BLE Failed");
    while (1);
  }

  BLE.setLocalName("Forklift_Tag");
  BLE.setDeviceName("Forklift_Tag");
  BLE.setAdvertisedService(dummyService);
  BLE.addService(dummyService);

  BLE.setManufacturerData(mfgData, 6);
  BLE.advertise();

  Serial.println("BLE Broadcasting...");
}

// ===============================
// MAIN LOOP
// ===============================
void loop() {
  BLE.poll();

  // --- IMU Motion Detection ---
  if (imuConnected) {
    sensors_event_t accel, gyro, temp;
    lsm6dsl.getEvent(&accel, &gyro, &temp);

    if (prevX == 0 && prevY == 0 && prevZ == 0) {
        prevX = accel.acceleration.x;
        prevY = accel.acceleration.y;
        prevZ = accel.acceleration.z;
    }

    float delta = abs(accel.acceleration.x - prevX)
                + abs(accel.acceleration.y - prevY)
                + abs(accel.acceleration.z - prevZ);

    prevX = accel.acceleration.x;
    prevY = accel.acceleration.y;
    prevZ = accel.acceleration.z;

    if (delta > MOVEMENT_THRESHOLD) {
      lastMotionTime = millis();
      if (!isMoving) {
        isMoving = true;
        Serial.println("[Motion] START");
        updateBLEAdvertisement();
      }
    }

    if (isMoving && (millis() - lastMotionTime > MOTION_TIMEOUT)) {
      isMoving = false;
      Serial.println("[Motion] STOP");
      updateBLEAdvertisement();
    }
  }

  // --- Button Logic ---
  int reading = digitalRead(BUTTON_PIN);

  if (reading != lastReading) lastDebounceTime = millis();

  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != buttonState) {
      buttonState = reading;

      if (buttonState == LOW) {
        forkliftOccupied = !forkliftOccupied;
        digitalWrite(LED_PIN, forkliftOccupied ? HIGH : LOW);
        Serial.println("[Button] TOGGLE");
        updateBLEAdvertisement();
      }
    }
  }

  lastReading = reading;

  // Constant BLE refresh
  updateBLEAdvertisement();
}
