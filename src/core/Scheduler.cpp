#include "core/Scheduler.h"

#include <Arduino.h>

#include "config/BoardConfig.h"
#include "network/BinanceClient.h"
#include "network/TimeManager.h"
#include "network/WeatherClient.h"
#include "network/WifiManager.h"

void Scheduler::begin(WifiManager& wifi, BinanceClient& btc, WeatherClient& wx, TimeManager& time) {
    wifi_ = &wifi;
    btc_ = &btc;
    weather_ = &wx;
    time_ = &time;
    mutex_ = xSemaphoreCreateMutex();

    xTaskCreatePinnedToCore(&Scheduler::taskEntry, "net_task", 8192, this, 1, &taskHandle_, 0);
}

void Scheduler::taskEntry(void* arg) { static_cast<Scheduler*>(arg)->taskLoop(); }

void Scheduler::taskLoop() {
    uint32_t lastBitcoinPoll = 0;
    uint32_t lastWeatherPoll = 0;

    for (;;) {
        wifi_->loop();
        bool connected = wifi_->isConnected();

        if (connected && !ntpStarted_) {
            time_->begin();
            ntpStarted_ = true;
        }

        if (xSemaphoreTake(mutex_, pdMS_TO_TICKS(50)) == pdTRUE) {
            wifiConnected_ = connected;
            xSemaphoreGive(mutex_);
        }

        uint32_t now = millis();

        if (connected && (now - lastBitcoinPoll >= board::timing::BITCOIN_POLL_MS || lastBitcoinPoll == 0)) {
            lastBitcoinPoll = now;
            BinanceTicker ticker;
            if (btc_->fetchTicker(ticker)) {
                if (xSemaphoreTake(mutex_, pdMS_TO_TICKS(100)) == pdTRUE) {
                    bitcoinSnap_.valid = true;
                    bitcoinSnap_.price = ticker.price;
                    bitcoinSnap_.changePercent = ticker.changePercent;
                    bitcoinSnap_.updatedAtMs = now;
                    xSemaphoreGive(mutex_);
                }
            }
        }

        if (connected &&
            (now - lastWeatherPoll >= board::timing::WEATHER_POLL_MS || lastWeatherPoll == 0)) {
            lastWeatherPoll = now;
            WeatherData data;
            if (weather_->fetchForecast(data)) {
                if (xSemaphoreTake(mutex_, pdMS_TO_TICKS(100)) == pdTRUE) {
                    weatherSnap_.valid = true;
                    weatherSnap_.tempC = data.tempC;
                    weatherSnap_.weatherCode = data.weatherCode;
                    weatherSnap_.humidityPct = data.humidityPct;
                    weatherSnap_.windKmh = data.windKmh;
                    weatherSnap_.rainProbabilityPct = data.rainProbabilityPct;
                    weatherSnap_.updatedAtMs = now;
                    xSemaphoreGive(mutex_);
                }
            }
        }

        vTaskDelay(pdMS_TO_TICKS(250));
    }
}

BitcoinSnapshot Scheduler::bitcoin() const {
    BitcoinSnapshot copy;
    if (mutex_ && xSemaphoreTake(mutex_, pdMS_TO_TICKS(20)) == pdTRUE) {
        copy = bitcoinSnap_;
        xSemaphoreGive(mutex_);
    }
    return copy;
}

WeatherSnapshot Scheduler::weather() const {
    WeatherSnapshot copy;
    if (mutex_ && xSemaphoreTake(mutex_, pdMS_TO_TICKS(20)) == pdTRUE) {
        copy = weatherSnap_;
        xSemaphoreGive(mutex_);
    }
    return copy;
}

bool Scheduler::wifiConnected() const {
    bool value = false;
    if (mutex_ && xSemaphoreTake(mutex_, pdMS_TO_TICKS(20)) == pdTRUE) {
        value = wifiConnected_;
        xSemaphoreGive(mutex_);
    }
    return value;
}
