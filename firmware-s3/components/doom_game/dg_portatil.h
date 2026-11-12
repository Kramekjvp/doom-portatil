/* Interno ao componente doom_game: ponte entre doom_game.c e dg_portatil.c. */
#pragma once

void dg_key_push(int pressed, unsigned char key);   /* enfileira evento de tecla para o Doom */
void dg_reset(void);                                 /* esvazia a fila de teclas              */
