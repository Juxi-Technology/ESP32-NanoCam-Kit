/**
 * NanoCam MakeCode Extension for micro:bit
 *
 * Control ESP32-NanoCam via serial (UART) AT commands.
 * Wire: micro:bit P0(TX)→NanoCam RX, P1(RX)→NanoCam TX
 *
 * Usage in MakeCode:
 *   Extensions → search for "nanocam" → add
 */

enum NanoCamMode {
    //% block="Stream"
    Stream = 0,
    //% block="Cat Face"
    CatFace = 1,
    //% block="Human Face"
    HumanFace = 2,
    //% block="Color"
    Color = 3,
    //% block="Face ID"
    FaceID = 4,
    //% block="QR Code"
    QRCode = 5,
    //% block="LLM Agent"
    LLM = 6
}

/**
 * Custom blocks for NanoCam
 */
//% weight=100 color=#e94560 icon="" block="NanoCam"
namespace nanocam {

    /**
     * Send a raw AT command to NanoCam.
     */
    //% block="send command $cmd"
    //% cmd.defl="wifi_ver"
    function sendCommand(cmd: string): void {
        serial.writeString(cmd + "\r\n")
        basic.pause(200)
    }

    /**
     * Switch AI mode.
     */
    //% block="switch to $mode"
    function setAIMode(mode: NanoCamMode): void {
        sendCommand("ai_mode:" + mode)
        basic.pause(3000)  // wait for reboot
    }

    /**
     * Read AI detection result from NanoCam serial output.
     * Returns JSON string if detected.
     */
    //% block="read detection result"
    function readDetection(): string {
        let result = ""
        let timeout = 0
        while (timeout < 100) {
            let ch = serial.readString()
            if (ch) {
                result += ch
                if (ch == "#") break  // end of detection message
            }
            basic.pause(10)
            timeout += 1
        }
        return result
    }

    /**
     * Get NanoCam firmware version.
     */
    //% block="get version"
    function getVersion(): string {
        sendCommand("wifi_ver")
        return serial.readString()
    }

    /**
     * Get NanoCam IP address.
     */
    //% block="get IP address"
    function getIP(): string {
        sendCommand("sta_ip")
        let resp = serial.readString()
        if (resp.includes("sta_ip:")) {
            return resp.substr(7)
        }
        return "N/A"
    }

    /**
     * Show detected object on micro:bit LED matrix.
     */
    //% block="show AI result on LEDs"
    function showOnLEDs(): void {
        let result = readDetection()
        if (result.includes("face")) {
            basic.showIcon(IconNames.Happy)
        } else if (result.includes("cat")) {
            basic.showIcon(IconNames.Surprised)
        } else if (result.includes("qr") || result.includes("QR")) {
            basic.showIcon(IconNames.Yes)
        } else {
            basic.showIcon(IconNames.No)
        }
    }
}
