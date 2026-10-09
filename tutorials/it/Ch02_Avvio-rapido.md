# Capitolo 2: Avvio rapido

## 2.1 Flashing del firmware

### Procedura

1. Estrai la cartella → `nanocam_xxx.bin`
2. Apri [esptool-js](https://espressif.github.io/esptool-js/)
3. Collega il NanoCam tramite Type-C
4. Fai clic su Connect → seleziona la porta seriale
5. Seleziona il file firmware, inserisci `0x0` come indirizzo
6. Fai clic su START → attendi il completamento

### Verifica

Collega il NanoCam con lo strumento seriale (115200 8N1), dovresti vedere:

```
NanoCam Board Ver:0.3.0
```

---

## 2.2 Configurazione WiFi

> Risultato: **NanoCam connesso al WiFi, IP ottenuto**

### Metodo A: configurazione via seriale (il più usato)

```
sta_ssid:nome_della_tua_WiFi
sta_pd:password_della_tua_WiFi
```

Alla ricezione di `OK` → configurazione riuscita. Dopo la modifica della password il dispositivo si riavvia automaticamente.

### Metodo B: connessione diretta all'hotspot AP

NanoCam dispone di un hotspot integrato: `NanoCam-AP`, password `12345678`
Dopo aver connesso lo smartphone, apri il browser su `http://192.168.4.1`

### Verifica

```
sta_ip
```

Restituisce: `sta_ip:192.168.x.x` ✅

---

## 2.3 Il primo frame

> Risultato: **immagine in tempo reale di NanoCam visibile nel browser**
1. Nel browser inserisci `http://<indirizzo IP>`
2. Visualizzi l'immagine MJPEG in tempo reale
3. Invia `ai_mode:1` dalla porta seriale → passaggio al rilevamento del muso dei gatti → nell'immagine compaiono i riquadri di rilevamento

### Descrizione degli endpoint

| URL | Funzione |
|---|---|
| `http://<IP>/` | Immagine in tempo reale (HTML) |
| `http://<IP>/stream` | Flusso MJPEG puro (leggibile da OpenCV/VLC) |
| `http://<IP>/status` | Stato del dispositivo in JSON |
| `http://<IP>/admin` | Pannello di amministrazione Web |
