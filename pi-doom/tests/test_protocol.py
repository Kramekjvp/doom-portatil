"""
Testes do protocolo serial — só precisam de Python padrão, sem hardware,
sem porta serial, sem uinput. Rodar com: python3 -m pytest tests/ ou
diretamente com python3 tests/test_protocol.py.
"""
import sys
import os

sys.path.insert(0, os.path.join(os.path.dirname(__file__), ".."))

import protocol as p


def test_checksum_is_xor():
    assert p.checksum(0x01, 0x84) == (0x01 ^ 0x84)


def test_build_and_parse_button_pressed():
    msg = p.build_button_message(p.BUTTON_UP, pressed=True)
    result = p.parse_message(msg)
    assert result is not None
    evt_type, payload = result
    assert evt_type == p.EVT_BUTTON
    pressed, button_id = p.parse_button_payload(payload)
    assert pressed is True
    assert button_id == p.BUTTON_UP


def test_build_and_parse_button_released():
    msg = p.build_button_message(p.BUTTON_ACTION_B, pressed=False)
    evt_type, payload = p.parse_message(msg)
    pressed, button_id = p.parse_button_payload(payload)
    assert pressed is False
    assert button_id == p.BUTTON_ACTION_B


def test_all_buttons_roundtrip():
    for button_id in p.BUTTON_NAMES:
        for pressed in (True, False):
            msg = p.build_button_message(button_id, pressed)
            evt_type, payload = p.parse_message(msg)
            got_pressed, got_id = p.parse_button_payload(payload)
            assert got_pressed == pressed
            assert got_id == button_id


def test_power_off_message():
    msg = p.build_power_off_message()
    evt_type, payload = p.parse_message(msg)
    assert evt_type == p.EVT_POWER_OFF
    assert payload == 0x00


def test_shutdown_ack_message():
    msg = p.build_shutdown_ack_message()
    evt_type, payload = p.parse_message(msg)
    assert evt_type == p.EVT_SHUTDOWN_ACK


def test_invalid_checksum_is_rejected():
    bad_msg = bytes([p.EVT_BUTTON, 0x84, 0xFF])  # checksum errado de propósito
    assert p.parse_message(bad_msg) is None


def test_wrong_length_is_rejected():
    assert p.parse_message(bytes([0x01, 0x02])) is None
    assert p.parse_message(bytes([0x01, 0x02, 0x03, 0x04])) is None


if __name__ == "__main__":
    tests = [obj for name, obj in list(globals().items()) if name.startswith("test_")]
    passed = 0
    for test in tests:
        test()
        passed += 1
        print(f"ok  - {test.__name__}")
    print(f"\n{passed}/{len(tests)} testes passaram")