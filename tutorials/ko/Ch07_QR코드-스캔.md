# 7장: QR 코드 스캔

## 원리

esp-code-scanner 사전 컴파일 라이브러리로 화면 속 QR 코드(QR Code / Barcode)를 실시간 디코딩합니다. 카메라가 출력한 RGB565 프레임은 그레이스케일 변환 없이 스캐너에 직접 전달됩니다. 매 프레임마다 새로운 스캐너 객체를 생성하고 스캔이 끝나면 즉시 파괴하여 내부 상태 누적을 방지합니다.
디코딩 결과는 다음을 통해 동시에 출력됩니다:
1. **시리얼 로그** 출력
2. **공유 버퍼** `g_last_code`에 최신 결과를 저장하여 HTTP/MJPEG 스트림 오버레이 표시에 제공
3. **웹 화면 하단**에 녹색 텍스트 라벨 오버레이

## 단계

### 7.1 모드 전환

```
ai_mode:5
```

### 7.2 스캔

QR 코드를 카메라 앞에 두면 시리얼로 출력됩니다:

```
I (xxxxx) qrcode: Decoded [QR-Code]: https://example.com
```

동시에 웹 화면 `http://<IP>/` 하단에 디코딩된 내용이 녹색 텍스트로 표시됩니다.

### 7.3 연속 스캔

다음 코드를 향하면 자동으로 디코딩되어 출력되며, 스캐너가 매 프레임 재생성되어 중단 없이 연속 작동합니다.

## 코드

### 핵심 스캔 로직

`main/ai/nano_qrcode.cpp`:

```cpp
// 매 프레임 새로운 스캐너 객체 생성
esp_image_scanner_t *scn = esp_code_scanner_create();
esp_code_scanner_config_t cfg = {
    ESP_CODE_SCANNER_MODE_FAST,
    ESP_CODE_SCANNER_IMAGE_RGB565,
    fb->width, fb->height
};
esp_code_scanner_set_config(scn, cfg);
int count = esp_code_scanner_scan_image(scn, fb->buf);
// 디코딩 성공
const esp_code_scanner_symbol_t result = esp_code_scanner_result(scn);
ESP_LOGI(TAG, "Decoded [%s]: %s", result.type_name, result.data);
// 웹 오버레이용 공유 버퍼에 저장
snprintf(g_last_code, sizeof(g_last_code), "%s: %s",
         result.type_name, result.data);
esp_code_scanner_destroy(scn);
```

## 결과

QR 코드를 향하면 → 시리얼로 디코딩 내용 출력 + 웹 화면 오버레이 표시.
