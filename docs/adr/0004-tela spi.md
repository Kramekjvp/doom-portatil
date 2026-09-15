# ADR-0004: Tela SPI (ILI9341/ST7789)

## Status
Aceita. Consequências atualizadas após a ADR-0006 — a tela conecta
direto no ESP32-S3, não mais via GPIO da Raspberry Pi.

## Contexto
O console precisa de um display para a saída do doomgeneric (e, no
futuro, dos outros jogos). Decisão original comparou tela SPI contra
HDMI pequena.

## Alternativas consideradas
Tela HDMI pequena (caminho de software mais simples, mas maior consumo,
maior volume físico, resolução acima de 320×200) vs. tela SPI
(ILI9341/ST7789 — mais barata, menor consumo, resolução próxima da
nativa do Doom, exige driver próprio).

## Decisão
Tela SPI (ILI9341 ou ST7789), mantida — priorizando portabilidade,
autonomia de bateria e valor técnico para a disciplina.

## Consequências
- Antes: o driver seria um módulo de kernel Linux (fbtft/tinydrm)
  expondo um `/dev/fbX` na Raspberry Pi. Agora: sem sistema operacional
  no ESP32-S3, o próprio firmware fala o protocolo SPI do controlador
  diretamente (biblioteca de driver de display bare-metal, ex.:
  LovyanGFX ou equivalente para ESP-IDF) — mais controle, mas a
  responsabilidade de performance passa a ser 100% do código do
  projeto, sem um driver de kernel já otimizado por trás.
- O backend de renderização do doomgeneric (ADR-0002) escreve direto
  nesse driver bare-metal, sem passar por framebuffer de sistema
  operacional nenhum.
- Modelo exato do painel (ILI9341 vs ST7789, tamanho, resolução) segue
  em aberto até a compra — verificar biblioteca de driver disponível
  para ESP-IDF antes de fechar o pedido.