# Capítulo 6: Reconhecimento de cores

## Princípio

Baseia-se no espaço de cor HSV (matiz-saturação-valor). A imagem RGB565 emitida pela câmara é processada pelo motor ColorDetector do esp-dl, que reduz a imagem para a resolução de 80×80 para eliminar ruído e, em seguida, converte-a pixel a pixel para valores HSV, comparando com os limiares predefinidos das 7 cores.

### Limiares de cor predefinidos (escala H padrão do OpenCV, escala 0-180)

| Cor | Matiz (H) | Saturação (S) | Valor (V) | Limiar de área |
|---|---|---|---|---|
| Vermelho | 0-15 | 70-255 | 90-255 | 64 |
| Amarelo | 23-33 | 70-255 | 90-255 | 64 |
| Verde | 34-75 | 70-255 | 90-255 | 64 |
| Azul | 97-124 | 70-255 | 90-255 | 64 |
| Roxo | 125-155 | 70-255 | 90-255 | 64 |
| Branco | 0-180 | 0-40 | 200-255 | 80 |
| Preto | 0-180 | 0-255 | 0-50 | 80 |

> O matiz usa a escala 0-180 do OpenCV (correspondente a 0-360°). `set_bgr(false)` garante que a biblioteca lê os dados RGB565 tal como estão, sem trocar os canais.

## Passos

### 6.1 Entrar no modo de cor

```
ai_mode:3
```

O dispositivo reinicia automaticamente e entra no modo de deteção de cor; o LED RGB WS2812 (GPIO18) mostra a cor atualmente reconhecida.

### 6.2 Observar o resultado do reconhecimento

Coloque um objeto de cor sólida em frente da câmara e abra `http://<IP>` no navegador; irá ver:
- **Retângulo colorido** a marcar a região da cor detetada
- **Texto da etiqueta de cor** (red/yellow/green/blue/purple/white/black)
- A cor da caixa e da etiqueta corresponde à cor realmente detetada
> O modo de cor apenas faz sobreposição na imagem (OSD), sem saída de registos pela porta série. Para obter as coordenadas, leia através dos registos I2C.

### 6.3 Ler os dados de deteção por I2C

O NanoCam funciona como I2C Slave (endereço `0x33`, GPIO SDA=41 SCL=42) e atualiza em tempo real as coordenadas do centro da caixa de deteção.

| Registo | Conteúdo | Tipo de dados |
|---|---|---|
| 0x28-0x29 | Centro X | int16 BE |
| 0x2A-0x2B | Centro Y | int16 BE |
| 0x2C-0x2D | ID reconhecido | int16 BE |

## Código

### Motor de deteção principal

`components/modules/ai/who_color_detection.cpp` — com base no ColorDetector do esp-dl:

```cpp
// Construir o detetor; set_bgr(false) garante canais de cor corretos
ColorDetector detector;
detector.set_bgr(false);
detector.set_detection_shape({80, 80, 1});
// Registar os limiares das 7 cores
detector.register_color({h_lo, h_hi, s_lo, s_hi, v_lo, v_hi}, area_min, "red");
// Deteção
auto &results = detector.detect((uint16_t *)frame->buf,
    {(int)frame->height, (int)frame->width, 3});

// Percorrer os resultados, desenhar caixas + etiquetas
for (int ci = 0; ci < (int)results.size(); ci++) {
    for (int ri = 0; ri < (int)results[ci].size(); ri++) {
        color_detect_result_t &res = results[ci][ri];
        draw_rect(frame, res.box[0], res.box[1], res.box[2], res.box[3], color_lcd);
        fb_gfx_print(frame, lx, ly, color_lcd, color_name);
    }
}
```

## Efeito

Objeto vermelho/verde/azul → cor reconhecida → caixa + etiqueta desenhadas → coordenadas pela I2C → é possível ligar um servo para triagem.
