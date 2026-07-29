#pragma once

struct BinanceTicker {
    double price = 0.0;
    double changePercent = 0.0;
};

// Uma única chamada a /ticker/24hr: dá preço e variação de 24h juntos.
class BinanceClient {
   public:
    bool fetchTicker(BinanceTicker& out);
};
