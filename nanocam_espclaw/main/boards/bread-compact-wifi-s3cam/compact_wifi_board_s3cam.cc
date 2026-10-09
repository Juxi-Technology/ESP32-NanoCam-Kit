#include "wifi_board.h"
#include "audio_codecs/es8311_audio_codec.h"
#include "display/display.h"
#include "application.h"
#include "button.h"
#include "config.h"
#include "led/single_led.h"
#include "assets/lang_config.h"
#include "esp32_camera.h"
#include "mcp_server.h"

#include <esp_log.h>
#include <driver/i2c_master.h>
#include <driver/gpio.h>
#include <led_strip.h>
#include <nvs_flash.h>
#include <nvs.h>
#include <esp_system.h>

#define TAG "CompactWifiBoardS3Cam"

class CompactWifiBoardS3Cam : public WifiBoard {
private:
    i2c_master_bus_handle_t codec_i2c_bus_;
    Button boot_button_;
    Esp32Camera* camera_;
    led_strip_handle_t led_strip_;

    void InitializeCodecI2c() {
        i2c_master_bus_config_t i2c_bus_cfg = {
            .i2c_port = I2C_NUM_0,
            .sda_io_num = AUDIO_CODEC_I2C_SDA_PIN,
            .scl_io_num = AUDIO_CODEC_I2C_SCL_PIN,
            .clk_source = I2C_CLK_SRC_DEFAULT,
            .glitch_ignore_cnt = 7,
            .intr_priority = 0,
            .trans_queue_depth = 0,
            .flags = {
                .enable_internal_pullup = 1,
            },
        };
        ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_bus_cfg, &codec_i2c_bus_));
    }

    void InitializeButtons() {
        boot_button_.OnClick([this]() {
            auto& app = Application::GetInstance();
            if (app.GetDeviceState() == kDeviceStateStarting) {
                EnterWifiConfigMode();
                return;
            }
            app.ToggleChatState();
        });
    }

    void InitializeCamera() {
        gpio_config_t pwdn_pin_cfg = {
            .pin_bit_mask = (1ULL << CAMERA_PIN_PWDN),
            .mode = GPIO_MODE_OUTPUT,
            .pull_up_en = GPIO_PULLUP_DISABLE,
            .pull_down_en = GPIO_PULLDOWN_DISABLE,
            .intr_type = GPIO_INTR_DISABLE,
        };
        gpio_config(&pwdn_pin_cfg);
        gpio_set_level(CAMERA_PIN_PWDN, 0);

        camera_config_t config = {};
        config.pin_d0 = CAMERA_PIN_D0;
        config.pin_d1 = CAMERA_PIN_D1;
        config.pin_d2 = CAMERA_PIN_D2;
        config.pin_d3 = CAMERA_PIN_D3;
        config.pin_d4 = CAMERA_PIN_D4;
        config.pin_d5 = CAMERA_PIN_D5;
        config.pin_d6 = CAMERA_PIN_D6;
        config.pin_d7 = CAMERA_PIN_D7;
        config.pin_xclk = CAMERA_PIN_XCLK;
        config.pin_pclk = CAMERA_PIN_PCLK;
        config.pin_vsync = CAMERA_PIN_VSYNC;
        config.pin_href = CAMERA_PIN_HREF;
        config.pin_sccb_sda = -1;
        config.pin_sccb_scl = CAMERA_PIN_SIOC;
        config.sccb_i2c_port = 0;
        config.pin_pwdn = -1;
        config.pin_reset = CAMERA_PIN_RESET;
        config.xclk_freq_hz = XCLK_FREQ_HZ;
        config.pixel_format = PIXFORMAT_RGB565;
        config.frame_size = FRAMESIZE_VGA;
        config.jpeg_quality = 12;
        config.fb_count = 1;
        config.fb_location = CAMERA_FB_IN_PSRAM;
        config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;

        camera_ = new Esp32Camera(config);
    }

    void InitializeLedStrip() {
        led_strip_config_t strip_config = {};
        strip_config.strip_gpio_num = BUILTIN_LED_GPIO;
        strip_config.max_leds = 1;
        strip_config.led_model = LED_MODEL_WS2812;
        strip_config.led_pixel_format = LED_PIXEL_FORMAT_GRB;
        led_strip_rmt_config_t rmt_config = {};
        rmt_config.resolution_hz = 10 * 1000 * 1000;
        ESP_ERROR_CHECK(led_strip_new_rmt_device(&strip_config, &rmt_config, &led_strip_));
        led_strip_clear(led_strip_);
    }

public:
    CompactWifiBoardS3Cam() :
        boot_button_(BOOT_BUTTON_GPIO) {
        InitializeLedStrip();
        InitializeCodecI2c();
        InitializeButtons();
        InitializeCamera();
        RegisterNanoCamMcpTools();
    }

    void RegisterNanoCamMcpTools() {
        auto& mcp = McpServer::GetInstance();

        mcp.AddTool("self.led.set_color", "Set NanoCam RGB LED color (0-255 per channel).",
            PropertyList({
                Property("r", kPropertyTypeInteger, 0, 255),
                Property("g", kPropertyTypeInteger, 0, 255),
                Property("b", kPropertyTypeInteger, 0, 255)
            }),
            [this](const PropertyList &props) -> ReturnValue {
                int r = props["r"].value<int>();
                int g = props["g"].value<int>();
                int b = props["b"].value<int>();
                led_strip_set_pixel(led_strip_, 0, r, g, b);
                led_strip_refresh(led_strip_);
                ESP_LOGI(TAG, "MCP: LED set to RGB(%d,%d,%d)", r, g, b);
                return std::string("ok");
            });

        mcp.AddTool("self.led.turn_off", "Turn off the RGB LED.",
            PropertyList(),
            [this](const PropertyList &) -> ReturnValue {
                led_strip_clear(led_strip_);
                return std::string("ok");
            });

        mcp.AddTool("self.camera.set_ai_mode",
            "Switch AI vision mode and reboot. 0=Stream 1=Cat 2=Face 3=Color 4=FaceID 5=QR 6=XiaoZhi 7=ESP-Claw.",
            PropertyList({
                Property("mode", kPropertyTypeInteger, 0, 7)
            }),
            [](const PropertyList &props) -> ReturnValue {
                int mode = props["mode"].value<int>();
                if (mode < 0 || mode > 7) {
                    return std::string("Error: mode must be 0-7");
                }
                nvs_handle_t h;
                if (nvs_open("nanocam", NVS_READWRITE, &h) == ESP_OK) {
                    nvs_set_u8(h, "aimode", (uint8_t)mode);
                    nvs_commit(h);
                    nvs_close(h);
                }
                vTaskDelay(pdMS_TO_TICKS(300));
                esp_restart();
                return std::string("ok");
            });

        mcp.AddTool("self.get_device_info", "Get NanoCam device information.",
            PropertyList(),
            [](const PropertyList &) -> ReturnValue {
                return std::string(
                    "{\"device\":\"NanoCam-ESPClaw\",\"chip\":\"ESP32-S3 N16R8\","
                    "\"camera\":\"GC2145\",\"audio\":\"ES8311\","
                    "\"flash\":\"16MB\",\"psram\":\"8MB\","
                    "\"framework\":\"XiaoZhi+ESP-Claw\",\"version\":\"1.0.0\"}");
            });

        mcp.AddTool("self.camera.inspect_image",
            "Take a photo and send it to the vision AI for analysis.",
            PropertyList({
                Property("prompt", kPropertyTypeString)
            }),
            [this](const PropertyList &props) -> ReturnValue {
                auto camera = GetCamera();
                if (!camera) {
                    return std::string("{\"error\":\"Camera not available\"}");
                }
                if (!camera->Capture()) {
                    return std::string("{\"error\":\"Camera capture failed\"}");
                }
                std::string prompt = props["prompt"].value<std::string>();
                try {
                    return camera->Explain(prompt);
                } catch (const std::exception& e) {
                    return std::string("{\"error\":\"") + e.what() + "\"}";
                }
            });

        ESP_LOGI(TAG, "Registered 5 NanoCam MCP tools");
    }

    virtual Led* GetLed() override {
        static NoLed led;
        return &led;
    }

    virtual AudioCodec* GetAudioCodec() override {
        static Es8311AudioCodec audio_codec(codec_i2c_bus_, I2C_NUM_0, AUDIO_INPUT_SAMPLE_RATE, AUDIO_OUTPUT_SAMPLE_RATE,
            AUDIO_I2S_GPIO_MCLK, AUDIO_I2S_GPIO_BCLK, AUDIO_I2S_GPIO_WS, AUDIO_I2S_GPIO_DOUT, AUDIO_I2S_GPIO_DIN,
            AUDIO_CODEC_PA_PIN, AUDIO_CODEC_ES8311_ADDR);
        return &audio_codec;
    }

    virtual Display* GetDisplay() override {
        static NoDisplay display;
        return &display;
    }

    virtual Camera* GetCamera() override {
        return camera_;
    }
};

DECLARE_BOARD(CompactWifiBoardS3Cam);
