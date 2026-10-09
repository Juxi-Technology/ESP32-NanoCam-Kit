# Manual do protocolo serial do ESP32-NanoCam

> Taxa de transmissão: 115200 | Bits de dados: 8 | Paridade: nenhuma | Bits de parada: 1 | Controle de fluxo: nenhum
> Compatível com o conjunto de comandos AT dos principais módulos de câmera, com novos comandos estendidos do NanoCam.

---

## 1. Regras gerais

- Os comandos não diferenciam maiúsculas de minúsculas (`STA_SSID` = `sta_ssid`)
- Após o comando, é necessário um sinal de pontuação ASCII qualquer (`,` `.` `:` `;` etc.) como terminador
- Alguns comandos reiniciam automaticamente o dispositivo após a alteração
- Cada comando termina com `\r\n` (o assistente de porta serial normalmente adiciona automaticamente)

## 2. Configuração de WiFi

### Modo STA (conectar ao roteador)

| Comando | Descrição | Exemplo | Retorno |
|---|---|---|---|
| `sta_ssid:nome` | Define o nome do WiFi | `sta_ssid:MyWiFi` | `OK` |
| `sta_pd:senha` | Define a senha do WiFi (reinicia após alterar) | `sta_pd:12345678` | `OK` (reinicia) |

> O nome e a senha do WiFi têm no máximo 30 caracteres e não suportam caracteres chineses.

### Modo AP (hotspot próprio)

| Comando | Descrição | Exemplo | Retorno |
|---|---|---|---|
| `ap_ssid:nome` | Define o nome do hotspot | `ap_ssid:NanoCam-AP` | `OK` |
| `ap_pd:senha` | Define a senha do hotspot (reinicia após alterar) | `ap_pd:12345678` | `OK` (reinicia) |

### Modo WiFi

| Comando | Descrição | Parâmetro | Retorno |
|---|---|---|---|
| `wifi_mode:X` | Alterna o modo | 0=AP 1=STA 2=AP+STA | `OK` (reinicia quando alterado) |

---

## 3. Troca de modos de IA

| Comando | Modo | Descrição | Reinício |
|---|---|---|---|
| `ai_mode:0` | Normal | Transmissão MJPEG, sem IA | ✅ |
| `ai_mode:1` | Detecção de rosto de gato | Caixa desenhada no rosto de gato em tempo real + confiança | ✅ |
| `ai_mode:2` | Detecção de rostos | Caixa desenhada no rosto em tempo real + coordenadas | ✅ |
| `ai_mode:3` | Reconhecimento de cores | Selecione a cor → detecção em tempo real | ✅ |
| `ai_mode:4` | Reconhecimento facial | Cadastro → identificação → exclusão | ✅ |
| `ai_mode:5` | Código QR | Decodificação em tempo real → saída pela porta serial | ✅ |
| `ai_mode:6` | Agente LLM | Conversa por voz XiaoZhi AI + visão por IA | ✅ |
| `ai_mode:7` | ESP-Claw | Controle por voz + análise visual por foto + OpenAI Vision | ✅ |

> Valores válidos de `ai_mode`: 0-7. Fora desse intervalo, o valor padrão passa a ser 0. O dispositivo reinicia automaticamente após a alteração; o novo modo entra em vigor após o reinício.

---

## 4. Consulta de informações

| Comando | Descrição | Exemplo de retorno |
|---|---|---|
| `sta_ip` | Consulta o IP do STA | `sta_ip:192.168.1.100` |
| `ap_ip` | Consulta o IP do AP | `ap_ip:192.168.4.1` |
| `wifi_ver` | Consulta a versão do firmware | `NanoCam Board Ver:0.2.0` |

---

## 5. Controle do sistema

| Comando | Descrição | Retorno |
|---|---|---|
| `wifi_reset` | Restaura as configurações de fábrica (reinicia) | `Reset_OK` |
| `nano_reboot` | Reset por software | `Rebooting...` |
| `nano_info` | Informações completas do dispositivo (JSON) | Ver abaixo |

### Exemplo de retorno do nano_info

```json
{
  "device": "NanoCam",
  "ver": "0.2.0",
  "chip": "ESP32-S3",
  "flash": "16MB",
  "psram": "8MB",
  "ai_mode": 1,
  "wifi_mode": 2,
  "sta_ip": "192.168.1.100",
  "free_heap": 245760
}
```

---

## 6. Comandos exclusivos do reconhecimento facial

> Válido apenas no ai_mode:4 (modo de reconhecimento facial).

| Comando | Descrição | Comportamento do rótulo | Exemplo de retorno |
|---|---|---|---|
| `face_eril` | Registra o rosto detectado no quadro atual | Azul "Enroll: ID N", pisca por 0.5s | `>>> face enroll triggered` |
| `face_rz` | Entra no modo de reconhecimento facial contínuo | Verde "ID: N" / vermelho "who?", **permanece visível sem desaparecer** | `>>> face recognize triggered` |
| `face_del` | Exclui o último ID de face registrado | Vermelho "N IDs left", pisca por 0.5s | `>>> face delete triggered` |
| `face_detect` | Sai do modo de reconhecimento e volta à detecção de rostos | Remove todos os rótulos | `>>> face detect mode` |

### Fluxo de operação do reconhecimento facial

```
ai_mode:4          # entra no modo de reconhecimento facial (o dispositivo reinicia automaticamente)
face_eril          # cadastra o rosto (garanta que apenas um rosto esteja no quadro)
face_rz            # inicia o reconhecimento contínuo — a etiqueta permanece visível
face_detect        # sai do modo de reconhecimento — as etiquetas são removidas
face_del           # exclui o último rosto cadastrado
```

### Observações sobre o reconhecimento facial

1. No cadastro, é preciso garantir que haja **apenas um rosto** no quadro, a uma distância de 30-50cm
2. No modo de reconhecimento (`face_rz`), o rótulo **permanece visível** e não desaparece após 0.5 segundo — este é o novo comportamento da versão 0.3.0
3. Para sair do modo de reconhecimento, é preciso enviar `face_detect`; caso contrário, o rótulo continua visível
4. As características faciais são armazenadas na partição `fr` da Flash, não se perdem com a falta de energia e suportam no máximo 47 IDs
5. O reconhecimento usa uma estratégia de salto de quadros (inferência MFN a cada 10 quadros)

---

## 7. Comandos estendidos (exclusivos do NanoCam)

| Comando | Descrição | Status |
|---|---|---|
| `nano_server:url` | Define o endereço do servidor LLM (salvo na NVS) | ✅ |
| `nano_api_key:key` | Define a chave de API do LLM (salva na NVS) | ✅ |
| `nano_mqtt:broker,port,topic` | Configura o servidor MQTT | 🔨 |
| `nano_led:R,G,B` | Define o LED RGB (WS2812, GPIO18 DIN) | 📋 |
| `nano_snap` | Fotografa e armazena (SPIFFS) | ✅ |
| `nano_stream:on/off` | Inicia/para a transmissão de imagem | 📋 |

### nano_server / nano_api_key

| Comando | Descrição | Exemplo | Retorno |
|---|---|---|---|
| `nano_server:URL` | Define o endereço do servidor LLM | `nano_server:https://api.openai.com` | `OK server=https://api.openai.com` |
| `nano_api_key:KEY` | Define a chave de API | `nano_api_key:sk-xxxx` | `OK` |

> Suporta qualquer API compatível com OpenAI (vLLM / Ollama / modelos locais são todos suportados).
O modo ESP-Claw (ai_mode:7) suporta o uso do `nano_server`; o XiaoZhi AI (ai_mode:6) usa uma configuração de servidor independente.

---

## 8. Observações

1. `sta_pd` / `ap_pd` reiniciam automaticamente após a alteração; a nova senha entra em vigor após o reinício
2. `ai_mode` reinicia automaticamente após a alteração (apenas quando o modo muda)
3. No modo de reconhecimento facial (mode 4), a configuração pela porta serial Type-C pode falhar (memória insuficiente)
4. O nome/a senha do WiFi não pode exceder 30 caracteres nem conter caracteres chineses
5. Após o comando, é necessário um sinal de pontuação como terminador
