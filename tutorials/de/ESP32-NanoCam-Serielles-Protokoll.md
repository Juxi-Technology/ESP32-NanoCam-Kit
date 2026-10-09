# ESP32-NanoCam Handbuch zum seriellen Protokoll

> Baudrate: 115200 | Datenbits: 8 | Parität: keine | Stoppbits: 1 | Flusskontrolle: keine
> Kompatibel mit dem AT-Befehlssatz gängiger Kameramodule; zusätzlich NanoCam-Erweiterungsbefehle.

---

## 1. Allgemeine Regeln

- Befehle sind **nicht case-sensitiv** (`STA_SSID` = `sta_ssid`)
- Nach dem Befehl muss ein **beliebiges ASCII-Satzzeichen** (`,` `.` `:` `;` usw.) als Abschluss folgen
- Einige Befehle lösen nach der Änderung einen **automatischen Neustart** aus
- Jeder Befehl endet mit `\r\n` (Seriell-Terminals fügen dies meist automatisch hinzu)

## 2. WiFi-Konfiguration

### STA-Modus (Verbindung mit Router)

| Befehl | Beschreibung | Beispiel | Rückgabe |
|---|---|---|---|
| `sta_ssid:Name` | WiFi-Namen festlegen | `sta_ssid:MyWiFi` | `OK` |
| `sta_pd:Passwort` | WiFi-Passwort festlegen (Neustart nach Änderung) | `sta_pd:12345678` | `OK` (Neustart) |

> WiFi-Name und -Passwort: maximal 30 Zeichen, keine chinesischen Zeichen.

### AP-Modus (eigener Hotspot)

| Befehl | Beschreibung | Beispiel | Rückgabe |
|---|---|---|---|
| `ap_ssid:Name` | Hotspot-Namen festlegen | `ap_ssid:NanoCam-AP` | `OK` |
| `ap_pd:Passwort` | Hotspot-Passwort festlegen (Neustart nach Änderung) | `ap_pd:12345678` | `OK` (Neustart) |

### WiFi-Modus

| Befehl | Beschreibung | Parameter | Rückgabe |
|---|---|---|---|
| `wifi_mode:X` | Modus wechseln | 0=AP 1=STA 2=AP+STA | `OK` (Neustart bei Änderung) |

---

## 3. KI-Moduswechsel

| Befehl | Modus | Beschreibung | Neustart |
|---|---|---|---|
| `ai_mode:0` | Normal | MJPEG-Videoübertragung, ohne KI | ✅ |
| `ai_mode:1` | Katzengesichtsdetektion | Echtzeit-Rahmen um Katzengesicht + Konfidenz | ✅ |
| `ai_mode:2` | Gesichtsdetektion | Echtzeit-Rahmen um Gesicht + Koordinaten | ✅ |
| `ai_mode:3` | Farberkennung | Auswahl einrahmen→Echtzeiterkennung | ✅ |
| `ai_mode:4` | Gesichtserkennung | Registrieren→Identifizieren→Löschen | ✅ |
| `ai_mode:5` | QR-Code | Echtzeit-Dekodierung→serielle Ausgabe | ✅ |
| `ai_mode:6` | LLM-Agent | XiaoZhi-AI-Sprachdialog + KI-Vision | ✅ |
| `ai_mode:7` | ESP-Claw | Sprachsteuerung + Foto-/Bildanalyse + OpenAI Vision | ✅ |

> Gültige Werte für `ai_mode`: 0-7. Außerhalb des Bereichs wird standardmäßig auf 0 gesetzt. Nach der Änderung erfolgt ein automatischer Neustart; danach ist der neue Modus aktiv.

---

## 4. Abfragen

| Befehl | Beschreibung | Rückgabebeispiel |
|---|---|---|
| `sta_ip` | STA-IP abfragen | `sta_ip:192.168.1.100` |
| `ap_ip` | AP-IP abfragen | `ap_ip:192.168.4.1` |
| `wifi_ver` | Firmware-Version abfragen | `NanoCam Board Ver:0.2.0` |

---

## 5. Systemsteuerung

| Befehl | Beschreibung | Rückgabe |
|---|---|---|
| `wifi_reset` | Werkseinstellungen wiederherstellen (Neustart) | `Reset_OK` |
| `nano_reboot` | Software-Reset | `Rebooting...` |
| `nano_info` | Vollständige Geräteinformationen (JSON) | siehe unten |

### nano_info Rückgabebeispiel

```json
{
  "device": "NanoCam",
  "ver": "0.2.0",
  "chip": "ESP32-S3",
  "flash": "16MB",
  "psram": "8MB",
  "ai_mode": 1,
  "wifi_mode": 2,
  "sta_ip": "192.168.1.100",
  "free_heap": 245760
}
```

---

## 6. Gesichtserkennungs-Befehle

> Nur im Modus ai_mode:4 (Gesichtserkennung) gültig.

| Befehl | Beschreibung | Label-Verhalten | Rückgabebeispiel |
|---|---|---|---|
| `face_eril` | Registriert das im aktuellen Bild erkannte Gesicht | Blau "Enroll: ID N", blinkt 0.5s auf | `>>> face enroll triggered` |
| `face_rz` | Wechselt in den kontinuierlichen Gesichtserkennungsmodus | Grün "ID: N" / Rot "who?", **bleibt dauerhaft sichtbar** | `>>> face recognize triggered` |
| `face_del` | Löscht die zuletzt registrierte Gesichts-ID | Rot "N IDs left", blinkt 0.5s auf | `>>> face delete triggered` |
| `face_detect` | Verlässt den Erkennungsmodus, zurück zur reinen Gesichtsdetektion | Löscht alle Labels | `>>> face detect mode` |

### Ablauf der Gesichtserkennung

```
ai_mode:4          # Gesichtserkennungsmodus betreten (Gerät startet automatisch neu)
face_eril          # Gesicht registrieren (sicherstellen, dass nur ein Gesicht im Bild ist)
face_rz            # Kontinuierliche Erkennung starten — das Label bleibt sichtbar und verschwindet nicht
face_detect        # Erkennungsmodus verlassen — Label wird gelöscht
face_del           # Zuletzt registriertes Gesicht löschen
```

### Hinweise zur Gesichtserkennung

1. Bei der Registrierung darf sich **nur ein Gesicht** im Bild befinden, Abstand 30-50cm
2. Im Erkennungsmodus (`face_rz`) bleiben die Labels **dauerhaft sichtbar** und verschwinden nicht nach 0.5 Sekunden — neues Verhalten ab 0.3.0
3. Zum Verlassen des Erkennungsmodus muss `face_detect` gesendet werden; andernfalls bleiben die Labels dauerhaft sichtbar
4. Gesichtsmerkmale werden in der Flash-Partition `fr` gespeichert, bleiben bei Stromausfall erhalten, maximal 47 IDs
5. Die Erkennung nutzt eine Frame-Skipping-Strategie (MFN-Inferenz einmal alle 10 Frames)

---

## 7. Erweiterungsbefehle (NanoCam-exklusiv)

| Befehl | Beschreibung | Status |
|---|---|---|
| `nano_server:url` | LLM-Serveradresse festlegen (NVS-gespeichert) | ✅ |
| `nano_api_key:key` | LLM-API-Schlüssel festlegen (NVS-gespeichert) | ✅ |
| `nano_mqtt:broker,port,topic` | MQTT-Server konfigurieren | 🔨 |
| `nano_led:R,G,B` | RGB-LED einstellen (WS2812, GPIO18 DIN) | 📋 |
| `nano_snap` | Foto aufnehmen und speichern (SPIFFS) | ✅ |
| `nano_stream:on/off` | Videoübertragung starten/stoppen | 📋 |

### nano_server / nano_api_key

| Befehl | Beschreibung | Beispiel | Rückgabe |
|---|---|---|---|
| `nano_server:URL` | LLM-Serveradresse festlegen | `nano_server:https://api.openai.com` | `OK server=https://api.openai.com` |
| `nano_api_key:KEY` | API-Schlüssel festlegen | `nano_api_key:sk-xxxx` | `OK` |

> Unterstützt beliebige OpenAI-kompatible APIs (vLLM / Ollama / lokale Modelle).
Der ESP-Claw-Modus (ai_mode:7) unterstützt `nano_server`; XiaoZhi AI (ai_mode:6) verwendet eine eigene Serverkonfiguration.

---

## 8. Hinweise

1. `sta_pd` / `ap_pd` startet nach Änderung automatisch neu; danach gilt das neue Passwort
2. `ai_mode` startet nach Änderung automatisch neu (nur wenn sich der Modus ändert)
3. Im Gesichtserkennungsmodus (mode 4) kann die Type-C-Seriellkonfiguration wegen Speichermangels ausfallen
4. WiFi-Name/-Passwort: maximal 30 Zeichen, keine chinesischen Zeichen
5. Nach dem Befehl muss ein Satzzeichen als Abschlusszeichen folgen
