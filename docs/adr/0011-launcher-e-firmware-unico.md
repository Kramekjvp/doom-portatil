# ADR-0011: Launcher e firmware único

## Status
Proposta. Numeração: segue a ADR-0010 (ver a observação sobre 0008/0009 nela).
Concretiza o item "launcher" da ADR-0005.

## Contexto
Tetris e Doom já existem como jogos (`game_ops_t`), cada um com seu projeto de teste em
`examples/`. Gravar um apaga o outro (os dois ocupam a partição `factory`). O console
precisa de um firmware único com um menu que escolha o jogo.

## Decisão

**1. O launcher é um `game_ops_t`** (`components/launcher`). O laço de quadros (placa:
`app/main/main.c`; PC: `sim/sim_main.c`) o trata como qualquer jogo. Dentro do `tick()`, ele
mostra o menu e, após a escolha, repassa `init/tick/deinit` ao jogo escolhido. Um `tick()` que
devolve `false` num jogo comum volta ao menu.

**2. O launcher não conhece nenhum jogo.** `launcher_set_entries()` recebe a lista
(`launcher_entry_t { game, once_per_boot }`) de quem monta o firmware. Depende só de
`game_api`, então roda igual na placa, no simulador e no `host_test`.

**3. Jogo "uma vez por boot".** O motor do Doom não é reentrante. Quando um jogo marcado
`once_per_boot` termina, o launcher faz `tick()` devolver `false` e
`launcher_reboot_requested()` vira `true`; quem tem o laço reinicia o chip (`esp_restart()`),
e o menu volta limpo. No simulador, o programa avisa que o chip reiniciaria e encerra.

**4. Projeto `firmware-s3/app/`.** É o firmware do console. Usa a **mesma tabela de partições**
do `doom_standalone` (partição `wad` no offset 0x310000), para o WAD gravado valer nos dois.
Os projetos de `examples/` permanecem para depurar cada jogo isolado.

**5. Interface.** Menu em retrato (240×320): CIMA/BAIXO escolhe, A ou START joga, até 5 jogos
(`LAUNCHER_MAX_ENTRIES`). O menu leva só bordas de botão, então um A segurado não relança o jogo
ao voltar. Antes de iniciar um jogo, a tela é limpa; ao voltar, a orientação é devolvida a retrato.

## Consequências
- Entrar de novo no Doom exige reiniciar (aceito: o menu volta sozinho em poucos segundos).
- A seleção do menu não é lembrada após o reinício (RTC/NVS ficam para depois, se fizer falta).
- Adicionar um jogo = uma linha em `JOGOS[]` nos dois lugares (`app/main/main.c` e
  `sim/sim_main.c`) mais o componente na lista `REQUIRES` do `app/main/CMakeLists.txt`.

## Verificação
- `host_test/test_launcher.c`: navegação (com volta nas pontas), início por A e START, A
  segurado não relança, jogo comum volta ao menu, jogo `once_per_boot` pede reinício e devolve
  a tela a retrato, lista vazia/menor/maior que o limite, fuzz de 200 mil quadros sem pixel
  fora da tela. Os quatro testes do `host_test` passam (0 falhas).
- Simulador: menu → Tetris → SELECT volta ao menu; menu → Doom → START+SELECT sai e pede reinício.
- **Não compilado com o ESP-IDF** (`app/`), nem testado na placa.
