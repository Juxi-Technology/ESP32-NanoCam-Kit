# 10장: AI 비전 이해

## 이번 장 안내

AI 비전 이해는 **ESP-Claw(모드 7)**의 전용 기능이며, XiaoZhi AI(모드 6)에서는 사용되지 않습니다.
> 이번 장에서 사용하는 `self.camera.take_photo`와 `self.camera.inspect_image` 도구의 시각 분석 API 주소는 MCP 핸드셰이크 단계에서 서버가 `capabilities.vision` 필드를 통해 자동으로 전달합니다. 펌웨어 측에서 API URL을 수동으로 구성할 필요가 없습니다 —— 즉, API 구성은 xiaozhi.me 콘솔 또는 자체 구축 서버에서 완료되며, 자세한 내용은 11장: ESP-Claw 음성 제어를 참고하세요.

## 원리

시각 분석의 전체 흐름:

```
사용자 음성 "책상 위에 뭐가 있는지 봐줘"
  → ASR 음성 인식
  → LLM 판단: 사진 촬영 분석 필요 → self.camera.take_photo 또는 self.camera.inspect_image 호출
  → 펌웨어: esp_camera_fb_get() 프레임 캡처 (VGA RGB565)
  → JPEG 압축
  → Explain()을 통해 서버가 전달한 Vision API로 전송
  → 멀티모달 LLM이 텍스트 설명 반환
  → TTS 음성 안내
```

### 두 사진 촬영 도구의 차이

| 도구 | 용도 | Vision API 발송 주체 |
|---|---|---|
| `self.camera.take_photo` | 촬영 후 LLM 내장 vision 기능으로 설명 | 서버 |
| `self.camera.inspect_image` (NanoCam 전용) | 촬영 후 `camera->Explain()` 호출 → 독립 멀티모달 API로 HTTP POST | 펌웨어 |

두 도구의 차이: `take_photo`는 XiaoZhi 서버의 LLM 비전(범용 구현)을 사용하고, `inspect_image`는 본 프로젝트의 전용 구현으로 펌웨어가 독립 멀티모달 API를 직접 호출합니다(주소는 서버가 전달).

## 단계

### 11.1 ESP-Claw 모드 확인

```
ai_mode:7
```

기기가 재부팅 후 ESP-Claw 모드로 진입합니다.

### 11.2 사진 촬영+AI 분석

웨이크 후 바로 질문을 말합니다:

```
💬 "여기 뭐가 있는지 봐줘"
💬 "내 앞에 컵이 있어?"
💬 "이 책은 무슨 색이야"
💬 "책상 위에 사과가 몇 개 놓여 있어"
💬 "이 종이에 무슨 글자가 쓰여 있는지 봐줘"
```

NanoCam이 사진을 촬영하고 업로드하여 분석한 뒤, 음성으로 결과를 답합니다.

### 11.3 장면 인식 예시

| 음성 입력 | AI 반환 예시 |
|---|---|
| "이게 뭐야" | "검은색 노트북이고, 옆에 흰색 커피잔이 있어요" |
| "사과 있어?" | "사과는 보이지 않아요. 책상 위에 책 두 권과 펜 하나가 있어요" |
| "무슨 색이야" | "가리키고 있는 것은 빨간 머그컵이에요" |
| "컵이 몇 개야" | "화면에 컵이 2개 있어요" |

## 코드

### 핵심 촬영+분석 콜백

`nanocam_espclaw/main/boards/nanocam/nanocam_board.cc` — MCP 도구 등록:

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

`nanocam_espclaw/main/boards/common/esp32_camera.cc` — Explain() 구현:

```cpp
std::string Esp32Camera::Explain(const std::string &question) {
    // explain_url_은 서버가 MCP 핸드셰이크 시 capabilities.vision.url을 통해 전달
    // 프레임 캡처 → JPEG 압축 → 멀티모달 API로 HTTP POST
    // LLM 분석 결과 반환
}
```

### 서버 측 MCP 핸드셰이크(Vision API 전달)

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

펌웨어는 수신 후 `camera->SetExplainUrl(url, token)`을 호출하여 API 주소를 저장하고, 이후 `inspect_image` 호출 시 바로 사용합니다.

## 지원되는 멀티모달 모델

서버가 다른 `vision.url`을 전달하면 모든 OpenAI 호환 API를 사용할 수 있습니다:

| 모델 | API 주소 예시 | 적용 시나리오 |
|---|---|---|
| `gpt-4o` | `https://api.openai.com/v1/chat/completions` | 최강 종합 성능 |
| `gpt-4o-mini` | `https://api.openai.com/v1/chat/completions` | 가성비 우수 |
| `qwen-vl-max` | `https://dashscope.aliyuncs.com/compatible-mode/v1/chat/completions` | 중국어 이해 우수 |
| `llava:13b` (Ollama) | `http://localhost:11434/v1/chat/completions` | 완전 오프라인 |
| `claude-fable-5` | 프록시 구성 필요 | 상세 장면 설명 |

## 결과

"여기 뭐가 있는지 봐줘" → 사진 촬영 후 업로드 → AI 분석 → 음성 안내 "I see a red cup on a wooden table" —— 진정한 AI의 눈.
