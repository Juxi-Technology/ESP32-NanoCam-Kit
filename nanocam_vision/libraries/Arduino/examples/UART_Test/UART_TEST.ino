
/*
  ESP32-S3 UART on GPIO19 and GPIO20 Example
  
  注意：
  1. GPIO19 (USB D-) 和 GPIO20 (USB D+) 默认用于 USB 通信。
  2. 将此代码烧录后，如果通过 USB 连接电脑，可能会失去 USB 串口连接。
  3. 建议使用外部 USB-TTL 模块连接至 GPIO19 (RX) 和 GPIO20 (TX) 进行通信测试。
  4. 连接方式：
     - 外部 TTL TX -> ESP32-S3 GPIO19 (RX)
     - 外部 TTL RX -> ESP32-S3 GPIO20 (TX)
     - GND -> GND
*/

// 定义使用的 UART 端口和引脚
#define UART_PORT 1
#define UART_RX_PIN 19
#define UART_TX_PIN 20
#define BAUD_RATE 115200

void setup() {
  // 初始化主串口（USB CDC），用于调试输出状态
  // 注意：如果 GPIO19/20 被占用且影响了 USB 外设，这里可能无法正常工作
  Serial.begin(115200);
  delay(1000); // 等待启动稳定
  
  Serial.println("ESP32-S3 UART Test on GPIO19/20");
  
  // 初始化 UART1，指定 RX 和 TX 引脚
  // Serial1 对应 UART1
  Serial1.begin(BAUD_RATE, SERIAL_8N1, UART_RX_PIN, UART_TX_PIN);
  
  if (Serial1) {
    Serial.println("UART1 initialized on GPIO19(RX) and GPIO20(TX).");
  } else {
    Serial.println("Failed to initialize UART1.");
  }
}

void loop() {
  // 从 UART1 (GPIO19/20) 读取数据并转发到 USB 串口监视器
  if (Serial1.available()) {
    char c = Serial1.read();
    Serial.print("Received from UART1: ");
    Serial.println(c);
    
    // 回显数据到 UART1
    Serial1.write(c);
  }
  
  // 每秒发送一个心跳包到 UART1
  static unsigned long lastTime = 0;
  if (millis() - lastTime > 1000) {
    lastTime = millis();
    Serial1.println("Heartbeat from ESP32-S3");
  }
}
