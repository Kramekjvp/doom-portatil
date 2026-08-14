# ADR 0002 — Source port do Doom: doomgeneric

## Status
Aceita

## Contexto
A tela final do console (SPI ou HDMI pequena) ainda não estava definida no
momento de começar o código. Era necessário um source port cuja camada de
renderização pudesse ser trocada sem alterar a lógica do jogo, para não
travar o desenvolvimento na decisão de hardware de display.

## Alternativas consideradas
- **Chocolate Doom** / **PrBoom+** — mais recursos prontos (menus, configs,
  compatibilidade), porém mais acoplados ao ambiente de desktop tradicional,
  exigindo mais trabalho para portar para um backend de framebuffer/DRM
  mínimo.
- **doomgeneric** — separa explicitamente o core do jogo da camada de
  desenho, pensado para portabilidade em plataformas embarcadas.

## Decisão
doomgeneric, com o backend de renderização implementado como módulo
isolado (interface simples de "desenhar frame"), permitindo trocar entre
um backend de teste (SDL, para desenvolvimento no notebook), framebuffer/
DRM da Pi e, futuramente, o driver da tela final.

## Consequências
- A escolha entre tela SPI e HDMI pode continuar em aberto sem bloquear o
  desenvolvimento do restante do jogo.
- doomgeneric tem menos recursos "de fábrica" (menus, configurações) que
  Chocolate Doom/PrBoom+ — aceitável, pois esses itens não fazem parte do
  MVP.
