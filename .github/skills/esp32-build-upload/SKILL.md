---
name: esp32-build-upload
description:
  "Build and upload this PlatformIO ESP32 project with one slash command. Use
  when the user asks to compile, build, flash, or upload firmware to the
  connected ESP32. Handles busy serial port recovery and can optionally upload
  LittleFS and open monitor."
argument-hint: "[firmware|fs|all|monitor]"
user-invocable: true
---

# ESP32 Build Upload

Use this skill to run PlatformIO commands for this repository with a single chat
invocation.

## When to Use

- User asks to compile and upload to ESP32.
- User asks to flash firmware quickly.
- Serial port may be busy and needs automatic recovery.
- User wants filesystem upload or serial monitor after flashing.

## Modes

- `firmware`: build and upload firmware only.
- `fs`: upload LittleFS image from `data/` only.
- `all`: upload firmware, then upload filesystem.
- `monitor`: open serial monitor only.

Default mode is `firmware` if no argument is provided.

## Procedure

1. Always run commands from repository root.
2. Use `platformio` command (not `pio`) to avoid local alias issues.
3. Environment name is `esp32-s3-lcd-13`.
4. Execute by mode:
   - `firmware`: `platformio run -e esp32-s3-lcd-13 -t upload`
   - `fs`: `platformio run -e esp32-s3-lcd-13 -t uploadfs`
   - `all`:
     `platformio run -e esp32-s3-lcd-13 -t upload && platformio run -e esp32-s3-lcd-13 -t uploadfs`
   - `monitor`: `platformio device monitor -b 115200`
5. If upload fails with busy port/lock error:
   - Detect locker process: `lsof /dev/cu.usbmodem*`
   - Kill the locker PID.
   - Retry the same command.
6. Report final status clearly: build result, upload result, port used, and any
   runtime serial errors.

## Notes

- If runtime logs show `File system is not mounted`, run `all` mode.
- If `all` still shows mount errors, inspect filesystem init and mount logic in
  source code.
