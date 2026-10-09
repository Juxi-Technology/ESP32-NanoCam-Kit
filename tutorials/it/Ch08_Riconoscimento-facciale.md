# Capitolo 8: Riconoscimento facciale

## Funzionamento

Riconoscimento facciale = **rilevamento volti** (pipeline a due stadi MSR01+MNP01) + **estrazione delle caratteristiche** (rete neurale MFN FaceRecognition112V1S8) + **confronto per similarità coseno**.

```
Frame RGB565 dalla fotocamera
  → rilevamento grossolano MSR01 (320×240, soglia 0.3F)
  → rilevamento fine MNP01 (sui candidati del rilevamento grossolano, soglia 0.4F)
  → estrazione di 10 keypoint del volto (occhi/estremità del naso/angoli della bocca)
  → allineamento sui keypoint → ritaglio volto 112×112
  → rete convoluzionale MFN → vettore di caratteristiche a 512 dimensioni
  → normalizzazione L2
  → calcolo della distanza coseno con i vettori di tutti gli ID registrati nel Flash
  → similarità coseno massima > soglia (0.55) → corrispondenza trovata → output ID
  → tutte le similarità < soglia → sconosciuto → output "who?"
```

### Ottimizzazione delle prestazioni

L'estrazione delle caratteristiche MFN e il confronto con l'intera libreria comportano un carico di calcolo considerevole: eseguirli a ogni frame causerebbe scatti nell'immagine. L'implementazione attuale adotta una **strategia di salto dei frame**: il rilevamento volti viene eseguito a ogni frame (a basso costo), il riconoscimento MFN una volta ogni 10 frame (ad alto costo), mentre l'etichetta continua a mostrare in sovrapposizione l'ultimo risultato di riconoscimento. Così l'immagine resta fluida e l'etichetta dell'ID non lampeggia.

### Memorizzazione delle caratteristiche del volto

Le caratteristiche dei volti registrati (id + embedding a 512 dimensioni) vengono archiviate in modo persistente nella partizione `fr` della Flash (96 KB, fino a 47 ID di volti). Non si perdono in caso di mancanza di alimentazione.

## Prerequisiti hardware

- Scheda core NanoCam + scheda base
- Cavo dati USB-C (collegato al PC per alimentazione e porta seriale)
- Strumento di comunicazione seriale (baud rate 115200)

## Procedura

### 8.1 Entrare in modalità riconoscimento facciale

```
ai_mode:4
```

Il dispositivo si riavvia automaticamente in modalità FaceID; il LED RGB WS2812 (GPIO18 DIN, alimentato a VDD50) diventa viola. Dopo il riavvio, sulla porta seriale dovresti vedere:

```
I (5526) MFN: fr partition size: 98304 bytes, maxminum 47 IDs can be stored
I (5526) MFN: No face ID in flash
```

`No face ID in flash` indica che non è ancora stato registrato alcun volto: è normale.

### 8.2 Registrare un volto

Posiziona il volto rivolto verso la fotocamera (distanza 30-50cm, illuminazione uniforme) e assicurati che nell'immagine ci sia **un solo volto**. Invia dalla porta seriale:

```
face_eril
```

Dopo aver rilevato il volto, il dispositivo ne estrae automaticamente le caratteristiche e lo registra nella Flash:

```
I (xxxx) ENROLL: ID 1 is enrolled
```

Sull'immagine viene sovrapposto il testo blu `Enroll: ID 1`, che scompare dopo circa 0.5 secondi.
> **Attenzione**: il comando è `face_eril` (abbreviazione di enroll), non `face_enroll`. Se vedi `fail: unknown command`, controlla l'ortografia.

### 8.3 Riconoscere i volti

Dopo la registrazione, invia il comando di riconoscimento:

```
face_rz
```

Il sistema entra in modalità di riconoscimento continuo. Il volto attuale viene confrontato con tutti gli ID registrati nella Flash:
- **Corrispondenza trovata**: la porta seriale emette `Similarity: 0.85, Match ID: 1` e sull'immagine viene sovrapposto stabilmente `ID: 1` in verde
- **Sconosciuto**: la porta seriale emette `Similarity: 0.32, Match ID: 0` e sull'immagine viene sovrapposto stabilmente `who?` in rosso
> L'etichetta **resta visualizzata** e non scompare. Per uscire dalla modalità di riconoscimento, invia `face_detect` per tornare alla semplice modalità di rilevamento.

### 8.4 Eliminare un volto

```
face_del
```

Elimina l'ultimo ID di volto registrato; la porta seriale restituisce `N IDs left` e sull'immagine viene mostrato brevemente il numero di ID rimanenti. Anche le caratteristiche corrispondenti nella Flash vengono eliminate.

### 8.5 Uscire dalla modalità di riconoscimento

```
face_detect
```

Torna alla modalità di semplice rilevamento volti (solo riquadri + punti chiave, senza riconoscimento) e cancella le etichette con gli ID.
> **Informazioni sulla modalità DETECT**: sull'ESP32-S3 la stampa delle coordinate sulla porta seriale è disabilitata nella modalità di semplice rilevamento volti (`#if !CONFIG_IDF_TARGET_ESP32S3`), per evitare che il log di rilevamento saturi la porta seriale. I log delle coordinate `detection_result` vengono emessi solo dopo essere entrati nella modalità di riconoscimento (`face_rz`).

## Riferimento rapido completo dei comandi

| Comando | Funzione | Comportamento dell'etichetta | Persistente |
|---|---|---|---|
| `face_eril` | Registra il volto attualmente rilevato | Blu "Enroll: ID N" | Appare per 0.5s |
| `face_rz` | Entra in modalità di riconoscimento continuo | Verde "ID: N" / rosso "who?" | ✅ Sì |
| `face_del` | Elimina l'ultimo ID registrato | Rosso "N IDs left" | Appare per 0.5s |
| `face_detect` | Esce dal riconoscimento e torna al semplice rilevamento | Cancella tutte le etichette | — |

## Esempio di flusso operativo

```
ai_mode:4                          # entra in modalità riconoscimento facciale
[il dispositivo si riavvia, LED viola]

face_eril                          # registra il primo volto (Mario)
→ ID 1 is enrolled

face_eril                          # registra il secondo volto (Luca)
→ ID 2 is enrolled

face_rz                            # avvia il riconoscimento continuo
→ Mario davanti alla fotocamera: l'immagine mostra in continuo "ID: 1"
→ Luca davanti alla fotocamera: l'immagine mostra in continuo "ID: 2"
→ Sconosciuto davanti alla fotocamera: l'immagine mostra in continuo "who?"

face_detect                        # esce dalla modalità riconoscimento
→ le etichette scompaiono, resta solo il riquadro di rilevamento

face_del                           # elimina Luca (ID 2)
→ 1 IDs left

face_rz                            # nuovo riconoscimento
→ Mario davanti alla fotocamera: "ID: 1"
→ Luca davanti alla fotocamera: "who?" (eliminato)
```

> La modalità di riconoscimento facciale ha un consumo di memoria elevato (modello MFN + doppio modello di rilevamento volti); la porta seriale Type-C (UART0) funziona comunque regolarmente. Se la porta seriale non risponde, controlla innanzitutto che il baud rate sia 115200.

## Codice

### Logica di riconoscimento principale

`components/modules/ai/who_human_face_recognition.cpp` — strategia di riconoscimento a salto di frame:

```cpp
case RECOGNIZE:
{
    // salto dei frame: 1 riconoscimento MFN ogni 10 rilevamenti
    static int recog_skip = 0;
    if (recog_skip <= 0) {
        recognize_result = recognizer->recognize(
            (uint16_t *)frame->buf,
            {(int)frame->height, (int)frame->width, 3},
            detect_results.front().keypoint);
        recog_skip = 10;
    }
    recog_skip--;
    frame_show_state = SHOW_STATE_RECOGNIZE;
    break;
}
```

## Risoluzione dei problemi

| Sintomo | Causa possibile | Soluzione |
|---|---|---|
| `No face ID in flash` | Normale: nessuna registrazione ancora effettuata | Invia `face_eril` per registrare |
| Il riconoscimento restituisce sempre `who?` | Illuminazione insufficiente / angolazione sfavorevole / similarità sotto la soglia | Registra di nuovo, con il volto rivolto verso la fotocamera e illuminazione uniforme |
| Nessuna reazione durante la registrazione | Nell'immagine il numero di volti ≠ 1 | Assicurati che ci sia un solo volto, a 30-50cm di distanza |
| Scatti nell'immagine durante il riconoscimento | Normale: l'inferenza MFN richiede tempo | Già ottimizzato con il salto dei frame: viene eseguita ogni 10 frame |
| L'etichetta lampeggia | — | Già risolto: l'etichetta resta visualizzata e non scompare |
| `fail: unknown command` | Errore di ortografia del comando | Controlla il comando: è `face_eril`, non `face_enroll` |

## Risultato

Registrazione del volto→riconoscimento continuo con visualizzazione dell'ID→risultato in uscita via I2C/porta seriale→controllo di relè/servo: una soluzione completa per il controllo degli accessi.
