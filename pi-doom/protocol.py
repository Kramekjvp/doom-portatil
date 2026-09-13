"""
Protocolo serial MCU -> Pi
Camada serial_daemon.py importa este modulo.

Formato Mensagem:
    byte 1 - tipo do evento
    byte 2 - payload
    byte 3 - checksum

Tipo de evento:
    EVT_BUTTON          (0x01) - payload = (pressed << 7) | button_id
    EVT_POWER_OFF       (0x02) - payload não usado (0x00)
    EVT_SHUTDOWN_ACK    (0x03) - payload não usado (0x00)

Botoes (button_id, 4 bits baixos de payload):
    0 = UP, 1 = DOWN, 2 = LEFT, 3 = RIGHT, 4 = Action A, 5 = Action B
"""

EVT_BUTTON = 0x01
EVT_POWER_OFF = 0x02
EVT_SHUTDOWN_ACK = 0x03

BUTTON_UP = 0
BUTTON_DOWN = 1
BUTTON_LEFT = 2
BUTTON_RIGHT = 3
BUTTON_ACTION_A = 4
BUTTON_ACTION_B = 5

BUTTON_NAMES = {
    BUTTON_UP: "UP",
    BUTTON_DOWN: "DOWN",
    BUTTON_LEFT: "LEFT",
    BUTTON_RIGHT: "RIGHT",
    BUTTON_ACTION_A: "ACTION_A",
    BUTTON_ACTION_B: "ACTION_B",
}

def checksum(byte1: int, byte2: int) -> int:
    return byte1 ^ byte2   


def build_button_message(button_id: int, pressed: bool) -> bytes:
    payload = (0x80 if pressed else 0x00) | (button_id & 0x0F)
    return bytes([EVT_BUTTON, payload, checksum(EVT_BUTTON, payload)])


def build_power_off_message() -> bytes:
    return bytes([EVT_POWER_OFF, 0x00, checksum(EVT_POWER_OFF, 0x00)])


def build_shutdown_ack_message() -> bytes:
    return bytes([EVT_SHUTDOWN_ACK, 0x00, checksum(EVT_SHUTDOWN_ACK, 0x00)])


def parse_button_payload(payload: int):
    pressed = bool(payload & 0x80)
    button_id = payload & 0x0F
    return pressed, button_id


def parse_message(raw: bytes):
    if len(raw) != 3:
        return None

    evt_type, payload, received_checksum = raw [0], raw[1], raw[2]

    if checksum(evt_type, payload) != received_checksum:
        return None

    return evt_type, payload