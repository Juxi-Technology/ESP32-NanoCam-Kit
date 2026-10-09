# Capítulo 7: Leitura de códigos QR

## Princípio

Utiliza a biblioteca pré-compilada esp-code-scanner para descodificar em tempo real os códigos QR (QR Code / Barcode) presentes na imagem. Os fotogramas RGB565 emitidos pela câmara são enviados diretamente para o scanner, sem necessidade de conversão para escala de cinzentos. A cada fotograma é criado um novo objeto scanner, destruído logo após a leitura, evitando a acumulação de estado interno.
Os resultados da descodificação são apresentados simultaneamente:
1. **no registo da porta série**
2. **no buffer partilhado** `g_last_code`, que guarda o resultado mais recente para sobreposição no fluxo HTTP/MJPEG
3. **na parte inferior da imagem da página web**, como texto verde sobreposto

## Passos

### 7.1 Mudar de modo

```
ai_mode:5
```

### 7.2 Ler o código

Coloque o código QR em frente da câmara; a porta série apresenta:

```
I (xxxxx) qrcode: Decoded [QR-Code]: https://example.com
```

Ao mesmo tempo, surge na parte inferior da imagem da página web `http://<IP>/` texto verde com o conteúdo descodificado.

### 7.3 Leitura contínua

Aponte para o código seguinte e a descodificação é emitida automaticamente; o scanner é recriado a cada fotograma, podendo funcionar continuamente sem falhas.

## Código

### Lógica de leitura principal

`main/ai/nano_qrcode.cpp`:

```cpp
// Criar um novo objeto scanner a cada fotograma
esp_image_scanner_t *scn = esp_code_scanner_create();
esp_code_scanner_config_t cfg = {
    ESP_CODE_SCANNER_MODE_FAST,
    ESP_CODE_SCANNER_IMAGE_RGB565,
    fb->width, fb->height
};
esp_code_scanner_set_config(scn, cfg);
int count = esp_code_scanner_scan_image(scn, fb->buf);
// Descodificação bem-sucedida
const esp_code_scanner_symbol_t result = esp_code_scanner_result(scn);
ESP_LOGI(TAG, "Decoded [%s]: %s", result.type_name, result.data);
// Guardar no buffer partilhado para sobreposição na página web
snprintf(g_last_code, sizeof(g_last_code), "%s: %s",
         result.type_name, result.data);
esp_code_scanner_destroy(scn);
```

## Efeito

Aponte para o código QR → conteúdo descodificado na porta série + sobreposição na imagem da página web.
