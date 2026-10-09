# Chapitre 6 : Reconnaissance des couleurs

## Principe

Basé sur l'espace colorimétrique HSV (teinte-saturation-valeur). L'image RGB565 fournie par la caméra est traitée par le moteur ColorDetector d'esp-dl : l'image est réduite à une résolution 80×80 pour réduire le bruit, puis chaque pixel est converti en valeurs HSV et comparé aux seuils prédéfinis des 7 couleurs.

### Seuils de couleurs prédéfinis (plage H standard OpenCV, échelle 0-180)

| Couleur | Teinte (H) | Saturation (S) | Valeur (V) | Seuil de surface |
|---|---|---|---|---|
| Rouge | 0-15 | 70-255 | 90-255 | 64 |
| Jaune | 23-33 | 70-255 | 90-255 | 64 |
| Vert | 34-75 | 70-255 | 90-255 | 64 |
| Bleu | 97-124 | 70-255 | 90-255 | 64 |
| Violet | 125-155 | 70-255 | 90-255 | 64 |
| Blanc | 0-180 | 0-40 | 200-255 | 80 |
| Noir | 0-180 | 0-255 | 0-50 | 80 |

> La teinte utilise l'échelle OpenCV 0-180 (correspondant à 0-360°). `set_bgr(false)` garantit que la bibliothèque lit les données RGB565 telles quelles, sans échanger les canaux.

## Étapes

### 6.1 Passer en mode couleur

```
ai_mode:3
```

L'appareil redémarre automatiquement en mode détection de couleur ; la LED RGB WS2812 (GPIO18) affiche la couleur actuellement reconnue.

### 6.2 Observer les résultats de reconnaissance

Placez un objet de couleur unie devant la caméra ; ouvrez `http://<IP>` dans le navigateur, vous verrez :
- Un **rectangle coloré** délimitant la zone de couleur détectée
- Une **étiquette de couleur** (red/yellow/green/blue/purple/white/black)
- La couleur du cadre et de l'étiquette correspond à la couleur réellement détectée
> Le mode couleur ne fait qu'une superposition à l'image (OSD), sans journaux sur le port série. Pour obtenir les coordonnées, lisez-les via les registres I2C.

### 6.3 Lire les données de détection en I2C

Le NanoCam agit comme esclave I2C (adresse `0x33`, GPIO SDA=41 SCL=42) et met à jour en temps réel les coordonnées du centre du cadre de détection.

| Registre | Contenu | Type de donnée |
|---|---|---|
| 0x28-0x29 | Centre X | int16 BE |
| 0x2A-0x2B | Centre Y | int16 BE |
| 0x2C-0x2D | ID reconnu | int16 BE |

## Code

### Moteur de détection principal

`components/modules/ai/who_color_detection.cpp` — basé sur esp-dl ColorDetector :

```cpp
// Créer le détecteur ; set_bgr(false) garantit des canaux de couleur corrects
ColorDetector detector;
detector.set_bgr(false);
detector.set_detection_shape({80, 80, 1});
// Enregistrer les seuils des 7 couleurs
detector.register_color({h_lo, h_hi, s_lo, s_hi, v_lo, v_hi}, area_min, "red");
// Détection
auto &results = detector.detect((uint16_t *)frame->buf,
    {(int)frame->height, (int)frame->width, 3});

// Parcourir les résultats pour dessiner cadres + étiquettes
for (int ci = 0; ci < (int)results.size(); ci++) {
    for (int ri = 0; ri < (int)results[ci].size(); ri++) {
        color_detect_result_t &res = results[ci][ri];
        draw_rect(frame, res.box[0], res.box[1], res.box[2], res.box[3], color_lcd);
        fb_gfx_print(frame, lx, ly, color_lcd, color_name);
    }
}
```

## Résultat

Objet rouge, vert ou bleu → reconnaissance de la couleur → cadre + étiquette → sortie des coordonnées en I2C → possibilité de tri par servomoteur.
