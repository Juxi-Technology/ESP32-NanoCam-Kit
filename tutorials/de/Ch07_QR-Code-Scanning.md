# Kapitel 7: QR-Code-Scanning

## Funktionsprinzip

Die vorkompilierte Bibliothek esp-code-scanner dekodiert QR-Codes (QR Code / Barcode) im Bild in Echtzeit. Die von der Kamera ausgegebenen RGB565-Frames werden direkt an den Scanner übergeben — eine Graustufenkonvertierung ist nicht erforderlich. Für jeden Frame wird ein neues Scanner-Objekt erstellt und nach dem Scan wieder zerstört, um eine Ansammlung internen Zustands zu vermeiden.
Die Dekodierungsergebnisse werden gleichzeitig ausgegeben über:
1. **Serielles Log**
2. **Gemeinsamer Puffer** `g_last_code`, der das neueste Ergebnis für die Überlagerung im HTTP-/MJPEG-Stream speichert
3. **Grüne Textüberlagerung** am unteren Rand des Web-Bilds

## Schritte

### 7.1 Modus wechseln

```
ai_mode:5
```

### 7.2 Code scannen

Halten Sie den QR-Code vor die Kamera; die serielle Ausgabe zeigt:

```
I (xxxxx) qrcode: Decoded [QR-Code]: https://example.com
```

Gleichzeitig erscheint am unteren Rand des Web-Bilds `http://<IP>/` eine grüne Textüberlagerung mit dem dekodierten Inhalt.

### 7.3 Fortlaufendes Scannen

Richten Sie die Kamera auf den nächsten Code — er wird automatisch dekodiert und ausgegeben; der Scanner wird bei jedem Frame neu aufgebaut und arbeitet fortlaufend stabil.

## Code

### Zentrale Scan-Logik

`main/ai/nano_qrcode.cpp`:

```cpp
// Für jeden Frame einen neuen Scanner erstellen
esp_image_scanner_t *scn = esp_code_scanner_create();
esp_code_scanner_config_t cfg = {
    ESP_CODE_SCANNER_MODE_FAST,
    ESP_CODE_SCANNER_IMAGE_RGB565,
    fb->width, fb->height
};
esp_code_scanner_set_config(scn, cfg);
int count = esp_code_scanner_scan_image(scn, fb->buf);
// Dekodierung erfolgreich
const esp_code_scanner_symbol_t result = esp_code_scanner_result(scn);
ESP_LOGI(TAG, "Decoded [%s]: %s", result.type_name, result.data);
// Zur Überlagerung im Web-Bild im gemeinsamen Puffer speichern
snprintf(g_last_code, sizeof(g_last_code), "%s: %s",
         result.type_name, result.data);
esp_code_scanner_destroy(scn);
```

## Ergebnis

QR-Code anvisieren → dekodierter Inhalt im seriellen Log + Überlagerung im Web-Bild.
