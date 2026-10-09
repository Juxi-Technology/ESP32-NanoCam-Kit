# Capítulo 7: Escaneo de códigos QR

## Principio

Se usa la biblioteca precompilada esp-code-scanner para decodificar en tiempo real los códigos QR (QR Code / Barcode) de la imagen. El fotograma RGB565 que emite la cámara se pasa directamente al escáner, sin conversión a escala de grises. En cada fotograma se crea un objeto escáner nuevo que se destruye nada más escanear, para evitar que se acumule estado interno.
El resultado de la decodificación se emite simultáneamente a través de:
1. Salida en el **registro del puerto serie**
2. El **búfer compartido** `g_last_code` guarda el último resultado para superponerlo en el flujo HTTP/MJPEG
3. Texto verde superpuesto en la **parte inferior de la imagen de la página web**

## Pasos

### 7.1 Cambiar de modo

```
ai_mode:5
```

### 7.2 Escanear un código

Coloca un código QR delante de la cámara; el puerto serie emite:

```
I (xxxxx) qrcode: Decoded [QR-Code]: https://example.com
```

A la vez, en la parte inferior de la imagen de la página web `http://<IP>/` aparece el contenido decodificado marcado con texto verde.

### 7.3 Escaneo continuo

Apunta al siguiente código y se decodifica y emite automáticamente; el escáner se recrea en cada fotograma y puede trabajar de forma continua sin caerse.

## Código

### Lógica central de escaneo

`main/ai/nano_qrcode.cpp`:

```cpp
// Crear un objeto escáner nuevo en cada fotograma
esp_image_scanner_t *scn = esp_code_scanner_create();
esp_code_scanner_config_t cfg = {
    ESP_CODE_SCANNER_MODE_FAST,
    ESP_CODE_SCANNER_IMAGE_RGB565,
    fb->width, fb->height
};
esp_code_scanner_set_config(scn, cfg);
int count = esp_code_scanner_scan_image(scn, fb->buf);
// Decodificación correcta
const esp_code_scanner_symbol_t result = esp_code_scanner_result(scn);
ESP_LOGI(TAG, "Decoded [%s]: %s", result.type_name, result.data);
// Guardar en el búfer compartido para superponer en la página web
snprintf(g_last_code, sizeof(g_last_code), "%s: %s",
         result.type_name, result.data);
esp_code_scanner_destroy(scn);
```

## Resultado

Apunta al código QR → el puerto serie emite el contenido decodificado + superposición en la imagen de la página web.
