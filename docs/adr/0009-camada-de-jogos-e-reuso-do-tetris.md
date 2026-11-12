# ADR-0009: Camada de jogos (contratos) e reuso do núcleo do Tetris

## Status
Proposta — contratos validados só no PC; falta validar com o Doom e na
placa (ver "Pendências de validação").

Data: 2026-10-16. Relacionada à ADR-0008 (tela i80) e detalha a estrutura de
pastas da ADR-0005 (launcher + múltiplos jogos).

## Contexto
O roadmap começa por **launcher e abstração de entrada**, depois Doom, Game Boy e
áudio. O projeto terá vários "jogos" (Tetris, Doom, Peanut-GB) que precisam de tela,
botões e tempo, mas sem conhecer o hardware.

A dupla entregou um Tetris (`Tetris_ESP32`) em PlatformIO + Arduino + TFT_eSPI, com tela
SPI e botões em GPIO direto. O código se divide em:

- `tetris.c/h` (177 + 47 linhas): regras em C puro, sem Arduino nem tela;
- `main.cpp` (364 linhas): desenho, botões, menu e laço, presos a Arduino/TFT_eSPI.

Análise do núcleo: 7-bag, pontuação (100/300/500/800 × nível) e gravidade corretas;
~1 milhão de ações aleatórias em 3.000 partidas sem violar invariantes. Única falha
encontrada: a peça I deitada no chão não girava (sem chute vertical).

## Alternativas consideradas
| Alternativa | Por que não |
|---|---|
| Manter Arduino como componente do ESP-IDF | Acoplamento a versões do IDF (ver ADR 0008). |
| Reescrever o Tetris do zero | O núcleo já está correto e testado; só falta a camada de tela e entrada. |
| Cada jogo ser dono do próprio laço | O launcher precisa de entrada, transição e saída uniformes entre jogos. |
| Jogos lendo GPIO/tela diretamente | Impede trocar SPI/i80 ou GPIO/MCP23017, e impede testar no PC. |

## Decisão
1. **Três contratos** no componente `game_api`, sem dependência de hardware:
   - `game_ops_t` (`game.h`): `init`, `tick(game_ctx_t*)`, `deinit`. O `tick` desenha e
     devolve `false` para voltar ao launcher.
   - `input.h`: o driver só entrega uma máscara de botões; bordas (apertou/soltou) e
     repetição automática ficam em código comum e testado.
   - `gfx.h`: backend mínimo (`gfx_fill_rect`, `gfx_blit`, largura/altura) implementado
     pelo driver da tela; texto 5x7 e retângulo arredondado ficam em código portátil.
2. **O launcher é dono do laço de quadros** (~60 quadros/s): lê a entrada, monta o
   `game_ctx_t` (tempo, `dt`, entropia) e chama `tick`. Jogos não chamam `millis()`,
   `esp_random()` nem leem GPIO.
3. **Reaproveitar o núcleo da dupla** como componente `tetris_core`, com uma alteração
   mínima: chute vertical de até 2 linhas no giro. Autoria registrada nos cabeçalhos.
   Tela e fluxo foram reescritos em `tetris_game` sobre os contratos acima.
4. **Backends trocáveis:** entrada por GPIO direto agora (`input_gpio`, bancada) e por
   MCP23017 depois, sem mudar nenhum jogo.
5. **Testes no PC** (`host_test`): backend gráfico falso (framebuffer em memória),
   testes das regras, da entrada e do fluxo completo, com capturas de tela.

6. **Relação com a estrutura de pastas proposta na ADR-0005.** A ADR-0005
   sugeriu `launcher/`, `games/`, `input/`, `graphics/` e `audio/` dentro de
   `firmware-s3/`. Aqui cada peça vira um **componente do ESP-IDF** (pasta com
   `CMakeLists.txt`) em `firmware-s3/components/`, o que permite compilar,
   testar e trocar cada camada separadamente:

   | Pasta sugerida (ADR-0005) | Componente (esta ADR) | Situação |
   |---|---|---|
   | `graphics/` | `lcd_i80` (driver) + `gfx.h` em `game_api` (contrato) | feito (falta testar na placa) |
   | `input/` | `input_gpio` (driver) + `input.h` em `game_api` (contrato) | feito; MCP23017 pendente |
   | `games/tetris/` | `tetris_core` (regras) + `tetris_game` (telas) | feito (testado no PC) |
   | `launcher/` | — | a criar (consome `game_ops_t`) |
   | `games/doom/`, `games/gameboy/` | — | a criar, via adaptador para `game_ops_t` |
   | `audio/` | — | por último, como na ADR-0005 |


## Consequências
**Positivas**

- Regras, fluxo e telas do Tetris testáveis sem hardware.
- Trocar tela ou botões afeta um componente, não os jogos.
- Reuso do trabalho da dupla com autoria preservada.

**Negativas / limites conhecidos**

- `gfx_blit` copia os pixels para um buffer DMA de 4 KB: bom para interface e jogos de
  tela pequena, **não** para quadros inteiros. O Doom (framebuffer completo) precisará
  de um caminho próprio de envio à tela, que merece um ADR separado.
- O `tick` pode não encaixar direto em motores com laço próprio (doomgeneric, Peanut-GB).
  A previsão é um adaptador que traduz seus callbacks para `game_ops_t`. **Hipótese a
  validar com o Doom**; se não couber, este ADR deve ser substituído.
- A repetição automática fica quantizada ao quadro (50 ms vira 64 ms a 60 quadros/s).
- Sem debounce por software: a leitura a cada 16 ms filtra a maior parte do repique,
  mas não foi medida com os botões reais.

## Pendências de validação (não verificado ainda)
- [ ] Build do exemplo `tetris_standalone` no ESP-IDF 6.0.3.
- [ ] Tetris rodando na placa (tela + botões), com sensação de controle aceitável.
- [ ] Adaptador `game_ops_t` para o Doom (doomgeneric).
- [ ] Backend de entrada por MCP23017.
- [ ] Validar com a dupla a alteração do giro (chute vertical) no `tetris_core`.

## Referências
- Código: `firmware-s3/components/{game_api,tetris_core,tetris_game,input_gpio}/`.
- Testes: `firmware-s3/host_test/` (`exemplo_telas.png` mostra as telas simuladas).
- Origem do núcleo: `Tetris_ESP32/src/tetris.c` e `tetris.h` (dupla do projeto).
