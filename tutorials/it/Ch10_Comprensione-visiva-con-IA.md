# Capitolo 10: Comprensione visiva con IA

## Informazioni su questo capitolo

La comprensione visiva con IA è una funzionalità esclusiva di **ESP-Claw (modalità 7)** e non viene utilizzata in XiaoZhi AI (modalità 6).
> Gli strumenti `self.camera.take_photo` e `self.camera.inspect_image` utilizzati in questo capitolo ricevono l'indirizzo dell'API di analisi visiva dal server durante la fase di handshake MCP, tramite il campo `capabilities.vision`. Il firmware non deve configurare manualmente l'URL dell'API — questo significa che la configurazione dell'API avviene nella console di xiaozhi.me o sul server autogestito; per i dettagli consulta il Capitolo 11: Controllo vocale ESP-Claw.

## Funzionamento

Il flusso completo dell'analisi visiva:

```
Comando vocale dell'utente "guarda cosa c'è sul tavolo"
  → riconoscimento vocale ASR
  → decisione dell'LLM: serve una foto da analizzare → chiama self.camera.take_photo o self.camera.inspect_image
  → firmware: esp_camera_fb_get() acquisisce un frame (VGA RGB565)
  → compressione JPEG
  → invio tramite Explain() alla Vision API fornita dal server
  → il LLM multimodale restituisce la descrizione testuale
  → annuncio vocale TTS
```

### Differenze tra i due strumenti di scatto

| Strumento | Utilizzo | Chi invia la Vision API |
|---|---|---|
| `self.camera.take_photo` | Scatta la foto e la descrive con la capacità vision integrata nell'LLM | Il server |
| `self.camera.inspect_image` (dedicato a NanoCam) | Scatta la foto e chiama `camera->Explain()` → HTTP POST verso un'API multimodale indipendente | Il firmware |

La differenza tra i due: `take_photo` passa per la visione LLM del server XiaoZhi AI (implementazione generica), mentre `inspect_image` è l'implementazione dedicata di questo progetto: il firmware chiama direttamente un'API multimodale indipendente (il cui indirizzo è fornito dal server).

## Procedura

### 11.1 Assicurarsi della modalità ESP-Claw

```
ai_mode:7
```

Dopo il riavvio il dispositivo entra in modalità ESP-Claw.

### 11.2 Scatto + analisi IA

Dopo il risveglio, poni direttamente la domanda:

```
💬 "Guarda cosa c'è qui"
💬 "C'è una tazza davanti a me"
💬 "Di che colore è questo libro"
💬 "Quante mele ci sono sul tavolo"
💬 "Aiutami a vedere cosa c'è scritto su questo foglio"
```

NanoCam scatterà la foto, la caricherà, la analizzerà e risponderà a voce con il risultato.

### 11.3 Esempi di riconoscimento di scenari

| Input vocale | Esempio di risposta dell'IA |
|---|---|
| "Che cos'è questo" | "Questo è un portatile nero, accanto c'è una tazza di caffè bianca" |
| "Ci sono mele" | "Non vedo mele. Sul tavolo ci sono due libri e una penna" |
| "Di che colore è" | "Quello che stai indicando è una tazza rossa" |
| "Quante tazze ci sono" | "Nell'immagine ci sono 2 tazze" |

## Codice

### Callback principale di scatto + analisi

`nanocam_espclaw/main/boards/nanocam/nanocam_board.cc` — registrazione dello strumento MCP:

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

`nanocam_espclaw/main/boards/common/esp32_camera.cc` — implementazione di Explain():

```cpp
std::string Esp32Camera::Explain(const std::string &question) {
    // explain_url_ viene fornito dal server durante l'handshake MCP tramite capabilities.vision.url
    // acquisizione frame → compressione JPEG → HTTP POST all'API multimodale
    // restituisce il risultato dell'analisi LLM
}
```

### Handshake MCP lato server (invio della Vision API)

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

Alla ricezione, il firmware chiama `camera->SetExplainUrl(url, token)` per salvare l'indirizzo dell'API, che viene poi utilizzato direttamente nelle successive chiamate a `inspect_image`.

## Modelli multimodali supportati

Fornendo dal server valori diversi di `vision.url` è possibile utilizzare qualsiasi API compatibile con OpenAI:

| Modello | Esempio di indirizzo API | Scenario d'uso |
|---|---|---|
| `gpt-4o` | `https://api.openai.com/v1/chat/completions` | Migliori capacità complessive |
| `gpt-4o-mini` | `https://api.openai.com/v1/chat/completions` | Ottimo rapporto qualità-prezzo |
| `qwen-vl-max` | `https://dashscope.aliyuncs.com/compatible-mode/v1/chat/completions` | Migliore comprensione del cinese |
| `llava:13b` (Ollama) | `http://localhost:11434/v1/chat/completions` | Completamente offline |
| `claude-fable-5` | Richiede la configurazione di un proxy | Descrizioni dettagliate delle scene |

## Risultato

"Guarda cosa c'è qui" → scatta la foto e la carica → l'IA analizza → annuncio vocale "I see a red cup on a wooden table" — un vero occhio IA.
