# Capítulo 11: Controle por voz do ESP-Claw

## Sobre este capítulo

Quando o dispositivo é alternado para `ai_mode:7`, o NanoCam entra no modo ESP-Claw. Ele **compartilha o mesmo firmware** (`nanocam_espclaw/`) com o XiaoZhi AI (`ai_mode:6`); a única diferença é que o modo ESP-Claw, além da conversa por voz, registra mais 5 ferramentas de controle de hardware.

| Aspecto | XiaoZhi AI (mode 6) | ESP-Claw (mode 7) |
|---|---|---|
| Conversa por voz | ✅ ASR→LLM→TTS | ✅ O mesmo pipeline de voz |
| Controle do LED | ❌ | ✅ Ajuste de cor / ligar-desligar por voz |
| Troca de modo de IA | ❌ | ✅ Troca por voz |
| Foto + análise visual por IA | ❌ | ✅ Tira a foto e chama IA multimodal para entender a cena |

## Princípio

Sobre o pipeline de voz, o modo ESP-Claw registra 5 ferramentas exclusivas do NanoCam via `RegisterMcpTools()`:

```
Comando de voz "Mude a luz para azul"
  → Reconhecimento de voz ASR (nuvem)
  → O LLM entende a intenção → chamada de self.led.set_color({"r":0, "g":0, "b":255})
  → O LED WS2812 do NanoCam fica azul
  → TTS: "Pronto, a luz já está azul"
```

## Passos

### 12.1 Gravar o firmware

O ESP-Claw usa o projeto de firmware independente `nanocam_espclaw/`:

```bash
cd nanocam_espclaw
idf.py set-target esp32s3
idf.py build
idf.py -p COMx flash
```

### 12.2 Alternar o modo

Após iniciar, defina o modo ESP-Claw:

```
ai_mode:7
```

O dispositivo reinicia automaticamente e entra no modo. Use `ai_mode:6` para voltar ao modo XiaoZhi AI.

### 12.3 Exemplos de controle por voz

Após a ativação, basta dizer o que você precisa:

```
💬 "Acenda a luz"                          → o WS2812 acende em branco
💬 "Mude a luz para azul"                  → o LED fica azul
💬 "Apague a luz"                          → o LED apaga
💬 "Mude para o modo de detecção de rosto" → salva ai_mode:2 na NVS + reinício
💬 "Veja o que tem aqui"                   → foto + envio para análise da IA multimodal
💬 "Tem uma caneca na minha frente?"       → reconhecimento multimodal da imagem
```

### 12.4 Foto + análise visual por IA

Quando o usuário diz algo como "veja...", o firmware captura um quadro VGA RGB565, comprime em JPEG e o envia à API multimodal configurada no servidor para análise; o resultado é falado via TTS.
> A URL e o token da API multimodal são entregues automaticamente pelo servidor na fase de handshake da conexão; não é preciso digitar comandos de configuração manualmente na porta serial.

## As 5 ferramentas exclusivas do NanoCam

| Nome da ferramenta | Função | Parâmetros |
|---|---|---|
| `self.led.set_color` | Define a cor do LED RGB WS2812 (GPIO18) | `r,g,b`: 0-255 |
| `self.led.turn_off` | Desliga o LED | Nenhum |
| `self.camera.set_ai_mode` | Alterna o modo de IA (salvo na NVS + reinício) | `mode`: 0-7 |
| `self.camera.inspect_image` | Foto + análise visual por LLM multimodal | `prompt`: descrição da pergunta |
| `self.get_device_info` | Informações do dispositivo em JSON | Nenhum |

## Arquivos de configuração

| Conteúdo | Caminho |
|---|---|
| Registro das ferramentas MCP | `nanocam_espclaw/main/boards/nanocam/nanocam_board.cc` |
| Lógica de envio do Vision | `nanocam_espclaw/main/boards/common/esp32_camera.cc` |
| Configuração padrão do SDK | `nanocam_espclaw/sdkconfig.defaults` |

> O firmware ESP-Claw é um projeto independente e não compartilha código com o `nanocam_vision`. Os dois firmwares precisam ser compilados e gravados separadamente.

## Como escolher

| Sua necessidade | Modo recomendado |
|---|---|
| Apenas conversa por voz e perguntas | mode 6 (XiaoZhi) |
| Controle do LED por voz | mode 7 (ESP-Claw) |
| Tirar fotos e a IA "ver" a imagem | mode 7 (ESP-Claw) |
| Troca do modo de detecção por voz | mode 7 (ESP-Claw) |

> O uso completo do ESP-Claw (configuração do servidor, desenvolvimento de ferramentas MCP personalizadas etc.) ainda está em exploração; a documentação será atualizada continuamente conforme o avanço das pesquisas.
