#ifndef DEVICE_SETTINGS_H
#define DEVICE_SETTINGS_H

// ===== WiFi Settings =====
const char* WIFI_SSID     = "TP-Link_3E19";
const char* WIFI_PASSWORD = "97651819";

// ===== ThingsBoard Token (anchor_A1) =====
const char* TB_TOKEN = "t73rK8pD6vJdVP5Cxx0q";

// ===== ThingsBoard Server =====
const char* TB_SERVER = "demo.thingsboard.io";
const int   TB_PORT   = 1883;

// ===== BLE Tag to Listen For =====
const char* TARGET_TAG_NAME = "Forklift_Tag_01";

// ===== Anchor ID =====
// 用于区分 A1 / A2 / A3
const char* ANCHOR_ID = "A1";

#endif
