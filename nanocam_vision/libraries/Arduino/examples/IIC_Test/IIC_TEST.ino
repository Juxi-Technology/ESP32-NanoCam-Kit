#include <Wire.h>

// ================= 配置区域 =================
// 请根据你的实际接线修改以下引脚号
// 常见 ESP32-S3 开发板引脚参考见下文“常见引脚对照表”
const int SDA_PIN = 41;  // 例如：Adafruit QT Py S3, ESP32-S3-DevKitC-1 (默认I2C0)
const int SCL_PIN = 42;  // 例如：Adafruit QT Py S3, ESP32-S3-DevKitC-1 (默认I2C0)
// ===========================================

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    delay(10); // 等待串口连接，防止启动太快错过输出
  }

  Serial.println("\n================================");
  Serial.println(" ESP32-S3 I2C Scanner Started ");
  Serial.println("================================");
  Serial.print("Using SDA Pin: ");
  Serial.println(SDA_PIN);
  Serial.print("Using SCL Pin: ");
  Serial.println(SCL_PIN);

  // 初始化 Wire 库，指定引脚
  Wire.begin(SDA_PIN, SCL_PIN);
  
  // 可选：设置 I2C 时钟频率，默认为 100kHz，可设为 400kHz (400000)
  // Wire.setClock(400000); 

  Serial.println("\nScanning for I2C devices...\n");
}

void loop() {
  byte count = 0;
  
  // I2C 7位地址范围通常是 0x01 到 0x7F (1-127)
  for (byte address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    byte error = Wire.endTransmission();

    if (error == 0) {
      Serial.print("Found device at address: 0x");
      if (address < 16) Serial.print("0");
      Serial.println(address, HEX);
      
      // 针对常见 OLED 屏幕的提示
      if (address == 0x3C) {
        Serial.println(" >>> Likely a standard SSD1306/SH1106 OLED (Use 0x3C)");
      } else if (address == 0x3D) {
        Serial.println(" >>> Likely an OLED with alternate address (Use 0x3D)");
      }
      count++;
    } 
    else if (error == 4) {
      Serial.print("Unknown error at address 0x");
      if (address < 16) Serial.print("0");
      Serial.println(address, HEX);
    }
    
    delay(5); // 短暂延时，让总线稳定
  }

  if (count == 0) {
    Serial.println("No I2C devices found!");
    Serial.println("Check your wiring (SDA, SCL, VCC, GND).");
    Serial.println("Ensure you are using the correct GPIO pins for your board.");
  } else {
    Serial.print("Total devices found: ");
    Serial.println(count);
  }

  Serial.println("\n--------------------------------\n");
  delay(5000); // 每5秒重新扫描一次
}
