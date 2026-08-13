# pi-doom

Código que roda na Raspberry Pi:

- Source port do Doom (doomgeneric), com backend de renderização abstraído
  para permitir trocar entre display de teste, framebuffer/DRM e,
  futuramente, o driver da tela final.
- Daemon que lê o protocolo serial vindo do `mcu-firmware/` e injeta os
  eventos de botão no jogo via `uinput`.

Ainda vazio — próximo passo.
