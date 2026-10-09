#ifndef NanoCam_h
#define NanoCam_h

#include <Arduino.h>
#include <Stream.h>

/**
 * NanoCam Arduino Library v0.1
 *
 * Control ESP32-NanoCam via serial AT commands.
 * Wire NanoCam TX→Arduino RX, NanoCam RX→Arduino TX.
 *
 * Usage:
 *   NanoCam cam(Serial1);
 *   cam.begin();
 *   cam.setWifi("MyWiFi", "password");
 *   cam.setAIMode(NANOCAM_FACE_DETECT);
 *   String result = cam.query("sta_ip");
 */

// AI modes
enum NanoCamAIMode {
    NANOCAM_STREAM      = 0,
    NANOCAM_CAT_FACE    = 1,
    NANOCAM_HUMAN_FACE  = 2,
    NANOCAM_COLOR       = 3,
    NANOCAM_FACE_ID     = 4,
    NANOCAM_QR_CODE     = 5,
    NANOCAM_LLM_AGENT   = 6,
    NANOCAM_ESPCLAW     = 7,
};

// WiFi modes
enum NanoCamWiFiMode {
    NANOCAM_WIFI_AP     = 0,
    NANOCAM_WIFI_STA    = 1,
    NANOCAM_WIFI_APSTA  = 2,
};

// Detection result
struct NanoCamDetection {
    int16_t x, y;       // top-left corner
    int16_t w, h;       // width, height
    int16_t id;         // face/color ID (-1 if N/A)
    int     confidence; // 0-100
    String  raw;        // raw response string
};

class NanoCam {
public:
    NanoCam(Stream &serial, uint32_t baud = 115200)
        : _serial(&serial), _baud(baud) {}

    /** Initialize serial. Call in setup(). */
    void begin() {
        if (_serial == &Serial) {
            Serial.begin(_baud);
        }
        delay(500);
    }

    /** Send raw command, return response. */
    String sendCommand(const String &cmd) {
        _serial->print(cmd);
        _serial->print("\r\n");
        return _readResponse(2000);
    }

    // ---- WiFi ----

    /** Set STA WiFi credentials. Password change triggers reboot. */
    void setWifi(const String &ssid, const String &password) {
        sendCommand("sta_ssid:" + ssid);
        delay(500);
        sendCommand("sta_pd:" + password);
        delay(3000);  // wait for reboot
    }

    /** Set AP hotspot credentials. */
    void setAP(const String &ssid, const String &password) {
        sendCommand("ap_ssid:" + ssid);
        delay(500);
        sendCommand("ap_pd:" + password);
        delay(3000);
    }

    /** Set WiFi mode (0=AP, 1=STA, 2=AP+STA). */
    void setWiFiMode(NanoCamWiFiMode mode) {
        sendCommand("wifi_mode:" + String(mode));
        delay(3000);
    }

    // ---- AI Mode ----

    /** Switch AI mode. Triggers reboot (wait 3s). */
    void setAIMode(NanoCamAIMode mode) {
        sendCommand("ai_mode:" + String(mode));
        delay(3000);
    }

    // ---- Queries ----

    /** Get STA IP address. */
    String getSTAIP() {
        String resp = sendCommand("sta_ip");
        if (resp.startsWith("sta_ip:"))
            return resp.substring(7);
        return "";
    }

    /** Get firmware version. */
    String getVersion() {
        return sendCommand("wifi_ver");
    }

    /** Get device info as JSON. */
    String getInfo() {
        return sendCommand("nano_info");
    }

    /** Send custom command, get full response. */
    String query(const String &cmd) {
        return sendCommand(cmd);
    }

    // ---- Virtual Keys ----

    void keyMenu()  { sendCommand("KEY_MENU"); }
    void keyPlay()  { sendCommand("KEY_PLAY"); }
    void keyUp()    { sendCommand("KEY_UPUP"); }
    void keyDown()  { sendCommand("KEY_DOWN"); }

    // ---- Control ----

    /** Factory reset (triggers reboot). */
    void factoryReset() {
        sendCommand("wifi_reset");
        delay(3000);
    }

    /** Soft reboot. */
    void reboot() {
        sendCommand("nano_reboot");
        delay(3000);
    }

    // ---- AI Detection Results ----

    /**
     * Read and parse AI detection result from serial.
     * NanoCam outputs detection frames in these formats:
     *   $face:x,y,w,h,conf#
     *   $cat:x,y,w,h,conf#
     *   $color:x,y,w,h,id,conf#
     *   $qr:data#
     *
     * Returns true if a valid detection was parsed.
     */
    bool readDetection(NanoCamDetection &det) {
        if (!_serial->available()) return false;

        String line = _serial->readStringUntil('\n');
        // Try I2C-style hex dump (e.g. "I2C: addr=0x33 reg=0x28 val=..." → skip)
        if (line.startsWith("I2C:")) return false;
        // Try MCP JSON (e.g. {"jsonrpc":"2.0"...}) → skip
        if (line.charAt(0) == '{')   return false;

        // Parse $type:x,y,w,h,...#
        int d1 = line.indexOf('$');
        int d2 = line.indexOf('#');
        if (d1 < 0 || d2 < d1) return false;

        String body = line.substring(d1 + 1, d2);  // e.g. "face:120,80,60,60,95"
        int colon = body.indexOf(':');
        if (colon < 0) return false;

        String type = body.substring(0, colon);
        String vals = body.substring(colon + 1);

        det.raw = line;
        det.confidence = 0;
        det.id = -1;

        if (type == "face" || type == "cat") {
            // $face:x,y,w,h,conf#
            int c1 = vals.indexOf(',');
            int c2 = vals.indexOf(',', c1 + 1);
            int c3 = vals.indexOf(',', c2 + 1);
            int c4 = vals.indexOf(',', c3 + 1);
            if (c1 < 0 || c2 < 0 || c3 < 0) return false;
            det.x  = vals.substring(0, c1).toInt();
            det.y  = vals.substring(c1 + 1, c2).toInt();
            det.w  = vals.substring(c2 + 1, c3).toInt();
            det.h  = vals.substring(c3 + 1, (c4 > 0 ? c4 : vals.length())).toInt();
            if (c4 > 0) det.confidence = vals.substring(c4 + 1).toInt();
            return true;
        }

        if (type == "color") {
            // $color:x,y,w,h,id,conf#
            int c1 = vals.indexOf(',');
            int c2 = vals.indexOf(',', c1 + 1);
            int c3 = vals.indexOf(',', c2 + 1);
            int c4 = vals.indexOf(',', c3 + 1);
            int c5 = vals.indexOf(',', c4 + 1);
            if (c1 < 0 || c2 < 0 || c3 < 0 || c4 < 0) return false;
            det.x  = vals.substring(0, c1).toInt();
            det.y  = vals.substring(c1 + 1, c2).toInt();
            det.w  = vals.substring(c2 + 1, c3).toInt();
            det.h  = vals.substring(c3 + 1, c4).toInt();
            det.id = vals.substring(c4 + 1, (c5 > 0 ? c5 : vals.length())).toInt();
            if (c5 > 0) det.confidence = vals.substring(c5 + 1).toInt();
            return true;
        }

        if (type == "qr") {
            // $qr:data#
            det.id   = 0;
            det.raw  = vals;
            return true;
        }

        return false;
    }

private:
    Stream *_serial;
    uint32_t _baud;

    String _readResponse(int timeoutMs) {
        String result;
        uint32_t start = millis();
        while (millis() - start < timeoutMs) {
            while (_serial->available()) {
                char c = _serial->read();
                if (c == '\r' || c == '\n') {
                    if (result.length() > 0) return result;
                } else {
                    result += c;
                }
            }
            delay(10);
        }
        return result;
    }
};

#endif
