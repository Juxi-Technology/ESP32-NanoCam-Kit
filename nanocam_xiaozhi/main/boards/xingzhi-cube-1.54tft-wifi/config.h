
#ifndef _BOARD_CONFIG_H_
#define _BOARD_CONFIG_H_

#include <driver/gpio.h>


//音频配置
//#define AUDIO_INPUT_SAMPLE_RATE  16000  //音频输入：16kHz
#define AUDIO_INPUT_SAMPLE_RATE  24000  // 音频输入采样率，设置为24kHz
#define AUDIO_OUTPUT_SAMPLE_RATE 24000  //输出的采样率：24kHz

#define AUDIO_I2S_GPIO_MCLK GPIO_NUM_39   // I2S主时钟信号引脚
#define AUDIO_I2S_GPIO_WS GPIO_NUM_47    // I2S字选择信号引脚（LRCK）
#define AUDIO_I2S_GPIO_BCLK GPIO_NUM_38  // I2S位时钟信号引脚（SCLK）
#define AUDIO_I2S_GPIO_DIN  GPIO_NUM_40  // I2S数据输入引脚（DOUT）
#define AUDIO_I2S_GPIO_DOUT GPIO_NUM_48  // I2S数据输出引脚（DIN）

#define AUDIO_CODEC_PA_PIN       GPIO_NUM_NC  // 音频编解码器功放控制引脚
#define AUDIO_CODEC_I2C_SDA_PIN  GPIO_NUM_41  // 音频编解码器I2C数据引脚
#define AUDIO_CODEC_I2C_SCL_PIN  GPIO_NUM_42  // 音频编解码器I2C时钟引脚
#define AUDIO_CODEC_ES8311_ADDR  (0x30)  // 音频编解码器ES8311的I2C地址，使用默认地址ES8311_CODEC_DEFAULT_ADDR

//按键的GPIO引脚
#define BOOT_BUTTON_GPIO        GPIO_NUM_0  //启动按钮

#define BUILTIN_LED_GPIO        GPIO_NUM_18

/* Camera pins */
#define CAMERA_PIN_PWDN GPIO_NUM_12    // 电源休眠控制引脚（未使用，设为-1）
#define CAMERA_PIN_RESET 14   // 硬件复位引脚（未使用，设为-1）
#define CAMERA_PIN_XCLK 9     // 主时钟输出引脚（24MHz）
#define CAMERA_PIN_SIOD 41    // SCCB数据线（等效I2C SDA）
#define CAMERA_PIN_SIOC 42    // SCCB时钟线（等效I2C SCL）

#define CAMERA_PIN_D7 10
#define CAMERA_PIN_D6 8
#define CAMERA_PIN_D5 7
#define CAMERA_PIN_D4 5
#define CAMERA_PIN_D3 3
#define CAMERA_PIN_D2 1
#define CAMERA_PIN_D1 2
#define CAMERA_PIN_D0 4
#define CAMERA_PIN_VSYNC 13    // 垂直同步信号引脚
#define CAMERA_PIN_HREF 11    // 水平同步信号引脚
#define CAMERA_PIN_PCLK 6     // 像素时钟输入引脚

#define XCLK_FREQ_HZ 24000000

#endif // _BOARD_CONFIG_H_
