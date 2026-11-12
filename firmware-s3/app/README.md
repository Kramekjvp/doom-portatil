# app — firmware único (launcher + Tetris + Doom)

Este é o firmware do console. Os projetos de `examples/` ficam para depurar um jogo isolado.

```
cd firmware-s3\app
idf.py set-target esp32s3
idf.py menuconfig        # "LCD i80", "Entrada: botoes em GPIO direto", "Doom (doomgeneric)"
idf.py build flash
python -m esptool --chip esp32s3 -p COMx write_flash 0x310000 ..\..\wad\doom1.wad
idf.py monitor
```

- Mesma tabela de partições do `examples/doom_standalone` (o WAD gravado vale nos dois).
  Vindo do `tetris_standalone` (outra tabela), rode `idf.py erase-flash` uma vez.
- Menu: CIMA/BAIXO escolhe, A ou START joga. Tetris: SELECT volta ao menu.
  Doom: START+SELECT por 1 s sai; o chip reinicia e o menu volta (o motor não é reentrante).
- SELECT precisa de GPIO (`input_gpio`, padrão -1 = ausente) para sair do Tetris e para o
  strafe/troca de arma do Doom.
- A lista de jogos (`JOGOS[]` em `main/main.c`) é a mesma do simulador (`sim/sim_main.c`).
- Status: validado no simulador e no `host_test`; **ainda não compilado com o ESP-IDF**.
