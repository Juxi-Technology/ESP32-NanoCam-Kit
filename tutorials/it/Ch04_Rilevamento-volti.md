# Capitolo 4: Rilevamento volti

## Funzionamento

Il rilevamento volti utilizza la libreria di deep learning ESP-DL, basata sul modello di rilevamento leggero MobileNet. Input: immagine RGB565 320x240; output: elenco di bounding box dei volti (posizione + dimensione + confidenza). L'inferenza viene eseguita sull'ESP32-S3, senza bisogno di connessione di rete.

### Formato del risultato di rilevamento

- Coordinate: angolo in alto a sinistra (x,y) + larghezza e altezza (w,h)
- Confidenza: numero in virgola mobile tra 0 e 1
- In presenza di più volti vengono restituiti più riquadri

## Procedura

### 4.1 Cambiare modalità

```
ai_mode:2
```

### 4.2 Osservare il risultato

Nel browser, su `http://<IP>`, vedrai il riquadro di rilevamento del volto.

### 4.3 Ottenere le coordinate

Formato di output della porta seriale:

```
I (xxxxx) detection_result: [ 0]: ( 45,  30, 180, 210)
I (xxxxx) detection_result:       left eye: ( 90,  80), right eye: (150,  80), nose: (120, 120), mouth left: ( 95, 150), mouth right: (145, 150)
```

- Prima riga: `[indice] (x, y, w, h)` — coordinate del riquadro del volto
- Seconda riga: 5 punti chiave — occhio sinistro, occhio destro, naso, angolo sinistro della bocca, angolo destro della bocca

## Codice

### Arduino: leggere le coordinate per controllare un servo

```cpp
// analizza il formato $face:x,y,w,h#
if (nanoSerial.available()) {
    String line = nanoSerial.readStringUntil('\n');
    if (line.startsWith("$face:")) {
        int x = line.substring(6).toInt();
        int y = line.substring(line.indexOf(',')+1).toInt();
        servoX.write(map(x, 0, 320, 0, 180));
    }
}
```

### Lettura con Python

```python
ser = serial.Serial("COM3", 115200)
line = ser.readline().decode()
if line.startswith("$face:"):
    parts = line[6:-1].split(",")
    x, y, w, h = map(int, parts)
```

## Risultato

Appare un volto davanti alla fotocamera→l'immagine viene annotata con un riquadro verde→la porta seriale emette le coordinate.
