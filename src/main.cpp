#include <Arduino.h>
#include <LittleFS.h>

#include "config/BoardConfig.h"
#include "config/Secrets.h"
#include "core/Scheduler.h"
#include "core/StateMachine.h"
#include "display/DisplayManager.h"
#include "network/BinanceClient.h"
#include "network/TimeManager.h"
#include "network/WeatherClient.h"
#include "network/WifiManager.h"
#include "power/PowerManager.h"
#include "robot/RobotFace.h"
#include "screens/BitcoinState.h"
#include "screens/BootState.h"
#include "screens/ClockState.h"
#include "screens/EyesState.h"
#include "screens/RainState.h"
#include "screens/TransitionState.h"
#include "screens/WeatherState.h"
#include "screens/WifiSearchState.h"

namespace {
DisplayManager display;
RobotFace robotFace;

WifiManager wifiManager;
BinanceClient binanceClient;
WeatherClient weatherClient;
TimeManager timeManager;
Scheduler scheduler;
PowerManager powerManager;
StateMachine stateMachine;

BootState bootState(display);
EyesState eyesState(robotFace);
TransitionState transitionState(display);
BitcoinState bitcoinState(scheduler);
WeatherState weatherState(scheduler);
RainState rainState(display, scheduler);
ClockState clockState;
WifiSearchState wifiSearchState(robotFace);

uint32_t lastFrameMs = 0;

bool rainForecast() {
    WeatherSnapshot snap = scheduler.weather();
    return snap.valid && snap.rainProbabilityPct >= board::timing::RAIN_PROBABILITY_THRESHOLD_PCT;
}
}  // namespace

void setup() {
    Serial.begin(115200);
    delay(200);
    Serial.println("\n[robot] boot");

    // Label explícito e casando com o nome em partitions.csv ("spiffs" —
    // veja o comentário lá). O mount procura a partição pelo LABEL, não pelo
    // subtype: com label errado nada de data/ é encontrado e todas as telas
    // caem nos efeitos procedurais.
    // formatOnFail=false de propósito: se falhar, é melhor avisar do que
    // apagar os assets já gravados via 'pio run -t uploadfs'.
    if (!LittleFS.begin(false, "/littlefs", 10, "spiffs")) {
        Serial.println("[robot] LittleFS: falha ao montar (assets indisponiveis, o resto funciona)");
        Serial.println("[robot] rodou 'pio run -e esp32-s3-lcd-13 -t uploadfs'?");
    }

    display.begin();

    robotFace.begin(DisplayManager::width() / 2, DisplayManager::height() / 2, 90, 34.0f);

    powerManager.begin();
    Serial.printf("[robot] IMU QMI8658: %s\n", powerManager.imuDetected() ? "detectado" : "nao detectado");

    wifiManager.begin(secrets::WIFI_SSID, secrets::WIFI_PASSWORD);
    scheduler.begin(wifiManager, binanceClient, weatherClient, timeManager);

    StateMachine::States states;
    states.boot = &bootState;
    states.eyes = &eyesState;
    states.transition = &transitionState;
    states.bitcoin = &bitcoinState;
    states.weather = &weatherState;
    states.rain = &rainState;
    states.clock = &clockState;
    states.wifiSearch = &wifiSearchState;
    stateMachine.begin(states, rainForecast);

    lastFrameMs = millis();
}

void loop() {
    uint32_t now = millis();
    uint32_t dt = now - lastFrameMs;
    lastFrameMs = now;

    if (scheduler.wifiConnected()) {
        stateMachine.notifyWifiUp();
    } else {
        stateMachine.notifyWifiDown();
    }

    powerManager.update(now, display, robotFace);

    stateMachine.update(dt, now);
    stateMachine.render(display);

    uint32_t frameTime = millis() - now;
    if (frameTime < board::timing::FRAME_INTERVAL_MS) {
        delay(board::timing::FRAME_INTERVAL_MS - frameTime);
    }
}
