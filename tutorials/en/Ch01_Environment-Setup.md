# Chapter 1: Environment Setup

## 1.1 Firmware Flashing Environment

### Method A: No Development Environment Needed (Recommended for Beginners)

1. Install the [CH340K serial driver](https://www.wch.cn/download/CH341SER_EXE.html)
2. Open your browser → [esptool-js](https://espressif.github.io/esptool-js/)
3. Connect the NanoCam, select the serial port, and select the firmware .bin file
4. Click Program to flash

### Method B: Command Line

```bash
pip install esptool
esptool.py --chip esp32s3 --port COM_x write_flash 0x0 nanocam_xxx.bin
```

### Method C: ESP-IDF Development Environment (Advanced)

1. Install VSCode + the ESP-IDF extension
2. F1 → `ESP-IDF: Configure ESP-IDF Extension`
3. Select or install ESP-IDF v5.4+
4. Build: `idf.py build flash monitor`

### Method D: ESP-EIM-GUI Installation

1. Download from the official website https://dl.espressif.cn/dl/eim/
2. Download and double-click to enter the EIM page; the top-right corner lets you switch to the Chinese version
3. Click Start Installation
4. Next, choose Custom Installation
5. Before that, you need `git` and `python3.12.x` installed (git download mirror for China: [CNPM Binaries Mirror](https://registry.npmmirror.com/binary.html?path=git-for-windows/v2.55.0.windows.3/))
6. Select esp32s3 as the target device
7. For the ESP-IDF version, check "Show old stable versions" and scroll down to select v5.4.1
8. Leave the download mirror unchanged, then click Next
9. For ESP-IDF features, selecting all is recommended; continue to the next step
10. Select the tools, click Next, then choose the installation location you want and install; wait for the installation to complete
After the installation completes, this version has an unzip issue: locate C:\Espressif\dist\xtensa-esp-elf-14.2.0_20241119-x86_64-w64-mingw32.zip, copy the archive to C:\Espressif\tools\xtensa-esp-elf, unzip it, find the xtensa-esp-elf folder, and replace the folders under C:\Espressif\tools\xtensa-esp-elf\esp-14.2.0_20241119; compilation will then succeed

## 1.2 Server Environment

### xiaozhi.me Official Service (Free)

1. Visit [xiaozhi.me](https://xiaozhi.me) and register an account
2. Enter the console
3. Once the module connects to the network, it announces a 6-digit verification code
4. Click Add Device on the right side of the "Agents" section
5. Enter the announced 6-digit verification code
6. After the device is bound, you can start chatting
