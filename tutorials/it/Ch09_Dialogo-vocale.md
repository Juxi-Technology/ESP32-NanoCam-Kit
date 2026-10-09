# Capitolo 9: Dialogo vocale (XiaoZhi AI)

## Informazioni su questo capitolo

Questo capitolo riguarda la modalità XiaoZhi AI (`ai_mode:6`). **Importante**: la modalità 6 (dialogo vocale) e la modalità 7 (ESP-Claw) **condividono lo stesso firmware** (`nanocam_espclaw/`); all'avvio il dispositivo carica semplicemente un insieme diverso di strumenti MCP in base al valore di `ai_mode` memorizzato nella NVS.

| Aspetto | XiaoZhi AI (mode 6) | ESP-Claw (mode 7) |
|---|---|---|
| Dialogo vocale | ✅ ASR→LLM→TTS | ✅ Stessa pipeline vocale |
| Strumenti MCP | Strumenti generici (volume / foto, ecc.) | **Strumenti generici + 5 strumenti dedicati all'hardware** |
| Comprensione visiva | `self.camera.take_photo` | **`self.camera.inspect_image`** (visione multimodale) |
| Controllo LED | ❌ | ✅ Regolazione vocale del colore |
| Scenari d'uso | Dialogo IA generico, educazione per bambini | Controllo hardware, ispezione visiva, domotica |

> Questo capitolo si concentra sulla funzionalità principale di dialogo vocale di **XiaoZhi AI (modalità 6)**. Per conoscere le capacità di controllo hardware di ESP-Claw, leggi il Capitolo 11: Controllo vocale ESP-Claw.

## Funzionamento

NanoCam integra il framework open source XiaoZhi AI e, tramite i protocolli WebSocket / MQTT, si connette al server LLM per realizzare una pipeline completa di interazione vocale:

```
L'utente parla → acquisizione dal microfono ES8311 → codifica Opus
  → WebSocket → riconoscimento vocale ASR nel cloud
  → generazione della risposta da parte del modello LLM
  → sintesi vocale TTS → decodifica Opus
  → amplificatore NS4150B → riproduzione dallo speaker
```

Progettazione full-duplex: l'utente può interrompere direttamente l'AI mentre parla (barge-in), per un'esperienza simile a una conversazione con una persona reale.

## Requisiti hardware

Questo capitolo riguarda le funzionalità audio e richiede il seguente hardware:
- Scheda core NanoCam (con ES8311 Codec + microfono AP2718AT)
- Scheda base NanoCam (con amplificatore NS4150B + CH340K)
- Altoparlante (da collegare al connettore altoparlante della scheda base, VON/VOP)
> È possibile eseguire i test anche con la sola scheda core (ascolto tramite l'uscita cuffie di ES8311). Il microfono è un MEMS in silicio analogico AP2718AT, collegato al pin MIC1P di ES8311 tramite il condensatore di disaccoppiamento C26.

## Procedura

### 10.1 Flashare il firmware XiaoZhi AI

XiaoZhi AI utilizza il progetto firmware indipendente `nanocam_espclaw/`:

```bash
cd nanocam_espclaw
idf.py set-target esp32s3
idf.py build
idf.py -p COMx flash monitor
```

All'avvio la modalità predefinita è quella di XiaoZhi AI.

### 10.2 Connettersi al servizio cloud xiaozhi.me

NanoCam è configurato in fabbrica per connettersi al servizio cloud ufficiale [xiaozhi.me](https://xiaozhi.me) (gratuito): non serve un server proprio.
1. Registra un account su [xiaozhi.me](https://xiaozhi.me)
2. All'accensione il dispositivo annuncia automaticamente un codice di attivazione di 6 cifre
3. Inserisci il codice di attivazione nella console di xiaozhi.me → associa il dispositivo
4. Seleziona il modello LLM nella console (Qwen / DeepSeek ecc.)
L'attivazione è necessaria una sola volta; dalle successive accensioni la connessione è automatica.

### 10.3 Prima conversazione

Dopo aver udito il segnale acustico puoi iniziare a conversare:

```
Tu: "你好小智, com'è il tempo oggi?"
NanoCam: "Fammi controllare il meteo di oggi..."
```

La parola di attivazione è **"你好小智"** (predefinita).

### 10.4 Scenari di conversazione comuni

```
💬 "Racconta una barzelletta" → risposta vocale dell'AI
💬 "Impostami una sveglia da 5 minuti" → funzione sveglia
💬 "Che ore sono" → annuncio dell'ora
💬 "Riproduci musica leggera" → riproduzione musicale in streaming
💬 "Cos'è un buco nero" → domande di cultura generale
```

## Server self-hosted (opzionale)

Se hai esigenze di privacy o desideri utilizzare un LLM self-hosted, puoi distribuire il server open source XiaoZhi AI:

```bash
git clone https://github.com/xinnan-tech/xiaozhi-esp32-server
cd xiaozhi-esp32-server
pip install -r requirements.txt
python app.py
```

L'indirizzo del server utilizzato dal firmware viene fornito tramite il sistema OTA (`CONFIG_OTA_URL` in sdkconfig); all'accensione il dispositivo richiede automaticamente l'indirizzo del server.
> XiaoZhi AI utilizza il server open source XiaoZhi AI (protocollo proprietario WebSocket + pipeline ASR/LLM/TTS). Nella modalità ESP-Claw, che si basa su di esso, la funzionalità di analisi visiva viene configurata dal server durante la fase di handshake MCP, che invia l'URL e il token della Vision API: il firmware non deve configurarli da solo.

## Risoluzione dei problemi

| Sintomo | Causa possibile | Soluzione |
|---|---|---|
| Nessun suono | Altoparlante non collegato | Controlla il connettore dell'altoparlante sulla scheda base |
| Riconoscimento vocale impreciso | Rumore ambientale eccessivo | Parla vicino al microfono (distanza < 1m) |
| Impossibile connettersi | WiFi non configurato | Configura prima la rete via seriale con `sta_ssid:xxx` |
| Nessun codice di attivazione | Primo avvio non completato | Attendi 30 secondi: il dispositivo lo annuncerà automaticamente |
| Risposte molto lente | Latenza del server LLM | Su xiaozhi.me scegli un modello più veloce, oppure usa un server autogestito |
