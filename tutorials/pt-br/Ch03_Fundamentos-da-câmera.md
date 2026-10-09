# Capítulo 3: Fundamentos da câmera

## Princípio

O NanoCam conecta a câmera pela interface DVP (vídeo paralelo digital). O sensor GC2145 envia dados de pixel paralelos de 8 bits; o periférico LCD_CAM do ESP32-S3 os armazena diretamente na PSRAM via DMA, e o servidor HTTP transmite o fluxo em formato MJPEG para o navegador.

### Conceitos-chave

- **DVP**: dados paralelos de 8 linhas + 3 linhas de sincronização (VSYNC/HREF/PCLK)
- **MJPEG**: cada quadro é uma imagem JPEG independente; o navegador as carrega em sequência para produzir o efeito de vídeo
- **PSRAM**: 8 MB de PSRAM usados como buffer de quadros, com capacidade para 2-4 quadros

## Passos

### 3.1 Ver a imagem padrão

Após a gravação do firmware, o modo padrão é o de transmissão de imagem; abra `http://<IP>` no navegador para ver a imagem.

| Comando | Função |
|---|---|
| ai_mode:0\r | Transmissão de imagem |
| ai_mode:1\r | Detecção de rosto de gato |
| ai_mode:2\r | Detecção de rosto |
| ai_mode:3\r | Reconhecimento de cores |
| ai_mode:4\r | Reconhecimento facial |
| ai_mode:5\r | Leitura de código QR |
