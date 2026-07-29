#include "network/TimeManager.h"

#include <Arduino.h>
#include <ctime>

#include "config/Secrets.h"

namespace {
const char* kWeekdays[] = {"domingo",     "segunda-feira", "terca-feira", "quarta-feira",
                            "quinta-feira", "sexta-feira",   "sabado"};
const char* kMonths[] = {"janeiro", "fevereiro", "marco",     "abril",   "maio",     "junho",
                          "julho",   "agosto",    "setembro", "outubro", "novembro", "dezembro"};
}  // namespace

void TimeManager::begin() {
    if (began_) return;
    configTzTime(secrets::TZ_POSIX, secrets::NTP_SERVER_1, secrets::NTP_SERVER_2);
    began_ = true;
}

bool TimeManager::synced() {
    time_t t = time(nullptr);
    return t > 1'700'000'000;  // depois de 2023 => já sincronizou via NTP
}

TimeManager::Now TimeManager::now() {
    time_t t = time(nullptr);
    struct tm tmInfo;
    localtime_r(&t, &tmInfo);

    Now n;
    n.hour = tmInfo.tm_hour;
    n.minute = tmInfo.tm_min;
    n.second = tmInfo.tm_sec;
    n.day = tmInfo.tm_mday;
    n.month = tmInfo.tm_mon + 1;
    n.year = tmInfo.tm_year + 1900;
    n.weekday = tmInfo.tm_wday;
    return n;
}

const char* TimeManager::weekdayName(int weekday) {
    if (weekday < 0 || weekday > 6) return "";
    return kWeekdays[weekday];
}

const char* TimeManager::monthName(int month1to12) {
    if (month1to12 < 1 || month1to12 > 12) return "";
    return kMonths[month1to12 - 1];
}
