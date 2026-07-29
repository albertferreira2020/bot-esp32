#pragma once

struct WeatherData {
    float tempC = 0.0f;
    int weatherCode = 0;
    int humidityPct = 0;
    float windKmh = 0.0f;
    int rainProbabilityPct = 0;  // próxima hora
};

// Uma única chamada ao Open-Meteo: current + hourly (probabilidade de chuva)
// numa query só.
class WeatherClient {
   public:
    bool fetchForecast(WeatherData& out);
};

// Texto em pt-BR pro código WMO (weather_code) do Open-Meteo.
const char* weatherCodeDescription(int code);
