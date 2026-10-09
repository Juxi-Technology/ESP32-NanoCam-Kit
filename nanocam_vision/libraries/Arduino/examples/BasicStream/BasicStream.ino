/**
 * NanoCam Basic Stream Example
 *
 * Configure NanoCam WiFi and open MJPEG stream.
 * After running, open browser to the printed IP address.
 *
 * Wiring: NanoCam RX→D3(TX), NanoCam TX→D2(RX) on Arduino
 *         Or use SoftwareSerial on any pins.
 */
#include <SoftwareSerial.h>
#include <NanoCam.h>

#define RX_PIN 2
#define TX_PIN 3

SoftwareSerial nanoSerial(RX_PIN, TX_PIN);
NanoCam cam(nanoSerial);

void setup() {
    Serial.begin(115200);
    nanoSerial.begin(115200);
    delay(1000);

    Serial.println("NanoCam Basic Stream");
    Serial.println("====================");

    // Configure WiFi (change to your network)
    cam.setWifi("YourWiFiSSID", "YourPassword");

    // Get IP and print
    String ip = cam.getSTAIP();
    Serial.print("NanoCam IP: ");
    Serial.println(ip);
    Serial.print("Open in browser: http://");
    Serial.println(ip);

    // Set to stream mode
    cam.setAIMode(NANOCAM_STREAM);
}

void loop() {
    // Print stats every 30 seconds
    Serial.println(cam.getInfo());
    delay(30000);
}
