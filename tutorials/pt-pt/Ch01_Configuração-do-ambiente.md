# Capítulo 1: Configuração do ambiente

## 1.1 Ambiente de gravação de firmware

### Método A: sem ambiente de desenvolvimento (recomendado para principiantes)

1. Instale o [controlador de porta série CH340K](https://www.wch.cn/download/CH341SER_EXE.html)
2. Abra o navegador → [esptool-js](https://espressif.github.io/esptool-js/)
3. Ligue o NanoCam, selecione a porta série e escolha o ficheiro de firmware .bin
4. Clique em Program para gravar

### Método B: linha de comandos

```bash
pip install esptool
esptool.py --chip esp32s3 --port COM_x write_flash 0x0 nanocam_xxx.bin
```

### Método C: ambiente de desenvolvimento ESP-IDF (avançado)

1. Instale o VSCode + extensão ESP-IDF
2. F1 → `ESP-IDF: Configure ESP-IDF Extension`
3. Selecione ou instale o ESP-IDF v5.4+
4. Compilar: `idf.py build flash monitor`

### Método D: instalação com o ESP-EIM-GUI

1. Descarregue do site oficial https://dl.espressif.cn/dl/eim/
2. Após o download, faça duplo clique para entrar na página do EIM; no canto superior direito é possível mudar para a versão em chinês
3. Clique para iniciar a instalação
4. No passo seguinte, escolha a instalação personalizada
5. Antes disso, é necessário ter o `git` e o `python3.12.x` instalados (fonte de download do git na China: [CNPM Binaries Mirror](https://registry.npmmirror.com/binary.html?path=git-for-windows/v2.55.0.windows.3/))
6. Selecione esp32s3 como dispositivo de destino
7. Na escolha da versão do ESP-IDF, é necessário marcar "mostrar versões estáveis antigas" e, deslizando para baixo, selecionar a versão v5.4.1
8. Mantenha o espelho de download predefinido e avance para o passo seguinte
9. Na seleção de funcionalidades do ESP-IDF, recomenda-se selecionar tudo e continuar para o passo seguinte
10. Na seleção de ferramentas, avance para o passo seguinte, escolha a localização de instalação pretendida e aguarde a conclusão da instalação
Após a instalação, esta versão apresenta um problema de extração: localize o diretório C:\Espressif\dist\xtensa-esp-elf-14.2.0_20241119-x86_64-w64-mingw32.zip, copie o ficheiro comprimido para C:\Espressif\tools\xtensa-esp-elf, extraia-o, encontre a pasta xtensa-esp-elf e substitua a pasta existente em C:\Espressif\tools\xtensa-esp-elf\esp-14.2.0_20241119 — assim a compilação será bem-sucedida

## 1.2 Ambiente do servidor

### Serviço oficial xiaozhi.me (gratuito)

1. Aceda a [xiaozhi.me](https://xiaozhi.me) e registe uma conta
2. Entre na consola
3. Depois de o módulo se ligar à rede, será anunciado um código de verificação de 6 dígitos
4. Clique em "Adicionar dispositivo", à direita do bloco "Agente"
5. Introduza o código de verificação de 6 dígitos anunciado
6. Após associar o dispositivo, pode começar a conversar
