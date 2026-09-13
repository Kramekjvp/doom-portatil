# ADR-0004: Tela SPI (ILI9341/ST7789)

## Status
Aceita

## Contexto
O console precisa de um display para a saída do doomgeneric. Desde o
roteiro original havia duas opções em aberto: tela SPI (ILI9341/ST7789)
ou uma tela HDMI pequena.

## Alternativas consideradas

**Tela HDMI pequena**
Caminho de software mais simples (DRM/KMS padrão da Pi), maior taxa de
quadros garantida, grande suporte da comunidade. Em troca: maior consumo
de energia, maior volume físico (menos portátil) e resolução nativa
tipicamente acima de 320×200, exigindo scaling.

**Tela SPI (ILI9341/ST7789)**
Mais barata, menor consumo (melhor autonomia), menor volume (compatível
com o formato "console de bolso"), resolução nativa próxima da nativa do
Doom (pouco ou nenhum scaling). Exige configurar um driver de framebuffer
via SPI (fbtft/tinydrm no kernel Linux) — mais uma fonte de risco na
Fase 5, mas demonstra trabalho de mais baixo nível, relevante pra
disciplina.

## Decisão
Tela SPI (ILI9341 ou ST7789), priorizando portabilidade, autonomia de
bateria e valor técnico para a disciplina, em troca de um caminho de
software mais arriscado.

## Consequências
- O backend de renderização do doomgeneric (já abstraído, ver ADR-0002)
  passa a apontar para o `/dev/fbX` exposto pelo driver de kernel do
  painel SPI (fbtft/tinydrm) em produção, em vez do backend SDL usado no
  desenvolvimento local — a abstração já prevista torna essa troca
  pontual, sem reescrever o core.
- Mais uma fonte de risco de cronograma na Fase 5 — reservar folga extra
  pra configurar o driver do kernel e validar a taxa de quadros na
  prática.
- Modelo exato do painel (ILI9341 vs ST7789, tamanho, resolução) ainda a
  escolher na hora da compra — verificar compatibilidade com drivers de
  kernel disponíveis antes de fechar o pedido.