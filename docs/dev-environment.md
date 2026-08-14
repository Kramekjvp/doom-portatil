# Ambiente de desenvolvimento

## pi-doom/ (source port + daemon)

- Linux (nativo ou WSL) para o desenvolvimento e build local antes de existir
  hardware — ver ADR-0002.
- Toolchain de build C: `gcc`/`make`.
- Backend de teste: SDL2 (`libsdl2-dev` no Debian/Ubuntu) para rodar o
  doomgeneric no notebook antes de portar para framebuffer/DRM da Pi.
- `python3` + `socat` para simular o MCU via porta serial virtual, sem
  hardware físico.
- Na Pi (mais adiante): sistema mínimo sem desktop, acesso a
  `/dev/fb0` ou DRM/KMS.

## mcu-firmware/ (ESP32)

- PlatformIO (CLI ou extensão do VS Code) com `framework = arduino`,
  `board = esp32dev` (ajustar conforme a placa específica escolhida).
- Cabo USB-serial para flashar e depurar o ESP32.
- Terminal serial (ex: `platformio device monitor`, `minicom` ou `screen`)
  para inspecionar manualmente as mensagens do protocolo (ver ADR-0003).

## Versões

A preencher conforme forem fixadas (versão do doomgeneric usada como base,
versão do PlatformIO, versão do framework Arduino para ESP32) — importante
registrar aqui assim que o ambiente estiver montado, para reprodutibilidade.

## Convenção de commits

`tipo: descrição curta`, tipos usados: `feat`, `fix`, `docs`, `chore`,
`test`. Ex: `feat: daemon uinput lê protocolo serial`.
