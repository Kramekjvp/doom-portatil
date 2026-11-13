# ADR-0002: doomgeneric como source port

## Status
Aceita. Atualização de 2026-11-12: o alvo é o ESP32-S3 único (ADR-0012) e a tela
é paralela i80, não SPI (ADR-0008).

Consequências revisadas após a ADR-0006 (o alvo de execução
deixou de ser Linux/Raspberry Pi e passou a ser bare-metal no ESP32-S3).

## Contexto
Precisa de um source port do Doom portável, cuja camada de renderização
possa ser trocada conforme a plataforma de destino — antes, entre
SDL (desenvolvimento local) e framebuffer/DRM (Raspberry Pi); agora,
entre SDL (desenvolvimento local) e um backend bare-metal que escreve
direto na tela SPI do ESP32-S3.

## Alternativas consideradas
Chocolate Doom, PrBoom+ (mais completos/fiéis, porém mais acoplados a
SDL/X11, mais trabalho pra portar pra um alvo sem sistema operacional)
vs. doomgeneric (já separa o core do jogo da camada de renderização via
uma interface fina de callback — motivo original da escolha, e o que
torna viável portar pra bare-metal sem reescrever a lógica do jogo).

## Decisão
doomgeneric, mantido.

## Consequências
- O backend de renderização usado durante o desenvolvimento (SDL, no
  notebook) continua válido como ambiente de teste rápido.
- O backend de produção deixa de ser framebuffer/DRM sobre Linux e passa
  a ser um backend bare-metal escrito para o ESP32-S3, escrevendo direto
  no controlador da tela SPI (ILI9341/ST7789, ver ADR-0004) — sem
  `/dev/fbX`, sem driver de kernel, sem sistema operacional embaixo.
- Existe precedente real dessa combinação específica (doomgeneric +
  ESP32-S3 + tela SPI ILI9341), reduzindo o risco de portabilidade.
- A build de referência em Linux/SDL (validada antes da ADR-0006) segue
  útil como ambiente de teste da lógica do jogo, mas deixa de ser o
  caminho de produção.