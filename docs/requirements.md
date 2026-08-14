# Requisitos

Derivados do MVP definido no roteiro de produção do projeto.

## Funcionais (MVP)

| ID | Requisito | Critério de aceite |
|---|---|---|
| RF01 | A Raspberry Pi deve executar o Doom via framebuffer/DRM, sem ambiente desktop. | Doom roda no boot direto para a tela, sem X11/Wayland carregado. |
| RF02 | O jogo deve ser controlável integralmente via D-pad + 2 botões de ação, através do microcontrolador. | Joga-se Doom inteiramente pelos botões físicos, sem teclado. |
| RF03 | MCU e Pi devem se comunicar via protocolo serial que traduza eventos de botão em entradas de jogo (ver ADR 0003). | Eventos de botão do MCU aparecem corretamente decodificados no daemon da Pi. |
| RF04 | O sistema deve ser alimentado por bateria com autonomia mínima testada. | Autonomia mínima validada por teste de uso contínuo (valor a definir na Fase 6). |
| RF05 | O sistema deve suportar desligamento seguro sem corromper o cartão SD. | 10 ciclos consecutivos de liga/desliga sem corrupção do sistema de arquivos. |

## Desejáveis (fora do MVP)

| ID | Requisito |
|---|---|
| RD01 | Indicador de carga da bateria (LED ou ícone na tela). |
| RD02 | Áudio estéreo com amplificador dedicado. |
| RD03 | Case impresso em 3D. |
| RD04 | Save states / menu customizado de boot. |

Um requisito desejável só entra em desenvolvimento se as fases anteriores
do roteiro estiverem fechadas com folga no cronograma.
