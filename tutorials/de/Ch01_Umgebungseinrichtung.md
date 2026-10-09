# Kapitel 1: Umgebungseinrichtung

## 1.1 Firmware-Flash-Umgebung

### Variante A: Ohne Entwicklungsumgebung (für Einsteiger empfohlen)

1. [CH340K-Serielltreiber](https://www.wch.cn/download/CH341SER_EXE.html) installieren
2. Browser öffnen → [esptool-js](https://espressif.github.io/esptool-js/)
3. NanoCam anschließen, seriellen Anschluss wählen, die .bin-Firmware-Datei auswählen
4. Auf "Program" klicken, um zu flashen

### Variante B: Kommandozeile

```bash
pip install esptool
esptool.py --chip esp32s3 --port COM_x write_flash 0x0 nanocam_xxx.bin
```

### Variante C: ESP-IDF-Entwicklungsumgebung (für Fortgeschrittene)

1. VSCode + ESP-IDF-Erweiterung installieren
2. F1 → `ESP-IDF: Configure ESP-IDF Extension`
3. ESP-IDF v5.4+ auswählen oder installieren
4. Kompilieren: `idf.py build flash monitor`

### Variante D: Installation über ESP-EIM-GUI

1. Von der offiziellen Website herunterladen: https://dl.espressif.cn/dl/eim/
2. Herunterladen und per Doppelklick die EIM-Seite öffnen; oben rechts kann auf die chinesische Version umgeschaltet werden
3. Auf "Installation starten" klicken
4. Im nächsten Schritt "Benutzerdefinierte Installation" wählen
5. Vorher müssen `git` und `python3.12.x` installiert sein (Git-Downloadquelle für China: [CNPM Binaries Mirror](https://registry.npmmirror.com/binary.html?path=git-for-windows/v2.55.0.windows.3/))
6. Zielgerät esp32s3 auswählen
7. Bei der ESP-IDF-Version "ältere stabile Versionen anzeigen" aktivieren und nach unten scrollen, um Version v5.4.1 zu wählen
8. Bei der Download-Quelle nichts ändern, weiter
9. Bei den ESP-IDF-Funktionen wird "alles auswählen" empfohlen, weiter
10. Bei den Tools weiter, dann den gewünschten Installationsort wählen und installieren; warten, bis die Installation abgeschlossen ist
Nach der Installation gibt es bei dieser Version ein Entpackungsproblem: Suchen Sie das Verzeichnis C:\Espressif\dist\xtensa-esp-elf-14.2.0_20241119-x86_64-w64-mingw32.zip, kopieren Sie das Archiv nach C:\Espressif\tools\xtensa-esp-elf, entpacken Sie es, und ersetzen Sie anschließend die Ordner im Verzeichnis C:\Espressif\tools\xtensa-esp-elf\esp-14.2.0_20241119 — danach ist die Kompilierung erfolgreich

## 1.2 Server-Umgebung

### Offizieller xiaozhi.me-Dienst (kostenlos)

1. [xiaozhi.me](https://xiaozhi.me) besuchen und ein Konto registrieren
2. Konsole öffnen
3. Nach der Netzwerkverbindung gibt das Modul einen 6-stelligen Aktivierungscode per Sprachansage aus
4. Im Bereich "Agent" rechts auf "Gerät hinzufügen" klicken
5. Den angesagten 6-stelligen Aktivierungscode eingeben
6. Nach der Gerätebindung kann der Dialog beginnen
