# 11장: ESP-Claw 음성 제어

## 이번 장 안내

기기를 `ai_mode:7`로 전환하면 NanoCam이 ESP-Claw 모드로 진입합니다. 이 모드는 XiaoZhi AI(`ai_mode:6`)와 **동일한 펌웨어**(`nanocam_espclaw/`)를 공유하며, 차이는 단 하나: ESP-Claw 모드는 음성 대화를 기반으로 하드웨어 제어 도구 5개를 추가로 등록한다는 점입니다.

| 비교 항목 | XiaoZhi AI (mode 6) | ESP-Claw (mode 7) |
|---|---|---|
| 음성 대화 | ✅ ASR→LLM→TTS | ✅ 동일한 음성 파이프라인 |
| LED 제어 | ❌ | ✅ 음성으로 색상 조절 / 켜기·끄기 |
| AI 모드 전환 | ❌ | ✅ 음성으로 전환 |
| 사진 촬영 + AI 비전 분석 | ❌ | ✅ 촬영 후 멀티모달 AI로 화면 이해 |

## 원리

ESP-Claw 모드는 음성 파이프라인 위에 `RegisterMcpTools()`를 통해 NanoCam 전용 도구 5개를 등록합니다:

```
사용자 음성 "조명을 파란색으로 바꿔줘"
  → ASR 음성 인식(클라우드)
  → LLM 의도 이해 → self.led.set_color({"r":0, "g":0, "b":255}) 호출
  → NanoCam WS2812 LED 파란색
  → TTS: "네, 조명을 파란색으로 바꿨습니다"
```

## 단계

### 12.1 펌웨어 플래싱

ESP-Claw는 `nanocam_espclaw/` 독립 펌웨어 프로젝트를 사용합니다:

```bash
cd nanocam_espclaw
idf.py set-target esp32s3
idf.py build
idf.py -p COMx flash
```

### 12.2 모드 전환

부팅 후 ESP-Claw 모드로 설정합니다:

```
ai_mode:7
```

기기가 자동으로 재부팅되어 진입합니다. `ai_mode:6`으로 XiaoZhi AI 모드로 되돌릴 수 있습니다.

### 12.3 음성 제어 예시

웨이크 후 바로 요구 사항을 말합니다:

```
💬 "조명 켜줘"              → WS2812 흰색 점등
💬 "조명을 파란색으로 바꿔줘"          → LED 파란색
💬 "조명 꺼줘"                  → LED 꺼짐
💬 "얼굴 검출 모드로 전환해줘"    → NVS에 ai_mode:2 저장 + 재부팅
💬 "여기 뭐가 있는지 봐줘"        → 사진 촬영 + 멀티모달 AI 분석 업로드
💬 "내 앞에 컵이 있어?"        → 멀티모달 AI가 화면 인식
```

### 12.4 사진 촬영 + AI 비전 분석

사용자가 "무엇이 보이는지..."라고 말하면, 펌웨어가 VGA RGB565 이미지 한 프레임을 캡처하여 JPEG로 압축하고, 서버에 구성된 멀티모달 API로 전송하여 분석한 뒤, 결과를 TTS 음성으로 안내합니다.
> 멀티모달 API의 URL과 token은 서버가 연결 핸드셰이크 단계에서 자동으로 전달하며, 시리얼로 구성 명령을 수동 입력할 필요가 없습니다.

## NanoCam 전용 도구 5개

| 도구명 | 기능 | 매개변수 |
|---|---|---|
| `self.led.set_color` | WS2812 RGB LED 설정 (GPIO18) | `r,g,b`: 0-255 |
| `self.led.turn_off` | LED 끄기 | 없음 |
| `self.camera.set_ai_mode` | AI 모드 전환 (NVS 저장 + 재부팅) | `mode`: 0-7 |
| `self.camera.inspect_image` | 사진 촬영 + 멀티모달 LLM 비전 분석 | `prompt`: 질문 설명 |
| `self.get_device_info` | 기기 정보 JSON | 없음 |

## 구성 파일

| 내용 | 경로 |
|---|---|
| MCP 도구 등록 | `nanocam_espclaw/main/boards/nanocam/nanocam_board.cc` |
| Vision 전송 로직 | `nanocam_espclaw/main/boards/common/esp32_camera.cc` |
| SDK 기본 구성 | `nanocam_espclaw/sdkconfig.defaults` |

> ESP-Claw 펌웨어는 독립 프로젝트로, `nanocam_vision`과 코드를 공유하지 않습니다. 두 펌웨어는 각각 컴파일 및 플래싱해야 합니다.

## 선택 방법

| 요구 사항 | 권장 모드 |
|---|---|
| 음성 채팅, 질의응답만 원함 | mode 6 (XiaoZhi) |
| 음성으로 LED 제어를 원함 | mode 7 (ESP-Claw) |
| 사진 촬영 + AI로 화면 "보기"를 원함 | mode 7 (ESP-Claw) |
| 음성으로 AI 검출 모드 전환을 원함 | mode 7 (ESP-Claw) |

> ESP-Claw의 전체 사용 방법(서버 구성, 사용자 정의 MCP 도구 개발 등)은 아직 탐구 중이며, 문서는 연구 진행에 따라 지속적으로 업데이트됩니다.
