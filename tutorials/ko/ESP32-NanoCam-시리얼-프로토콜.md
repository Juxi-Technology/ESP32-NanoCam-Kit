# ESP32-NanoCam 시리얼 AT 프로토콜 매뉴얼

> 보드레이트: 115200 | 데이터 비트: 8 | 패리티: 없음 | 정지 비트: 1 | 흐름 제어: 없음
> 주요 카메라 모듈 AT 명령 세트와 호환되며, NanoCam 확장 명령이 추가되었습니다.

---

## 1. 일반 규칙

- 명령 대소문자는 **구분하지 않습니다**(`STA_SSID` = `sta_ssid`)
- 명령 뒤에는 **임의의 영문 부호**(`,` `.` `:` `;` 등)를 종료 문자로 붙여야 합니다
- 일부 명령은 수정 후 **자동으로 재부팅**됩니다
- 각 명령은 `\r\n`으로 끝납니다(시리얼 터미널이 보통 자동으로 추가)

## 2. WiFi 구성

### STA 모드(라우터 연결)

| 명령 | 설명 | 예시 | 반환값 |
|---|---|---|---|
| `sta_ssid:이름` | WiFi 이름 설정 | `sta_ssid:MyWiFi` | `OK` |
| `sta_pd:비밀번호` | WiFi 비밀번호 설정 (수정 후 재부팅) | `sta_pd:12345678` | `OK` (재부팅) |

> WiFi 이름과 비밀번호는 최대 30자이며, 중국어는 지원하지 않습니다.

### AP 모드(자체 핫스팟)

| 명령 | 설명 | 예시 | 반환값 |
|---|---|---|---|
| `ap_ssid:이름` | 핫스팟 이름 설정 | `ap_ssid:NanoCam-AP` | `OK` |
| `ap_pd:비밀번호` | 핫스팟 비밀번호 설정 (수정 후 재부팅) | `ap_pd:12345678` | `OK` (재부팅) |

### WiFi 모드

| 명령 | 설명 | 파라미터 | 반환값 |
|---|---|---|---|
| `wifi_mode:X` | 모드 전환 | 0=AP 1=STA 2=AP+STA | `OK` (변경 시 재부팅) |

---

## 3. AI 모드 전환

| 명령 | 모드 | 설명 | 재부팅 |
|---|---|---|---|
| `ai_mode:0` | 일반 | MJPEG 영상 전송, AI 없음 | ✅ |
| `ai_mode:1` | 고양이 얼굴 검출 | 실시간 고양이 얼굴 검출 상자 + 신뢰도 | ✅ |
| `ai_mode:2` | 얼굴 검출 | 실시간 얼굴 검출 상자 + 좌표 | ✅ |
| `ai_mode:3` | 색상 인식 | 색상 지정→실시간 검출 | ✅ |
| `ai_mode:4` | 얼굴 인식 | 등록→인식→삭제 | ✅ |
| `ai_mode:5` | QR 코드 | 실시간 디코딩→시리얼 출력 | ✅ |
| `ai_mode:6` | LLM 에이전트 | XiaoZhi AI 음성 대화 + AI 비전 | ✅ |
| `ai_mode:7` | ESP-Claw | 음성 제어 + 사진 촬영 비전 분석 + OpenAI Vision | ✅ |

> `ai_mode` 유효값: 0-7. 범위를 벗어나면 기본값 0이 됩니다. 수정 후 자동으로 재부팅되며, 재부팅 후 새 모드가 적용됩니다.

---

## 4. 정보 조회

| 명령 | 설명 | 반환값 예시 |
|---|---|---|
| `sta_ip` | STA IP 조회 | `sta_ip:192.168.1.100` |
| `ap_ip` | AP IP 조회 | `ap_ip:192.168.4.1` |
| `wifi_ver` | 펌웨어 버전 조회 | `NanoCam Board Ver:0.2.0` |

---

## 5. 시스템 제어

| 명령 | 설명 | 반환값 |
|---|---|---|
| `wifi_reset` | 공장 초기화 (재부팅) | `Reset_OK` |
| `nano_reboot` | 소프트 리셋 | `Rebooting...` |
| `nano_info` | 전체 기기 정보 (JSON) | 아래 참조 |

### nano_info 반환 예시

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

## 6. 얼굴 인식 전용 명령

> ai_mode:4(얼굴 인식 모드)에서만 유효합니다.

| 명령 | 설명 | 라벨 동작 | 반환 예시 |
|---|---|---|---|
| `face_eril` | 현재 화면에서 검출된 얼굴 등록 | 파란색 "Enroll: ID N", 0.5초 깜빡임 | `>>> face enroll triggered` |
| `face_rz` | 지속 얼굴 인식 모드 진입 | 초록색 "ID: N" / 빨간색 "who?", **지속 표시되어 사라지지 않음** | `>>> face recognize triggered` |
| `face_del` | 마지막으로 등록된 얼굴 ID 삭제 | 빨간색 "N IDs left", 0.5초 깜빡임 | `>>> face delete triggered` |
| `face_detect` | 인식 모드 종료, 순수 얼굴 검출로 복귀 | 모든 라벨 삭제 | `>>> face detect mode` |

### 얼굴 인식 작업 절차

```
ai_mode:4          # 얼굴 인식 모드 진입 (장치 자동 재부팅)
face_eril          # 얼굴 등록 (화면에 얼굴이 하나만 있도록 보장)
face_rz            # 지속 인식 시작 — 라벨이 계속 표시되며 사라지지 않음
face_detect        # 인식 모드 종료 — 라벨 제거
face_del           # 마지막으로 등록한 얼굴 삭제
```

### 얼굴 인식 주의 사항

1. 등록 시 화면에 **얼굴이 하나만** 있어야 하며, 거리는 30-50cm입니다
2. 인식 모드(`face_rz`)에서는 라벨이 **지속 표시**되어 0.5초 후 사라지지 않습니다——이것은 0.3.0의 새로운 동작입니다
3. 인식 모드를 종료하려면 `face_detect`를 전송해야 하며, 그렇지 않으면 라벨이 계속 표시됩니다
4. 얼굴 특징은 Flash `fr` 파티션에 저장되어 전원이 꺼져도 유지되며, 최대 47개 ID를 지원합니다
5. 인식에는 프레임 건너뛰기 전략이 사용됩니다(10프레임마다 MFN 추론 1회 실행)

---

## 7. 확장 명령(NanoCam 전용)

| 명령 | 설명 | 상태 |
|---|---|---|
| `nano_server:url` | LLM 서버 주소 설정 (NVS 저장) | ✅ |
| `nano_api_key:key` | LLM API 키 설정 (NVS 저장) | ✅ |
| `nano_mqtt:broker,port,topic` | MQTT 서버 구성 | 🔨 |
| `nano_led:R,G,B` | RGB LED 설정 (WS2812, GPIO18 DIN) | 📋 |
| `nano_snap` | 사진 촬영 저장 (SPIFFS) | ✅ |
| `nano_stream:on/off` | 영상 전송 시작/정지 | 📋 |

### nano_server / nano_api_key

| 명령 | 설명 | 예시 | 반환값 |
|---|---|---|---|
| `nano_server:URL` | LLM 서버 주소 설정 | `nano_server:https://api.openai.com` | `OK server=https://api.openai.com` |
| `nano_api_key:KEY` | API 키 설정 | `nano_api_key:sk-xxxx` | `OK` |

> 임의의 OpenAI 호환 API를 지원합니다(vLLM / Ollama / 로컬 모델 모두 가능).
ESP-Claw 모드(ai_mode:7)는 `nano_server` 사용을 지원하며, XiaoZhi AI(ai_mode:6)는 별도 서버 구성을 사용합니다.

---

## 8. 주의 사항

1. `sta_pd` / `ap_pd`는 수정 후 자동으로 재부팅되며, 재부팅 후 새 비밀번호가 적용됩니다
2. `ai_mode`는 수정 후 자동으로 재부팅됩니다(모드가 변경된 경우에만)
3. 얼굴 인식 모드(mode 4)에서는 Type-C 시리얼 구성 기능이 동작하지 않을 수 있습니다(메모리 부족)
4. WiFi 이름/비밀번호는 30자를 초과할 수 없으며, 중국어를 사용할 수 없습니다
5. 명령 뒤에는 부호를 종료 문자로 붙여야 합니다
