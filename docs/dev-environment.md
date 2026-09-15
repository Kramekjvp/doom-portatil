# Ambiente de desenvolvimento

Atualizado após a ADR-0006 — não há mais um alvo Linux (Raspberry Pi) no
caminho de produção. Os dois firmwares são bare-metal/FreeRTOS via
ESP-IDF.

## firmware-s3/ (ESP32-S3 — jogos)

- ESP-IDF (toolchain oficial da Espressif) ou PlatformIO com
  `platform = espressif32`, `board` apontando pro módulo N16R8
  (configurar 16MB flash / 8MB PSRAM octal no `menuconfig`/`platformio.ini`).
- Biblioteca de driver de display bare-metal para o painel SPI escolhido
  (ex.: LovyanGFX ou equivalente compatível com ESP-IDF) — ver ADR-0004.
- doomgeneric vendorizado como submódulo, com um backend de renderização
  novo escrito para esse driver (ver ADR-0002).
- Cabo USB-serial (ou USB nativo, a placa N16R8 costuma trazer) pra
  flashar e depurar.

## firmware-c3/ (ESP32-C3 — I/O)

- Mesma toolchain (ESP-IDF/PlatformIO), alvo `esp32c3`.
- ADC interno do C3 pra leitura de tensão da bateria — conferir na
  documentação atual da Espressif se a calibração de fábrica é confiável
  pra esse uso ou se precisa de calibração manual.
- UART pra conversar com o ESP32-S3 seguindo o protocolo (ADR-0003).

## tools/ (referência de protocolo, não roda em produção)

Continua útil como ambiente de teste da lógica do protocolo antes de
portar pra C, sem precisar de nenhum dos dois chips:

```bash
cd tools/tests && python3 test_protocol.py
```

## O que fica obsoleto

- `socat` + par de portas seriais virtuais + `uinput` — eram específicos
  do modelo antigo (daemon Linux injetando teclado num processo SDL).
  Sem Linux no caminho de produção, não se aplicam mais. Testar a
  comunicação real agora significa ligar os dois ESP32 fisicamente (ou
  simular um lado com um terceiro dispositivo serial/USB).
- SDL2/`libsdl2-dev` como *caminho de produção* — continua útil só como
  ambiente de teste rápido da lógica do jogo antes de portar pro backend
  bare-metal.

## Convenção de commits

`tipo: descrição curta` (`feat`, `fix`, `docs`, `chore`, `test`).