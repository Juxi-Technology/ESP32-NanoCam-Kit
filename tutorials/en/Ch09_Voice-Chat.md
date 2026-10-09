# Chapter 9: Voice Chat (XiaoZhi AI)

## About This Chapter

This chapter covers the XiaoZhi AI mode (`ai_mode:6`). **Important**: mode 6 (voice chat) and mode 7 (ESP-Claw) **share the same firmware** (`nanocam_espclaw/`); the device simply loads a different MCP tool set at boot depending on the `ai_mode` value stored in NVS.

| Comparison | XiaoZhi AI (mode 6) | ESP-Claw (mode 7) |
|---|---|---|
| Voice chat | ✅ ASR→LLM→TTS | ✅ The same voice pipeline |
| MCP tools | General-purpose tools (volume / photo capture, etc.) | **General-purpose tools + 5 hardware-specific tools** |
| Vision understanding | `self.camera.take_photo` | **`self.camera.inspect_image`** (multimodal vision) |
| LED control | ❌ | ✅ Voice-controlled color |
| Use cases | General AI chat, children's education | Hardware control, visual inspection, smart home |

> This chapter focuses on the core voice chat features of **XiaoZhi AI (mode 6)**. To learn about ESP-Claw's hardware control capabilities, see Chapter 11: ESP-Claw Voice Control.

## Principle

The NanoCam integrates the open-source XiaoZhi AI framework, connecting to an LLM server over WebSocket / MQTT to provide a complete voice interaction pipeline:

```
User speaks → ES8311 microphone capture → Opus encoding
  → WebSocket → cloud ASR speech recognition
  → LLM generates a reply
  → TTS speech synthesis → Opus decoding
  → NS4150B amplifier → speaker playback
```

Full-duplex design: the user can interrupt the AI directly while it is speaking (barge-in), for an experience close to talking to a real person.

## Hardware Requirements

This chapter involves audio features and requires the following hardware:
- NanoCam core board (with ES8311 codec + AP2718AT microphone)
- NanoCam base board (with NS4150B amplifier + CH340K)
- Speaker (connect to the speaker interface on the base board, VON/VOP)
> The core board alone can also be used for testing (monitor through the ES8311 headphone output). The microphone is an AP2718AT analog MEMS silicon microphone, connected to ES8311 MIC1P through the C26 DC-blocking capacitor.

## Steps

### 10.1 Flashing the XiaoZhi AI Firmware

XiaoZhi AI uses the standalone `nanocam_espclaw/` firmware project:

```bash
cd nanocam_espclaw
idf.py set-target esp32s3
idf.py build
idf.py -p COMx flash monitor
```

After boot it defaults to XiaoZhi AI mode.

### 10.2 Connecting to the xiaozhi.me Cloud Service

By default the NanoCam connects to the [xiaozhi.me](https://xiaozhi.me) official cloud service (free); no self-hosted server is required.
1. Register an account at [xiaozhi.me](https://xiaozhi.me)
2. After the device powers on, it automatically announces a 6-digit activation code
3. Enter the activation code in the xiaozhi.me console → bind the device
4. Choose an LLM model in the console (Qwen / DeepSeek, etc.)
Activation is needed only once; after that the device connects automatically every time it powers on.

### 10.3 First Conversation

Once you hear the prompt tone, you can start talking:

```
You: "你好小智, what's the weather like today?"
NanoCam: "Let me check today's weather for you..."
```

The wake word is **"你好小智"** (Ni Hao Xiao Zhi, "Hello Xiaozhi") by default.

### 10.4 Common Conversation Scenarios

```
💬 "Tell me a joke"              → AI answers by voice
💬 "Set an alarm for 5 minutes"  → alarm feature
💬 "What time is it"             → time announcement
💬 "Play some light music"       → play music online
💬 "What is a black hole"        → knowledge Q&A
```

## Self-Hosted Server (Optional)

If you have privacy requirements, or want to use your own LLM, you can deploy the open-source XiaoZhi AI server:

```bash
git clone https://github.com/xinnan-tech/xiaozhi-esp32-server
cd xiaozhi-esp32-server
pip install -r requirements.txt
python app.py
```

The firmware's server address is delivered through the OTA system (`CONFIG_OTA_URL` in sdkconfig); the device automatically requests the server address after powering on.
> XiaoZhi AI uses the open-source XiaoZhi AI server (a private WebSocket protocol + ASR/LLM/TTS pipeline). In ESP-Claw mode, on top of this, the server delivers the Vision API URL and token during the MCP handshake for the vision analysis feature — the firmware does not need to configure anything itself.

## Troubleshooting

| Symptom | Possible cause | Solution |
|---|---|---|
| No sound | Speaker not connected | Check the speaker interface on the base board |
| Speech recognition inaccurate | Too much ambient noise | Speak closer to the microphone (distance < 1m) |
| Cannot connect | WiFi not configured | Provision over serial first: `sta_ssid:xxx` |
| No activation code | First startup not complete | Wait 30 seconds; the device will announce it automatically |
| Slow replies | LLM server latency | Choose a faster model on xiaozhi.me, or self-host a server |
