# Capítulo 3: Noções básicas da câmara

## Princípio

O NanoCam liga a câmara através da interface DVP (vídeo digital paralelo). O sensor GC2145 emite dados de píxeis paralelos de 8 bits; o periférico LCD_CAM do ESP32-S3 armazena-os diretamente na PSRAM por DMA e, em seguida, o servidor HTTP transmite-os em formato MJPEG para o navegador.

### Conceitos-chave

- **DVP**: dados paralelos de 8 linhas + 3 linhas de sinais de sincronização (VSYNC/HREF/PCLK)
- **MJPEG**: cada fotograma é uma imagem JPEG independente; o navegador carrega-os em sequência, criando o efeito de vídeo
- **PSRAM**: 8 MB de PSRAM utilizados como buffer de fotogramas, com capacidade para 2 a 4 fotogramas

## Passos

### 3.1 Ver a imagem predefinida

Após a gravação do firmware, o modo predefinido é o de transmissão; abra `http://<IP>` no navegador para ver a imagem.

| Comando | Função |
|---|---|
| ai_mode:0\r | Transmissão de vídeo |
| ai_mode:1\r | Deteção de faces de gato |
| ai_mode:2\r | Deteção de rostos |
| ai_mode:3\r | Reconhecimento de cores |
| ai_mode:4\r | Reconhecimento facial |
| ai_mode:5\r | Leitura de códigos QR |
