# Capítulo 4: Detecção de rosto

## Princípio

A detecção de rosto usa a biblioteca de aprendizado profundo ESP-DL, com base no modelo leve de detecção MobileNet. Recebe uma imagem RGB565 de 320x240 e retorna uma lista de caixas delimitadoras de rosto (posição + tamanho + confiança). A inferência roda no ESP32-S3, sem necessidade de conexão à internet.

### Formato do resultado da detecção

- Coordenadas: canto superior esquerdo (x,y) + largura e altura (w,h)
- Confiança: número de ponto flutuante entre 0 e 1
- Com vários rostos, várias caixas são retornadas

## Passos

### 4.1 Alternar o modo

```
ai_mode:2
```

### 4.2 Observar o resultado

No navegador, `http://<IP>` mostra a caixa de detecção de rosto.

### 4.3 Obter as coordenadas

Formato da saída serial:

```
I (xxxxx) detection_result: [ 0]: ( 45,  30, 180, 210)
I (xxxxx) detection_result:       left eye: ( 90,  80), right eye: (150,  80), nose: (120, 120), mouth left: ( 95, 150), mouth right: (145, 150)
```

- Primeira linha: `[número] (x, y, w, h)` — coordenadas da caixa de rosto
- Segunda linha: 5 pontos-chave — olho esquerdo, olho direito, nariz, canto esquerdo da boca, canto direito da boca

## Código

### Arduino: ler as coordenadas e controlar o servo

```cpp
// Analisar o formato $face:x,y,w,h#
if (nanoSerial.available()) {
    String line = nanoSerial.readStringUntil('\n');
    if (line.startsWith("$face:")) {
        int x = line.substring(6).toInt();
        int y = line.substring(line.indexOf(',')+1).toInt();
        servoX.write(map(x, 0, 320, 0, 180));
    }
}
```

### Leitura em Python

```python
ser = serial.Serial("COM3", 115200)
line = ser.readline().decode()
if line.startswith("$face:"):
    parts = line[6:-1].split(",")
    x, y, w, h = map(int, parts)
```

## Resultado

Um rosto aparece diante da câmera → caixa verde marcada na imagem → coordenadas enviadas pela porta serial.
