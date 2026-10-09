# Capitolo 7: Scansione codici QR

## Funzionamento

Utilizza la libreria precompilata esp-code-scanner per decodificare in tempo reale i codici presenti nell'immagine (QR Code / Barcode). I frame RGB565 emessi dalla fotocamera vengono passati direttamente allo scanner, senza conversione in scala di grigi. A ogni frame viene creato un nuovo oggetto scanner, distrutto subito dopo la scansione, per evitare l'accumulo di stato interno.
Il risultato della decodifica viene fornito contemporaneamente tramite:
1. Il **log della porta seriale**
2. Il **buffer condiviso** `g_last_code`, che memorizza il risultato più recente per la sovrapposizione sullo stream HTTP/MJPEG
3. Una **sovrapposizione di testo verde in basso nella pagina Web**

## Procedura

### 7.1 Cambiare modalità

```
ai_mode:5
```

### 7.2 Scansione di un codice

Metti il codice QR davanti alla fotocamera; la porta seriale emette:

```
I (xxxxx) qrcode: Decoded [QR-Code]: https://example.com
```

Contemporaneamente, in basso nella pagina `http://<IP>/` compare un'annotazione di testo verde con il contenuto decodificato.

### 7.3 Scansioni consecutive

Inquadra il codice successivo: la decodifica e l'output avvengono automaticamente; lo scanner viene ricreato a ogni frame e può funzionare in modo continuo senza crash.

## Codice

### Logica di scansione principale

`main/ai/nano_qrcode.cpp`:

```cpp
// crea un nuovo oggetto scanner a ogni frame
esp_image_scanner_t *scn = esp_code_scanner_create();
esp_code_scanner_config_t cfg = {
    ESP_CODE_SCANNER_MODE_FAST,
    ESP_CODE_SCANNER_IMAGE_RGB565,
    fb->width, fb->height
};
esp_code_scanner_set_config(scn, cfg);
int count = esp_code_scanner_scan_image(scn, fb->buf);
// decodifica riuscita
const esp_code_scanner_symbol_t result = esp_code_scanner_result(scn);
ESP_LOGI(TAG, "Decoded [%s]: %s", result.type_name, result.data);
// salva nel buffer condiviso per la sovrapposizione nella pagina web
snprintf(g_last_code, sizeof(g_last_code), "%s: %s",
         result.type_name, result.data);
esp_code_scanner_destroy(scn);
```

## Risultato

Inquadra il codice QR→la porta seriale emette il contenuto decodificato + sovrapposizione nella pagina Web.
