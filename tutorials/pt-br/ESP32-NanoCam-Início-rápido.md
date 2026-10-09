# Início rápido do ESP32-NanoCam

---

## Preparação

- Placa principal + placa base do NanoCam (ESP32-S3 N16R8 + CH340K)
- Cabo USB Type-C (com suporte a transferência de dados)
- Computador (Windows / Mac / Linux)
- Módulo de câmera GC2145 (conectado de fábrica)

---

## Passo 1: Gravar o firmware (3 minutos)

### Método A: sem ambiente de desenvolvimento (recomendado)

1. Abra o navegador e acesse o [esptool-js](https://espressif.github.io/esptool-js/)
2. Conecte o NanoCam ao computador com um cabo Type-C
3. Selecione a porta serial, taxa de transmissão 115200
4. Localize o arquivo de firmware `nanocam_xxx.bin` dentro do pacote descompactado
5. Selecione o arquivo de firmware `nanocam_xxx.bin`, endereço `0x0`
6. Clique em "START" e aguarde a conclusão

### Método B: linha de comando (avançado)

```bash
pip install esptool
esptool.py --chip esp32s3 --port COM3 write_flash 0x0 nanocam.bin
```

---

## Passo 2: Conectar ao WiFi (2 minutos)

Por padrão, o NanoCam opera em **modo duplo AP+STA simultâneo**, sem necessidade de alternância:
- O **hotspot AP** fica sempre ativo; conecte o celular diretamente a `NanoCam-AP` (senha `12345678`) e abra `http://192.168.4.1` no navegador
- O **STA com roteador** exige configurar o WiFi uma vez:
Conecte o conector Type-C do NanoCam com uma ferramenta de porta serial (taxa de transmissão **115200 8N1**):

```
sta_ssid:MinhaWiFi
sta_pd:MinhaSenha
```

> O retorno `OK` indica que a configuração foi bem-sucedida. Após alterar a senha, o dispositivo reinicia automaticamente.
Para alternar o modo WiFi (normalmente não é necessário):

| Comando | Modo | Descrição |
|---|---|---|
| `wifi_mode:0` | Somente AP | Desativa o STA e mantém apenas o hotspot |
| `wifi_mode:1` | Somente STA | Desativa o hotspot e conecta apenas ao roteador |
| `wifi_mode:2` | AP+STA | Padrão; ambos operam simultaneamente |

---

## Passo 3: Ver a imagem (1 minuto)

1. Envie `sta_ip` pela porta serial para obter o IP do STA
2. Digite `http://<endereço IP>` no navegador (ou use `http://192.168.4.1` no modo AP)
3. A página exibe a imagem em tempo real

---

## Passo 4: Experimentar a IA (2 minutos)

Envie os comandos a seguir pela porta serial para alternar entre os modos:

| Comando | Modo | Efeito |
|---|---|---|
| `ai_mode:0` | Transmissão normal | Imagem MJPEG em tempo real |
| `ai_mode:1` | Detecção de rosto de gato | Caixa de detecção de rosto de gato na imagem |
| `ai_mode:2` | Detecção de rostos | Caixa de detecção de rostos na imagem |
| `ai_mode:3` | Reconhecimento de cores | Selecione a cor → rastreamento em tempo real |
| `ai_mode:4` | Reconhecimento facial | Cadastro → identificação → exclusão |
| `ai_mode:5` | Leitura de código QR | Aponte para o código QR → conteúdo na porta serial |
| `ai_mode:6` | Agente LLM | Ativação por voz "你好小智" (XiaoZhi AI) |
| `ai_mode:7` | ESP-Claw | ESP-Claw AI Agent (framework oficial da Espressif) |

> Cada troca de modo exige reinício manual; pressione o botão RST do módulo para reiniciar — o novo modo entra em vigor após o reinício.

---

## Passo 5: Integrar ao seu projeto

### Controle com Arduino

```cpp
Serial.begin(115200);
Serial.print("ai_mode:2");  // alternar para detecção de rostos
```

### Controle com Python

```python
import serial
ser = serial.Serial("COM3", 115200)
ser.write(b"ai_mode:1\r\n")  # alternar para detecção de rosto de gato
```

### Ver os comandos completos

→ Manual do protocolo serial

---

## Perguntas frequentes

| Problema | Solução |
|---|---|
| Falha na gravação | Verifique se o cabo Type-C suporta dados; mantenha o botão S2(BOOT) da placa base pressionado ao ligar novamente |
| Sem imagem | Envie `sta_ip` pela porta serial para confirmar o IP; verifique se estão na mesma rede |
| Câmera não funciona | Verifique se o cabo FPC está bem encaixado com os contatos metálicos para baixo; verifique PWDN(IO12)/RESET(IO14) |
| Não conecta ao WiFi | Envie `wifi_reset` para restaurar os padrões de fábrica e reconfigure |

Mais perguntas → [FAQ](https://FAQ.md)

---

## Próximos passos

- 📖 Manual do protocolo serial — referência completa dos comandos AT
- 🎓 Programa do tutorial — tutorial progressivo de 28 capítulos
- 🔧 Especificações de hardware — mapeamento completo dos pinos GPIO
- 🤖 Guia de integração ROS2 — tutorial de micro-ROS
