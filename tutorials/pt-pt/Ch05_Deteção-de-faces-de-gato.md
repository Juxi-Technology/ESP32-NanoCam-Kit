# Capítulo 5: Deteção de faces de gato

## Princípio

A deteção de faces de gato utiliza o modelo CatFaceDetectMN03, otimizado e treinado especificamente para as características da face do gato (orelhas triangulares / distância interpupilar ampla / nariz). Entrada: imagem RGB565 de 320x240; saída: lista de caixas delimitadoras de faces de gato. Partilha com a deteção de rostos do Capítulo 4 o mesmo formato de saída `print_detection_result`.

### Diferenças entre os modelos de deteção de faces de gato e de rostos

| Dimensão de comparação | Deteção de rostos (ai_mode:2) | Deteção de faces de gato (ai_mode:1) |
|---|---|---|
| Modelo | Cascata dupla MSR01 + MNP01 | CatFaceDetectMN03 fase única |
| Pontos-chave | 10 (olhos/nariz/cantos da boca) | Nenhum (o modelo não os produz) |
| Limiar de confiança | MSR01=0.3, MNP01=0.4 | 0.4 |
| Desenho da caixa | Retângulo verde vazio + 5 pontos-chave | Retângulo verde vazio (sem pontos-chave) |

## Passos

### 5.1 Mudar de modo

```
ai_mode:1
```

O dispositivo reinicia automaticamente e entra no modo de deteção de faces de gato

### 5.2 Observar o resultado

Coloque um gato ou uma imagem de gato em frente da câmara e abra `http://<IP>` no navegador para ver a caixa de deteção verde a marcar a face do gato.

### 5.3 Saída da porta série

Quando é detetada uma face de gato, a porta série apresenta:

```
I (xxxxx) detection_result: [ 0]: ( 45,  30, 180, 210)
```

- Formato: `[número] (x, y, w, h)` — coordenadas do canto superior esquerdo da caixa da face de gato + largura e altura
- O modelo de faces de gato não produz pontos-chave (ao contrário da deteção de rostos)

## Código

### Lógica de deteção principal

`components/modules/ai/who_cat_face_detection.cpp`:

```cpp
CatFaceDetectMN03 detector(0.4F, 0.3F, 10, 0.3F);
std::list<dl::detect::result_t> &detect_results = detector.infer(
    (uint16_t *)frame->buf, {(int)frame->height, (int)frame->width, 3});

if (detect_results.size() > 0) {
    draw_detection_result((uint16_t *)frame->buf, frame->height, frame->width, detect_results);
    print_detection_result(detect_results);  // saída das coordenadas pela porta serial
}
```

### Arduino: ler as coordenadas para controlar um servo

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
import serial
ser = serial.Serial("COM3", 115200)
while True:
    line = ser.readline().decode().strip()
    if "detection_result" in line:
        print(line)
```

## Efeito

O gato aparece → caixa verde na imagem → coordenadas emitidas pela porta série. É possível usar Arduino/Python para ler as coordenadas e controlar um servo de seguimento.
