# Lista de componentes (BOM)

Atualizada em 2026-11-12 (ADR-0012: ESP32-S3 único). Os preços são as estimativas
originais; confira os valores reais pagos.

## MVP — Doom funcionando no hardware

| Item | Preço estimado | Onde comprar |
| :--- | :--- | :--- |
| ESP32-S3 N16R8 DevKitC | R$ 85–120 | Mercado Livre, Curto Circuito, Usinainfo, SmartKits |
| Shield TFT 2,4" ILI9341, paralelo de 8 bits (já em mãos, ADR-0008) | — | — |
| Expansor de I/O MCP23017 (I²C) — botões | a preencher | — |
| Bateria LiPo (1000–2000 mAh) — *depende da decisão de RF04* | R$ 30–60 | Mercado Livre, lojas de drone/RC |
| Módulo TP4056 (carregador) — *idem* | R$ 5–15 | Qualquer loja de eletrônica |
| Regulador de tensão | R$ 10–25 | Idem |
| 8 botões táteis (D-pad + A/B/Start/Select) | R$ 8–20 | Idem (o kit todo) |
| Botão/chave física de power | R$ 3–10 | Idem |
| Resistores (divisor de tensão da bateria — *depende de RF04*) | ~R$ 2 | Idem |
| Cabo USB-C | R$ 15–30 | Idem |
| Protoboard/perfboard, fios, conectores | R$ 30–60 | Idem |

O ESP32-C3 e a tela SPI saíram do BOM (ADR-0012 e ADR-0008). O subtotal anterior
(R$ 280–500) incluía os dois: recalcule com os preços reais.

## Extensão condicional (Game Boy, áudio, armazenamento)

| Item | Preço estimado | Onde comprar |
| :--- | :--- | :--- |
| Amplificador MAX98357A (I²S) | R$ 20–40 | AliExpress / conferir lojas nacionais |
| Alto-falante pequeno 3–4 Ω, 3 W | R$ 10–25 | Mercado Livre |
| Leitor de cartão microSD (SPI) — *o shield da tela já tem slot; conferir pinos antes de comprar módulo* | R$ 10–20 | Mercado Livre, Curto Circuito |
| Cartão microSD (8–32 GB) | R$ 25–40 | Qualquer loja |
