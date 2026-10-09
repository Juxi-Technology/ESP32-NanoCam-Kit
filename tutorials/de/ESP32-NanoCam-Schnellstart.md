# ESP32-NanoCam Schnellstart

---

## Vorbereitung

- NanoCam-Kernboard + Basisboard (ESP32-S3 N16R8 + CH340K)
- USB-Type-C-Datenkabel (unterstützt Datenübertragung)
- Computer (Windows / Mac / Linux)
- GC2145-Kameramodul (ab Werk angeschlossen)

---

## Schritt 1: Firmware flashen (3 Minuten)

### Variante A: Ohne Entwicklungsumgebung (empfohlen)

1. Im Browser [esptool-js](https://espressif.github.io/esptool-js/) öffnen
2. NanoCam mit dem Type-C-Kabel an den Computer anschließen
3. Seriellen Anschluss wählen, 115200 Baud
4. Die Firmware-Datei `nanocam_xxx.bin` im entpackten Archiv finden
5. Firmware-Datei `nanocam_xxx.bin` wählen, Adresse `0x0`
6. Auf "START" klicken und auf den Abschluss warten

### Variante B: Kommandozeile (für Fortgeschrittene)

```bash
pip install esptool
esptool.py --chip esp32s3 --port COM3 write_flash 0x0 nanocam.bin
```

---

## Schritt 2: WiFi verbinden (2 Minuten)

NanoCam läuft standardmäßig im **AP+STA-Dualmodus** gleichzeitig — kein Umschalten erforderlich:
- **AP-Hotspot** ist immer aktiv; Smartphone direkt mit `NanoCam-AP` verbinden (Passwort `12345678`), im Browser `http://192.168.4.1` öffnen
- **STA mit Router** erfordert eine einmalige WiFi-Konfiguration:
Mit einem Seriell-Tool (Baudrate **115200 8N1**) über den Type-C-Anschluss des NanoCam verbinden:

```
sta_ssid:dein_WiFi_Name
sta_pd:dein_WiFi_Passwort
```

> Die Antwort `OK` bedeutet, dass die Einstellung erfolgreich war. Nach einer Passwortänderung startet das Gerät automatisch neu.
Falls der WiFi-Modus gewechselt werden soll (normalerweise nicht erforderlich):

| Befehl | Modus | Beschreibung |
|---|---|---|
| `wifi_mode:0` | Nur AP | STA aus, nur Hotspot aktiv |
| `wifi_mode:1` | Nur STA | Hotspot aus, nur Router-Verbindung |
| `wifi_mode:2` | AP+STA | Standard, beide gleichzeitig aktiv |

---

## Schritt 3: Bild öffnen (1 Minute)

1. `sta_ip` über die serielle Schnittstelle senden, um die STA-IP abzurufen
2. Im Browser `http://<IP-Adresse>` eingeben (oder im AP-Modus `http://192.168.4.1`)
3. Die Webseite zeigt das Live-Bild

---

## Schritt 4: KI-Modi nutzen (2 Minuten)

Senden Sie die folgenden Befehle über die serielle Schnittstelle, um den Modus zu wechseln:

| Befehl | Modus | Wirkung |
|---|---|---|
| `ai_mode:0` | Normale Videoübertragung | Live-MJPEG-Bild |
| `ai_mode:1` | Katzengesichtsdetektion | Erkennungsrahmen für Katzengesichter erscheint im Bild |
| `ai_mode:2` | Gesichtsdetektion | Erkennungsrahmen für Gesichter erscheint im Bild |
| `ai_mode:3` | Farberkennung | Farbe markieren→Echtzeit-Verfolgung |
| `ai_mode:4` | Gesichtserkennung | Registrieren→Identifizieren→Löschen |
| `ai_mode:5` | QR-Code-Scan | Auf QR-Code richten→Ausgabe des Inhalts über die serielle Schnittstelle |
| `ai_mode:6` | LLM-Agent | Sprachaktivierung `你好小智` (XiaoZhi AI) |
| `ai_mode:7` | ESP-Claw | ESP-Claw AI Agent (offizielles Espressif-Framework) |

> Nach jedem Moduswechsel ist ein manueller Neustart erforderlich — drücken Sie die RST-Taste des Moduls; nach dem Neustart ist der neue Modus aktiv.

---

## Schritt 5: In Ihr Projekt integrieren

### Arduino-Steuerung

```cpp
Serial.begin(115200);
Serial.print("ai_mode:2");  // Zur Gesichtsdetektion wechseln
```

### Python-Steuerung

```python
import serial
ser = serial.Serial("COM3", 115200)
ser.write(b"ai_mode:1\r\n")  # Zur Katzengesichtsdetektion wechseln
```

### Vollständige Befehlsreferenz

→ Handbuch zum seriellen Protokoll

---

## Häufige Fragen

| Problem | Lösung |
|---|---|
| Flashen fehlgeschlagen | Prüfen, ob das Type-C-Kabel Datenübertragung unterstützt; S2 (BOOT) am Basisboard gedrückt halten und dann einschalten |
| Kein Bild sichtbar | `sta_ip` über die serielle Schnittstelle senden und IP prüfen; prüfen, ob sich beide im selben Subnetz befinden |
| Kamera ohne Funktion | Prüfen, ob das FPC-Flachkabel mit den Metallkontakten nach unten fest eingesteckt ist; PWDN(IO12)/RESET(IO14) prüfen |
| WiFi-Verbindung fehlgeschlagen | `wifi_reset` senden, um die Werkseinstellungen wiederherzustellen, dann neu konfigurieren |

Weitere Fragen → [FAQ](https://FAQ.md)

---

## Nächste Schritte

- 📖 Handbuch zum seriellen Protokoll — vollständige AT-Befehlsreferenz
- 🎓 Tutorial-Übersicht — aufbauendes Tutorial mit 28 Kapiteln
- 🔧 Hardware-Spezifikation — vollständige GPIO-Pinbelegung
- 🤖 ROS2-Integrationsanleitung — micro-ROS-Tutorial
