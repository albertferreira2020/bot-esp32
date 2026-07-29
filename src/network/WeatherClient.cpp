#include "network/WeatherClient.h"

#include <Arduino.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <cstring>

#include "config/Secrets.h"

bool WeatherClient::fetchForecast(WeatherData& out) {
    String url = String("https://api.open-meteo.com/v1/forecast?latitude=") +
                 String(secrets::WEATHER_LATITUDE, 4) + "&longitude=" +
                 String(secrets::WEATHER_LONGITUDE, 4) +
                 "&current=temperature_2m,relative_humidity_2m,wind_speed_10m,weather_code"
                 "&hourly=precipitation_probability&forecast_days=1&timezone=" +
                 secrets::WEATHER_TIMEZONE;

    WiFiClientSecure client;
    client.setInsecure();

    HTTPClient http;
    http.setConnectTimeout(8000);
    http.setTimeout(8000);
    if (!http.begin(client, url)) return false;

    int code = http.GET();
    if (code != HTTP_CODE_OK) {
        Serial.printf("[WeatherClient] HTTP %d\n", code);
        http.end();
        return false;
    }

    // getString() (e não getStream()) de propósito: o Open-Meteo responde com
    // Transfer-Encoding: chunked, e ler o socket cru entrega os prefixos
    // hexadecimais de tamanho de cada chunk junto com o JSON — o parser falha.
    // getString() desfaz o chunking. A resposta tem ~1.1 KB, sem problema.
    String payload = http.getString();
    http.end();

    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, payload);
    if (err) {
        Serial.printf("[WeatherClient] JSON invalido: %s\n", err.c_str());
        return false;
    }

    JsonObject current = doc["current"];
    out.tempC = current["temperature_2m"] | 0.0f;
    out.humidityPct = current["relative_humidity_2m"] | 0;
    out.windKmh = current["wind_speed_10m"] | 0.0f;
    out.weatherCode = current["weather_code"] | 0;

    const char* nowStr = current["time"] | "";
    JsonArray times = doc["hourly"]["time"];
    JsonArray probs = doc["hourly"]["precipitation_probability"];

    int idx = 0;
    for (int i = 0; i < (int)times.size(); ++i) {
        const char* t = times[i];
        idx = i;
        if (t && strcmp(t, nowStr) >= 0) break;
    }
    out.rainProbabilityPct = (idx < (int)probs.size()) ? probs[idx].as<int>() : 0;
    return true;
}

const char* weatherCodeDescription(int code) {
    switch (code) {
        case 0: return "Ceu limpo";
        case 1: return "Poucas nuvens";
        case 2: return "Parcialmente nublado";
        case 3: return "Nublado";
        case 45:
        case 48: return "Neblina";
        case 51:
        case 53:
        case 55: return "Garoa";
        case 61:
        case 63:
        case 65: return "Chuva";
        case 66:
        case 67: return "Chuva congelante";
        case 71:
        case 73:
        case 75: return "Neve";
        case 77: return "Graos de neve";
        case 80:
        case 81:
        case 82: return "Pancadas de chuva";
        case 85:
        case 86: return "Pancadas de neve";
        case 95: return "Trovoada";
        case 96:
        case 99: return "Trovoada com granizo";
        default: return "Tempo indefinido";
    }
}
