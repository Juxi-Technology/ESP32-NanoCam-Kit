# Capítulo 6: Reconhecimento de cores

## Princípio

Baseia-se no espaço de cores HSV (matiz-saturação-valor). A imagem RGB565 da câmera é processada pelo mecanismo ColorDetector do esp-dl, que redimensiona a imagem para 80×80 para reduzir ruído, converte pixel a pixel em valores HSV e os compara com os 7 limiares de cor predefinidos.

### Limiares de cor predefinidos (escala H padrão do OpenCV, 0-180)

| Cor | Matiz (H) | Saturação (S) | Valor (V) | Limiar de área |
|---|---|---|---|---|
| Vermelho | 0-15 | 70-255 | 90-255 | 64 |
| Amarelo | 23-33 | 70-255 | 90-255 | 64 |
| Verde | 34-75 | 70-255 | 90-255 | 64 |
| Azul | 97-124 | 70-255 | 90-255 | 64 |
| Roxo | 125-155 | 70-255 | 90-255 | 64 |
| Branco | 0-180 | 0-40 | 200-255 | 80 |
| Preto | 0-180 | 0-255 | 0-50 | 80 |

> O matiz usa a escala 0-180 do OpenCV (correspondente a 0-360°). `set_bgr(false)` garante que a biblioteca leia os dados RGB565 como estão, sem trocar os canais.

## Passos

### 6.1 Entrar no modo de cores

```
ai_mode:3
```

O dispositivo reinicia automaticamente no modo de detecção de cores; o LED RGB WS2812 (GPIO18) mostra a cor reconhecida no momento.

### 6.2 Observar o resultado do reconhecimento

Coloque um objeto de cor sólida diante da câmera e abra `http://<IP>` no navegador; será exibido:
- **Retângulo colorido** marcando a região de cor detectada
- **Texto do rótulo de cor** (red/yellow/green/blue/purple/white/black)
- A cor da caixa e do rótulo corresponde à cor detectada
> O modo de cores apenas faz sobreposição na imagem (OSD) e não gera logs na porta serial. Para obter as coordenadas, leia os registradores I2C.

### 6.3 Ler os dados de detecção via I2C

O NanoCam atua como slave I2C (endereço `0x33`, GPIO SDA=41 SCL=42) e atualiza em tempo real as coordenadas do ponto central da caixa de detecção.

| Registrador | Conteúdo | Tipo de dado |
|---|---|---|
| 0x28-0x29 | Centro X | int16 BE |
| 0x2A-0x2B | Centro Y | int16 BE |
| 0x2C-0x2D | ID reconhecido | int16 BE |

## Código

### Mecanismo central de detecção

`components/modules/ai/who_color_detection.cpp` — baseado no ColorDetector do esp-dl:

```cpp
// Construir o detector; set_bgr(false) garante os canais de cor corretos
ColorDetector detector;
detector.set_bgr(false);
detector.set_detection_shape({80, 80, 1});
// Registrar os limiares das 7 cores
detector.register_color({h_lo, h_hi, s_lo, s_hi, v_lo, v_hi}, area_min, "red");
// Detecção
auto &results = detector.detect((uint16_t *)frame->buf,
    {(int)frame->height, (int)frame->width, 3});

// Percorrer os resultados e desenhar caixa + rótulo
for (int ci = 0; ci < (int)results.size(); ci++) {
    for (int ri = 0; ri < (int)results[ci].size(); ri++) {
        color_detect_result_t &res = results[ci][ri];
        draw_rect(frame, res.box[0], res.box[1], res.box[2], res.box[3], color_lcd);
        fb_gfx_print(frame, lx, ly, color_lcd, color_name);
    }
}
```

## Resultado

Objetos vermelhos/verdes/azuis → cor identificada → caixa + rótulo desenhados → coordenadas enviadas via I2C → prontas para triagem com um servo.
