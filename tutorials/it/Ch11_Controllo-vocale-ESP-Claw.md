# Capitolo 11: Controllo vocale ESP-Claw

## Informazioni su questo capitolo

Quando il dispositivo passa a `ai_mode:7`, NanoCam entra in modalità ESP-Claw. Essa **condivide lo stesso firmware** di XiaoZhi AI (`ai_mode:6`) (`nanocam_espclaw/`); l'unica differenza è che la modalità ESP-Claw, oltre al dialogo vocale, registra 5 strumenti aggiuntivi per il controllo hardware.

| Aspetto | XiaoZhi AI (mode 6) | ESP-Claw (mode 7) |
|---|---|---|
| Dialogo vocale | ✅ ASR→LLM→TTS | ✅ Stessa pipeline vocale |
| Controllo LED | ❌ | ✅ Regolazione vocale del colore / accensione e spegnimento |
| Cambio di modalità IA | ❌ | ✅ Cambio vocale |
| Scatto + analisi visiva IA | ❌ | ✅ Scatta la foto e usa un'IA multimodale per comprendere l'immagine |

## Funzionamento

Oltre alla pipeline vocale, la modalità ESP-Claw registra tramite `RegisterMcpTools()` 5 strumenti dedicati a NanoCam:

```
Comando vocale dell'utente "metti la luce blu"
  → riconoscimento vocale ASR (cloud)
  → l'LLM comprende l'intento → chiama self.led.set_color({"r":0, "g":0, "b":255})
  → il LED WS2812 di NanoCam diventa blu
  → TTS: "Ok, la luce è stata impostata sul blu"
```

## Procedura

### 12.1 Flashare il firmware

ESP-Claw utilizza il progetto firmware indipendente `nanocam_espclaw/`:

```bash
cd nanocam_espclaw
idf.py set-target esp32s3
idf.py build
idf.py -p COMx flash
```

### 12.2 Cambiare modalità

Dopo l'avvio, imposta la modalità ESP-Claw:

```
ai_mode:7
```

Il dispositivo entra in questa modalità dopo il riavvio automatico. Con `ai_mode:6` puoi tornare alla modalità XiaoZhi AI.

### 12.3 Esempi di controllo vocale

Dopo il risveglio, esprimi direttamente la richiesta:

```
💬 "Accendi la luce" → il WS2812 si accende in bianco
💬 "Metti la luce blu" → il LED diventa blu
💬 "Spegni la luce" → il LED si spegne
💬 "Passa alla modalità rilevamento del volto" → la NVS salva ai_mode:2 + riavvio
💬 "Guarda cosa c'è qui" → scatto + invio all'analisi AI multimodale
💬 "C'è una tazza davanti a me" → l'AI multimodale riconosce la scena
```

### 12.4 Scatto + analisi visiva IA

Quando l'utente dice "guarda...", il firmware acquisisce un frame VGA RGB565, lo comprime in JPEG e lo invia all'API multimodale configurata sul server per l'analisi; il risultato viene annunciato vocalmente tramite TTS.
> L'URL e il token dell'API multimodale vengono forniti automaticamente dal server durante la fase di handshake della connessione: non è necessario inserire manualmente comandi di configurazione dalla porta seriale.

## 5 strumenti dedicati a NanoCam

| Strumento | Funzione | Parametri |
|---|---|---|
| `self.led.set_color` | Imposta il LED RGB WS2812 (GPIO18) | `r,g,b`: 0-255 |
| `self.led.turn_off` | Spegne il LED | Nessuno |
| `self.camera.set_ai_mode` | Cambia modalità IA (salvataggio in NVS + riavvio) | `mode`: 0-7 |
| `self.camera.inspect_image` | Scatto + analisi visiva con LLM multimodale | `prompt`: descrizione della domanda |
| `self.get_device_info` | Informazioni sul dispositivo in JSON | Nessuno |

## File di configurazione

| Contenuto | Percorso |
|---|---|
| Registrazione degli strumenti MCP | `nanocam_espclaw/main/boards/nanocam/nanocam_board.cc` |
| Logica di invio della Vision | `nanocam_espclaw/main/boards/common/esp32_camera.cc` |
| Configurazione SDK predefinita | `nanocam_espclaw/sdkconfig.defaults` |

> Il firmware ESP-Claw è un progetto indipendente e non condivide il codice con `nanocam_vision`. I due firmware devono essere compilati e flashati separatamente.

## Come scegliere

| La tua esigenza | Modalità consigliata |
|---|---|
| Vuoi solo chattare e fare domande a voce | mode 6 (XiaoZhi) |
| Vuoi controllare il LED a voce | mode 7 (ESP-Claw) |
| Vuoi scattare foto e far "vedere" l'immagine all'IA | mode 7 (ESP-Claw) |
| Vuoi cambiare modalità di rilevamento IA a voce | mode 7 (ESP-Claw) |

> Le modalità d'uso complete di ESP-Claw (configurazione del server, sviluppo di strumenti MCP personalizzati, ecc.) sono ancora in fase di esplorazione; la documentazione verrà aggiornata man mano che la ricerca progredisce.
