#!/usr/bin/env python3
"""
Simula o MCU enviando eventos de botão pelo protocolo (protocol.py), pra
testar o serial_daemon.py sem hardware nenhum.

Pré-requisito: um par de portas seriais virtuais criado com socat:
    socat -d -d pty,raw,echo=0,link=/tmp/ttyMCU pty,raw,echo=0,link=/tmp/ttyPI

O daemon escuta /tmp/ttyPI e este script escreve em /tmp/ttyMCU — os dois
lados enxergam como se fosse uma porta serial de verdade.

Depende de: pyserial
    pip install pyserial

Uso:
    python3 tools/simulate_mcu.py /tmp/ttyMCU
"""
import sys
import time

import serial

sys.path.insert(0, __file__.rsplit("/tools/", 1)[0])
import protocol as p


def main() -> None:
    if len(sys.argv) < 2:
        print("uso: simulate_mcu.py <porta>")
        sys.exit(1)

    ser = serial.Serial(sys.argv[1], 115200, timeout=1)
    time.sleep(0.5)  # dá tempo do outro lado abrir a porta

    # Pressiona e solta cada botão, um de cada vez
    for button_id, name in p.BUTTON_NAMES.items():
        msg = p.build_button_message(button_id, pressed=True)
        ser.write(msg)
        print(f"[sim] {name} pressionado -> {msg.hex()}")
        time.sleep(0.2)

        msg = p.build_button_message(button_id, pressed=False)
        ser.write(msg)
        print(f"[sim] {name} solto -> {msg.hex()}")
        time.sleep(0.2)

    # Mensagem com checksum propositalmente inválido, pra testar o descarte
    bad_msg = bytes([p.EVT_BUTTON, 0x84, 0xFF])
    ser.write(bad_msg)
    print(f"[sim] enviado de propósito com checksum inválido -> {bad_msg.hex()}")

    ser.close()


if __name__ == "__main__":
    main()