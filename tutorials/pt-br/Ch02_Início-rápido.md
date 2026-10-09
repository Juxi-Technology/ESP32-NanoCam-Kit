# Capítulo 2: Início rápido

## 2.1 Gravação do firmware

### Passos

1. Extraia a pasta compactada → `nanocam_xxx.bin`
2. Abra o [esptool-js](https://espressif.github.io/esptool-js/)
3. Conecte o NanoCam via Type-C
4. Clique em Connect → selecione a porta serial
5. Escolha o arquivo de firmware e preencha o endereço com `0x0`
6. Clique em START → aguarde a conclusão

### Verificação

Conecte o NanoCam com uma ferramenta de porta serial (115200 8N1); você deve ver:

```
NanoCam Board Ver:0.3.0
```

---

## 2.2 Configuração de rede WiFi

> Resultado: **NanoCam conectado ao WiFi, com IP obtido**

### Método A: configuração pela porta serial (mais comum)

```
sta_ssid:nome_da_sua_rede_WiFi
sta_pd:senha_da_sua_rede_WiFi
```

Ao receber `OK` → configuração bem-sucedida. Após alterar a senha, o dispositivo reinicia automaticamente.

### Método B: conexão direta via ponto de acesso AP

O NanoCam possui um ponto de acesso próprio: `NanoCam-AP`, senha `12345678`
Após conectar o celular, abra no navegador `http://192.168.4.1`

### Verificação

```
sta_ip
```

Retorno: `sta_ip:192.168.x.x` ✅

---

## 2.3 Primeiro quadro

> Resultado: **imagem ao vivo do NanoCam visível no navegador**
1. Digite `http://<IP>` no navegador
2. Veja a imagem MJPEG ao vivo
3. Envie `ai_mode:1` pela porta serial → alterna para a detecção de rosto de gato → a caixa de detecção aparece na imagem

### Descrição dos endpoints

| URL | Uso |
|---|---|
| `http://<IP>/` | Imagem ao vivo (HTML) |
| `http://<IP>/stream` | Fluxo MJPEG puro (legível no OpenCV/VLC) |
| `http://<IP>/status` | Status do dispositivo em JSON |
| `http://<IP>/admin` | Painel de administração web |
