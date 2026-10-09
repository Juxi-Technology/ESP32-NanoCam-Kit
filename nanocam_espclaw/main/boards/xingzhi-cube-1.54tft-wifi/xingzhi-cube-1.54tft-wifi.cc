#include "wifi_board.h"
#include "audio_codecs/no_audio_codec.h"

#include "audio_codecs/es8311_audio_codec.h"

#include "display.h"
#include "system_reset.h"
#include "application.h"
#include "button.h"
#include "config.h"
#include "iot/thing_manager.h"
#include "led/single_led.h"
#include "assets/lang_config.h"
#include <esp_efuse_table.h>
#include <driver/i2c_master.h>
#include <esp_lcd_panel_io.h>
#include <esp_lcd_panel_ops.h>
#include <esp_lcd_gc9a01.h>
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp32_camera.h"  // 摄像头驱动
#include <driver/ledc.h>   // LEDC定时器（用于XCLK时钟生成）

#include <esp_log.h>
#include <esp_lcd_panel_vendor.h>
#include <wifi_station.h>

#include <driver/rtc_io.h>
#include <esp_sleep.h>

#define TAG "XINGZHI_CUBE_1_54TFT_WIFI"

LV_FONT_DECLARE(font_puhui_20_4);
LV_FONT_DECLARE(font_awesome_20_4);


class XINGZHI_CUBE_1_54TFT_WIFI : public WifiBoard {
private:

    i2c_master_bus_handle_t codec_i2c_bus_;  // 定义一个I2C主总线句柄，用于管理与音频编解码器（如ES8311）的I2C通信

    Button boot_button_;
    Display* display_ = nullptr;
    esp_lcd_panel_io_handle_t panel_io_ = nullptr;
    esp_lcd_panel_handle_t panel_ = nullptr;

    Esp32Camera* camera_;
     

 
    void InitializeCodecI2c() {
        // 初始化I2C外设
        i2c_master_bus_config_t i2c_bus_cfg = {
            .i2c_port = I2C_NUM_0,  // 使用I2C端口0         0->1
            .sda_io_num = AUDIO_CODEC_I2C_SDA_PIN,  // 设置I2C数据线（SDA）的GPIO引脚
            .scl_io_num = AUDIO_CODEC_I2C_SCL_PIN,  // 设置I2C时钟线（SCL）的GPIO引脚
            .clk_source = I2C_CLK_SRC_DEFAULT,  // 使用默认的I2C时钟源
            .glitch_ignore_cnt = 7,  // 设置毛刺忽略计数，用于过滤信号中的噪声
            .intr_priority = 0,  // 设置中断优先级为0（最低优先级）
            .trans_queue_depth = 0,  // 设置传输队列深度为0（不使用队列）
            .flags = {
                .enable_internal_pullup = 1,  // 启用内部上拉电阻，确保信号稳定
            },
        };
        // 创建I2C主总线，并检查是否成功
        ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_bus_cfg, &codec_i2c_bus_));
    }

    void InitializeButtons() {

        static uint64_t lastClickTime = 0;

        const uint64_t doubleClickThreshold = 500;

        // 设置 boot_button_ 的点击事件
        boot_button_.OnClick([this]() {
            // 唤醒电源保存定时器
            //power_save_timer_->WakeUp();

            uint64_t currentTime = esp_timer_get_time() / 1000;

            // 判断是否为双击
            if (currentTime - lastClickTime <= doubleClickThreshold) {
                // 获取应用实例
                auto& app = Application::GetInstance();

                //if ( app.GetDeviceState() == kDeviceStateIdle )
                //{
                    ResetWifiConfiguration();
                //}

                lastClickTime = 0;

            } 
            else 
            {
                // 获取应用实例
                auto& app = Application::GetInstance();
                // 切换聊天状态
                app.ToggleChatState();

                lastClickTime = currentTime;
            }
        });
    
    
#if CONFIG_USE_ALARM
        // 设置 boot_button_ 的长按事件（仅在启用报警功能时有效）
        boot_button_.OnLongPress([this]() {
            // 获取应用实例
            auto& app = Application::GetInstance();
            // 如果报警管理器存在，则清除报警铃声
            if (app.alarm_m_ != nullptr) {
                ESP_LOGI(TAG, "ClearRing");
                app.alarm_m_->ClearRing();
                // 设置设备状态为空闲
                app.SetDeviceState(kDeviceStateIdle);
                // 更新IoT状态
                app.UpdateIotStates();
                // 清空音频解码队列
                app.audio_decode_queue_.clear();
            }
        });
#endif
    }
    
    void InitializeDisplay() {
        display_ = new NoDisplay();
    }
	
    void InitializeCamera() {
        // Open camera power
        //pca9557_->SetOutputState(2, 0); //控制PCA9557 GPIO扩展芯片为摄像头供电

        gpio_config_t pwdn_pin_cfg = {
            .pin_bit_mask = (1ULL << CAMERA_PIN_PWDN),
            .mode = GPIO_MODE_OUTPUT,
            .pull_up_en = GPIO_PULLUP_DISABLE,  // 硬件已上拉，软件无需启用
            .pull_down_en = GPIO_PULLDOWN_DISABLE,
            .intr_type = GPIO_INTR_DISABLE,
        };
        gpio_config(&pwdn_pin_cfg);

        // 拉低 PWDN 引脚以唤醒摄像头（低电平有效）
        gpio_set_level(CAMERA_PIN_PWDN, 0);

        camera_config_t config = {};
        config.ledc_channel = LEDC_CHANNEL_2;  // LEDC通道选择  用于生成XCLK时钟 但是S3不用
        config.ledc_timer = LEDC_TIMER_2; // LEDC timer选择  用于生成XCLK时钟 但是S3不用
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
        config.pin_sccb_sda = -1;   // 这里写-1 表示使用已经初始化的I2C接口
        config.pin_sccb_scl = CAMERA_PIN_SIOC;
        config.sccb_i2c_port = 0;
        //config.pin_pwdn = CAMERA_PIN_PWDN;
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


    void InitializeIot() {
        auto& thing_manager = iot::ThingManager::GetInstance();
        thing_manager.AddThing(iot::CreateThing("Speaker"));

#if CONFIG_USE_ALARM
        thing_manager.AddThing(iot::CreateThing("AlarmIot"));
#endif

    }

public:
    XINGZHI_CUBE_1_54TFT_WIFI() :
        boot_button_(BOOT_BUTTON_GPIO)
        {
        InitializeCodecI2c();
        InitializeButtons();
        InitializeDisplay();

        InitializeCamera();  // 摄像头初始化

        InitializeIot();
    }

    virtual Led* GetLed() override {
        static SingleLed led(BUILTIN_LED_GPIO);
        return &led;
    }

    virtual AudioCodec* GetAudioCodec() override {
        static Es8311AudioCodec audio_codec(codec_i2c_bus_, I2C_NUM_0, AUDIO_INPUT_SAMPLE_RATE, AUDIO_OUTPUT_SAMPLE_RATE,
            AUDIO_I2S_GPIO_MCLK, AUDIO_I2S_GPIO_BCLK, AUDIO_I2S_GPIO_WS, AUDIO_I2S_GPIO_DOUT, AUDIO_I2S_GPIO_DIN,
            AUDIO_CODEC_PA_PIN, AUDIO_CODEC_ES8311_ADDR);
        return &audio_codec;
    }


    virtual Display* GetDisplay() override {
        return display_;
    }
    

    virtual Camera* GetCamera() override {
        return camera_;
    }    
};

DECLARE_BOARD(XINGZHI_CUBE_1_54TFT_WIFI);
