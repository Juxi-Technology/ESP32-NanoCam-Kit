# 第10章: AI 画像理解

## 本章について

AI 画像理解は **ESP-Claw(モード 7)** の独自機能で、XiaoZhi AI(モード 6)では使用しません。
> 本章で使用する `self.camera.take_photo` と `self.camera.inspect_image` ツールでは、視覚分析 API アドレスが MCP ハンドシェイク段階でサーバーから `capabilities.vision` フィールドを通じて自動配信されます。ファームウェア側で API URL を手動設定する必要はありません —— これは、API の設定が xiaozhi.me コンソールまたは自前構築のサーバー側で行われることを意味します。詳細は第11章: ESP-Claw 音声制御を参照してください。

## 原理

視覚分析の完全な流れ:

```
ユーザーの音声「テーブルの上に何があるか見て」
  → ASR 音声認識
  → LLM が判断:撮影分析が必要 → self.camera.take_photo または self.camera.inspect_image を呼び出し
  → ファームウェア:esp_camera_fb_get() でフレーム取得 (VGA RGB565)
  → JPEG 圧縮
  → Explain() で、サーバーから配信された Vision API へ送信
  → マルチモーダル LLM がテキスト記述を返却
  → TTS 音声で読み上げ
```

### 2 つの撮影ツールの違い

| ツール | 用途 | Vision API を送信する側 |
|---|---|---|
| `self.camera.take_photo` | 撮影後、LLM 内蔵の vision 機能で説明 | サーバー側 |
| `self.camera.inspect_image` (NanoCam 専用) | 撮影後 `camera->Explain()` を呼び出し → HTTP POST で独立したマルチモーダル API へ | ファームウェア側 |

両者の違い: `take_photo` は XiaoZhi サーバー側の LLM 視覚(汎用実装)を通り、`inspect_image` は本プロジェクト専用の実装で、ファームウェアが独立したマルチモーダル API を直接呼び出します(アドレスはサーバーから配信)。

## 手順

### 11.1 ESP-Claw モードの確認

```
ai_mode:7
```

デバイスは再起動後に ESP-Claw モードに入ります。

### 11.2 撮影+AI 分析

ウェイクアップ後、そのまま質問を話します:

```
💬 "ここに何があるか見て"
💬 "私の前にカップはある?"
💬 "この本は何色?"
💬 "テーブルの上にリンゴはいくつある?"
💬 "この紙に何が書いてあるか見て"
```

NanoCam は撮影・アップロード・分析を行い、結果を音声で回答します。

### 11.3 シーン認識の例

| 音声入力 | AI の返答例 |
|---|---|
| "これは何?" | "これは黒いノートパソコンで、隣に白いコーヒーカップがあります" |
| "リンゴはある?" | "リンゴは見当たりません。テーブルの上には本が 2 冊とペンが 1 本あります" |
| "何色?" | "あなたが指しているのは赤いマグカップです" |
| "カップはいくつ?" | "画面にはカップが 2 つあります" |

## コード

### コアの撮影+分析コールバック

`nanocam_espclaw/main/boards/nanocam/nanocam_board.cc` — MCP ツール登録:

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

`nanocam_espclaw/main/boards/common/esp32_camera.cc` — Explain() の実装:

```cpp
std::string Esp32Camera::Explain(const std::string &question) {
    // explain_url_ はサーバーが MCP ハンドシェイク時に capabilities.vision.url で配信
    // フレーム取得 → JPEG 圧縮 → HTTP POST でマルチモーダル API へ
    // LLM の分析結果を返却
}
```

### サーバー側 MCP ハンドシェイク(Vision API の配信)

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

ファームウェアは受信後 `camera->SetExplainUrl(url, token)` を呼び出して API アドレスを保存し、以降の `inspect_image` 呼び出しでそのまま使用します。

## 対応マルチモーダルモデル

サーバーから異なる `vision.url` を配信することで、任意の OpenAI 互換 API を利用できます:

| モデル | API アドレス例 | 適用シーン |
|---|---|---|
| `gpt-4o` | `https://api.openai.com/v1/chat/completions` | 総合能力が最も高い |
| `gpt-4o-mini` | `https://api.openai.com/v1/chat/completions` | コストパフォーマンスが高い |
| `qwen-vl-max` | `https://dashscope.aliyuncs.com/compatible-mode/v1/chat/completions` | 中国語の理解に優れる |
| `llava:13b` (Ollama) | `http://localhost:11434/v1/chat/completions` | 完全オフライン |
| `claude-fable-5` | プロキシの設定が必要 | 詳細なシーン記述 |

## 効果

「ここに何があるか見て」→ 撮影・アップロード → AI 分析 → 音声で "I see a red cup on a wooden table" と読み上げ —— まさに AI の目。
