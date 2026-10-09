"""
NanoCam Python Library v0.1

Control ESP32-NanoCam via serial AT commands.

Usage:
    from nanocam import NanoCam
    cam = NanoCam("COM3")
    cam.set_wifi("MyWiFi", "password")
    cam.set_ai_mode(1)          # cat face detection
    print(cam.get_sta_ip())
"""

import serial
import time
import json


class NanoCam:
    AI_STREAM     = 0
    AI_CAT_FACE   = 1
    AI_HUMAN_FACE = 2
    AI_COLOR      = 3
    AI_FACE_ID    = 4
    AI_QR_CODE    = 5
    AI_LLM_AGENT  = 6

    WIFI_AP    = 0
    WIFI_STA   = 1
    WIFI_APSTA = 2

    def __init__(self, port: str, baudrate: int = 115200, timeout: float = 2.0):
        self._ser = serial.Serial(port, baudrate, timeout=timeout)

    def _send(self, cmd: str) -> str:
        self._ser.write(f"{cmd}\r\n".encode())
        time.sleep(0.1)
        resp = self._ser.readline().decode().strip()
        return resp

    # ---- WiFi ----
    def set_wifi(self, ssid: str, password: str):
        """Set STA WiFi. Password change triggers reboot."""
        self._send(f"sta_ssid:{ssid}")
        time.sleep(0.5)
        self._send(f"sta_pd:{password}")
        time.sleep(3)

    def set_ap(self, ssid: str, password: str):
        self._send(f"ap_ssid:{ssid}")
        time.sleep(0.5)
        self._send(f"ap_pd:{password}")
        time.sleep(3)

    def set_wifi_mode(self, mode: int):
        self._send(f"wifi_mode:{mode}")
        time.sleep(3)

    # ---- AI Mode ----
    def set_ai_mode(self, mode: int):
        """Switch AI mode (0-6). Triggers reboot."""
        self._send(f"ai_mode:{mode}")
        time.sleep(3)

    # ---- Queries ----
    def get_sta_ip(self) -> str:
        resp = self._send("sta_ip")
        if resp.startswith("sta_ip:"):
            return resp[7:]
        return ""

    def get_version(self) -> str:
        return self._send("wifi_ver")

    def get_info(self) -> dict:
        resp = self._send("nano_info")
        try:
            return json.loads(resp)
        except json.JSONDecodeError:
            return {"raw": resp}

    # ---- Virtual Keys ----
    def key_menu(self):  self._send("KEY_MENU")
    def key_play(self):  self._send("KEY_PLAY")
    def key_up(self):    self._send("KEY_UPUP")
    def key_down(self):  self._send("KEY_DOWN")

    # ---- Control ----
    def reboot(self):
        self._send("nano_reboot")
        time.sleep(3)

    def factory_reset(self):
        self._send("wifi_reset")
        time.sleep(3)

    def close(self):
        self._ser.close()

    def __enter__(self):
        return self

    def __exit__(self, *args):
        self.close()


# ---- CLI ----
if __name__ == "__main__":
    import sys
    if len(sys.argv) < 2:
        print("Usage: python nanocam.py <COM_PORT> [command]")
        print("  python nanocam.py COM3            # show info")
        print("  python nanocam.py COM3 ai_mode:1  # send command")
        sys.exit(1)

    cam = NanoCam(sys.argv[1])
    if len(sys.argv) > 2:
        cmd = " ".join(sys.argv[2:])
        print(f"> {cmd}")
        print(cam._send(cmd))
    else:
        info = cam.get_info()
        print(json.dumps(info, indent=2))
    cam.close()
