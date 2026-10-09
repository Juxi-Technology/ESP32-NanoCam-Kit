# Capítulo 1: Configuração do ambiente

## 1.1 Ambiente de gravação de firmware

### Método A: sem ambiente de desenvolvimento (recomendado para iniciantes)

1. Instale o [driver de porta serial CH340K](https://www.wch.cn/download/CH341SER_EXE.html)
2. Abra o navegador → [esptool-js](https://espressif.github.io/esptool-js/)
3. Conecte o NanoCam, selecione a porta serial e escolha o arquivo de firmware .bin
4. Clique em Program para gravar

### Método B: linha de comando

```bash
pip install esptool
esptool.py --chip esp32s3 --port COM_x write_flash 0x0 nanocam_xxx.bin
```

### Método C: ambiente de desenvolvimento ESP-IDF (avançado)

1. Instale o VSCode + plugin ESP-IDF
2. F1 → `ESP-IDF: Configure ESP-IDF Extension`
3. Selecione ou instale o ESP-IDF v5.4+
4. Compile: `idf.py build flash monitor`

### Método D: instalação via ESP-EIM-GUI

1. Baixe no site oficial https://dl.espressif.cn/dl/eim/
2. Após baixar, dê um clique duplo para entrar na página do EIM; no canto superior direito é possível alternar para a versão em chinês
3. Clique em iniciar a instalação
4. Na etapa seguinte, escolha a instalação personalizada
5. Antes disso, é preciso ter o `git` e o `python3.12.x` instalados (fonte de download do git na China: [CNPM Binaries Mirror](https://registry.npmmirror.com/binary.html?path=git-for-windows/v2.55.0.windows.3/))
6. Selecione o dispositivo de destino como esp32s3
7. Escolha a versão do ESP-IDF; aqui é preciso marcar "exibir versões estáveis antigas" e rolar para baixo para selecionar a versão v5.4.1
8. Mantenha o espelho de download inalterado e avance
9. Selecione os recursos do ESP-IDF; aqui recomenda-se marcar todos os itens e continuar para a próxima etapa
10. Selecione as ferramentas e avance; em seguida, escolha o local desejado para instalar e aguarde a instalação terminar
Após a instalação, essa versão terá um problema de descompactação: localize o diretório C:\Espressif\dist\xtensa-esp-elf-14.2.0_20241119-x86_64-w64-mingw32.zip, copie o pacote compactado para C:\Espressif\tools\xtensa-esp-elf, descompacte e localize a pasta xtensa-esp-elf; substitua a pasta dentro do diretório C:\Espressif\tools\xtensa-esp-elf\esp-14.2.0_20241119 e a compilação funcionará

## 1.2 Ambiente de servidor

### Serviço oficial xiaozhi.me (gratuito)

1. Acesse [xiaozhi.me](https://xiaozhi.me) e crie uma conta
2. Entre no console
3. Após o módulo se conectar à rede, ele anunciará um código de verificação de 6 dígitos
4. Clique em adicionar dispositivo à direita do bloco "Agente"
5. Insira o código de verificação de 6 dígitos anunciado
6. Após vincular o dispositivo, você já pode iniciar a conversa
