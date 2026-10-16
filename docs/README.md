# docs

Índice da documentação. Visão geral e decisões resumidas: `../README.md`.

## O que tem aqui

| Arquivo / pasta | Para que serve |
|---|---|
| `adr/` | Decisões de arquitetura, uma por arquivo. Cada uma registra status, contexto, alternativas consideradas, decisão e consequências. |
| `requirements.md` | Requisitos formais do MVP, derivados do roteiro de produção. |
| `tasks.md` | Quadro de tarefas rastreável, por fase. É a fonte de verdade do que já foi feito. |
| `dev-environment.md` | Toolchain e dependências para reproduzir o ambiente de desenvolvimento. |
| `architecture.svg` | Diagrama de blocos. **Ainda mostra dois chips (S3 + C3) e tela SPI** — desatualizado em relação às ADRs 0008 e ao ESP32-S3 único. |

## Como ler os ADRs

Nome do arquivo: `NNNN-assunto-em-minusculas-com-hifen.md`.
Todo ADR tem uma seção **Status** logo no início: *Proposta*, *Aceita* ou
*Superada pela ADR-NNNN*. ADRs superadas **não são apagadas** — são histórico de
engenharia e entram no relatório final da disciplina.

Ordem sugerida para quem chega agora: 0006 → 0002 → 0005 → 0008 → 0009.
(0001, 0003 e 0004 completam o quadro; 0001 e 0004 estão superadas.)

Não existe ADR 0007 neste repositório.

## Antes de escrever uma ADR nova

1. Use o próximo número livre e o modelo: Status, Contexto, Alternativas
   consideradas, Decisão, Consequências.
2. Se ela substitui outra, marque a antiga como "Superada pela ADR-NNNN" no
   mesmo commit.
3. Atualize a tabela de ADRs do `../README.md` e o `tasks.md`.

Este material é reaproveitado diretamente no relatório final da disciplina.
