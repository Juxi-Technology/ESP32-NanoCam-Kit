/**
 * NanoCam Face Tracking Example
 * Pan-tilt servo control based on face position.
 *
 * Wiring: NanoCam RX→D2(TX), TX→D3(RX)
 *         ServoX→D9, ServoY→D10
 */
#include <SoftwareSerial.h>
#include <Servo.h>
#include <NanoCam.h>

SoftwareSerial nanoSerial(2, 3);
NanoCam cam(nanoSerial);
Servo servoX, servoY;

const int CENTER_X = 160;  // QVGA center
const int CENTER_Y = 120;

void setup() {
    Serial.begin(115200);
    nanoSerial.begin(115200);
    servoX.attach(9);
    servoY.attach(10);
    servoX.write(90);  // center
    servoY.write(90);
    delay(1000);

    cam.setAIMode(NANOCAM_HUMAN_FACE);
    Serial.println("Face Tracking Started");
}

void loop() {
    // Read detection via serial output (formats: $face:x,y,w,h#)
    if (nanoSerial.available()) {
        String line = nanoSerial.readStringUntil('\n');
        if (line.startsWith("$face:")) {
            int x = line.substring(6).toInt();
            int y = line.substring(line.indexOf(',')+1).toInt();
            trackFace(x, y);
        }
    }
}

void trackFace(int fx, int fy) {
    int dx = fx - CENTER_X;
    int dy = fy - CENTER_Y;
    if (abs(dx) > 20) servoX.write(constrain(servoX.read() + dx/10, 0, 180));
    if (abs(dy) > 20) servoY.write(constrain(servoY.read() + dy/10, 0, 180));
}
