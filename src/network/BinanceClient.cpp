#include "network/BinanceClient.h"

#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>

#include "config/Secrets.h"

bool BinanceClient::fetchTicker(BinanceTicker& out) {
    WiFiClientSecure client;
    client.setInsecure();  // API pública, sem necessidade de pin de certificado

    HTTPClient http;
    http.setConnectTimeout(8000);
    http.setTimeout(8000);
    if (!http.begin(client, secrets::BINANCE_TICKER_24H_URL)) return false;

    int code = http.GET();
    if (code != HTTP_CODE_OK) {
        http.end();
        return false;
    }

    String payload = http.getString();
    http.end();

    JsonDocument doc;
    if (deserializeJson(doc, payload)) return false;

    const char* priceStr = doc["lastPrice"];
    const char* changeStr = doc["priceChangePercent"];
    if (!priceStr || !changeStr) return false;

    out.price = atof(priceStr);
    out.changePercent = atof(changeStr);
    return true;
}
