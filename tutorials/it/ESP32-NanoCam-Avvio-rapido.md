# Guida rapida ESP32-NanoCam

---

## Prerequisiti

- Scheda core NanoCam + scheda base (ESP32-S3 N16R8 + CH340K)
- Cavo dati USB Type-C (con supporto al trasferimento dati)
- PC (Windows / Mac / Linux)
- Modulo fotocamera GC2145 (collegato in fabbrica)

---

## Passo 1: flashare il firmware (3 minuti)

### Metodo A: senza ambiente di sviluppo (consigliato)

1. Apri il browser e visita [esptool-js](https://espressif.github.io/esptool-js/)
2. Collega il NanoCam al PC con un cavo Type-C
3. Seleziona la porta seriale, baud rate 115200
4. Trova il file firmware `nanocam_xxx.bin` all'interno dell'archivio scompattato
5. Seleziona il file firmware `nanocam_xxx.bin`, indirizzo `0x0`
6. Fai clic su "START" e attendi il completamento

### Metodo B: riga di comando (avanzato)

```bash
pip install esptool
esptool.py --chip esp32s3 --port COM3 write_flash 0x0 nanocam.bin
```

---

## Passo 2: connettersi al WiFi (2 minuti)

NanoCam funziona per impostazione predefinita in **doppia modalità AP+STA simultanea**, senza necessità di commutazione:
- L'**hotspot AP** resta sempre attivo: collega direttamente lo smartphone a `NanoCam-AP` (password `12345678`) e apri il browser su `http://192.168.4.1`
- La **connessione STA al router** richiede una configurazione WiFi una tantum:
Collega lo strumento seriale (baud rate **115200 8N1**) alla porta Type-C del NanoCam:

```
sta_ssid:nome_della_tua_WiFi
sta_pd:password_della_tua_WiFi
```

> La ricezione di `OK` indica che la configurazione è riuscita. Dopo la modifica della password il dispositivo si riavvia automaticamente.
Per cambiare la modalità WiFi (di solito non necessario):

| Comando | Modalità | Descrizione |
|---|---|---|
| `wifi_mode:0` | Solo AP | Disattiva lo STA, mantiene solo l'hotspot |
| `wifi_mode:1` | Solo STA | Disattiva l'hotspot, si connette solo al router |
| `wifi_mode:2` | AP+STA | Predefinita: entrambe attive contemporaneamente |

---

## Passo 3: visualizzare l'immagine (1 minuto)

1. Invia `sta_ip` dalla porta seriale per ottenere l'IP STA
2. Nel browser inserisci `http://<indirizzo IP>` (oppure, in modalità AP, `http://192.168.4.1`)
3. Nella pagina Web puoi vedere l'immagine in tempo reale

---

## Passo 4: esplorare le funzionalità IA (2 minuti)

Invia i seguenti comandi dalla porta seriale per cambiare modalità:

| Comando | Modalità | Effetto |
|---|---|---|
| `ai_mode:0` | Trasmissione video standard | Immagine MJPEG in tempo reale |
| `ai_mode:1` | Rilevamento del muso dei gatti | Nell'immagine compare il riquadro di rilevamento del muso |
| `ai_mode:2` | Rilevamento volti | Nell'immagine compare il riquadro di rilevamento del volto |
| `ai_mode:3` | Riconoscimento dei colori | Selezione del colore→inseguimento in tempo reale |
| `ai_mode:4` | Riconoscimento facciale | Registrazione→riconoscimento→eliminazione |
| `ai_mode:5` | Scansione codici QR | Inquadra il codice QR→contenuto emesso sulla porta seriale |
| `ai_mode:6` | Agente LLM | Attivazione vocale "你好小智" (XiaoZhi AI) |
| `ai_mode:7` | ESP-Claw | ESP-Claw AI Agent (framework ufficiale Espressif) |

> Per ogni cambio di modalità è necessario un riavvio manuale: puoi riavviare premendo il pulsante RST del modulo; dopo il riavvio la nuova modalità diventa attiva.

---

## Passo 5: integrare nel tuo progetto

### Controllo con Arduino

```cpp
Serial.begin(115200);
Serial.print("ai_mode:2");  // Passa al rilevamento dei volti
```

### Controllo con Python

```python
import serial
ser = serial.Serial("COM3", 115200)
ser.write(b"ai_mode:1\r\n")  # Passa al rilevamento dei volti di gatto
```

### Consultare l'elenco completo dei comandi

→ Manuale del protocollo seriale AT

---

## Domande frequenti

| Problema | Soluzione |
|---|---|
| Flashing non riuscito | Verifica che il cavo Type-C supporti il trasferimento dati; tieni premuto S2 (BOOT) sulla scheda base e ridai alimentazione |
| Nessuna immagine visibile | Invia `sta_ip` dalla seriale per confermare l'IP; verifica che i dispositivi siano nella stessa sottorete |
| La fotocamera non si accende | Controlla che il flat FPC sia inserito a fondo con i contatti metallici rivolti verso il basso; verifica PWDN(IO12)/RESET(IO14) |
| Impossibile connettersi al WiFi | Invia `wifi_reset` per ripristinare le impostazioni di fabbrica, poi riconfigura |

Per ulteriori problemi → [FAQ](https://FAQ.md)

---

## Passi successivi

- 📖 Manuale del protocollo seriale — riferimento completo dei comandi AT
- 🎓 Programma del tutorial — percorso progressivo in 28 capitoli
- 🔧 Specifiche hardware — mappatura completa dei pin GPIO
- 🤖 Guida all'integrazione ROS2 — tutorial micro-ROS
