#pragma once
#include <cstdint>

// Conecta e mantém a conexão WiFi sem nunca bloquear: begin() dispara a
// conexão assíncrona; loop() (chamado pela task de rede do Scheduler) só
// confere o status e tenta de novo com um intervalo mínimo se caiu.
class WifiManager {
   public:
    void begin(const char* ssid, const char* password);
    void loop();
    bool isConnected() const;

   private:
    const char* ssid_ = nullptr;
    const char* password_ = nullptr;
    uint32_t nextAttemptAt_ = 0;
};
