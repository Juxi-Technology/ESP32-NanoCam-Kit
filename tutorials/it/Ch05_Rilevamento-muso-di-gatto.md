# Capitolo 5: Rilevamento del muso dei gatti

## Funzionamento

Il rilevamento del muso dei gatti utilizza il modello CatFaceDetectMN03, addestrato e ottimizzato appositamente per le caratteristiche del muso dei gatti (orecchie triangolari / distanza interpupillare ampia / naso). Input: immagine RGB565 320x240; output: elenco di bounding box dei musi. Condivide con il rilevamento volti del capitolo 4 lo stesso formato di output `print_detection_result`.

### Differenze tra il modello per musi di gatto e quello per volti

| Aspetto | Rilevamento volti (ai_mode:2) | Rilevamento muso dei gatti (ai_mode:1) |
|---|---|---|
| Modello | MSR01 + MNP01, doppia cascata | CatFaceDetectMN03, singolo stadio |
| Punti chiave | 10 (occhi / punta del naso / angoli della bocca) | Nessuno (il modello non li produce) |
| Soglia di confidenza | MSR01=0.3, MNP01=0.4 | 0.4 |
| Disegno del riquadro | Rettangolo verde vuoto + 5 punti chiave | Rettangolo verde vuoto (senza punti chiave) |

## Procedura

### 5.1 Cambiare modalità

```
ai_mode:1
```

Il dispositivo si riavvia automaticamente e passa alla modalità di rilevamento del muso dei gatti

### 5.2 Osservare il risultato

Metti un gatto o l'immagine di un gatto davanti alla fotocamera; aprendo `http://<IP>` nel browser vedrai un riquadro verde di rilevamento attorno al muso del gatto.

### 5.3 Output della porta seriale

Quando viene rilevato un muso di gatto, la porta seriale emette:

```
I (xxxxx) detection_result: [ 0]: ( 45,  30, 180, 210)
```

- Formato: `[indice] (x, y, w, h)` — coordinate dell'angolo in alto a sinistra del riquadro del muso + larghezza e altezza
- Il modello per musi di gatto non produce punti chiave (a differenza del rilevamento volti)

## Codice

### Logica di rilevamento principale

`components/modules/ai/who_cat_face_detection.cpp`:

```cpp
CatFaceDetectMN03 detector(0.4F, 0.3F, 10, 0.3F);
std::list<dl::detect::result_t> &detect_results = detector.infer(
    (uint16_t *)frame->buf, {(int)frame->height, (int)frame->width, 3});

if (detect_results.size() > 0) {
    draw_detection_result((uint16_t *)frame->buf, frame->height, frame->width, detect_results);
    print_detection_result(detect_results);  // output seriale delle coordinate
}
```

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

### Lettura dalla porta seriale con Python

```python
import serial
ser = serial.Serial("COM3", 115200)
while True:
    line = ser.readline().decode().strip()
    if "detection_result" in line:
        print(line)
```

## Risultato

Appare un gatto→l'immagine viene annotata con un riquadro verde→la porta seriale emette le coordinate. Le coordinate possono essere lette con Arduino/Python per pilotare un servo all'inseguimento.
