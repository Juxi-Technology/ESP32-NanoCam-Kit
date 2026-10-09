/**
 * NanoCam AI Mode Switch Example
 *
 * Cycle through all AI modes with a button press.
 *
 * Wiring: NanoCam RX→D3, TX→D2, Button between D4 and GND
 */
#include <SoftwareSerial.h>
#include <NanoCam.h>

SoftwareSerial nanoSerial(2, 3);
NanoCam cam(nanoSerial);

const int BUTTON_PIN = 4;
const char *modeNames[] = {"Stream","CatFace","HumanFace","Color","FaceID","QR","LLM"};
int currentMode = 0;

void setup() {
    Serial.begin(115200);
    nanoSerial.begin(115200);
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    delay(1000);

    Serial.println("NanoCam AI Mode Switch - Press button to cycle");
    cam.setAIMode((NanoCamAIMode)currentMode);
}

void loop() {
    if (digitalRead(BUTTON_PIN) == LOW) {
        delay(50);  // debounce
        if (digitalRead(BUTTON_PIN) == LOW) {
            currentMode = (currentMode + 1) % 7;
            Serial.print("Switching to: ");
            Serial.println(modeNames[currentMode]);
            cam.setAIMode((NanoCamAIMode)currentMode);
            while (digitalRead(BUTTON_PIN) == LOW) delay(10);  // wait release
        }
    }
}
