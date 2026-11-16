# CLAUDE.md

Guia curto para continuar este projeto — humano ou IA. Comandos de build e teste **não**
ficam aqui: estão em `firmware/README.md` (evita duas cópias para manter).

## Contexto

Console portátil que roda Doom e Tetris num **ESP32-S3 único** (ADR-0012), tela ILI9341
paralela i80 (ADR-0008; na bancada o painel do shield não respondeu, contingência SPI na
ADR-0013), botões por MCP23017 ou GPIO direto. Disciplina de Sistemas Embarcados;
apresentação em **18/11/2026**. Arquiteturas anteriores (Raspberry Pi; S3 + C3) são
histórico: `docs/historico.md`.

## Onde está cada coisa

- `README.md` — visão geral e mapa do repositório
- `docs/tasks.md` — **fonte de verdade** do que está feito, com nível de prova (escrito / PC / compila / placa)
- `docs/bancada.md` — diário de bancada: o diagnóstico da tela (não repita testes já feitos)
- `docs/requirements.md`, `docs/adr/` — requisitos e decisões (índice em `docs/adr/README.md`)
- `firmware/README.md` — camadas, build, gravação, testes, riscos da primeira compilação
- `hardware/pinos.md` — pinos e pendências de hardware; `hardware/bom.md` — materiais

## Regras do código

- Cada camada só conhece a de baixo. **Jogos nunca leem GPIO nem falam com a tela**: só usam
  `game_api` (`game.h`, `input.h`, `gfx.h`). Só `lcd_i80` e `input_gpio` tocam hardware.
- A lista de jogos existe num lugar só: `components/game_registry`.
- Validar no PC (`host_test`, `sim`) antes de ir à placa; o simulador compila os mesmos fontes.
- Comentários e mensagens em C/Makefile/shell ficam **sem acento** (evita problema de
  codificação em terminais Windows); a documentação `.md` é em português **com** acentos.

## Convenções

- Commits: `tipo: descrição curta` (`feat`, `fix`, `docs`, `chore`, `test`).
- Decisão de arquitetura nova → ADR em `docs/adr/`. ADR substituída fica marcada
  "Superada", não é apagada.
- Item concluído ou novo → atualizar `docs/tasks.md` no mesmo commit, dizendo o **nível de prova**.
- Fins de linha: LF (`.gitattributes`). Não versionar `build/`, `sdkconfig`, WAD nem ROMs.

## Pegadinhas conhecidas

- **Na placa só rodou o firmware sem o Doom, até o launcher (log serial).** Sem imagem na tela;
  o que está "PC" ou "compila" pode falhar no hardware.
- **CIMA e SELECT vêm desligados** (GPIO `-1`) no `input_gpio`: sem GPIO atribuído não há como
  andar para a frente no Doom. Ver `hardware/pinos.md`.
- **Tela paralela (shield):** fiação, alimentação e sinais chegam ao shield, mas **o painel não
  responde** (`docs/bancada.md`). Não culpe o driver `lcd_i80`: o teste por GPIO simples também falha.
- **RD do shield:** com RD baixo os buffers de dados invertem o sentido e brigam com o ESP32
  (~1,5 V no fio). Em qualquer teste de escrita, mantenha o RD **alto**.
- **GPIO 16 desta placa não sobe:** o LCD_D5 foi para o GPIO 12 (`hardware/pinos.md`); o padrão do
  Kconfig ainda diz 16.
- **Pinos do S3 N16R8:** evitar 0, 3, 19, 20, 26–37 (flash/PSRAM octal) e 43–46; 38 e 48 podem ser o LED RGB.
- **Build sem o Doom:** use a **variável de ambiente** `PORTATIL_SEM_DOOM=1` (PowerShell:
  `$env:PORTATIL_SEM_DOOM = "1"`, depois `idf.py reconfigure`). A flag `-D` **não funciona**: o ESP-IDF
  lê os `REQUIRES` antes de ver variáveis do CMake.
- **ESP-IDF 6:** cada componente declara o que usa (`main` precisa de `esp_timer`, `esp_system`,
  `esp_hw_support`, `log`). Erro "X.h: No such file" costuma ser `REQUIRES` faltando.
- **Doom no link:** estoura a DRAM em 31.728 bytes (13/11); falta mover o `.bss` do Doom para a PSRAM.
- **Doom não é reentrante:** depois de sair, só reiniciando o chip. Zona de 6 MB na PSRAM.
- **WAD:** na placa só cabe o `doom1.wad` shareware (~4 MB, partição de 6 MB); o Freedoom (28 MB) só roda no simulador.
- **ESP-IDF:** `sdkconfig.defaults` só vale enquanto não existe `sdkconfig` gerado. Mudou a tabela de
  partições → `idf.py erase-flash`.
- **Áudio (I²S)** costuma conflitar com pinos da tela; é a última camada a entrar (ADR-0005).
- **Walnut-CGB é um fork**, não a biblioteca oficial: manter a cadeia de autoria (Peanut-GB → fork CGB → Walnut-CGB).
- **Submódulo movido:** o doomgeneric está em `third_party/doomgeneric`; o nome interno do submódulo
  (`firmware-s3/third_party/doomgeneric` no `.gitmodules`) é herança do layout antigo e funciona assim.