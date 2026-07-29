#pragma once

// NTP + formatação de hora em pt-BR. Nomes de dia/mês sem acentuação de
// propósito: a fonte padrão do LovyanGFX é ASCII/Latin-1 básico.
class TimeManager {
   public:
    struct Now {
        int hour = 0, minute = 0, second = 0;
        int day = 0, month = 0, year = 0;
        int weekday = 0;  // 0 = domingo
    };

    void begin();  // idempotente; chama configTzTime uma vez após WiFi conectar

    static bool synced();
    static Now now();
    static const char* weekdayName(int weekday);
    static const char* monthName(int month1to12);

   private:
    bool began_ = false;
};
