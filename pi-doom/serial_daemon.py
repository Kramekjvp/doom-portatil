"""
Le protocolo serial e injeta eventos de teclado via uinput

Depende de pyserial, evdev

Uso: 
    python3 serial_daemon.py /dev/ttyUSB0       #com o esp
    python3 serial_daemon.py /tmp/ttyPI 115200  #simulador

"""
import sys
import serial
from evdev import UInput, ecodes as e
import protocol as p

BUTTON_KEYMAP = {
    p.BUTTON_UP:        e.KEY_UP,
    p.BUTTON_DOWN:      e.KEY_DOWN,
    p.BUTTON_LEFT:      e.KEY_LEFT,
    p.BUTTON_RIGHT:     e.KEY_RIGHT,
    p.BUTTON_ACTION_A:  e.KEY_LEFTCTRL,
    p.BUTTON_ACTION_B:  e.KEY_SPACE,
}

def handle_button(ui: UInput, payload: int) -> None:
    pressed, button_id = p.parse_button_payload(payload)
    key = BUTTON_KEYMAP.get(button_id)
    if key is None:
        print(f"[daemon] botão desconhecido: {button_id}")
        return
    ui.write(e.EV_KEY, key, 1 if pressed else 0)
    ui.syn()
    estado = "pressionado" if pressed else "solto"
    print(f"[daemon] {p.BUTTON_NAMES.get(button_id, button_id)} {estado}")
 
 
def handle_power_off(ser: serial.Serial) -> None:
    print("[daemon] power-off solicitado pelo MCU — iniciando shutdown seguro")
    # TODO (Fase 4): sync + desmontar filesystem antes de confirmar
    ser.write(p.build_shutdown_ack_message())
 
 
def main() -> None:
    if len(sys.argv) < 2:
        print("uso: serial_daemon.py <porta> [baud]")
        sys.exit(1)
 
    port = sys.argv[1]
    baud = int(sys.argv[2]) if len(sys.argv) > 2 else 115200
 
    ser = serial.Serial(port, baud, timeout=1)
    ui = UInput({e.EV_KEY: list(BUTTON_KEYMAP.values())}, name="doom-portatil-controller")
 
    print(f"[daemon] escutando {port} @ {baud}")
 
    try:
        while True:
            raw = ser.read(3)
            result = p.parse_message(raw)
            if result is None:
                if raw:
                    print(f"[daemon] mensagem descartada (tamanho/checksum): {raw.hex()}")
                continue
 
            evt_type, payload = result
 
            if evt_type == p.EVT_BUTTON:
                handle_button(ui, payload)
            elif evt_type == p.EVT_POWER_OFF:
                handle_power_off(ser)
            elif evt_type == p.EVT_SHUTDOWN_ACK:
                print("[daemon] ack de shutdown recebido (inesperado vindo do MCU)")
            else:
                print(f"[daemon] tipo de evento desconhecido: {evt_type:#04x}")
 
    except KeyboardInterrupt:
        print("\n[daemon] encerrando")
    finally:
        ui.close()
        ser.close()
 
 
if __name__ == "__main__":
    main()
