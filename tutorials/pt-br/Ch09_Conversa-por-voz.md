# Capítulo 9: Conversa por voz (XiaoZhi AI)

## Sobre este capítulo

Este capítulo aborda o modo XiaoZhi AI (`ai_mode:6`). **Importante**: o modo 6 (conversa por voz) e o modo 7 (ESP-Claw) **compartilham o mesmo firmware** (`nanocam_espclaw/`); a diferença é que, ao iniciar, o dispositivo carrega conjuntos diferentes de ferramentas MCP de acordo com o valor de `ai_mode` armazenado na NVS.

| Aspecto | XiaoZhi AI (mode 6) | ESP-Claw (mode 7) |
|---|---|---|
| Conversa por voz | ✅ ASR→LLM→TTS | ✅ O mesmo pipeline de voz |
| Ferramentas MCP | Ferramentas gerais (volume/foto etc.) | **Ferramentas gerais + 5 ferramentas dedicadas ao hardware** |
| Compreensão visual | `self.camera.take_photo` | **`self.camera.inspect_image`** (visão multimodal) |
| Controle do LED | ❌ | ✅ Ajuste de cor por voz |
| Cenários de uso | Conversa geral com IA, educação infantil | Controle de hardware, inspeção visual, casa inteligente |

> Este capítulo foca na função central de conversa por voz do **XiaoZhi AI (modo 6)**. Para conhecer os recursos de controle de hardware do ESP-Claw, leia o Capítulo 11: Controle por voz do ESP-Claw.

## Princípio

O NanoCam integra o framework de código aberto XiaoZhi AI e, por meio dos protocolos WebSocket / MQTT, conecta-se a um servidor LLM para implementar um pipeline completo de interação por voz:

```
O usuário fala → captura pelo microfone ES8311 → codificação Opus
  → WebSocket → reconhecimento de voz ASR na nuvem
  → O LLM gera a resposta
  → Síntese de voz TTS → decodificação Opus
  → Amplificador NS4150B → reprodução no alto-falante
```

Design full-duplex: o usuário pode interromper (barge-in) enquanto a IA fala, proporcionando uma experiência próxima de uma conversa real.

## Requisitos de hardware

Este capítulo envolve funções de áudio e requer o seguinte hardware:
- Placa principal do NanoCam (com codec ES8311 + microfone AP2718AT)
- Placa base do NanoCam (com amplificador NS4150B + CH340K)
- Alto-falante (conectado à interface de alto-falante da placa base, VON/VOP)
> Também é possível testar apenas com a placa principal (monitorando pela saída de fone de ouvido do ES8311). O microfone é um MEMS de silício analógico AP2718AT, conectado ao MIC1P do ES8311 através do capacitor de bloqueio C26.

## Passos

### 10.1 Gravar o firmware XiaoZhi AI

O XiaoZhi AI usa o projeto de firmware independente `nanocam_espclaw/`:

```bash
cd nanocam_espclaw
idf.py set-target esp32s3
idf.py build
idf.py -p COMx flash monitor
```

Após iniciar, o modo XiaoZhi AI já é o padrão.

### 10.2 Conectar ao serviço de nuvem xiaozhi.me

De fábrica, o NanoCam se conecta por padrão ao serviço oficial de nuvem [xiaozhi.me](https://xiaozhi.me) (gratuito); não é preciso montar um servidor próprio.
1. Crie uma conta em [xiaozhi.me](https://xiaozhi.me)
2. Ao ligar, o dispositivo anuncia automaticamente um código de ativação de 6 dígitos
3. Insira o código de ativação no console do xiaozhi.me → vincule o dispositivo
4. Escolha o modelo LLM no console (Qwen / DeepSeek etc.)
A ativação é necessária apenas uma vez; depois, a conexão é automática a cada vez que o dispositivo é ligado.

### 10.3 Primeira conversa

Após ouvir o tom de aviso, você já pode conversar:

```
Você: "你好小智, como está o tempo hoje?"
NanoCam: "Vou verificar o tempo de hoje para você..."
```

A palavra de ativação é **"你好小智"** (Olá, XiaoZhi; padrão).

### 10.4 Cenários comuns de conversa

```
💬 "Conte uma piada"                           → resposta por voz da IA
💬 "Configure um alarme de 5 minutos para mim" → função de alarme
💬 "Que horas são"                             → anúncio da hora
💬 "Toque uma música leve"                     → reprodução de música pela internet
💬 "O que é um buraco negro"                   → perguntas de conhecimento
```

## Servidor auto-hospedado (opcional)

Se você tem exigências de privacidade ou quer usar um LLM próprio, pode implantar o servidor de código aberto do XiaoZhi AI:

```bash
git clone https://github.com/xinnan-tech/xiaozhi-esp32-server
cd xiaozhi-esp32-server
pip install -r requirements.txt
python app.py
```

O endereço do servidor do firmware é entregue pelo sistema OTA (`CONFIG_OTA_URL` no sdkconfig); ao ligar, o dispositivo solicita o endereço do servidor automaticamente.
> O XiaoZhi AI usa o servidor de código aberto do XiaoZhi AI (protocolo proprietário WebSocket + pipeline ASR/LLM/TTS). No modo ESP-Claw, sobre essa base, a função de análise visual tem a URL e o token da Vision API entregues pelo servidor durante o handshake MCP, sem necessidade de configuração no firmware.

## Solução de problemas

| Sintoma | Causa possível | Solução |
|---|---|---|
| Sem som | Alto-falante não conectado | Verifique a interface de alto-falante da placa base |
| Reconhecimento de voz impreciso | Ruído ambiente alto | Fale perto do microfone (distância < 1m) |
| Não conecta | WiFi não configurado | Configure a rede primeiro pela porta serial: `sta_ssid:xxx` |
| Sem código de ativação | Primeira inicialização não concluída | Aguarde 30 segundos; o dispositivo o anunciará automaticamente |
| Respostas lentas | Latência do servidor LLM | Escolha um modelo mais rápido no xiaozhi.me ou use um servidor próprio |
