# Kapitel 10: KI-Bildverständnis

## Über dieses Kapitel

Das KI-Bildverständnis ist eine exklusive Funktion von **ESP-Claw (Modus 7)** und wird im XiaoZhi AI (Modus 6) nicht verwendet.
> Bei den in diesem Kapitel verwendeten Tools `self.camera.take_photo` und `self.camera.inspect_image` wird die Adresse der visuellen Analyse-API vom Server während des MCP-Handshakes automatisch über das Feld `capabilities.vision` übermittelt. Die Firmware muss die API-URL nicht manuell konfigurieren — das bedeutet, die API-Konfiguration erfolgt in der xiaozhi.me-Konsole oder auf einem selbst gehosteten Server; Details siehe Kapitel 11: ESP-Claw-Sprachsteuerung.

## Funktionsprinzip

Der vollständige Ablauf der visuellen Analyse:

```
Nutzer-Sprachbefehl "Schau, was auf dem Tisch ist"
  → ASR-Spracherkennung
  → LLM-Entscheidung: Fotoanalyse erforderlich → Aufruf von self.camera.take_photo oder self.camera.inspect_image
  → Firmware: esp_camera_fb_get() erfasst Frame (VGA RGB565)
  → JPEG-Komprimierung
  → Über Explain() an die vom Server übermittelte Vision-API senden
  → Multimodales LLM gibt eine Textbeschreibung zurück
  → TTS-Sprachausgabe
```

### Unterschied der beiden Foto-Tools

| Tool | Zweck | Wer sendet die Vision-API |
|---|---|---|
| `self.camera.take_photo` | Nach dem Foto mit der integrierten Vision-Fähigkeit des LLM beschreiben | Serverseite |
| `self.camera.inspect_image` (NanoCam-spezifisch) | Nach dem Foto `camera->Explain()` aufrufen → HTTP POST an eine unabhängige multimodale API | Firmware-Seite |

Der Unterschied: `take_photo` nutzt die LLM-Vision der XiaoZhi-Serverseite (allgemeine Implementierung); `inspect_image` ist die projektspezifische Implementierung, bei der die Firmware direkt eine unabhängige multimodale API aufruft (Adresse wird vom Server übermittelt).

## Schritte

### 11.1 ESP-Claw-Modus sicherstellen

```
ai_mode:7
```

Nach dem Neustart wechselt das Gerät in den ESP-Claw-Modus.

### 11.2 Foto + KI-Analyse

Stellen Sie nach dem Aktivierungswort direkt Ihre Frage:

```
💬 "Schau, was hier ist"
💬 "Ist eine Tasse vor mir"
💬 "Welche Farbe hat dieses Buch"
💬 "Wie viele Äpfel liegen auf dem Tisch"
💬 "Hilf mir zu lesen, was auf diesem Blatt steht"
```

Der NanoCam nimmt ein Foto auf, lädt es hoch, analysiert es und antwortet dann per Sprache.

### 11.3 Beispiele für die Szenenerkennung

| Spracheingabe | Beispiel der KI-Antwort |
|---|---|
| "Was ist das" | "Das ist ein schwarzer Laptop, daneben steht eine weiße Kaffeetasse" |
| "Gibt es Äpfel" | "Ich sehe keine Äpfel. Auf dem Tisch liegen zwei Bücher und ein Stift" |
| "Welche Farbe" | "Das, worauf Sie zeigen, ist eine rote Tasse" |
| "Wie viele Tassen" | "Im Bild sind 2 Tassen" |

## Code

### Zentraler Foto- + Analyse-Callback

`nanocam_espclaw/main/boards/nanocam/nanocam_board.cc` — MCP-Tool-Registrierung:

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

`nanocam_espclaw/main/boards/common/esp32_camera.cc` — Explain()-Implementierung:

```cpp
std::string Esp32Camera::Explain(const std::string &question) {
    // explain_url_ wird vom Server beim MCP-Handshake über capabilities.vision.url übermittelt
    // Frame erfassen → JPEG-Komprimierung → HTTP POST an die multimodale API
    // Analyseergebnis des LLM zurückgeben
}
```

### MCP-Handshake auf der Serverseite (Übermittlung der Vision-API)

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

Nach Empfang ruft die Firmware `camera->SetExplainUrl(url, token)` auf, um die API-Adresse zu speichern; bei späteren `inspect_image`-Aufrufen wird sie direkt verwendet.

## Unterstützte multimodale Modelle

Durch die Übermittlung unterschiedlicher `vision.url`-Werte vom Server kann jede OpenAI-kompatible API verwendet werden:

| Modell | Beispiel-API-Adresse | Anwendungsszenario |
|---|---|---|
| `gpt-4o` | `https://api.openai.com/v1/chat/completions` | Stärkste Gesamtleistung |
| `gpt-4o-mini` | `https://api.openai.com/v1/chat/completions` | Bestes Preis-Leistungs-Verhältnis |
| `qwen-vl-max` | `https://dashscope.aliyuncs.com/compatible-mode/v1/chat/completions` | Besseres chinesisches Sprachverständnis |
| `llava:13b` (Ollama) | `http://localhost:11434/v1/chat/completions` | Vollständig offline |
| `claude-fable-5` | Proxy-Konfiguration erforderlich | Detaillierte Szenenbeschreibung |

## Ergebnis

"Schau, was hier ist" → Foto hochladen → KI-Analyse → Sprachausgabe "I see a red cup on a wooden table" — echte KI-Augen.
