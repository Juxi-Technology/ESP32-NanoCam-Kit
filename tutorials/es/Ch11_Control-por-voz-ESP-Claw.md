# Capítulo 11: Control por voz ESP-Claw

## Sobre este capítulo

Cuando el dispositivo cambia a `ai_mode:7`, NanoCam entra en el modo ESP-Claw. **Comparte el mismo firmware** (`nanocam_espclaw/`) con XiaoZhi AI (`ai_mode:6`); la única diferencia es que el modo ESP-Claw, además de la conversación de voz, registra 5 herramientas adicionales de control de hardware.

| Aspecto | XiaoZhi AI (mode 6) | ESP-Claw (mode 7) |
|---|---|---|
| Conversación de voz | ✅ ASR→LLM→TTS | ✅ El mismo pipeline de voz |
| Control del LED | ❌ | ✅ Color / encendido y apagado por voz |
| Cambio de modo de IA | ❌ | ✅ Cambio por voz |
| Foto + análisis visual con IA | ❌ | ✅ Hace una foto y usa IA multimodal para comprender la escena |

## Principio

Sobre el pipeline de voz, el modo ESP-Claw registra 5 herramientas exclusivas de NanoCam mediante `RegisterMcpTools()`:

```
Voz del usuario "pon la luz en azul"
  → Reconocimiento de voz ASR (en la nube)
  → El LLM entiende la intención → llama a self.led.set_color({"r":0, "g":0, "b":255})
  → El LED WS2812 de NanoCam se vuelve azul
  → TTS: "De acuerdo, la luz ya está en azul"
```

## Pasos

### 12.1 Grabar el firmware

ESP-Claw usa el proyecto de firmware independiente `nanocam_espclaw/`:

```bash
cd nanocam_espclaw
idf.py set-target esp32s3
idf.py build
idf.py -p COMx flash
```

### 12.2 Cambiar de modo

Tras el arranque, configúralo en modo ESP-Claw:

```
ai_mode:7
```

El dispositivo se reinicia automáticamente y entra en él. Con `ai_mode:6` puedes volver al modo XiaoZhi AI.

### 12.3 Ejemplos de control por voz

Tras la activación, di lo que necesitas directamente:

```
💬 "Enciende la luz"                    → WS2812 en blanco
💬 "Pon la luz en azul"                 → El LED se vuelve azul
💬 "Apaga la luz"                       → LED apagado
💬 "Cambia al modo de detección de rostros" → Guarda ai_mode:2 en NVS + reinicio
💬 "Mira qué hay aquí"                  → Foto + análisis con IA multimodal
💬 "¿Hay una taza delante de mí?"       → La IA multimodal reconoce la escena
```

### 12.4 Foto + análisis visual con IA

Cuando el usuario dice "mira...", el firmware captura un fotograma VGA RGB565, lo comprime en JPEG y lo envía a la API multimodal configurada en el servidor para su análisis; el resultado se anuncia por voz mediante TTS.
> La URL y el token de la API multimodal los entrega automáticamente el servidor durante el handshake de conexión; no hace falta introducir comandos de configuración manualmente por el puerto serie.

## Las 5 herramientas exclusivas de NanoCam

| Herramienta | Función | Parámetros |
|---|---|---|
| `self.led.set_color` | Configura el LED RGB WS2812 (GPIO18) | `r,g,b`: 0-255 |
| `self.led.turn_off` | Apaga el LED | Ninguno |
| `self.camera.set_ai_mode` | Cambia el modo de IA (guardado en NVS + reinicio) | `mode`: 0-7 |
| `self.camera.inspect_image` | Foto + análisis visual con LLM multimodal | `prompt`: descripción de la pregunta |
| `self.get_device_info` | JSON con información del dispositivo | Ninguno |

## Archivos de configuración

| Contenido | Ruta |
|---|---|
| Registro de herramientas MCP | `nanocam_espclaw/main/boards/nanocam/nanocam_board.cc` |
| Lógica de envío de Vision | `nanocam_espclaw/main/boards/common/esp32_camera.cc` |
| Configuración por defecto del SDK | `nanocam_espclaw/sdkconfig.defaults` |

> El firmware de ESP-Claw es un proyecto independiente y no comparte código con `nanocam_vision`. Los dos firmwares deben compilarse y grabarse por separado.

## Cómo elegir

| Tu necesidad | Modo recomendado |
|---|---|
| Solo quieres conversar por voz y preguntar cosas | mode 6 (XiaoZhi) |
| Quieres controlar el LED por voz | mode 7 (ESP-Claw) |
| Quieres hacer fotos y que la IA "vea" la escena | mode 7 (ESP-Claw) |
| Quieres cambiar el modo de detección de IA por voz | mode 7 (ESP-Claw) |

> La forma de uso completa de ESP-Claw (configuración del servidor, desarrollo de herramientas MCP personalizadas, etc.) sigue en exploración; la documentación se actualizará a medida que avance la investigación.
