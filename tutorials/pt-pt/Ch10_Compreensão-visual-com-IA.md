# Capítulo 10: Compreensão visual com IA

## Sobre este capítulo

A compreensão visual com IA é uma funcionalidade exclusiva do **ESP-Claw (modo 7)** e não é usada no XiaoZhi AI (modo 6).
> Quanto às ferramentas `self.camera.take_photo` e `self.camera.inspect_image` usadas neste capítulo, o endereço da API de análise visual é enviado automaticamente pelo servidor através do campo `capabilities.vision` na fase de handshake MCP. O firmware não precisa de configurar manualmente o URL da API — isto significa que a configuração da API é feita na consola xiaozhi.me ou no servidor próprio; para mais detalhes, consulte o Capítulo 11: Controlo por voz ESP-Claw.

## Princípio

O fluxo completo da análise visual:

```
Voz do utilizador "Vê o que está na mesa"
  → reconhecimento de voz ASR
  → decisão do LLM: é preciso fotografar e analisar → chamar self.camera.take_photo ou self.camera.inspect_image
  → firmware: esp_camera_fb_get() captura o fotograma (VGA RGB565)
  → compressão JPEG
  → envio através de Explain() para a Vision API indicada pelo servidor
  → o LLM multimodal devolve uma descrição textual
  → anúncio por voz TTS
```

### Diferenças entre as duas ferramentas de fotografia

| Ferramenta | Utilização | Quem envia para a Vision API |
|---|---|---|
| `self.camera.take_photo` | Depois de fotografar, usa a capacidade de visão integrada do LLM para descrever | Servidor |
| `self.camera.inspect_image` (específica do NanoCam) | Depois de fotografar, chama `camera->Explain()` → HTTP POST para uma API multimodal independente | Firmware |

A diferença entre as duas: `take_photo` usa a visão do LLM do servidor XiaoZhi (implementação genérica); `inspect_image` é a implementação específica deste projeto, em que o firmware chama diretamente uma API multimodal independente (endereço indicado pelo servidor).

## Passos

### 11.1 Garantir o modo ESP-Claw

```
ai_mode:7
```

Após o reinício, o dispositivo entra no modo ESP-Claw.

### 11.2 Fotografar + análise de IA

Após a ativação por voz, faça a pergunta diretamente:

```
💬 "Vê o que está aqui"
💬 "Tenho um copo à minha frente?"
💬 "De que cor é este livro"
💬 "Quantas maçãs estão na mesa"
💬 "Vê o que está escrito nesta folha de papel"
```

O NanoCam tira a fotografia, envia-a, analisa-a e responde por voz com o resultado.

### 11.3 Exemplos de reconhecimento de cenas

| Entrada de voz | Exemplo de resposta da IA |
|---|---|
| "O que é isto" | "É um portátil preto, com uma caneca de café branca ao lado" |
| "Há maçãs?" | "Não vejo maçãs. Há dois livros e uma caneta na mesa" |
| "De que cor é" | "O que está a apontar é uma caneca vermelha" |
| "Quantos copos" | "Há 2 copos na imagem" |

## Código

### Callback principal de fotografia + análise

`nanocam_espclaw/main/boards/nanocam/nanocam_board.cc` — registo das ferramentas MCP:

```cpp
mcp.AddTool("self.camera.inspect_image",
    "Take a photo with the camera and send it to the vision AI for analysis.",
    PropertyList({ Property("prompt", kPropertyTypeString) }),
    [this](const PropertyList &props) -> ReturnValue {
        auto camera = GetCamera();
        if (!camera->Capture()) {
            return std::string("{\"error\":\"Camera capture failed\"}");
        }
        std::string prompt = props["prompt"].value<std::string>();
        return camera->Explain(prompt);
    });
```

`nanocam_espclaw/main/boards/common/esp32_camera.cc` — implementação de Explain():

```cpp
std::string Esp32Camera::Explain(const std::string &question) {
    // explain_url_ é enviado pelo servidor no handshake MCP através de capabilities.vision.url
    // captura do fotograma → compressão JPEG → HTTP POST para a API multimodal
    // devolve o resultado da análise do LLM
}
```

### Handshake MCP no servidor (entrega da Vision API)

```json
{
  "capabilities": {
    "vision": {
      "url": "https://api.openai.com/v1/chat/completions",
      "token": "sk-..."
    }
  }
}
```

Após receber isto, o firmware chama `camera->SetExplainUrl(url, token)` para guardar o endereço da API, usado diretamente nas chamadas posteriores de `inspect_image`.

## Modelos multimodais suportados

Através do envio de diferentes `vision.url` pelo servidor, é possível usar qualquer API compatível com OpenAI:

| Modelo | Exemplo de endereço da API | Cenário de aplicação |
|---|---|---|
| `gpt-4o` | `https://api.openai.com/v1/chat/completions` | Capacidade global mais forte |
| `gpt-4o-mini` | `https://api.openai.com/v1/chat/completions` | Boa relação qualidade/preço |
| `qwen-vl-max` | `https://dashscope.aliyuncs.com/compatible-mode/v1/chat/completions` | Melhor compreensão de chinês |
| `llava:13b` (Ollama) | `http://localhost:11434/v1/chat/completions` | Totalmente offline |
| `claude-fable-5` | Requer configuração de proxy | Descrições de cena detalhadas |

## Efeito

"Vê o que está aqui" → fotografar e enviar → análise da IA → anúncio por voz "I see a red cup on a wooden table" —— verdadeiros olhos de IA.
