# Kapitel 3: Kamera-Grundlagen

## Funktionsprinzip

NanoCam verbindet die Kamera über die DVP-Schnittstelle (Digital Video Parallel). Der GC2145-Sensor gibt 8-Bit-Parallelpixeldaten aus; das LCD_CAM-Peripheriemodul des ESP32-S3 speichert sie per DMA direkt im PSRAM, und der HTTP-Server streamt sie im MJPEG-Format zum Browser.

### Schlüsselkonzepte

- **DVP**: 8 parallele Datenleitungen + 3 Synchronisationsleitungen (VSYNC/HREF/PCLK)
- **MJPEG**: Jedes Bild ist ein eigenständiges JPEG; der Browser lädt sie fortlaufend und erzeugt so einen Videoeffekt
- **PSRAM**: 8 MB PSRAM dienen als Framebuffer und fassen 2-4 Frames

## Schritte

### 3.1 Das Standardbild anzeigen

Nach dem Flashen der Firmware befindet sich das Gerät standardmäßig im Streaming-Modus; öffnen Sie `http://<IP>` im Browser, um das Bild zu sehen.

| Befehl | Funktion |
|---|---|
| ai_mode:0\r | Videoübertragung |
| ai_mode:1\r | Katzengesichtsdetektion |
| ai_mode:2\r | Gesichtsdetektion |
| ai_mode:3\r | Farberkennung |
| ai_mode:4\r | Gesichtserkennung |
| ai_mode:5\r | QR-Code-Scan |
