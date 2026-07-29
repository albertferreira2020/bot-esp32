# Robô Holográfico — ESP32-S3-LCD-1.3

Firmware PlatformIO/Arduino para a placa Waveshare-family **ESP32-S3-LCD-1.3**
(ST7789 240x240 SPI, ESP32-S3R8 com 8MB PSRAM octal, 16MB flash, IMU QMI8658
onboard, prisma refrativo acoplado ao display). Interface toda em fundo
`#000000` para preservar a transparência do prisma; arquitetura modular,
orientada a estados, rodando em loop contínuo.

## Hardware

| Sinal | GPIO |
|---|---|
| LCD DC | 38 |
| LCD CS | 39 |
| LCD SCLK | 40 |
| LCD MOSI | 41 |
| LCD RST | 42 |
| IMU SDA | 47 |
| IMU SCL | 48 |
| IMU INT1 | 46 |
| IMU INT2 | 45 |

Pinos confirmados via documentação pública da família de placas (o modelo
com prisma tem inclusive demo oficial "AstronautClock_Prism"). **Sem GPIOs
externos** — tudo onboard, sem fiação adicional, sem cartão SD.
Se o seu exemplar específico divergir em algum pino, ajuste em
[src/config/BoardConfig.h](src/config/BoardConfig.h) — é o único lugar que
precisa mudar.

Não há pino de backlight (BL) dedicado confirmado nesta placa (fica sempre
ligado). Por isso a "economia de energia" do IMU (3min parado → reduz
brilho) é feita por software, escurecendo as cores desenhadas
(`DisplayManager::tint`), não por PWM de backlight. Se o seu exemplar tiver
um pino de BL controlável, defina `LCD_PIN_BL` em `BoardConfig.h` e ele passa
a ser usado automaticamente.

## Build

```bash
cp src/config/Secrets.h.example src/config/Secrets.h
# edite src/config/Secrets.h com seu wifi e sua cidade (Secrets.h está no
# .gitignore — nunca é commitado, o repo é público)

pio run -e esp32-s3-lcd-13
pio run -e esp32-s3-lcd-13 -t uploadfs   # envia data/ (LittleFS) pro dispositivo
pio run -e esp32-s3-lcd-13 -t upload     # flash do firmware
pio device monitor -e esp32-s3-lcd-13
```

**`uploadfs` é um passo separado de `upload` e fácil de esquecer.** Sem
rodar `uploadfs`, a partição LittleFS do dispositivo fica vazia — o
firmware sobe normalmente, mas cai sempre no efeito procedural (glitch/
scanlines) porque não encontra os vídeos de `data/videos/boot|transition`.
Rode `uploadfs` de novo sempre que mudar algo em `data/`. Pra confirmar que
os vídeos foram encontrados, veja o monitor serial no boot: deve aparecer
`[MjpegPlayer] /videos/boot: 155 frames @ 30 fps` (e o mesmo pra
`/videos/transition`). Se aparecer `nao encontrado`, é sinal de que o
`uploadfs` não rodou (ou rodou antes de gerar os frames).

WiFi e localização do clima (`WEATHER_LATITUDE`/`WEATHER_LONGITUDE`, padrão
Belo Horizonte, MG) ficam em `src/config/Secrets.h` — arquivo local, gerado a
partir de `Secrets.h.example`, nunca commitado.

## Arquitetura

```
src/
  config/     BoardConfig.h (pinos/timings), Secrets.h (wifi/apis)
  display/    DisplayManager (LovyanGFX + back-buffer PSRAM + dirty-rect + dimming)
  core/       IAppState, StateMachine (ciclo + transições + preempção wifi), Scheduler (task de rede)
  animation/  Easing, Tween, HoloFX (glitch/scanlines/partículas/halo procedurais)
  robot/      EyeRenderer (desenho do olho), RobotFace (humor/olhar/piscar/respiração)
  media/      SpriteEngine, GifPlayer (AnimatedGIF), MjpegPlayer (TJpg_Decoder)
  network/    WifiManager, BinanceClient, WeatherClient, TimeManager
  imu/        QMI8658 (driver mínimo por registrador)
  power/      PowerManager (liga IMU à economia de energia + wake do robô)
  screens/    Boot/Eyes/Transition/Bitcoin/Weather/Rain/Clock/WifiSearch (cada um = IAppState)
data/         conteúdo do LittleFS (ver data/README.md)
tools/        pinterest_to_mjpeg.py (pipeline de conversão de vídeo)
```

Ciclo (`StateMachine`): `Eyes → Bitcoin → Eyes → Weather → Eyes → Rain (só se
previsão indicar chuva) → Clock → (repete)`, com `TransitionState` (vídeo
holográfico, 5s) sempre entre duas telas de conteúdo (~10s cada). Se o WiFi
cair, `WifiSearchState` preempta na hora e o ciclo retoma em `Eyes` ao
reconectar.

Adicionar um novo fluxo = implementar `IAppState` + registrar um slot no
`switch` de `StateMachine::peekNextContentState()` — o resto (transições,
render, timing) já funciona.

## Vídeos holográficos

`BootState`/`TransitionState`/`RainState` tocam `data/videos/{boot,transition,rain}/`
automaticamente se existirem (via `MjpegPlayer`); senão caem no efeito
procedural equivalente (já incluso, funciona sem nenhum asset).

**`data/videos/boot/` e `data/videos/transition/` já vêm preenchidos** — um
efeito de "matrix digital" e uma hélice de DNA holográfica, ambos convertidos
de vídeos baixados do Pinterest via `tools/pinterest_to_mjpeg.py`, cortados
para 240x240, fundo preto preservado.

`data/videos/rain/` foi deixado vazio de propósito: o vídeo de chuva baixado
tinha um céu cinza-claro ao fundo — ao forçar contraste/gamma pra chegar em
preto puro, as gotas (o próprio assunto do vídeo) desapareciam junto. Nesse
caso o efeito procedural (gotas caindo, já implementado em `RainState`) fica
melhor que o vídeo pra esse conteúdo específico. Pra usar um vídeo de chuva
de verdade, procure uma fonte que já seja escura/holográfica (como as duas
que deram certo), não uma foto realista de janela molhada.

### Pipeline de conversão (`tools/pinterest_to_mjpeg.py`)

Requer `ffmpeg` (`brew install ffmpeg`) e, só se for converter direto de uma
URL, `yt-dlp` (`pip install -r tools/requirements.txt`):

```bash
python3 tools/pinterest_to_mjpeg.py "https://www.pinterest.com/pin/..." transition
python3 tools/pinterest_to_mjpeg.py ~/Downloads/algum_video.mp4 boot --duration 5
```

Corta 1:1 centralizado → 240x240 → contraste/gamma/nitidez → 30 FPS →
sequência `frame_00001.jpg...` + `manifest.json`, direto em
`data/videos/<nome>/`. `--duration 5` (padrão) limita aos primeiros 5s, que é
o quanto `BootState`/`TransitionState` realmente tocam — gera arquivos bem
menores. Use `--duration 0` pra manter o vídeo inteiro (ex.: se quiser um
`rain/` mais longo antes de repetir).

**Escolha a fonte com cuidado**: funciona bem com vídeos que já são
naturalmente escuros/com fundo preto (hologramas, efeitos digitais, partículas
sobre preto). Fontes claras (céu, ambientes bem iluminados) não sobrevivem
bem ao "fundo 100% preto" exigido pelo prisma.

## Limitações conhecidas / próximos passos

- Sem hardware físico à mão nesta sessão para flashar e validar visualmente
  — o firmware compila limpo (`pio run`) e a imagem LittleFS monta dentro do
  orçamento da partição, mas vale um teste real de: offset do painel (se a
  imagem vier deslocada, ajuste `offset_x/offset_y` em
  `DisplayManager.cpp`), orientação (`invert`/`rgb_order`) e o pino de BL.
- Calibração do QMI8658 (escala accel/gyro) é aproximada — suficiente pra
  detectar movimento/parado (o que o `PowerManager` precisa), mas não é
  leitura calibrada em g/dps de precisão.
- `data/icons/` e `data/gifs/` estão vazios — clima/Bitcoin são desenhados
  proceduralmente; dá pra melhorar com sprites reais via `SpriteEngine`.
