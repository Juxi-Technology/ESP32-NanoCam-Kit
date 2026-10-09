# Manuale del protocollo seriale AT ESP32-NanoCam

> Baud rate: 115200 | Bit di dati: 8 | Parità: Nessuna | Bit di stop: 1 | Controllo di flusso: Nessuno
> Compatibile con l'insieme di comandi AT dei principali moduli fotocamera, con l'aggiunta dei comandi estesi NanoCam.

---

## 1. Regole generali

- I comandi **non distinguono maiuscole e minuscole** (`STA_SSID` = `sta_ssid`)
- Dopo il comando è necessario un **qualsiasi segno di punteggiatura ASCII** (`,` `.` `:` `;` ecc.) come carattere di terminazione
- Alcuni comandi, dopo la modifica, causano un **riavvio automatico**
- Ogni comando termina con `\r\n` (lo strumento seriale di solito lo aggiunge automaticamente)

## 2. Configurazione WiFi

### Modalità STA (connessione al router)

| Comando | Descrizione | Esempio | Valore restituito |
|---|---|---|---|
| `sta_ssid:nome` | Imposta il nome WiFi | `sta_ssid:MyWiFi` | `OK` |
| `sta_pd:password` | Imposta la password WiFi (riavvio dopo la modifica) | `sta_pd:12345678` | `OK` (riavvio) |

> Il nome e la password del WiFi possono contenere al massimo 30 caratteri e non supportano il cinese.

### Modalità AP (hotspot integrato)

| Comando | Descrizione | Esempio | Valore restituito |
|---|---|---|---|
| `ap_ssid:nome` | Imposta il nome dell'hotspot | `ap_ssid:NanoCam-AP` | `OK` |
| `ap_pd:password` | Imposta la password dell'hotspot (riavvio dopo la modifica) | `ap_pd:12345678` | `OK` (riavvio) |

### Modalità WiFi

| Comando | Descrizione | Parametri | Valore restituito |
|---|---|---|---|
| `wifi_mode:X` | Cambia modalità | 0=AP 1=STA 2=AP+STA | `OK` (riavvio in caso di modifica) |

---

## 3. Cambio della modalità IA

| Comando | Modalità | Descrizione | Riavvio |
|---|---|---|---|
| `ai_mode:0` | Normale | Trasmissione MJPEG, senza IA | ✅ |
| `ai_mode:1` | Rilevamento del muso dei gatti | Riquadro sul muso in tempo reale + confidenza | ✅ |
| `ai_mode:2` | Rilevamento volti | Riquadro sul volto in tempo reale + coordinate | ✅ |
| `ai_mode:3` | Riconoscimento dei colori | Selezione del colore→rilevamento in tempo reale | ✅ |
| `ai_mode:4` | Riconoscimento facciale | Registrazione→riconoscimento→eliminazione | ✅ |
| `ai_mode:5` | Codici QR | Decodifica in tempo reale→output sulla porta seriale | ✅ |
| `ai_mode:6` | Agente LLM | Dialogo vocale XiaoZhi AI + visione IA | ✅ |
| `ai_mode:7` | ESP-Claw | Controllo vocale + analisi visiva con foto + OpenAI Vision | ✅ |

> Valori validi di `ai_mode`: 0-7. Oltre l'intervallo il valore predefinito diventa 0. Dopo la modifica il riavvio è automatico e la nuova modalità diventa attiva.

---

## 4. Interrogazione delle informazioni

| Comando | Descrizione | Esempio di valore restituito |
|---|---|---|
| `sta_ip` | Interroga l'IP STA | `sta_ip:192.168.1.100` |
| `ap_ip` | Interroga l'IP AP | `ap_ip:192.168.4.1` |
| `wifi_ver` | Interroga la versione del firmware | `NanoCam Board Ver:0.2.0` |

---

## 5. Controllo del sistema

| Comando | Descrizione | Valore restituito |
|---|---|---|
| `wifi_reset` | Ripristino delle impostazioni di fabbrica (riavvio) | `Reset_OK` |
| `nano_reboot` | Reset software | `Rebooting...` |
| `nano_info` | Informazioni complete sul dispositivo (JSON) | Vedi sotto |

### Esempio di risposta di nano_info

```json
{
  "device": "NanoCam",
  "ver": "0.2.0",
  "chip": "ESP32-S3",
  "flash": "16MB",
  "psram": "8MB",
  "ai_mode": 1,
  "wifi_mode": 2,
  "sta_ip": "192.168.1.100",
  "free_heap": 245760
}
```

---

## 6. Comandi dedicati al riconoscimento facciale

> Validi solo con ai_mode:4 (modalità riconoscimento facciale).

| Comando | Descrizione | Comportamento dell'etichetta | Esempio di risposta |
|---|---|---|---|
| `face_eril` | Registra il volto rilevato nell'immagine corrente | Blu "Enroll: ID N", appare per 0.5s | `>>> face enroll triggered` |
| `face_rz` | Entra in modalità di riconoscimento continuo | Verde "ID: N" / rosso "who?", **visualizzata in modo persistente, non scompare** | `>>> face recognize triggered` |
| `face_del` | Elimina l'ultimo ID di volto registrato | Rosso "N IDs left", appare per 0.5s | `>>> face delete triggered` |
| `face_detect` | Esce dalla modalità di riconoscimento e torna al semplice rilevamento volti | Cancella tutte le etichette | `>>> face detect mode` |

### Flusso operativo del riconoscimento facciale

```
ai_mode:4          # Entra in modalità riconoscimento facciale (il dispositivo si riavvia automaticamente)
face_eril          # Registra un volto (assicurati che nell'inquadratura ci sia un solo volto)
face_rz            # Avvia il riconoscimento continuo — l'etichetta resta visibile senza sparire
face_detect        # Esci dalla modalità di riconoscimento — l'etichetta viene cancellata
face_del           # Elimina l'ultimo volto registrato
```

### Note sul riconoscimento facciale

1. Durante la registrazione assicurati che nell'immagine ci sia **un solo volto**, a 30-50cm di distanza
2. In modalità di riconoscimento (`face_rz`) l'etichetta **resta visualizzata** e non scompare dopo 0.5 secondi — questo è il nuovo comportamento della versione 0.3.0
3. Per uscire dalla modalità di riconoscimento occorre inviare `face_detect`, altrimenti l'etichetta resta sempre visibile
4. Le caratteristiche dei volti sono archiviate nella partizione `fr` della Flash, non si perdono in caso di mancanza di alimentazione e sono supportati fino a 47 ID
5. Il riconoscimento adotta una strategia di salto dei frame (inferenza MFN una volta ogni 10 frame)

---

## 7. Comandi estesi (esclusivi di NanoCam)

| Comando | Descrizione | Stato |
|---|---|---|
| `nano_server:url` | Imposta l'indirizzo del server LLM (salvataggio in NVS) | ✅ |
| `nano_api_key:key` | Imposta la chiave API del LLM (salvataggio in NVS) | ✅ |
| `nano_mqtt:broker,port,topic` | Configura il server MQTT | 🔨 |
| `nano_led:R,G,B` | Imposta il LED RGB (WS2812, GPIO18 DIN) | 📋 |
| `nano_snap` | Scatto con salvataggio (SPIFFS) | ✅ |
| `nano_stream:on/off` | Avvia/arresta la trasmissione video | 📋 |

### nano_server / nano_api_key

| Comando | Descrizione | Esempio | Valore restituito |
|---|---|---|---|
| `nano_server:URL` | Imposta l'indirizzo del server LLM | `nano_server:https://api.openai.com` | `OK server=https://api.openai.com` |
| `nano_api_key:KEY` | Imposta la chiave API | `nano_api_key:sk-xxxx` | `OK` |

> Supporta qualsiasi API compatibile con OpenAI (vLLM / Ollama / modelli locali).
La modalità ESP-Claw (ai_mode:7) supporta `nano_server`, mentre XiaoZhi AI (ai_mode:6) utilizza una configurazione server indipendente.

---

## 8. Note

1. `sta_pd` / `ap_pd` causano un riavvio automatico dopo la modifica; dopo il riavvio la nuova password diventa attiva
2. `ai_mode` causa un riavvio automatico dopo la modifica (solo se la modalità è cambiata)
3. In modalità riconoscimento facciale (mode 4) la configurazione tramite porta seriale Type-C può non funzionare (memoria insufficiente)
4. Il nome e la password del WiFi non possono superare i 30 caratteri e non possono contenere caratteri cinesi
5. Dopo il comando occorre una punteggiatura come carattere di terminazione
