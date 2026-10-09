# Capítulo 8: Reconocimiento facial

## Principio

Reconocimiento facial = **detección de rostros** (pipeline de dos etapas MSR01+MNP01) + **extracción de características** (red neuronal MFN FaceRecognition112V1S8) + **comparación por similitud de coseno**.

```
Fotograma RGB565 de la cámara
  → Detección gruesa MSR01 (320×240, umbral 0.3F)
  → Detección fina MNP01 (a partir de los candidatos de la detección gruesa, umbral 0.4F)
  → Extracción de 10 puntos clave faciales (ojos / punta de la nariz / comisuras de la boca)
  → Alineación de puntos clave → recorte del rostro 112×112
  → Red convolucional MFN → vector de características de 512 dimensiones
  → Normalización L2
  → Cálculo de la distancia de coseno con cada vector de los ID registrados en Flash
  → Similitud de coseno máxima > umbral (0.55) → coincidencia → se emite el ID
  → Todas las similitudes < umbral → desconocido → se emite "who?"
```

### Optimización del rendimiento

La extracción de características MFN y la comparación con toda la base tienen una carga de cálculo elevada; ejecutarlas en cada fotograma ralentizaría la imagen. La implementación actual usa una **estrategia de salto de fotogramas**: la detección de rostros se ejecuta en cada fotograma (barata) y el reconocimiento MFN se ejecuta una vez cada 10 fotogramas (caro); la etiqueta se sigue mostrando superpuesta con el último resultado de reconocimiento. Así la imagen se mantiene fluida y la etiqueta del ID no parpadea.

### Almacenamiento de características faciales

Las características faciales registradas (id + embedding de 512 dimensiones) se guardan de forma persistente en la partición `fr` de la Flash (96 KB, hasta 47 ID de rostros). No se pierden al apagar el dispositivo.

## Preparación de hardware

- Placa central de NanoCam + placa base
- Cable de datos USB-C (para alimentación + puerto serie conectado al ordenador)
- Asistente de puerto serie (velocidad 115200)

## Pasos

### 8.1 Entrar en el modo de reconocimiento facial

```
ai_mode:4
```

El dispositivo se reinicia automáticamente y entra en el modo FaceID; el LED RGB WS2812 (GPIO18 DIN, alimentación VDD50) se ilumina en púrpura. Tras el reinicio, el puerto serie debería mostrar:

```
I (5526) MFN: fr partition size: 98304 bytes, maxminum 47 IDs can be stored
I (5526) MFN: No face ID in flash
```

`No face ID in flash` indica que todavía no se ha registrado ningún rostro; es normal.

### 8.2 Registrar un rostro

Coloca el rostro frente a la cámara (a 30-50cm, con iluminación uniforme) y asegúrate de que en la imagen haya **solo un rostro**. Envía por el puerto serie:

```
face_eril
```

Cuando el dispositivo detecte el rostro, extraerá las características automáticamente y lo registrará en la Flash:

```
I (xxxx) ENROLL: ID 1 is enrolled
```

Sobre la imagen se superpone el texto azul `Enroll: ID 1`, que desaparece tras unos 0.5 segundos.
> **Atención**: el comando es `face_eril` (abreviatura de enroll), no `face_enroll`. Si ves `fail: unknown command`, revisa la ortografía.

### 8.3 Identificar rostros

Una vez completado el registro, envía el comando de reconocimiento:

```
face_rz
```

El sistema entra en el modo de reconocimiento continuo. El rostro actual se compara con todos los ID registrados en la Flash:
- **Coincidencia**: el puerto serie emite `Similarity: 0.85, Match ID: 1` y sobre la imagen se superpone de forma continua `ID: 1` en verde
- **Persona desconocida**: el puerto serie emite `Similarity: 0.32, Match ID: 0` y sobre la imagen se superpone de forma continua `who?` en rojo
> La etiqueta **se muestra de forma continua** y no desaparece. Para salir del modo de reconocimiento, envía `face_detect` para volver al modo de detección pura.

### 8.4 Eliminar un rostro

```
face_del
```

Elimina el último ID de rostro registrado; el puerto serie devuelve `N IDs left` y la imagen muestra brevemente el número de ID restantes. Las características de la Flash se eliminan a la vez.

### 8.5 Salir del modo de reconocimiento

```
face_detect
```

Vuelve al modo de detección de rostros puro (solo dibuja el cuadro + los puntos clave, sin reconocer) y se borran las etiquetas de ID.
> **Sobre el modo DETECT**: en el ESP32-S3, la impresión de coordenadas por puerto serie del modo de detección de rostros puro está deshabilitada (`#if !CONFIG_IDF_TARGET_ESP32S3`); así se evita que el puerto serie se sature con los registros de detección. Solo al entrar en el modo de reconocimiento (`face_rz`) se emiten los registros de coordenadas `detection_result`.

## Referencia rápida de comandos

| Comando | Función | Comportamiento de la etiqueta | ¿Continua? |
|---|---|---|---|
| `face_eril` | Registrar el rostro detectado en ese momento | Azul "Enroll: ID N" | Destello 0.5s |
| `face_rz` | Entrar en el modo de reconocimiento continuo | Verde "ID: N" / rojo "who?" | ✅ Continua |
| `face_del` | Eliminar el último ID registrado | Rojo "N IDs left" | Destello 0.5s |
| `face_detect` | Salir del reconocimiento y volver a la detección pura | Borrar todas las etiquetas | — |

## Ejemplo de flujo de operación

```
ai_mode:4                          # Entrar en el modo de reconocimiento facial
[El dispositivo se reinicia, LED púrpura]

face_eril                          # Registrar el primer rostro (Zhang San)
→ ID 1 is enrolled

face_eril                          # Registrar el segundo rostro (Li Si)
→ ID 2 is enrolled

face_rz                            # Iniciar el reconocimiento continuo
→ Zhang San frente a la cámara: la imagen muestra continuamente "ID: 1"
→ Li Si frente a la cámara: la imagen muestra continuamente "ID: 2"
→ Persona desconocida frente a la cámara: la imagen muestra continuamente "who?"

face_detect                        # Salir del modo de reconocimiento
→ Las etiquetas desaparecen; solo se dibuja el cuadro de detección

face_del                           # Eliminar a Li Si (ID 2)
→ 1 IDs left

face_rz                            # Reconocer de nuevo
→ Zhang San frente a la cámara: "ID: 1"
→ Li Si frente a la cámara: "who?" (ya eliminado)
```

> El modo de reconocimiento facial consume bastante memoria (modelo MFN + doble modelo de detección de rostros); el puerto serie Type-C (UART0) funciona con normalidad. Si el puerto serie no responde, comprueba primero que la velocidad sea 115200.

## Código

### Lógica central de reconocimiento

`components/modules/ai/who_human_face_recognition.cpp` — estrategia de reconocimiento con salto de fotogramas:

```cpp
case RECOGNIZE:
{
    // Salto de fotogramas: ejecutar 1 reconocimiento MFN cada 10 detecciones
    static int recog_skip = 0;
    if (recog_skip <= 0) {
        recognize_result = recognizer->recognize(
            (uint16_t *)frame->buf,
            {(int)frame->height, (int)frame->width, 3},
            detect_results.front().keypoint);
        recog_skip = 10;
    }
    recog_skip--;
    frame_show_state = SHOW_STATE_RECOGNIZE;
    break;
}
```

## Solución de problemas

| Síntoma | Posible causa | Solución |
|---|---|---|
| `No face ID in flash` | Normal, todavía no hay registros | Envía `face_eril` para registrar |
| El resultado del reconocimiento siempre es `who?` | Poca luz / ángulo desviado / similitud por debajo del umbral | Vuelve a registrar, de frente a la cámara y con luz uniforme |
| No pasa nada al registrar | En la imagen no hay exactamente 1 rostro | Asegúrate de que haya un solo rostro, a 30-50cm |
| La imagen se ralentiza al reconocer | Normal, la inferencia MFN necesita tiempo | Ya se ha optimizado con salto de fotogramas, se ejecuta una vez cada 10 fotogramas |
| La etiqueta parpadea | — | Ya está corregido: la etiqueta se muestra de forma continua sin desaparecer |
| `fail: unknown command` | Error de ortografía en el comando | Revisa el comando: es `face_eril`, no `face_enroll` |

## Resultado

Registrar rostros → reconocimiento continuo que muestra el ID → salida de resultados por I2C/puerto serie → control de relés/servos: solución completa de control de acceso.
