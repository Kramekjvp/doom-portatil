# ADR-0005: Suporte a múltiplos jogos (extensão em camadas, pós-Doom)

## Status
Proposta — revisada após a ADR-0006 (arquitetura ESP32-S3 + ESP32-C3; desde a
ADR-0012, S3 único). O launcher foi concretizado na ADR-0011.
A estrutura básica (launcher + abstração de input) entra cedo, junto do
Doom; o conteúdo dos jogos adicionais é condicional ao tempo disponível.

## Contexto
Vontade de tornar o console mais genérico, somando ao Doom pelo menos
mais dois jogos: um título antigo de Game Boy (ex.: Pokémon, a partir de
ROM extraída de cartucho próprio) e Tetris. Um documento de arquitetura
recebido propôs tratar isso como parte do sistema principal desde o
início (launcher com todos os jogos, áudio e SD já na primeira versão);
esta ADR decide, em vez disso, tratar a estrutura como parte do núcleo e
o conteúdo dos jogos como camadas condicionais, seguindo o princípio de
controle de escopo já usado no resto do projeto.

## Alternativas consideradas

**RetroArch + core libretro** — solução completa, mas interface própria
pesada e mais dependências; foge do estilo enxuto do resto do projeto.

**Peanut-GB (DMG apenas)** — biblioteca single-header em C99, leve,
comprovadamente rápida até em microcontroladores (roda em tempo real num
Raspberry Pi Pico). Só suporta Game Boy original (DMG); suporte a Game
Boy Color é "trabalho preliminar" na biblioteca oficial.

**Walnut-CGB** — reescrita do Peanut-GB com suporte completo a Game Boy
Color, com otimizações específicas pra ESP32-S3, usada de verdade no
firmware de um handheld baseado em ESP32-S3 (M5Stack Cardputer). Suporte
a CGB mais maduro que o Peanut-GB oficial, mas é um fork com comunidade
menor que a biblioteca original — mais risco de bugs de borda.

**Tetris: ROM de Game Boy vs. clone open-source** — rodar o cartucho
original reaproveitaria o mesmo emulador, mas mantém dependência de ROM
comercial. Um clone de código aberto rodando direto no framebuffer evita
essa questão por completo.

## Decisão

- **Estrutura (entra cedo, junto do Doom):** um launcher simples e
  próprio (não um frontend completo tipo EmulationStation) e uma camada
  de abstração de input (GPIO → ação virtual → jogo), construídos desde
  o início mesmo que só exista um jogo (Doom) pra selecionar no início.
  Evita retrabalho de arquitetura quando os outros jogos entrarem.
- **Game Boy — sequência em duas etapas:**
  1. Peanut-GB (DMG), pelos mesmos motivos do `doomgeneric` — leve,
     comprovado, poucas dependências. Cobre o Pokémon original
     (Vermelho/Azul/Amarelo, todos DMG).
  2. Se sobrar tempo depois do DMG funcionando de ponta a ponta: trocar
     para o Walnut-CGB como upgrade incremental (API derivada do
     Peanut-GB), habilitando Game Boy Color.
- **Tetris:** clone de código aberto rodando direto no framebuffer,
  evitando depender de ROM comercial para esse jogo.
- **Pokémon (ou qualquer jogo comercial):** ROM extraída de um cartucho
  de propriedade do usuário (dump legal) — nunca de fonte de terceiros
  na internet. Origem da ROM é responsabilidade de quem for rodar o
  console, fora do escopo técnico desta ADR.
- **Áudio:** tratado como camada final, não pré-requisito — projetos
  parecidos (Doom em ESP32-S3 com tela SPI) relataram conflito de pinos
  I2S × SPI; o launcher e os jogos devem funcionar perfeitamente sem som.

## Consequências
- Reaproveita o protocolo serial (ADR-0003) sem mudanças — o ESP32-C3
  não sabe nem precisa saber qual jogo está rodando do outro lado.
- Cada jogo implementa seu próprio backend de vídeo seguindo o mesmo
  padrão de "desenhar num framebuffer" usado no Doom (ADR-0002).
- Se Walnut-CGB for adotado, citar corretamente a cadeia de autoria
  (Peanut-GB de deltabeard → fork CGB → Walnut-CGB) na documentação,
  mesmo padrão de rigor já usado no resto do projeto.
- Estrutura de pastas proposta (dentro de `firmware-s3/`, ver ADR-0006):
  ```
  firmware-s3/
  ├── launcher/           # menu que lista e inicia o jogo escolhido
  ├── games/
  │   ├── doom/           # doomgeneric
  │   ├── gameboy/        # Peanut-GB, depois Walnut-CGB
  │   └── tetris/         # clone open-source
  ├── input/               # abstração GPIO/protocolo → ação virtual
  ├── graphics/            # backend de framebuffer compartilhado
  └── audio/               # entra por último
  ```
- Ponto de checagem no meio do prazo: se Doom + launcher + input
  estiverem funcionando de verdade no hardware, seguem pro Game Boy; se
  não, o que não coube vira "trabalho futuro" no relatório, sem
  prejuízo pro que já foi entregue — mesma lógica de risco do roteiro
  original.