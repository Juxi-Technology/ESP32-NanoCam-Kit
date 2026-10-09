# Kapitel 2: Schnellstart

## 2.1 Firmware flashen

### Schritte

1. Ordner entpacken → `nanocam_xxx.bin`
2. [esptool-js](https://espressif.github.io/esptool-js/) öffnen
3. NanoCam per Type-C-Kabel anschließen
4. Auf "Connect" klicken → seriellen Anschluss wählen
5. Firmware-Datei wählen, Adresse `0x0` eintragen
6. Auf "START" klicken → warten, bis der Vorgang abgeschlossen ist

### Überprüfung

Mit dem Seriell-Tool (115200 8N1) mit dem NanoCam verbinden — Sie sollten sehen:

```
NanoCam Board Ver:0.3.0
```

---

## 2.2 WiFi einrichten

> Ergebnis: **NanoCam ist mit dem WiFi verbunden und hat eine IP erhalten**

### Variante A: Konfiguration über die serielle Schnittstelle (am häufigsten verwendet)

```
sta_ssid:dein_WiFi_Name
sta_pd:dein_WiFi_Passwort
```

Bei Antwort `OK` → Einstellung erfolgreich. Nach einer Passwortänderung startet das Gerät automatisch neu.

### Variante B: Direktverbindung über AP-Hotspot

NanoCam hat einen eigenen Hotspot: `NanoCam-AP`, Passwort `12345678`
Nachdem das Smartphone verbunden ist, im Browser `http://192.168.4.1` öffnen

### Überprüfung

```
sta_ip
```

Rückgabe: `sta_ip:192.168.x.x` ✅

---

## 2.3 Das erste Bild

> Ergebnis: **Das Live-Bild des NanoCam ist im Browser sichtbar**
1. Im Browser `http://<IP-Adresse>` eingeben
2. Das Live-MJPEG-Bild wird angezeigt
3. `ai_mode:1` seriell senden → Wechsel zur Katzengesichtsdetektion → Erkennungsrahmen erscheint im Bild

### Endpunkt-Übersicht

| URL | Zweck |
|---|---|
| `http://<IP>/` | Live-Bild (HTML) |
| `http://<IP>/stream` | Reiner MJPEG-Stream (lesbar mit OpenCV/VLC) |
| `http://<IP>/status` | Gerätestatus als JSON |
| `http://<IP>/admin` | Web-Verwaltungsoberfläche |
