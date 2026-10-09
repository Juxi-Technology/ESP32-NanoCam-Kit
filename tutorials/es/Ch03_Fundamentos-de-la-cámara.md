# Capítulo 3: Fundamentos de la cámara

## Principio

NanoCam conecta la cámara mediante la interfaz DVP (vídeo digital en paralelo). El sensor GC2145 emite datos de píxel en paralelo de 8 bits; el periférico LCD_CAM del ESP32-S3 los almacena directamente en la PSRAM mediante DMA y, a continuación, el servidor HTTP transmite el flujo al navegador en formato MJPEG.

### Conceptos clave

- **DVP**: datos en paralelo de 8 líneas + señales de sincronización de 3 líneas (VSYNC/HREF/PCLK)
- **MJPEG**: cada fotograma es una imagen JPEG independiente; el navegador los carga de forma continua para lograr el efecto de vídeo
- **PSRAM**: 8 MB de PSRAM usados como búfer de fotogramas; caben 2-4 fotogramas

## Pasos

### 3.1 Ver la imagen predeterminada

Tras grabar el firmware, el modo predeterminado es el de transmisión; abre `http://<IP>` en el navegador para ver la imagen.

| Comando | Función |
|---|---|
| ai_mode:0\r | Transmisión de vídeo |
| ai_mode:1\r | Detección de cara de gato |
| ai_mode:2\r | Detección de rostros |
| ai_mode:3\r | Reconocimiento de colores |
| ai_mode:4\r | Reconocimiento facial |
| ai_mode:5\r | Reconocimiento de códigos QR |
