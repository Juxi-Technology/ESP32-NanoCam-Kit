# Capítulo 9: Diálogo por voz (XiaoZhi AI)

## Sobre este capítulo

Este capítulo aborda o modo XiaoZhi AI (`ai_mode:6`). **Importante**: o modo 6 (diálogo por voz) e o modo 7 (ESP-Claw) **partilham o mesmo firmware** (`nanocam_espclaw/`); no arranque, o dispositivo carrega apenas um conjunto de ferramentas MCP diferente, de acordo com o valor `ai_mode` guardado na NVS.

| Dimensão de comparação | XiaoZhi AI (modo 6) | ESP-Claw (modo 7) |
|---|---|---|
| Diálogo por voz | ✅ ASR→LLM→TTS | ✅ O mesmo pipeline de voz |
| Ferramentas MCP | Ferramentas gerais (volume/fotografia, etc.) | **Ferramentas gerais + 5 ferramentas específicas de hardware** |
| Compreensão visual | `self.camera.take_photo` | **`self.camera.inspect_image`** (visão multimodal) |
| Controlo do LED | ❌ | ✅ Ajuste de cor por voz |
| Cenários de aplicação | Diálogo geral com IA, educação infantil | Controlo de hardware, inspeção visual, casa inteligente |

> Este capítulo foca a funcionalidade central de diálogo por voz do **XiaoZhi AI (modo 6)**. Para conhecer as capacidades de controlo de hardware do ESP-Claw, consulte o Capítulo 11: Controlo por voz ESP-Claw.

## Princípio

O NanoCam integra a estrutura de código aberto XiaoZhi AI e liga-se a servidores LLM através dos protocolos WebSocket / MQTT para uma pipeline completa de interação por voz:

```
O utilizador fala → captação pelo microfone ES8311 → codificação Opus
  → WebSocket → reconhecimento de voz ASR na cloud
  → geração de resposta pelo LLM
  → síntese de voz TTS → descodificação Opus
  → amplificador NS4150B → reprodução no altifalante
```

Design full-duplex: o utilizador pode interromper enquanto a IA fala (barge-in), com uma experiência próxima de uma conversa real.

## Requisitos de hardware

Este capítulo envolve funcionalidades de áudio e requer o seguinte hardware:
- Placa de núcleo NanoCam (com codec ES8311 + microfone AP2718AT)
- Placa base NanoCam (com amplificador NS4150B + CH340K)
- Altifalante (ligado à interface de altifalante da placa base, VON/VOP)
> Apenas com a placa de núcleo também é possível testar (monitorização através da saída de auscultadores do ES8311). O microfone é um MEMS de silício analógico AP2718AT, ligado à entrada MIC1P do ES8311 através do condensador de bloqueio de DC C26.

## Passos

### 10.1 Gravar o firmware XiaoZhi AI

O XiaoZhi AI usa o projeto de firmware independente `nanocam_espclaw/`:

```bash
cd nanocam_espclaw
idf.py set-target esp32s3
idf.py build
idf.py -p COMx flash monitor
```

Após o arranque, entra por predefinição no modo XiaoZhi AI.

### 10.2 Ligar ao serviço cloud xiaozhi.me

O NanoCam liga-se por predefinição ao serviço cloud oficial [xiaozhi.me](https://xiaozhi.me) (gratuito), sem necessidade de servidor próprio.
1. Registe uma conta em [xiaozhi.me](https://xiaozhi.me)
2. Após ligar o dispositivo, este anuncia automaticamente um código de ativação de 6 dígitos
3. Introduza o código de ativação na consola xiaozhi.me → associar o dispositivo
4. Escolha o modelo LLM na consola (Qwen / DeepSeek, etc.)
A ativação só é necessária uma vez; depois disso, a ligação é automática em cada arranque.

### 10.3 Primeira conversa

Após ouvir o sinal sonoro, já pode conversar:

```
Utilizador: "你好小智 (ni hao xiao zhi), como está o tempo hoje?"
NanoCam: "Vou verificar o tempo de hoje para ti..."
```

A palavra de ativação é **"你好小智"** (predefinida).

### 10.4 Cenários de diálogo comuns

```
💬 "Conte uma piada"              → resposta por voz da IA
💬 "Põe um alarme para daqui a 5 minutos"    → função de alarme
💬 "Que horas são"             → informação da hora
💬 "Reproduz uma música calma"         → reprodução de música online
💬 "O que é um buraco negro"             → perguntas e respostas de conhecimento
```

## Servidor auto-alojado (opcional)

Se tiver requisitos de privacidade, ou quiser usar um LLM próprio, pode implementar o servidor de código aberto do XiaoZhi AI:

```bash
git clone https://github.com/xinnan-tech/xiaozhi-esp32-server
cd xiaozhi-esp32-server
pip install -r requirements.txt
python app.py
```

O endereço do servidor do firmware é distribuído pelo sistema OTA (`CONFIG_OTA_URL` no sdkconfig); após o arranque, o dispositivo pede automaticamente o endereço do servidor.
> O XiaoZhi AI usa o servidor de código aberto do XiaoZhi AI (protocolo privado WebSocket + pipeline ASR/LLM/TTS). No modo ESP-Claw, por cima disto, a funcionalidade de análise visual é entregue pelo servidor na fase de handshake MCP, que envia o URL e o token da Vision API; o firmware não precisa de os configurar.

## Resolução de problemas

| Sintoma | Causa possível | Solução |
|---|---|---|
| Não se ouve som | Altifalante não ligado | Verifique a interface de altifalante da placa base |
| Reconhecimento de voz impreciso | Ruído ambiente elevado | Fale perto do microfone (distância < 1m) |
| Não liga | WiFi não configurada | Configure primeiro a rede pela porta série `sta_ssid:xxx` |
| Sem código de ativação | Primeiro arranque não concluído | Aguarde 30 segundos; o dispositivo anuncia automaticamente |
| Respostas lentas | Latência do servidor LLM | Escolha um modelo mais rápido em xiaozhi.me, ou use um servidor próprio |
