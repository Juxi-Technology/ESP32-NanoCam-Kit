# Capítulo 10: Comprensión visual con IA

## Sobre este capítulo

La comprensión visual con IA es una función exclusiva de **ESP-Claw (modo 7)**; no se usa en XiaoZhi AI (modo 6).
> En este capítulo se usan las herramientas `self.camera.take_photo` y `self.camera.inspect_image`; la dirección de la API de análisis visual la entrega automáticamente el servidor mediante el campo `capabilities.vision` durante el handshake MCP. El firmware no necesita configurar manualmente la URL de la API — es decir, la configuración de la API se hace en la consola de xiaozhi.me o en el servidor autoalojado; consulta el Capítulo 11: Control por voz ESP-Claw para más detalles.

## Principio

Flujo completo del análisis visual:

```
Voz del usuario "mira qué hay sobre la mesa"
  → Reconocimiento de voz ASR
  → Decisión del LLM: hace falta una foto para analizar → llama a self.camera.take_photo o self.camera.inspect_image
  → Firmware: esp_camera_fb_get() captura un fotograma (VGA RGB565)
  → Compresión JPEG
  → Mediante Explain() se envía a la Vision API entregada por el servidor
  → El LLM multimodal devuelve una descripción de texto
  → Anuncio de voz TTS
```

### Diferencias entre las dos herramientas de foto

| Herramienta | Uso | Quién envía la Vision API |
|---|---|---|
| `self.camera.take_photo` | Tras hacer la foto, describe con la capacidad vision integrada del LLM | El servidor |
| `self.camera.inspect_image` (exclusiva de NanoCam) | Tras hacer la foto, llama a `camera->Explain()` → HTTP POST a una API multimodal independiente | El firmware |

La diferencia entre ambas: `take_photo` usa la visión del LLM del servidor XiaoZhi (implementación genérica); `inspect_image` es la implementación específica de este proyecto, en la que el firmware llama directamente a una API multimodal independiente (cuya dirección entrega el servidor).

## Pasos

### 11.1 Asegurarse del modo ESP-Claw

```
ai_mode:7
```

Tras reiniciarse, el dispositivo entra en el modo ESP-Claw.

### 11.2 Foto + análisis de IA

Tras la activación, plantea la pregunta directamente:

```
💬 "Mira qué hay aquí"
💬 "¿Hay una taza delante de mí?"
💬 "¿De qué color es este libro?"
💬 "¿Cuántas manzanas hay sobre la mesa?"
💬 "Ayúdame a ver qué pone en este papel"
```

NanoCam hace la foto, la sube, la analiza y responde con el resultado por voz.

### 11.3 Ejemplos de reconocimiento de escenas

| Entrada de voz | Ejemplo de respuesta de la IA |
|---|---|
| "¿Qué es esto?" | "Es un ordenador portátil negro, y al lado hay una taza de café blanca" |
| "¿Hay manzanas?" | "No veo manzanas. Sobre la mesa hay dos libros y un bolígrafo" |
| "¿De qué color?" | "Lo que señalas es una taza roja" |
| "¿Cuántas tazas hay?" | "En la imagen hay 2 tazas" |

## Código

### Callback central de foto + análisis

`nanocam_espclaw/main/boards/nanocam/nanocam_board.cc` — registro de la herramienta MCP:

```cpp
mcp.AddTool("self.camera.inspect_image",
    "Take a photo with the camera and send it to the vision AI for analysis.",
    PropertyList({ Property("prompt", kPropertyTypeString) }),
    [this](const PropertyList &props) -> ReturnValue {
        auto camera = GetCamera();
        if (!camera->Capture()) {
            return std::string("{\"error\":\"Camera capture failed\"}");
        }
        std::string prompt = props["prompt"].value<std::string>();
        return camera->Explain(prompt);
    });
```

`nanocam_espclaw/main/boards/common/esp32_camera.cc` — implementación de Explain():

```cpp
std::string Esp32Camera::Explain(const std::string &question) {
    // explain_url_ lo entrega el servidor mediante capabilities.vision.url durante el handshake MCP
    // Captura de fotograma → compresión JPEG → HTTP POST a la API multimodal
    // Devuelve el resultado del análisis del LLM
}
```

### Handshake MCP del servidor (entrega de la Vision API)

```json
{
  "capabilities": {
    "vision": {
      "url": "https://api.openai.com/v1/chat/completions",
      "token": "sk-..."
    }
  }
}
```

Al recibirlo, el firmware llama a `camera->SetExplainUrl(url, token)` para guardar la dirección de la API y la usa directamente en las siguientes llamadas a `inspect_image`.

## Modelos multimodales compatibles

Entregando distintos `vision.url` desde el servidor se puede usar cualquier API compatible con OpenAI:

| Modelo | Ejemplo de dirección de API | Escenario de uso |
|---|---|---|
| `gpt-4o` | `https://api.openai.com/v1/chat/completions` | La mayor capacidad integral |
| `gpt-4o-mini` | `https://api.openai.com/v1/chat/completions` | Buena relación calidad-precio |
| `qwen-vl-max` | `https://dashscope.aliyuncs.com/compatible-mode/v1/chat/completions` | Mejor comprensión del chino |
| `llava:13b` (Ollama) | `http://localhost:11434/v1/chat/completions` | Totalmente sin conexión |
| `claude-fable-5` | Requiere configurar un proxy | Descripción detallada de escenas |

## Resultado

"Mira qué hay aquí" → foto y subida → análisis de la IA → anuncio de voz "I see a red cup on a wooden table" — unos auténticos ojos de IA.
