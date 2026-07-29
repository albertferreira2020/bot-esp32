#!/usr/bin/env python3
"""
pinterest_to_mjpeg.py — pipeline de conversão de vídeo para o robô ESP32-S3.

    Pinterest URL (ou arquivo .mp4 local)
        -> baixa (yt-dlp, se for URL)
        -> corta para 1:1 (centralizado)
        -> redimensiona para 240x240
        -> contraste/gamma/nitidez
        -> 30 FPS
        -> sequência de JPEGs + manifest.json

    Saída pronta pra copiar pra data/videos/<nome>/ e o firmware
    (MjpegPlayer) toca automaticamente.

Requisitos na sua máquina (não no ESP32): ffmpeg + ffprobe no PATH, e
yt-dlp (`pip install -r requirements.txt`) só se for converter direto de URL.

Uso:
    python3 pinterest_to_mjpeg.py <url_ou_arquivo.mp4> <nome> [opções]

Exemplos:
    python3 pinterest_to_mjpeg.py "https://www.pinterest.com/pin/123..." transition
    python3 pinterest_to_mjpeg.py ~/Downloads/chuva.mp4 rain --no-loop

O resultado vai para ../data/videos/<nome>/ (relativo a este script), a
menos que --data-dir aponte outro lugar.
"""
import argparse
import json
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path

FRAME_SIZE = 240
FPS = 30


def die(msg: str) -> None:
    print(f"[erro] {msg}", file=sys.stderr)
    sys.exit(1)


def check_tool(name: str) -> None:
    if shutil.which(name) is None:
        die(
            f"'{name}' nao encontrado no PATH. Instale com 'brew install ffmpeg' "
            f"(inclui ffmpeg e ffprobe)."
        )


def download_if_url(source: str, workdir: Path) -> Path:
    if not source.startswith("http://") and not source.startswith("https://"):
        path = Path(source).expanduser().resolve()
        if not path.is_file():
            die(f"arquivo nao encontrado: {path}")
        return path

    if shutil.which("yt-dlp") is None:
        die(
            "source e uma URL, mas 'yt-dlp' nao esta instalado. "
            "Rode: pip install -r requirements.txt"
        )

    print(f"[1/5] baixando {source} ...")
    out_template = str(workdir / "source.%(ext)s")
    subprocess.run(
        ["yt-dlp", "-f", "mp4/best", "-o", out_template, source],
        check=True,
    )
    downloaded = list(workdir.glob("source.*"))
    if not downloaded:
        die("yt-dlp nao gerou nenhum arquivo de saida")
    return downloaded[0]


def build_filter_chain(trim_border_pct: float) -> str:
    filters = []
    if trim_border_pct > 0:
        # Remove uma margem fixa (bordas pretas do Pinterest) antes do crop quadrado.
        p = trim_border_pct / 100.0
        filters.append(
            f"crop=iw*{1 - 2 * p:.4f}:ih*{1 - 2 * p:.4f}:iw*{p:.4f}:ih*{p:.4f}"
        )
    # Crop central quadrado (1:1) + resize 240x240 + contraste/gamma/nitidez.
    filters += [
        "crop='min(iw,ih)':'min(iw,ih)'",
        f"scale={FRAME_SIZE}:{FRAME_SIZE}:flags=lanczos",
        "eq=contrast=1.18:gamma=1.05:brightness=0.0",
        "unsharp=5:5:0.6:5:5:0.0",
        f"fps={FPS}",
        "format=yuvj420p",
    ]
    return ",".join(filters)


def convert(input_path: Path, out_dir: Path, trim_border_pct: float, duration: float, quality: int) -> int:
    out_dir.mkdir(parents=True, exist_ok=True)
    for old in out_dir.glob("frame_*.jpg"):
        old.unlink()

    filter_chain = build_filter_chain(trim_border_pct)
    pattern = str(out_dir / "frame_%05d.jpg")

    print("[2/5] cortando para 1:1 e centralizando...")
    print("[3/5] aplicando contraste/gamma/nitidez...")
    print(f"[4/5] convertendo para {FPS} FPS + sequencia JPEG...")
    cmd = ["ffmpeg", "-y"]
    if duration and duration > 0:
        cmd += ["-t", str(duration)]
    cmd += [
        "-i", str(input_path),
        "-vf", filter_chain,
        "-q:v", str(quality),
        "-start_number", "1",
        pattern,
    ]
    subprocess.run(cmd, check=True)

    frames = sorted(out_dir.glob("frame_*.jpg"))
    if not frames:
        die("ffmpeg nao gerou nenhum frame — verifique o video de entrada")
    return len(frames)


def write_manifest(out_dir: Path, frame_count: int, loop: bool) -> None:
    manifest = {"fps": FPS, "frames": frame_count, "loop": loop}
    (out_dir / "manifest.json").write_text(json.dumps(manifest, indent=2))
    print(f"[5/5] manifest.json escrito ({frame_count} frames, loop={loop})")


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("source", help="URL do Pinterest ou caminho de um .mp4 local")
    parser.add_argument("name", help="nome da pasta de saida (ex.: transition, rain, boot)")
    parser.add_argument("--data-dir", default=None, help="pasta data/ do firmware (padrao: ../data relativo a este script)")
    parser.add_argument("--no-loop", action="store_true", help="marca manifest.json com loop=false")
    parser.add_argument("--trim-border", type=float, default=0.0, help="%% de margem a remover de cada borda antes do crop quadrado (0-20)")
    parser.add_argument("--duration", type=float, default=5.0, help="segundos a manter do inicio do video (padrao: 5.0, igual a duracao de boot/transicao no firmware; use 0 para o video inteiro)")
    parser.add_argument("--quality", type=int, default=5, help="qualidade JPEG do ffmpeg, 2 (melhor/maior) a 10 (pior/menor). Padrao 5 — bom equilibrio pra caber na flash")
    args = parser.parse_args()

    check_tool("ffmpeg")
    check_tool("ffprobe")

    data_dir = Path(args.data_dir) if args.data_dir else Path(__file__).resolve().parent.parent / "data"
    out_dir = data_dir / "videos" / args.name

    with tempfile.TemporaryDirectory(prefix="pin2mjpeg_") as tmp:
        workdir = Path(tmp)
        input_path = download_if_url(args.source, workdir)
        frame_count = convert(input_path, out_dir, args.trim_border, args.duration, args.quality)
        write_manifest(out_dir, frame_count, loop=not args.no_loop)

    print(f"\nPronto: {out_dir}")
    print("Copie (ou já está) em data/videos/ — o firmware detecta automaticamente")
    print("via 'pio run -t uploadfs'.")


if __name__ == "__main__":
    main()
