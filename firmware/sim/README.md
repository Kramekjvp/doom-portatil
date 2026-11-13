# sim — console no PC (Doom e Tetris, sem hardware)

Roda os mesmos jogos da placa sobre `game_api`; só tela e botões são trocados
(decisão: `docs/adr/0010-doom-no-s3-e-simulador-no-pc.md`). A lista de jogos vem de
`components/game_registry`, a mesma da placa.

## Windows (MSYS2, terminal UCRT64)
```
pacman -S mingw-w64-ucrt-x86_64-gcc make mingw-w64-ucrt-x86_64-SDL2
git submodule update --init --recursive     # doomgeneric (third_party/, na raiz do repositório)
cd firmware/sim
make
./build/portatil_sim.exe --wad C:/caminho/doom1.wad                 # launcher: o menu da placa (Tetris e Doom)
./build/portatil_sim.exe --game doom --wad C:/caminho/doom1.wad     # so o Doom
./build/portatil_sim.exe --game tetris                              # so o Tetris
```
Sem SDL2: `make headless` (só modo `--headless`). `make clean` limpa tudo.

## Teclado
setas = direcional · **Z** = A (atirar) · **X** = B (usar) · **Enter** = START ·
**Shift direito** ou **Backspace** = SELECT · **Esc** = sair do simulador.
No launcher: CIMA/BAIXO escolhe, A ou START joga; ao sair do Doom o simulador avisa que o chip
reiniciaria e encerra (rode de novo).
No Doom: SELECT segurado + ←/→ = strafe; toque em SELECT = próxima arma;
START+SELECT por 1 s = sair (como na placa).

## Sem janela (testes e capturas)
```
build/portatil_sim --game doom --wad doom1.wad --headless --frames 420 \
  --script "10:*A 30:*A 50:*A 70:*A 150:+UP 230:-UP" --shot 140 --shot 419
```
Roteiro: `quadro:+BOTAO` aperta, `-` solta, `*` toque (8 quadros). Botões: UP DOWN LEFT
RIGHT A B START SELECT. Capturas em `shots/*.ppm`. No fim imprime pixels/quadro enviados ao
"LCD" e o teto de quadros/s do barramento i80 (só transferência de pixels).
O menu do Doom aceita A cinco vezes seguidas até iniciar a partida (título → menu → New
Game → episódio → dificuldade).

## O que o simulador NÃO diz
Desempenho e memória do ESP32-S3. Ele valida lógica, controles, cores, proporção e o
volume de dados por quadro. WAD de teste livre: Freedoom (`freedoom1.wad`, 28 MB) — só no PC;
na placa só cabe o `doom1.wad` shareware.