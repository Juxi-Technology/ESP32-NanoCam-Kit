# Kapitel 9: Sprachdialog (XiaoZhi AI)

## Über dieses Kapitel

Dieses Kapitel behandelt den XiaoZhi-AI-Modus (`ai_mode:6`). **Wichtig**: Modus 6 (Sprachdialog) und Modus 7 (ESP-Claw) **verwenden dieselbe Firmware** (`nanocam_espclaw/`); das Gerät lädt beim Start lediglich je nach dem in der NVS gespeicherten `ai_mode`-Wert einen anderen MCP-Tool-Satz.

| Merkmal | XiaoZhi AI (mode 6) | ESP-Claw (mode 7) |
|---|---|---|
| Sprachdialog | ✅ ASR→LLM→TTS | ✅ Dieselbe Sprachpipeline |
| MCP-Tools | Allgemeine Tools (Lautstärke/Foto usw.) | **Allgemeine Tools + 5 hardwarespezifische Tools** |
| Visuelles Verständnis | `self.camera.take_photo` | **`self.camera.inspect_image`** (multimodale Vision) |
| LED-Steuerung | ❌ | ✅ Sprachliche Farbsteuerung |
| Anwendungsszenarien | Allgemeiner KI-Dialog, Kinderbildung | Hardware-Steuerung, visuelle Inspektion, Smart Home |

> Dieses Kapitel konzentriert sich auf die Kernfunktion Sprachdialog des **XiaoZhi AI (Modus 6)**. Wenn Sie die Hardware-Steuerungsfunktionen von ESP-Claw kennenlernen möchten, lesen Sie Kapitel 11: ESP-Claw-Sprachsteuerung.

## Funktionsprinzip

Der NanoCam integriert das Open-Source-Framework XiaoZhi AI und verbindet sich über WebSocket-/MQTT-Protokolle mit einem LLM-Server für eine vollständige Sprachinteraktions-Pipeline:

```
Nutzer spricht → ES8311-Mikrofonaufnahme → Opus-Kodierung
  → WebSocket → ASR-Spracherkennung in der Cloud
  → LLM generiert die Antwort
  → TTS-Sprachsynthese → Opus-Dekodierung
  → NS4150B-Verstärker → Wiedergabe über Lautsprecher
```

Vollduplex-Design: Der Nutzer kann die KI mitten im Sprechen direkt unterbrechen (Barge-in) — das Erlebnis kommt einem echten Gespräch nahe.

## Hardware-Anforderungen

Dieses Kapitel betrifft Audiofunktionen; folgende Hardware ist erforderlich:
- NanoCam-Kernboard (mit ES8311-Codec + AP2718AT-Mikrofon)
- NanoCam-Basisboard (mit NS4150B-Verstärker + CH340K)
- Lautsprecher (an den Lautsprecheranschluss VON/VOP des Basisboards anschließen)
> Auch nur mit dem Kernboard kann getestet werden (Mithören über den ES8311-Kopfhörerausgang). Das Mikrofon ist ein analoges AP2718AT-MEMS-Siliziummikrofon, das über den DC-Blockkondensator C26 an ES8311 MIC1P angeschlossen ist.

## Schritte

### 10.1 XiaoZhi-AI-Firmware flashen

XiaoZhi AI verwendet das eigenständige Firmware-Projekt `nanocam_espclaw/`:

```bash
cd nanocam_espclaw
idf.py set-target esp32s3
idf.py build
idf.py -p COMx flash monitor
```

Nach dem Start befindet sich das Gerät standardmäßig im XiaoZhi-AI-Modus.

### 10.2 Verbindung mit dem xiaozhi.me-Clouddienst

Der NanoCam verbindet sich ab Werk standardmäßig mit dem offiziellen Clouddienst [xiaozhi.me](https://xiaozhi.me) (kostenlos); ein eigener Server ist nicht erforderlich.
1. Registrieren Sie ein Konto auf [xiaozhi.me](https://xiaozhi.me)
2. Nach dem Einschalten gibt das Gerät automatisch einen 6-stelligen Aktivierungscode aus (Sprachansage)
3. Geben Sie den Aktivierungscode in der xiaozhi.me-Konsole ein → Gerät binden
4. Wählen Sie in der Konsole das LLM-Modell (Qwen / DeepSeek usw.)
Die Aktivierung ist nur einmal erforderlich; danach verbindet sich das Gerät bei jedem Einschalten automatisch.

### 10.3 Erster Dialog

Nach dem Signalton können Sie das Gespräch beginnen:

```
Du: "你好小智, wie ist das Wetter heute?"
NanoCam: "Ich schaue schnell nach dem Wetter von heute..."
```

Das Aktivierungswort ist **`你好小智`** (Standard).

### 10.4 Häufige Dialogszenarien

```
💬 "Erzähl mir einen Witz"          → KI-Sprachantwort
💬 "Stell mir einen 5-Minuten-Wecker" → Wecker-Funktion
💬 "Wie spät ist es"                → Zeitansage
💬 "Spiel leichte Musik ab"         → Musikwiedergabe über das Internet
💬 "Was ist ein schwarzes Loch"     → Wissensfragen
```

## Selbst gehosteter Server (optional)

Wenn Sie Wert auf Datenschutz legen oder ein eigenes LLM verwenden möchten, können Sie den Open-Source-Server von XiaoZhi AI selbst deployen:

```bash
git clone https://github.com/xinnan-tech/xiaozhi-esp32-server
cd xiaozhi-esp32-server
pip install -r requirements.txt
python app.py
```

Die Serveradresse der Firmware wird über das OTA-System (`CONFIG_OTA_URL` in sdkconfig) bereitgestellt; das Gerät fragt die Serveradresse beim Einschalten automatisch ab.
> XiaoZhi AI verwendet den Open-Source-Server von XiaoZhi AI (privates WebSocket-Protokoll + ASR/LLM/TTS-Pipeline). Im ESP-Claw-Modus wird darauf aufbauend die Vision-API-URL und das Token für die visuelle Analyse vom Server während des MCP-Handshakes übermittelt; die Firmware muss nichts selbst konfigurieren.

## Fehlerbehebung

| Symptom | Mögliche Ursache | Lösung |
|---|---|---|
| Kein Ton hörbar | Lautsprecher nicht angeschlossen | Lautsprecheranschluss am Basisboard prüfen |
| Spracherkennung ungenau | Zu laute Umgebungsgeräusche | Näher am Mikrofon sprechen (Abstand < 1m) |
| Keine Verbindung | WiFi nicht konfiguriert | Zuerst per serieller Schnittstelle konfigurieren: `sta_ssid:xxx` |
| Kein Aktivierungscode | Erster Start nicht abgeschlossen | 30 Sekunden warten; das Gerät gibt ihn automatisch aus |
| Antworten sind langsam | Verzögerung des LLM-Servers | Auf xiaozhi.me ein schnelleres Modell wählen oder einen eigenen Server betreiben |
