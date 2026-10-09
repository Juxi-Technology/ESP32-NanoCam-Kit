# Guia de início rápido do ESP32-NanoCam

---

## Preparação

- Placa de núcleo NanoCam + placa base (ESP32-S3 N16R8 + CH340K)
- Cabo USB Type-C (com suporte a transferência de dados)
- Computador (Windows / Mac / Linux)
- Módulo de câmara GC2145 (ligado de fábrica)

---

## Passo 1: gravar o firmware (3 minutos)

### Método A: sem ambiente de desenvolvimento (recomendado)

1. Abra o navegador e aceda ao [esptool-js](https://espressif.github.io/esptool-js/)
2. Ligue o NanoCam ao computador com um cabo Type-C
3. Selecione a porta série, com velocidade de transmissão de 115200
4. Localize o ficheiro de firmware `nanocam_xxx.bin` dentro do pacote descomprimido
5. Selecione o ficheiro de firmware `nanocam_xxx.bin` e o endereço `0x0`
6. Clique em "START" e aguarde a conclusão

### Método B: linha de comandos (avançado)

```bash
pip install esptool
esptool.py --chip esp32s3 --port COM3 write_flash 0x0 nanocam.bin
```

---

## Passo 2: ligar ao WiFi (2 minutos)

O NanoCam funciona por predefinição em **modo duplo AP+STA em simultâneo**, sem necessidade de alternar:
- **O hotspot AP** está sempre ativo; ligue o telemóvel diretamente a `NanoCam-AP` (palavra-passe `12345678`) e abra `http://192.168.4.1` no navegador
- **A ligação STA ao router** requer uma configuração única do WiFi:
Use uma ferramenta de porta série (velocidade de transmissão **115200 8N1**) para ligar à porta Type-C do NanoCam:

```
sta_ssid:o_nome_da_tua_rede_WiFi
sta_pd:a_palavra_passe_da_tua_rede_WiFi
```

> Ao receber `OK`, a configuração foi bem-sucedida. Após a alteração da palavra-passe, o dispositivo reinicia automaticamente.
Se precisar de alternar o modo WiFi (normalmente não é necessário):

| Comando | Modo | Descrição |
|---|---|---|
| `wifi_mode:0` | Apenas AP | Desativa o STA e mantém apenas o hotspot |
| `wifi_mode:1` | Apenas STA | Desativa o hotspot e liga-se apenas ao router |
| `wifi_mode:2` | AP+STA | Predefinição, ambos funcionam em simultâneo |

---

## Passo 3: abrir a imagem (1 minuto)

1. Envie `sta_ip` pela porta série para obter o IP do STA
2. Introduza `http://<endereço IP>` no navegador (ou `http://192.168.4.1` em modo AP)
3. A página web mostra a imagem em tempo real

---

## Passo 4: explorar a IA (2 minutos)

Envie os seguintes comandos pela porta série para alternar os modos:

| Comando | Modo | Efeito |
|---|---|---|
| `ai_mode:0` | Transmissão normal | Imagem MJPEG em tempo real |
| `ai_mode:1` | Deteção de faces de gato | Aparece a caixa de deteção de faces de gato na imagem |
| `ai_mode:2` | Deteção de rostos | Aparece a caixa de deteção de rostos na imagem |
| `ai_mode:3` | Reconhecimento de cores | Selecione a cor com uma caixa → rastreamento em tempo real |
| `ai_mode:4` | Reconhecimento facial | Registar → identificar → eliminar |
| `ai_mode:5` | Leitura de códigos QR | Aponte para o código QR → conteúdo enviado pela porta série |
| `ai_mode:6` | Agente LLM | Ativação por voz "你好小智" (XiaoZhi AI) |
| `ai_mode:7` | ESP-Claw | ESP-Claw AI Agent (estrutura oficial da Espressif) |

> Cada mudança de modo requer um reinício manual; pode reiniciar premindo o botão RST do módulo. O novo modo fica ativo após o reinício.

---

## Passo 5: integrar no seu projeto

### Controlo com Arduino

```cpp
Serial.begin(115200);
Serial.print("ai_mode:2");  // Mudar para deteção de rostos
```

### Controlo com Python

```python
import serial
ser = serial.Serial("COM3", 115200)
ser.write(b"ai_mode:1\r\n")  # Mudar para deteção de caras de gato
```

### Ver todos os comandos

→ Manual do protocolo AT da porta série

---

## Perguntas frequentes

| Problema | Solução |
|---|---|
| Falha na gravação | Verifique se o cabo Type-C suporta dados; mantenha o botão S2 (BOOT) da placa base premido e volte a ligar a alimentação |
| Sem imagem | Envie `sta_ip` pela porta série para confirmar o IP; verifique se está na mesma sub-rede |
| Câmara não funciona | Verifique se os contactos metálicos do cabo FPC estão virados para baixo e bem encaixados; verifique PWDN(IO12)/RESET(IO14) |
| WiFi não liga | Envie `wifi_reset` para restaurar as definições de fábrica e configure novamente |

Mais perguntas → [FAQ](https://FAQ.md)

---

## Próximos passos

- 📖 Manual do protocolo AT da porta série — referência completa dos comandos AT
- 🎓 Programa do tutorial — tutorial progressivo com 28 capítulos
- 🔧 Especificações de hardware — mapeamento completo dos pinos GPIO
- 🤖 Guia de integração ROS2 — tutorial micro-ROS
