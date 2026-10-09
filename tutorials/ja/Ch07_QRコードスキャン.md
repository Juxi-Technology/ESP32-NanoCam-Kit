# 第7章: QRコードスキャン

## 原理

esp-code-scanner のプリコンパイル済みライブラリで、画面内の QR コード(QR Code / Barcode)をリアルタイムにデコードします。カメラが出力する RGB565 フレームをそのままスキャナに渡すため、グレースケール変換は不要です。フレームごとに新しいスキャナオブジェクトを生成し、スキャン後すぐに破棄することで内部状態の蓄積を防ぎます。
デコード結果は同時に:
1. **シリアルログ**に出力
2. **共有バッファ** `g_last_code` に最新結果を保存し、HTTP/MJPEG ストリームのオーバーレイ表示に使用
3. **Web 画面の下部**に緑色の文字をオーバーレイ表示

## 手順

### 7.1 モード切り替え

```
ai_mode:5
```

### 7.2 スキャン

QR コードをカメラの前に置くと、シリアルに出力されます:

```
I (xxxxx) qrcode: Decoded [QR-Code]: https://example.com
```

同時に Web 画面 `http://<IP>/` の下部に、デコード内容が緑色の文字で表示されます。

### 7.3 連続スキャン

次のコードに向けると自動的にデコード・出力されます。スキャナは毎フレーム再構築されるため、連続して安定に動作します。

## コード

### コアスキャンロジック

`main/ai/nano_qrcode.cpp`:

```cpp
// フレームごとに新しいスキャナオブジェクトを生成
esp_image_scanner_t *scn = esp_code_scanner_create();
esp_code_scanner_config_t cfg = {
    ESP_CODE_SCANNER_MODE_FAST,
    ESP_CODE_SCANNER_IMAGE_RGB565,
    fb->width, fb->height
};
esp_code_scanner_set_config(scn, cfg);
int count = esp_code_scanner_scan_image(scn, fb->buf);
// デコード成功
const esp_code_scanner_symbol_t result = esp_code_scanner_result(scn);
ESP_LOGI(TAG, "Decoded [%s]: %s", result.type_name, result.data);
// 共有バッファに保存し、Web ページのオーバーレイに使用
snprintf(g_last_code, sizeof(g_last_code), "%s: %s",
         result.type_name, result.data);
esp_code_scanner_destroy(scn);
```

## 効果

QR コードに向ける → シリアルにデコード内容を出力 + Web 画面にオーバーレイ表示。
