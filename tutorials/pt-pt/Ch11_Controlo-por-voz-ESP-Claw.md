# Capítulo 11: Controlo por voz ESP-Claw

## Sobre este capítulo

Quando o dispositivo é mudado para `ai_mode:7`, o NanoCam entra no modo ESP-Claw. Este **partilha o mesmo firmware** (`nanocam_espclaw/`) com o XiaoZhi AI (`ai_mode:6`); a única diferença é que o modo ESP-Claw, além do diálogo por voz, regista mais 5 ferramentas de controlo de hardware.

| Dimensão de comparação | XiaoZhi AI (modo 6) | ESP-Claw (modo 7) |
|---|---|---|
| Diálogo por voz | ✅ ASR→LLM→TTS | ✅ O mesmo pipeline de voz |
| Controlo do LED | ❌ | ✅ Ajuste de cor / ligar-desligar por voz |
| Mudança de modo de IA | ❌ | ✅ Mudança por voz |
| Fotografia + análise visual com IA | ❌ | ✅ Fotografar e chamar IA multimodal para compreender a imagem |

## Princípio

O modo ESP-Claw, por cima do pipeline de voz, regista 5 ferramentas específicas do NanoCam através de `RegisterMcpTools()`:

```
Voz do utilizador "Muda a luz para azul"
  → reconhecimento de voz ASR (cloud)
  → o LLM compreende a intenção → chama self.led.set_color({"r":0, "g":0, "b":255})
  → o LED WS2812 do NanoCam fica azul
  → TTS: "Muito bem, a luz ficou azul"
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

### 12.2 Mudar de modo

Após o arranque, defina o modo ESP-Claw:

```
ai_mode:7
```

O dispositivo reinicia automaticamente e entra nesse modo. Use `ai_mode:6` para voltar ao modo XiaoZhi AI.

### 12.3 Exemplos de controlo por voz

Após a ativação por voz, diga diretamente o que precisa:

```
💬 "Acende a luz"              → WS2812 acende a branco
💬 "Muda a luz para azul"          → LED fica azul
💬 "Apaga a luz"                  → LED apaga
💬 "Muda para o modo de deteção de rostos"    → guarda ai_mode:2 na NVS + reinicia
💬 "Vê o que está aqui"        → fotografar + enviar para análise da IA multimodal
💬 "Tenho um copo à minha frente?"        → a IA multimodal reconhece a imagem
```

### 12.4 Fotografia + análise visual com IA

Quando o utilizador diz "Vê...", o firmware captura um fotograma VGA RGB565, comprime-o em JPEG e envia-o para a API multimodal configurada no servidor para análise; o resultado é anunciado por voz através de TTS.
> O URL e o token da API multimodal são enviados automaticamente pelo servidor na fase de handshake da ligação; não é necessário introduzir comandos de configuração manualmente pela porta série.

## As 5 ferramentas específicas do NanoCam

| Nome da ferramenta | Função | Parâmetros |
|---|---|---|
| `self.led.set_color` | Definir o LED RGB WS2812 (GPIO18) | `r,g,b`: 0-255 |
| `self.led.turn_off` | Desligar o LED | Nenhum |
| `self.camera.set_ai_mode` | Mudar o modo de IA (grava na NVS + reinicia) | `mode`: 0-7 |
| `self.camera.inspect_image` | Fotografar + análise visual por LLM multimodal | `prompt`: descrição da pergunta |
| `self.get_device_info` | Informações do dispositivo em JSON | Nenhum |

## Ficheiros de configuração

| Conteúdo | Caminho |
|---|---|
| Registo das ferramentas MCP | `nanocam_espclaw/main/boards/nanocam/nanocam_board.cc` |
| Lógica de envio para a Vision | `nanocam_espclaw/main/boards/common/esp32_camera.cc` |
| Configuração predefinida do SDK | `nanocam_espclaw/sdkconfig.defaults` |

> O firmware ESP-Claw é um projeto independente e não partilha código com o `nanocam_vision`. Os dois firmwares têm de ser compilados e gravados separadamente.

## Como escolher

| A sua necessidade | Modo recomendado |
|---|---|
| Só quero conversar por voz e perguntas/respostas | modo 6 (XiaoZhi) |
| Quero controlar o LED por voz | modo 7 (ESP-Claw) |
| Quero fotografar + IA a "ver" a imagem | modo 7 (ESP-Claw) |
| Quero mudar o modo de deteção de IA por voz | modo 7 (ESP-Claw) |

> O modo de utilização completo do ESP-Claw (configuração do servidor, desenvolvimento de ferramentas MCP personalizadas, etc.) ainda está em exploração e a documentação será atualizada à medida que a investigação avançar.
