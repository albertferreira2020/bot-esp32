#pragma once
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>
#include <cstdint>

class WifiManager;
class BinanceClient;
class WeatherClient;
class TimeManager;

struct BitcoinSnapshot {
    bool valid = false;
    double price = 0.0;
    double changePercent = 0.0;
    uint32_t updatedAtMs = 0;
};

struct WeatherSnapshot {
    bool valid = false;
    float tempC = 0.0f;
    int weatherCode = 0;
    int humidityPct = 0;
    float windKmh = 0.0f;
    int rainProbabilityPct = 0;  // próxima hora
    uint32_t updatedAtMs = 0;
};

// Task de rede (core 0): faz polling assíncrono de Bitcoin/Clima/WiFi/NTP em
// intervalos próprios e publica snapshots protegidos por mutex. A task de
// render (core 1 / loop principal) só lê os snapshots — nunca bloqueia em
// HTTP.
class Scheduler {
   public:
    void begin(WifiManager& wifi, BinanceClient& btc, WeatherClient& wx, TimeManager& time);

    BitcoinSnapshot bitcoin() const;
    WeatherSnapshot weather() const;
    bool wifiConnected() const;

   private:
    static void taskEntry(void* arg);
    [[noreturn]] void taskLoop();

    WifiManager* wifi_ = nullptr;
    BinanceClient* btc_ = nullptr;
    WeatherClient* weather_ = nullptr;
    TimeManager* time_ = nullptr;

    SemaphoreHandle_t mutex_ = nullptr;
    TaskHandle_t taskHandle_ = nullptr;

    BitcoinSnapshot bitcoinSnap_;
    WeatherSnapshot weatherSnap_;
    bool wifiConnected_ = false;
    bool ntpStarted_ = false;
};
