# Capítulo 5: Detección de caras de gatos

## Principio

La detección de caras de gatos usa el modelo CatFaceDetectMN03, entrenado y optimizado específicamente para rasgos faciales de gatos (orejas triangulares / distancia amplia entre pupilas / nariz). Recibe una imagen RGB565 de 320x240 y devuelve una lista de cuadros delimitadores de caras de gatos. Comparte con la detección de rostros del Capítulo 4 el mismo formato de salida `print_detection_result`.

### Diferencias entre el modelo de cara de gato y el de rostro

| Aspecto | Detección de rostros (ai_mode:2) | Detección de caras de gatos (ai_mode:1) |
|---|---|---|
| Modelo | MSR01 + MNP01 en cascada doble | CatFaceDetectMN03 simple |
| Puntos clave | 10 (ojos / punta de la nariz / comisuras de la boca) | Ninguno (el modelo no los emite) |
| Umbral de confianza | MSR01=0.3, MNP01=0.4 | 0.4 |
| Dibujo del cuadro de detección | Rectángulo hueco verde + 5 puntos clave | Rectángulo hueco verde (sin puntos clave) |

## Pasos

### 5.1 Cambiar de modo

```
ai_mode:1
```

El dispositivo se reinicia automáticamente y entra en el modo de detección de caras de gatos.

### 5.2 Observar el resultado

Coloca un gato o una imagen de gato delante de la cámara y abre `http://<IP>` en el navegador; verás un cuadro de detección verde marcando la cara del gato.

### 5.3 Salida del puerto serie

Al detectar una cara de gato, el puerto serie emite:

```
I (xxxxx) detection_result: [ 0]: ( 45,  30, 180, 210)
```

- Formato: `[número] (x, y, w, h)` — coordenadas de la esquina superior izquierda del cuadro de la cara de gato + ancho y alto
- El modelo de cara de gato no emite puntos clave (a diferencia de la detección de rostros)

## Código

### Lógica central de detección

`components/modules/ai/who_cat_face_detection.cpp`:

```cpp
CatFaceDetectMN03 detector(0.4F, 0.3F, 10, 0.3F);
std::list<dl::detect::result_t> &detect_results = detector.infer(
    (uint16_t *)frame->buf, {(int)frame->height, (int)frame->width, 3});

if (detect_results.size() > 0) {
    draw_detection_result((uint16_t *)frame->buf, frame->height, frame->width, detect_results);
    print_detection_result(detect_results);  // salida de coordenadas por el puerto serie
}
```

### Leer las coordenadas con Arduino para controlar un servo

```cpp
// Parsear el formato $face:x,y,w,h#
if (nanoSerial.available()) {
    String line = nanoSerial.readStringUntil('\n');
    if (line.startsWith("$face:")) {
        int x = line.substring(6).toInt();
        int y = line.substring(line.indexOf(',')+1).toInt();
        servoX.write(map(x, 0, 320, 0, 180));
    }
}
```

### Lectura por puerto serie con Python

```python
import serial
ser = serial.Serial("COM3", 115200)
while True:
    line = ser.readline().decode().strip()
    if "detection_result" in line:
        print(line)
```

## Resultado

Aparece el gato → la imagen marca el cuadro verde → el puerto serie emite las coordenadas. Puedes leer las coordenadas con Arduino/Python para que un servo haga seguimiento.
