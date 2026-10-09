# Manual del protocolo serie del ESP32-NanoCam

> Velocidad en baudios: 115200 | Bits de datos: 8 | Paridad: ninguna | Bits de parada: 1 | Control de flujo: ninguno
> Compatible con el conjunto de comandos AT de los principales módulos de cámara; se añaden los comandos extendidos del NanoCam.

---

## 1. Reglas generales

- Los comandos **no distinguen mayúsculas y minúsculas** (`STA_SSID` = `sta_ssid`)
- Tras el comando debe ir **cualquier signo de puntuación en inglés** (`,` `.` `:` `;` etc.) como terminador
- Algunos comandos provocan un **reinicio automático** tras modificarse
- Cada comando termina con `\r\n` (el asistente de puerto serie suele añadirlo automáticamente)

## 2. Configuración de WiFi

### Modo STA (conexión al router)

| Comando | Descripción | Ejemplo | Valor de retorno |
|---|---|---|---|
| `sta_ssid:nombre` | Establece el nombre del WiFi | `sta_ssid:MyWiFi` | `OK` |
| `sta_pd:contraseña` | Establece la contraseña del WiFi (se reinicia tras modificarla) | `sta_pd:12345678` | `OK` (reinicio) |

> El nombre y la contraseña del WiFi admiten un máximo de 30 caracteres; no se admiten caracteres chinos.

### Modo AP (punto de acceso propio)

| Comando | Descripción | Ejemplo | Valor de retorno |
|---|---|---|---|
| `ap_ssid:nombre` | Establece el nombre del punto de acceso | `ap_ssid:NanoCam-AP` | `OK` |
| `ap_pd:contraseña` | Establece la contraseña del punto de acceso (se reinicia tras modificarla) | `ap_pd:12345678` | `OK` (reinicio) |

### Modo WiFi

| Comando | Descripción | Parámetro | Valor de retorno |
|---|---|---|---|
| `wifi_mode:X` | Cambia de modo | 0=AP 1=STA 2=AP+STA | `OK` (reinicio al cambiar) |

---

## 3. Cambio de modo de IA

| Comando | Modo | Descripción | Reinicio |
|---|---|---|---|
| `ai_mode:0` | Normal | Transmisión MJPEG, sin IA | ✅ |
| `ai_mode:1` | Detección de caras de gato | Recuadro de caras de gato en tiempo real + nivel de confianza | ✅ |
| `ai_mode:2` | Detección de rostros | Recuadro de rostros en tiempo real + coordenadas | ✅ |
| `ai_mode:3` | Reconocimiento de color | Selección con recuadro → detección en tiempo real | ✅ |
| `ai_mode:4` | Reconocimiento facial | Registrar → identificar → eliminar | ✅ |
| `ai_mode:5` | Código QR | Decodificación en tiempo real → salida por el puerto serie | ✅ |
| `ai_mode:6` | Agente LLM | Diálogo por voz XiaoZhi AI + visión IA | ✅ |
| `ai_mode:7` | ESP-Claw | Control por voz + análisis visual por foto + OpenAI Vision | ✅ |

> Valores válidos de `ai_mode`: 0-7. Fuera de rango se ajusta a 0 de forma predeterminada. Se reinicia automáticamente tras modificarse y el nuevo modo surte efecto tras el reinicio.

---

## 4. Consulta de información

| Comando | Descripción | Ejemplo de valor de retorno |
|---|---|---|
| `sta_ip` | Consulta la IP de STA | `sta_ip:192.168.1.100` |
| `ap_ip` | Consulta la IP del AP | `ap_ip:192.168.4.1` |
| `wifi_ver` | Consulta la versión del firmware | `NanoCam Board Ver:0.2.0` |

---

## 5. Control del sistema

| Comando | Descripción | Valor de retorno |
|---|---|---|
| `wifi_reset` | Restablecimiento de fábrica (reinicio) | `Reset_OK` |
| `nano_reboot` | Reinicio por software | `Rebooting...` |
| `nano_info` | Información completa del dispositivo (JSON) | Ver abajo |

### Ejemplo de retorno de nano_info

```json
{
  "device": "NanoCam",
  "ver": "0.2.0",
  "chip": "ESP32-S3",
  "flash": "16MB",
  "psram": "8MB",
  "ai_mode": 1,
  "wifi_mode": 2,
  "sta_ip": "192.168.1.100",
  "free_heap": 245760
}
```

---

## 6. Comandos exclusivos del reconocimiento facial

> Solo son válidos en ai_mode:4 (modo de reconocimiento facial).

| Comando | Descripción | Comportamiento de la etiqueta | Ejemplo de retorno |
|---|---|---|---|
| `face_eril` | Registra el rostro detectado en la imagen actual | Azul "Enroll: ID N", aparece 0.5s | `>>> face enroll triggered` |
| `face_rz` | Entra en el modo de reconocimiento facial continuo | Verde "ID: N" / rojo "who?", **se muestra de forma continua sin desaparecer** | `>>> face recognize triggered` |
| `face_del` | Elimina el último ID de rostro registrado | Rojo "N IDs left", aparece 0.5s | `>>> face delete triggered` |
| `face_detect` | Sale del modo de reconocimiento y vuelve a la detección de rostros pura | Borra todas las etiquetas | `>>> face detect mode` |

### Flujo de operación del reconocimiento facial

```
ai_mode:4          # Entrar en el modo de reconocimiento facial (el dispositivo se reinicia automáticamente)
face_eril          # Registrar un rostro (asegurarse de que solo haya un rostro en la imagen)
face_rz            # Iniciar el reconocimiento continuo — la etiqueta permanece visible sin desaparecer
face_detect        # Salir del modo de reconocimiento — se borran las etiquetas
face_del           # Eliminar el último rostro registrado
```

### Notas sobre el reconocimiento facial

1. Al registrar hay que asegurarse de que haya **solo un rostro** en la imagen, a una distancia de 30-50cm
2. En el modo de reconocimiento (`face_rz`) la etiqueta **se muestra de forma continua** y no desaparece a los 0.5 segundos; este es el nuevo comportamiento de la versión 0.3.0
3. Para salir del modo de reconocimiento hay que enviar `face_detect`; de lo contrario, la etiqueta permanece visible
4. Las características faciales se almacenan en la partición `fr` del Flash, no se pierden al apagar y admiten hasta 47 ID
5. El reconocimiento usa una estrategia de salto de fotogramas (la inferencia MFN se ejecuta cada 10 fotogramas)

---

## 7. Comandos extendidos (exclusivos del NanoCam)

| Comando | Descripción | Estado |
|---|---|---|
| `nano_server:url` | Establece la dirección del servidor LLM (guardado en NVS) | ✅ |
| `nano_api_key:key` | Establece la clave de la API del LLM (guardado en NVS) | ✅ |
| `nano_mqtt:broker,port,topic` | Configura el servidor MQTT | 🔨 |
| `nano_led:R,G,B` | Configura el LED RGB (WS2812, DIN en GPIO18) | 📋 |
| `nano_snap` | Captura y almacena una foto (SPIFFS) | ✅ |
| `nano_stream:on/off` | Inicia/detiene la transmisión de vídeo | 📋 |

### nano_server / nano_api_key

| Comando | Descripción | Ejemplo | Valor de retorno |
|---|---|---|---|
| `nano_server:URL` | Establece la dirección del servidor LLM | `nano_server:https://api.openai.com` | `OK server=https://api.openai.com` |
| `nano_api_key:KEY` | Establece la clave de la API | `nano_api_key:sk-xxxx` | `OK` |

> Compatible con cualquier API compatible con OpenAI (vLLM / Ollama / modelos locales).
El modo ESP-Claw (ai_mode:7) admite el uso de `nano_server`; XiaoZhi AI (ai_mode:6) utiliza una configuración de servidor independiente.

---

## 8. Notas

1. `sta_pd` / `ap_pd` se reinician automáticamente tras modificarse; la nueva contraseña surte efecto tras el reinicio
2. `ai_mode` se reinicia automáticamente tras modificarse (solo cuando cambia el modo)
3. En el modo de reconocimiento facial (modo 4), la función de configuración por el puerto serie Type-C puede dejar de funcionar (memoria insuficiente)
4. El nombre y la contraseña del WiFi no pueden superar los 30 caracteres ni contener caracteres chinos
5. Tras el comando hay que añadir un signo de puntuación como terminador
