# Capítulo 9: Conversación de voz (XiaoZhi AI)

## Sobre este capítulo

Este capítulo trata el modo XiaoZhi AI (`ai_mode:6`). **Importante**: el modo 6 (conversación de voz) y el modo 7 (ESP-Claw) **comparten el mismo firmware** (`nanocam_espclaw/`); la única diferencia es que al arrancar el dispositivo se carga un conjunto distinto de herramientas MCP según el valor de `ai_mode` almacenado en la NVS.

| Aspecto | XiaoZhi AI (mode 6) | ESP-Claw (mode 7) |
|---|---|---|
| Conversación de voz | ✅ ASR→LLM→TTS | ✅ El mismo pipeline de voz |
| Herramientas MCP | Herramientas genéricas (volumen / foto, etc.) | **Herramientas genéricas + 5 herramientas exclusivas de hardware** |
| Comprensión visual | `self.camera.take_photo` | **`self.camera.inspect_image`** (visión multimodal) |
| Control del LED | ❌ | ✅ Color por voz |
| Escenarios de uso | Conversación de IA general, educación infantil | Control de hardware, inspección visual, hogar inteligente |

> Este capítulo se centra en la función central de conversación de voz de **XiaoZhi AI (modo 6)**. Para conocer las capacidades de control de hardware de ESP-Claw, consulta el Capítulo 11: Control por voz ESP-Claw.

## Principio

NanoCam integra el framework de código abierto XiaoZhi AI y se conecta al servidor LLM mediante WebSocket / MQTT para completar todo el pipeline de interacción de voz:

```
El usuario habla → Captura del micrófono ES8311 → Codificación Opus
  → WebSocket → Reconocimiento de voz ASR en la nube
  → El modelo grande LLM genera la respuesta
  → Síntesis de voz TTS → Decodificación Opus
  → Amplificador NS4150B → Reproducción por el altavoz
```

Diseño full-duplex: el usuario puede interrumpir mientras la IA habla (barge-in), una experiencia cercana a una conversación real.

## Requisitos de hardware

Este capítulo implica funciones de audio, por lo que se necesita el siguiente hardware:
- Placa central de NanoCam (con códec ES8311 + micrófono AP2718AT)
- Placa base de NanoCam (con amplificador NS4150B + CH340K)
- Altavoz (conectado a la interfaz de altavoz de la placa base, VON/VOP)
> También se puede probar solo con la placa central (escuchando por la salida de auriculares del ES8311). El micrófono es un micrófono MEMS de silicio analógico AP2718AT, conectado al MIC1P del ES8311 a través del condensador de bloqueo de CC C26.

## Pasos

### 10.1 Grabar el firmware XiaoZhi AI

XiaoZhi AI usa el proyecto de firmware independiente `nanocam_espclaw/`:

```bash
cd nanocam_espclaw
idf.py set-target esp32s3
idf.py build
idf.py -p COMx flash monitor
```

Tras el arranque, el modo XiaoZhi AI es el predeterminado.

### 10.2 Conectar al servicio en la nube xiaozhi.me

NanoCam se conecta de fábrica al servicio en la nube oficial de [xiaozhi.me](https://xiaozhi.me) (gratuito); no hace falta montar un servidor propio.
1. Registra una cuenta en [xiaozhi.me](https://xiaozhi.me)
2. Al encender el dispositivo, este anuncia automáticamente un código de activación de 6 dígitos
3. Introduce el código de activación en la consola de xiaozhi.me → vincula el dispositivo
4. Elige el modelo LLM en la consola (Qwen / DeepSeek, etc.)
La activación solo hay que hacerla una vez; después se conecta automáticamente en cada encendido.

### 10.3 Primera conversación

Cuando oigas el tono de aviso, ya puedes conversar:

```
Tú: "你好小智, ¿qué tiempo hace hoy?"
NanoCam: "Voy a consultarte el tiempo de hoy..."
```

La palabra de activación es **"你好小智"** (la predeterminada).

### 10.4 Escenarios de conversación habituales

```
💬 "Cuéntame un chiste"              → Respuesta de voz de la IA
💬 "Ponme una alarma de 5 minutos"   → Función de alarma
💬 "¿Qué hora es?"                   → Anuncio de la hora
💬 "Pon música ligera"               → Reproduce música en línea
💬 "¿Qué es un agujero negro?"       → Preguntas y respuestas de conocimiento
```

## Servidor autoalojado (opcional)

Si tienes requisitos de privacidad o quieres usar tu propio LLM, puedes desplegar el servidor de código abierto de XiaoZhi AI:

```bash
git clone https://github.com/xinnan-tech/xiaozhi-esp32-server
cd xiaozhi-esp32-server
pip install -r requirements.txt
python app.py
```

La dirección del servidor del firmware se entrega a través del sistema OTA (`CONFIG_OTA_URL` en sdkconfig); el dispositivo solicita la dirección del servidor automáticamente al encenderse.
> XiaoZhi AI usa el servidor de código abierto de XiaoZhi AI (protocolo privado WebSocket + pipeline ASR/LLM/TTS). Sobre esta base, en el modo ESP-Claw el servidor entrega la URL y el token de la Vision API durante el handshake MCP para la función de análisis visual; el firmware no necesita configurarlos por su cuenta.

## Solución de problemas

| Síntoma | Posible causa | Solución |
|---|---|---|
| No se oye nada | Altavoz no conectado | Comprueba la interfaz de altavoz de la placa base |
| El reconocimiento de voz es impreciso | Demasiado ruido ambiental | Habla cerca del micrófono (distancia < 1m) |
| No se puede conectar | WiFi no configurado | Configura primero la red por puerto serie `sta_ssid:xxx` |
| No aparece el código de activación | El primer arranque no ha terminado | Espera 30 segundos; el dispositivo lo anunciará automáticamente |
| Las respuestas son lentas | Latencia del servidor LLM | Elige un modelo más rápido en xiaozhi.me o monta tu propio servidor |
