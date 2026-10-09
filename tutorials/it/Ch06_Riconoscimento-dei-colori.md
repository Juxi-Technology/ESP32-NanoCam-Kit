# Capitolo 6: Riconoscimento dei colori

## Funzionamento

Basato sullo spazio colore HSV (tonalità-saturazione-valore). L'immagine RGB565 emessa dalla fotocamera viene elaborata dal motore ColorDetector di esp-dl: dopo aver ridotto l'immagine a 80×80 di risoluzione per attenuare il rumore, ogni pixel viene convertito in valore HSV e confrontato con le soglie predefinite di 7 colori.

### Soglie di colore predefinite (intervallo H standard di OpenCV, scala 0-180)

| Colore | Tonalità (H) | Saturazione (S) | Valore (V) | Soglia di area |
|---|---|---|---|---|
| Rosso | 0-15 | 70-255 | 90-255 | 64 |
| Giallo | 23-33 | 70-255 | 90-255 | 64 |
| Verde | 34-75 | 70-255 | 90-255 | 64 |
| Blu | 97-124 | 70-255 | 90-255 | 64 |
| Viola | 125-155 | 70-255 | 90-255 | 64 |
| Bianco | 0-180 | 0-40 | 200-255 | 80 |
| Nero | 0-180 | 0-255 | 0-50 | 80 |

> La tonalità utilizza la scala 0-180 di OpenCV (corrispondente a 0-360°). `set_bgr(false)` garantisce che la libreria legga i dati RGB565 così come sono, senza scambiare i canali.

## Procedura

### 6.1 Entrare in modalità colore

```
ai_mode:3
```

Il dispositivo si riavvia automaticamente e passa alla modalità di rilevamento dei colori; il LED RGB WS2812 (GPIO18) mostra il colore attualmente riconosciuto.

### 6.2 Osservare il risultato del riconoscimento

Metti un oggetto di colore uniforme davanti alla fotocamera; aprendo `http://<IP>` nel browser vedrai:
- Un **riquadro colorato** che delimita l'area del colore rilevato
- **Un'etichetta testuale del colore** (red/yellow/green/blue/purple/white/black)
- Il colore del riquadro e dell'etichetta corrisponde a quello effettivamente rilevato
> La modalità colore esegue solo la sovrapposizione sull'immagine (OSD) e non produce log sulla porta seriale. Per ottenere le coordinate, leggile tramite i registri I2C.

### 6.3 Lettura dei dati di rilevamento via I2C

NanoCam opera come slave I2C (indirizzo `0x33`, GPIO SDA=41 SCL=42) e aggiorna in tempo reale le coordinate del centro del riquadro rilevato.

| Registro | Contenuto | Tipo di dato |
|---|---|---|
| 0x28-0x29 | Centro X | int16 BE |
| 0x2A-0x2B | Centro Y | int16 BE |
| 0x2C-0x2D | ID riconosciuto | int16 BE |

## Codice

### Motore di rilevamento principale

`components/modules/ai/who_color_detection.cpp` — basato su esp-dl ColorDetector:

```cpp
// costruisce il detector, set_bgr(false) garantisce i canali colore corretti
ColorDetector detector;
detector.set_bgr(false);
detector.set_detection_shape({80, 80, 1});
// registra le soglie dei 7 colori
detector.register_color({h_lo, h_hi, s_lo, s_hi, v_lo, v_hi}, area_min, "red");
// rilevamento
auto &results = detector.detect((uint16_t *)frame->buf,
    {(int)frame->height, (int)frame->width, 3});

// scorre i risultati disegnando riquadro + etichetta
for (int ci = 0; ci < (int)results.size(); ci++) {
    for (int ri = 0; ri < (int)results[ci].size(); ri++) {
        color_detect_result_t &res = results[ci][ri];
        draw_rect(frame, res.box[0], res.box[1], res.box[2], res.box[3], color_lcd);
        fb_gfx_print(frame, lx, ly, color_lcd, color_name);
    }
}
```

## Risultato

Oggetto rosso/verde/blu→riconoscimento del colore→riquadro + etichetta→coordinate in uscita via I2C→possibile collegare un servo per lo smistamento.
