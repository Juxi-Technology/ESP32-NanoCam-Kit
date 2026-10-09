# Capítulo 5: Detecção de rosto de gato

## Princípio

A detecção de rosto de gato usa o modelo CatFaceDetectMN03, treinado com otimização específica para as características do rosto de gatos (orelhas triangulares / distância interpupilar ampla / nariz). Recebe uma imagem RGB565 de 320x240 e retorna uma lista de caixas delimitadoras de rosto de gato. Compartilha o mesmo formato de saída `print_detection_result` da detecção de rosto do Capítulo 4.

### Diferenças entre os modelos de rosto de gato e de rosto humano

| Aspecto | Detecção de rosto (ai_mode:2) | Detecção de rosto de gato (ai_mode:1) |
|---|---|---|
| Modelo | MSR01 + MNP01 em cascata dupla | CatFaceDetectMN03 em estágio único |
| Pontos-chave | 10 (dois olhos / ponta do nariz / cantos da boca) | Nenhum (o modelo não fornece) |
| Limiar de confiança | MSR01=0.3, MNP01=0.4 | 0.4 |
| Desenho da caixa de detecção | Retângulo vazado verde + 5 pontos-chave | Retângulo vazado verde (sem pontos-chave) |

## Passos

### 5.1 Alternar o modo

```
ai_mode:1
```

O dispositivo reinicia automaticamente e entra no modo de detecção de rosto de gato

### 5.2 Observar o resultado

Coloque um gato ou uma foto de gato diante da câmera e abra `http://<IP>` no navegador para ver a caixa de detecção verde marcando o rosto do gato.

### 5.3 Saída serial

Quando um rosto de gato é detectado, a porta serial exibe:

```
I (xxxxx) detection_result: [ 0]: ( 45,  30, 180, 210)
```

- Formato: `[número] (x, y, w, h)` — coordenadas do canto superior esquerdo da caixa do rosto de gato + largura e altura
- O modelo de rosto de gato não fornece pontos-chave (diferente da detecção de rosto humano)

## Código

### Lógica central de detecção

`components/modules/ai/who_cat_face_detection.cpp`:

```cpp
CatFaceDetectMN03 detector(0.4F, 0.3F, 10, 0.3F);
std::list<dl::detect::result_t> &detect_results = detector.infer(
    (uint16_t *)frame->buf, {(int)frame->height, (int)frame->width, 3});

if (detect_results.size() > 0) {
    draw_detection_result((uint16_t *)frame->buf, frame->height, frame->width, detect_results);
    print_detection_result(detect_results);  // Enviar as coordenadas pela porta serial
}
```

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

### Leitura serial em Python

```python
import serial
ser = serial.Serial("COM3", 115200)
while True:
    line = ser.readline().decode().strip()
    if "detection_result" in line:
        print(line)
```

## Resultado

O gato aparece → caixa verde marcada na imagem → coordenadas enviadas pela porta serial. Use Arduino/Python para ler as coordenadas e mover o servo para rastrear.
