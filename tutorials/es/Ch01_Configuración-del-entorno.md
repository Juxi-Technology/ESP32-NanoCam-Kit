# Capítulo 1: Configuración del entorno

## 1.1 Entorno de grabación de firmware

### Método A: Sin entorno de desarrollo (recomendado para principiantes)

1. Instala el [controlador de puerto serie CH340K](https://www.wch.cn/download/CH341SER_EXE.html)
2. Abre el navegador → [esptool-js](https://espressif.github.io/esptool-js/)
3. Conecta NanoCam, selecciona el puerto serie y elige el archivo de firmware .bin
4. Haz clic en Program para grabar

### Método B: Línea de comandos

```bash
pip install esptool
esptool.py --chip esp32s3 --port COM_x write_flash 0x0 nanocam_xxx.bin
```

### Método C: Entorno de desarrollo ESP-IDF (avanzado)

1. Instala VSCode + la extensión ESP-IDF
2. F1 → `ESP-IDF: Configure ESP-IDF Extension`
3. Selecciona o instala ESP-IDF v5.4+
4. Compila: `idf.py build flash monitor`

### Método D: Instalación con ESP-EIM-GUI

1. Descarga desde la web oficial https://dl.espressif.cn/dl/eim/
2. Haz doble clic tras la descarga para entrar en la página de EIM; en la esquina superior derecha se puede cambiar al idioma chino
3. Haz clic en comenzar la instalación
4. En el paso siguiente, selecciona la instalación personalizada
5. Antes de esto hay que tener instalados `git` y `python3.12.x` (fuente de descarga de git en China: [CNPM Binaries Mirror](https://registry.npmmirror.com/binary.html?path=git-for-windows/v2.55.0.windows.3/))
6. Selecciona esp32s3 como dispositivo de destino
7. Selecciona la versión de ESP-IDF: aquí hay que marcar "mostrar versiones estables antiguas" y desplazarse hacia abajo hasta la versión v5.4.1
8. En la selección de la imagen de descarga no cambies nada y pulsa siguiente
9. En las funciones de ESP-IDF se recomienda seleccionar todas y continuar al paso siguiente
10. En la selección de herramientas pulsa siguiente y luego elige la ubicación donde quieras instalar; espera a que termine la instalación
Tras completar la instalación, esta versión presenta un problema de descompresión: localiza el directorio C:\Espressif\dist\xtensa-esp-elf-14.2.0_20241119-x86_64-w64-mingw32.zip, copia el archivo comprimido a C:\Espressif\tools\xtensa-esp-elf, descomprímelo y localiza la carpeta xtensa-esp-elf; sustituye la carpeta del directorio C:\Espressif\tools\xtensa-esp-elf\esp-14.2.0_20241119 y la compilación se realizará correctamente

## 1.2 Entorno de servidor

### Servicio oficial xiaozhi.me (gratuito)

1. Visita [xiaozhi.me](https://xiaozhi.me) y registra una cuenta
2. Entra en la consola
3. Cuando el módulo se conecte a la red, anunciará un código de verificación de 6 dígitos
4. Haz clic en "añadir dispositivo" a la derecha del bloque "agente"
5. Introduce el código de verificación de 6 dígitos anunciado
6. Tras vincular el dispositivo, ya puedes empezar a conversar
