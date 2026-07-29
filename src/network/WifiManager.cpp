#include "network/WifiManager.h"

#include <Arduino.h>
#include <WiFi.h>

#include "config/BoardConfig.h"

void WifiManager::begin(const char* ssid, const char* password) {
    ssid_ = ssid;
    password_ = password;
    WiFi.persistent(false);
    WiFi.mode(WIFI_STA);
    WiFi.setAutoReconnect(true);
    WiFi.begin(ssid_, password_);
    nextAttemptAt_ = millis() + board::timing::WIFI_CONNECT_TIMEOUT_MS;
}

void WifiManager::loop() {
    if (WiFi.status() == WL_CONNECTED) return;

    uint32_t now = millis();
    if (now >= nextAttemptAt_) {
        WiFi.disconnect();
        WiFi.begin(ssid_, password_);
        nextAttemptAt_ = now + board::timing::WIFI_RETRY_MS;
    }
}

bool WifiManager::isConnected() const { return WiFi.status() == WL_CONNECTED; }
