# data/ — conteúdo do LittleFS

Tudo aqui é gravado na partição `littlefs` da flash (~11MB) via
`pio run -t uploadfs`. O firmware funciona sem nenhum arquivo aqui (cai nos
efeitos procedurais); adicionar assets só melhora a experiência.

## videos/<nome>/ — sequências MJPEG

Cada pasta é um "vídeo": uma sequência de JPEGs 240x240 numerados,
`frame_00001.jpg`, `frame_00002.jpg`, ... a 30 FPS, mais um `manifest.json`:

```json
{ "fps": 30, "frames": 150, "loop": false }
```

Gerado automaticamente por `tools/pinterest_to_mjpeg.py` (veja o README raiz).
Nomes reconhecidos pelo `TransitionState`/`RainState`: `videos/transition/` e
`videos/rain/`. Se a pasta não existir, o efeito procedural equivalente é
usado (glitch/scanline para transição, chuva procedural para o rain screen).

## gifs/

GIFs animados (ex.: ícones de clima, logo do Bitcoin) tocados pelo
`GifPlayer` (AnimatedGIF). Sem restrição de nome — cada tela referencia o
arquivo que espera (ver comentário no topo de cada `screens/*State.cpp`).

## icons/

Sprites estáticos (PNG convertido para RGB565 bruto — ver script de conversão
no README raiz) usados pelo `SpriteEngine` fora de contexto de animação.

## fonts/

Fontes customizadas (formato vlw/u8g2), opcional — sem nenhuma aqui, usa a
fonte padrão do LovyanGFX.
