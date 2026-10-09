# Capítulo 4: Detección de rostros

## Principio

La detección de rostros usa la biblioteca de aprendizaje profundo ESP-DL, basada en el modelo ligero de detección MobileNet. Recibe una imagen RGB565 de 320x240 y devuelve una lista de cuadros delimitadores de rostros (posición + tamaño + confianza). La inferencia se ejecuta en el ESP32-S3, sin necesidad de conexión a la red.

### Formato del resultado de detección

- Coordenadas: esquina superior izquierda (x, y) + ancho y alto (w, h)
- Confianza: número decimal entre 0 y 1
- Con varios rostros se devuelven varios cuadros

## Pasos

### 4.1 Cambiar de modo

```
ai_mode:2
```

### 4.2 Observar el resultado

En el navegador `http://<IP>` se ve el cuadro de detección de rostros.

### 4.3 Obtener las coordenadas

Formato de salida del puerto serie:

```
I (xxxxx) detection_result: [ 0]: ( 45,  30, 180, 210)
I (xxxxx) detection_result:       left eye: ( 90,  80), right eye: (150,  80), nose: (120, 120), mouth left: ( 95, 150), mouth right: (145, 150)
```

- Primera línea: `[número] (x, y, w, h)` — coordenadas del cuadro del rostro
- Segunda línea: 5 puntos clave — ojo izquierdo, ojo derecho, nariz, comisura izquierda de la boca, comisura derecha de la boca

## Código

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

### Lectura con Python

```python
ser = serial.Serial("COM3", 115200)
line = ser.readline().decode()
if line.startswith("$face:"):
    parts = line[6:-1].split(",")
    x, y, w, h = map(int, parts)
```

## Resultado

Aparece un rostro delante de la cámara → la imagen marca el cuadro verde → el puerto serie emite las coordenadas.
