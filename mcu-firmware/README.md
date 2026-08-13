# mcu-firmware

Firmware do ESP32 (PlatformIO + framework Arduino):

- Leitura de botões (D-pad + 2 botões de ação) com debounce.
- Leitura de tensão da bateria via ADC.
- Lógica de detecção do botão de power e shutdown seguro.
- Envio dos eventos pela UART seguindo o protocolo definido em
  `docs/adr/` e testado contra o daemon de `pi-doom/`.

Ainda vazio — próximo passo.
