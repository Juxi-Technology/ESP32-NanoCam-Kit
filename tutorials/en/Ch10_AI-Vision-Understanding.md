# Chapter 10: AI Vision Understanding

## About This Chapter

AI vision understanding is a feature exclusive to **ESP-Claw (mode 7)**; it is not used in XiaoZhi AI (mode 6).
> For the `self.camera.take_photo` and `self.camera.inspect_image` tools used in this chapter, the vision analysis API address is delivered automatically by the server during the MCP handshake through the `capabilities.vision` field. The firmware side does not need to configure the API URL manually — this means the API configuration is done in the xiaozhi.me console or on a self-hosted server; for details see Chapter 11: ESP-Claw Voice Control.

## Principle

The complete vision analysis flow:

```
User voice "Take a look at what's on the table"
  → ASR speech recognition
  → LLM decides: photo analysis is needed → calls self.camera.take_photo or self.camera.inspect_image
  → Firmware: esp_camera_fb_get() captures a frame (VGA RGB565)
  → JPEG compression
  → Sent through Explain() to the Vision API delivered by the server
  → Multimodal LLM returns a text description
  → TTS voice announcement
```

### Differences Between the Two Photo Tools

| Tool | Purpose | Who calls the Vision API |
|---|---|---|
| `self.camera.take_photo` | Takes a photo and describes it with the LLM's built-in vision capability | Server side |
| `self.camera.inspect_image` (NanoCam-specific) | Takes a photo, then calls `camera->Explain()` → HTTP POST to a dedicated multimodal API | Firmware side |

The difference between the two: `take_photo` goes through the XiaoZhi server's LLM vision (a general-purpose implementation), while `inspect_image` is this project's dedicated implementation — the firmware calls a dedicated multimodal API directly (with the address delivered by the server).

## Steps

### 11.1 Make Sure You Are in ESP-Claw Mode

```
ai_mode:7
```

After the device reboots, it enters ESP-Claw mode.

### 11.2 Photo + AI Analysis

After waking it up, just ask your question:

```
💬 "Take a look at what's here"
💬 "Is there a cup in front of me"
💬 "What color is this book"
💬 "How many apples are on the table"
💬 "Read what's written on this sheet of paper"
```

The NanoCam takes a photo, uploads it, analyzes it, and answers with voice.

### 11.3 Scene Recognition Examples

| Voice input | Example AI reply |
|---|---|
| "What is this" | "This is a black laptop with a white coffee cup next to it" |
| "Are there any apples" | "I don't see any apples. There are two books and a pen on the table" |
| "What color" | "You're pointing at a red mug" |
| "How many cups" | "There are 2 cups in the image" |

## Code

### Core Photo + Analysis Callback

`nanocam_espclaw/main/boards/nanocam/nanocam_board.cc` — MCP tool registration:

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

`nanocam_espclaw/main/boards/common/esp32_camera.cc` — Explain() implementation:

```cpp
std::string Esp32Camera::Explain(const std::string &question) {
    // explain_url_ is delivered by the server during the MCP handshake through capabilities.vision.url
    // Capture frame → JPEG compression → HTTP POST to the multimodal API
    // Return the LLM analysis result
}
```

### Server-Side MCP Handshake (Vision API Delivery)

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

After receiving it, the firmware calls `camera->SetExplainUrl(url, token)` to save the API address, which is then used directly by subsequent `inspect_image` calls.

## Supported Multimodal Models

By delivering a different `vision.url` from the server, you can use any OpenAI-compatible API:

| Model | API address example | Use case |
|---|---|---|
| `gpt-4o` | `https://api.openai.com/v1/chat/completions` | Strongest overall capability |
| `gpt-4o-mini` | `https://api.openai.com/v1/chat/completions` | Great cost-performance |
| `qwen-vl-max` | `https://dashscope.aliyuncs.com/compatible-mode/v1/chat/completions` | Better Chinese understanding |
| `llava:13b` (Ollama) | `http://localhost:11434/v1/chat/completions` | Fully offline |
| `claude-fable-5` | Proxy configuration required | Detailed scene descriptions |

## Result

"Take a look at what's here" → photo upload → AI analysis → voice announcement "I see a red cup on a wooden table" — a true AI eye.
