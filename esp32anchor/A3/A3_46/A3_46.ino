// =======================================================
//  Anchor V4.6 – ESP32 Edge Computing (Final Polished)
//  - Feature: Auto WiFi Roaming (WiFiMulti)
//  - Logic: Rolling Window + State Smoothing
//  - Fixed: Tag Re-entry Logic (Index Reset Bug)
// =======================================================

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <PubSubClient.h>

#include "device_settings.h"

// ===================== GLOBAL OBJECTS =====================
WiFiMulti   wifiMulti;
WiFiClient  espClient;
PubSubClient client(espClient);
BLEScan* pBLEScan = nullptr;

// Timing
unsigned long lastSendTime    = 0;
const int SCAN_WINDOW_MS      = BLE_SCAN_TIME * 1000; 
const unsigned long TAG_LOSS_TIMEOUT_MS = 5000; 

// ===================== ROLLING WINDOW SETTINGS =====================
const int RSSI_WINDOW_SIZE = 10;
int   rssiWindow[RSSI_WINDOW_SIZE];
int   rssiIndex  = 0;
int   rssiCount  = 0;

const int STATE_WINDOW_SIZE = 5;
int   occWindow[STATE_WINDOW_SIZE];
int   movWindow[STATE_WINDOW_SIZE];
int   stateIndex = 0;
int   stateCount = 0;

unsigned long lastTagSeenTime = 0;

// ===================== MQTT RECONNECT =====================
void reconnect() {
  while (wifiMulti.run() == WL_CONNECTED && !client.connected()) {
    Serial.print("Connecting to MQTT... ");
    if (client.connect(ANCHOR_ID, TB_TOKEN, nullptr)) {
      Serial.println("OK");
    } else {
      Serial.print("Fail rc="); Serial.print(client.state()); Serial.println(" retry 3s...");
      delay(3000);
    }
  }
}

// ===================== BLE CALLBACK =====================
class TagScanCallback : public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice adv) override {
    if (!adv.haveManufacturerData()) return;
    std::string data = adv.getManufacturerData();
    if (data.length() < 6) return;
    const uint8_t *raw = (const uint8_t *)data.data();
    if (raw[0] != 0x4B || raw[1] != 0x01) return;

    // Save Data
    int occ = raw[3];
    int mov = raw[4];
    int rssi = adv.getRSSI();

    // Update RSSI Window
    rssiWindow[rssiIndex] = rssi;
    rssiIndex = (rssiIndex + 1) % RSSI_WINDOW_SIZE;
    if (rssiCount < RSSI_WINDOW_SIZE) rssiCount++;

    // Update State Window
    occWindow[stateIndex] = occ;
    movWindow[stateIndex] = mov;
    stateIndex = (stateIndex + 1) % STATE_WINDOW_SIZE;
    if (stateCount < STATE_WINDOW_SIZE) stateCount++;

    lastTagSeenTime = millis();
  }
};

// ===================== SETUP =====================
void setup() {
  Serial.begin(115200);
  
  // WiFi Setup
  for (int i = 0; i < WIFI_COUNT; i++) {
    wifiMulti.addAP(WIFI_LIST[i].ssid, WIFI_LIST[i].password);
  }
  Serial.println("Connecting WiFi...");
  while (wifiMulti.run() != WL_CONNECTED) { delay(500); Serial.print("."); }
  Serial.println("\nWiFi Connected");

  // MQTT Setup
  client.setServer(TB_SERVER, TB_PORT);
  client.setBufferSize(1024, 1024);

  // BLE Setup
  BLEDevice::init(ANCHOR_ID);
  pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new TagScanCallback());
  pBLEScan->setActiveScan(true);
  pBLEScan->setInterval(100);
  pBLEScan->setWindow(99);

  // Clear memory
  memset(rssiWindow, 0, sizeof(rssiWindow));
  memset(occWindow, 0, sizeof(occWindow));
  memset(movWindow, 0, sizeof(movWindow));
}

// ===================== HELPER =====================
int majorityVote(const int *arr, int count) {
  if (count <= 0) return 0;
  int sum = 0;
  for (int i = 0; i < count; i++) sum += arr[i];
  return (sum * 2 >= count) ? 1 : 0;
}

// ===================== LOOP =====================
void loop() {
  if (wifiMulti.run() != WL_CONNECTED) Serial.println("WiFi Scanning...");
  if (!client.connected()) reconnect();
  client.loop();

  pBLEScan->start(BLE_SCAN_TIME, false);
  pBLEScan->clearResults(); // Important!

  if (millis() - lastSendTime > SCAN_WINDOW_MS) {
    lastSendTime = millis();
    unsigned long now = millis();
    bool isTagOnline = (now - lastTagSeenTime) <= TAG_LOSS_TIMEOUT_MS;

    if (isTagOnline && rssiCount > 0) {
      // 1. Calculations
      long sum = 0;
      for (int i = 0; i < rssiCount; i++) sum += rssiWindow[i];
      float avgRssi = (float)sum / rssiCount;

      float varSum = 0.0f;
      for (int i = 0; i < rssiCount; i++) {
        float diff = rssiWindow[i] - avgRssi;
        varSum += diff * diff;
      }
      float variance = (rssiCount > 1) ? (varSum / rssiCount) : 0.0f;

      int finalOcc = majorityVote(occWindow, stateCount);
      int finalMov = majorityVote(movWindow, stateCount);

      // 2. Send JSON
      String json = "{";
      json += "\"anchor\":\"" + String(ANCHOR_ID) + "\",";
      json += "\"rssi\":" + String(avgRssi, 1) + ",";
      json += "\"var\":"  + String(variance, 1) + ",";
      json += "\"occ\":"  + String(finalOcc) + ","; 
      json += "\"mov\":"  + String(finalMov) + ","; 
      json += "\"cnt\":"  + String(rssiCount) + ",";
      json += "\"visible\":1";
      json += "}";
      client.publish("v1/devices/me/telemetry", json.c_str());
      Serial.print("[Data] "); Serial.println(json);

    } else {
      // 3. Heartbeat (Tag Lost)
      String hb = "{\"anchor\":\"" + String(ANCHOR_ID) + "\",\"visible\":0}";
      client.publish("v1/devices/me/telemetry", hb.c_str());
      Serial.println("[Heartbeat] Tag lost");

      // [FIX] Reset EVERYTHING
      rssiCount = 0; rssiIndex = 0;
      stateCount = 0; stateIndex = 0;
      memset(rssiWindow, 0, sizeof(rssiWindow));
      memset(occWindow, 0, sizeof(occWindow));
      memset(movWindow, 0, sizeof(movWindow));
    }
  }
}