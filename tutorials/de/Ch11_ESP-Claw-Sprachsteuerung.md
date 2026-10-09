# Kapitel 11: ESP-Claw-Sprachsteuerung

## Über dieses Kapitel

Wenn das Gerät auf `ai_mode:7` umgestellt wird, wechselt der NanoCam in den ESP-Claw-Modus. Er **verwendet dieselbe Firmware** (`nanocam_espclaw/`) wie XiaoZhi AI (`ai_mode:6`); der einzige Unterschied: Der ESP-Claw-Modus registriert zusätzlich zu den Sprachdialogfunktionen 5 Hardware-Steuerungstools.

| Merkmal | XiaoZhi AI (mode 6) | ESP-Claw (mode 7) |
|---|---|---|
| Sprachdialog | ✅ ASR→LLM→TTS | ✅ Dieselbe Sprachpipeline |
| LED-Steuerung | ❌ | ✅ Sprachliche Farbwahl / Ein-/Ausschalten |
| KI-Moduswechsel | ❌ | ✅ Sprachliche Umschaltung |
| Foto + KI-Visionsanalyse | ❌ | ✅ Foto aufnehmen und multimodale KI zur Bildanalyse aufrufen |

## Funktionsprinzip

Der ESP-Claw-Modus registriert über `RegisterMcpTools()` zusätzlich zur Sprachpipeline 5 NanoCam-spezifische Tools:

```
Nutzer-Sprachbefehl "Stell das Licht auf Blau"
  → ASR-Spracherkennung (Cloud)
  → LLM versteht die Absicht → Aufruf self.led.set_color({"r":0, "g":0, "b":255})
  → NanoCam WS2812 LED wird blau
  → TTS: "Okay, das Licht ist jetzt blau"
```

## Schritte

### 12.1 Firmware flashen

ESP-Claw verwendet das eigenständige Firmware-Projekt `nanocam_espclaw/`:

```bash
cd nanocam_espclaw
idf.py set-target esp32s3
idf.py build
idf.py -p COMx flash
```

### 12.2 Modus wechseln

Stellen Sie nach dem Start den ESP-Claw-Modus ein:

```
ai_mode:7
```

Das Gerät startet automatisch neu und wechselt in den Modus. Mit `ai_mode:6` können Sie zum XiaoZhi-AI-Modus zurückkehren.

### 12.3 Beispiele für die Sprachsteuerung

Sagen Sie nach dem Aktivierungswort direkt, was Sie möchten:

```
💬 "Mach das Licht an"            → WS2812 leuchtet weiß
💬 "Stell das Licht auf Blau"     → LED wird blau
💬 "Mach das Licht aus"           → LED aus
💬 "Wechsle in den Gesichtsdetektionsmodus" → ai_mode:2 in NVS speichern + Neustart
💬 "Schau, was hier ist"          → Foto + multimodale KI-Analyse
💬 "Ist eine Tasse vor mir"       → Die multimodale KI erkennt die Szene
```

### 12.4 Foto + KI-Visionsanalyse

Wenn der Nutzer "Schau, ..." sagt, erfasst die Firmware ein VGA-RGB565-Bild, komprimiert es als JPEG und sendet es an die serverseitig konfigurierte multimodale API zur Analyse; das Ergebnis wird per TTS-Sprachausgabe ausgegeben.
> URL und Token der multimodalen API werden vom Server während des Verbindungs-Handshakes automatisch übermittelt; es müssen keine Konfigurationsbefehle manuell über die serielle Schnittstelle eingegeben werden.

## 5 NanoCam-spezifische Tools

| Tool-Name | Funktion | Parameter |
|---|---|---|
| `self.led.set_color` | WS2812-RGB-LED einstellen (GPIO18) | `r,g,b`: 0-255 |
| `self.led.turn_off` | LED ausschalten | Keine |
| `self.camera.set_ai_mode` | KI-Modus wechseln (NVS-Speicherung + Neustart) | `mode`: 0-7 |
| `self.camera.inspect_image` | Foto + multimodale LLM-Visionsanalyse | `prompt`: Problembeschreibung |
| `self.get_device_info` | Geräteinformationen als JSON | Keine |

## Konfigurationsdateien

| Inhalt | Pfad |
|---|---|
| MCP-Tool-Registrierung | `nanocam_espclaw/main/boards/nanocam/nanocam_board.cc` |
| Vision-Sendelogik | `nanocam_espclaw/main/boards/common/esp32_camera.cc` |
| SDK-Standardkonfiguration | `nanocam_espclaw/sdkconfig.defaults` |

> Die ESP-Claw-Firmware ist ein eigenständiges Projekt und teilt keinen Code mit `nanocam_vision`. Beide Firmwares müssen getrennt kompiliert und geflasht werden.

## Wie Sie wählen

| Ihr Bedarf | Empfohlener Modus |
|---|---|
| Nur Sprachchat und Frage-Antwort | Modus 6 (XiaoZhi) |
| Sprachsteuerung der LED | Modus 7 (ESP-Claw) |
| Foto + KI-"Blick" auf die Szene | Modus 7 (ESP-Claw) |
| Sprachliches Umschalten des KI-Erkennungsmodus | Modus 7 (ESP-Claw) |

> Die vollständige Nutzung von ESP-Claw (Serverkonfiguration, Entwicklung eigener MCP-Tools usw.) wird noch erforscht; die Dokumentation wird mit dem Fortschritt der Untersuchungen laufend aktualisiert.
