# Chapitre 4 : Détection de visage

## Principe

La détection de visage utilise la bibliothèque d'apprentissage profond ESP-DL, basée sur le modèle de détection léger MobileNet. Entrée : une image 320x240 RGB565 ; sortie : une liste de boîtes englobantes de visages (position + taille + confiance). L'inférence s'exécute sur l'ESP32-S3, sans connexion réseau.

### Format des résultats de détection

- Coordonnées : coin supérieur gauche (x,y) + largeur et hauteur (w,h)
- Confiance : nombre flottant entre 0 et 1
- En présence de plusieurs visages, plusieurs cadres sont renvoyés

## Étapes

### 4.1 Changer de mode

```
ai_mode:2
```

### 4.2 Observer le résultat

Dans le navigateur, `http://<IP>` affiche le cadre de détection de visage.

### 4.3 Obtenir les coordonnées

Format de sortie du port série :

```
I (xxxxx) detection_result: [ 0]: ( 45,  30, 180, 210)
I (xxxxx) detection_result:       left eye: ( 90,  80), right eye: (150,  80), nose: (120, 120), mouth left: ( 95, 150), mouth right: (145, 150)
```

- Première ligne : `[index] (x, y, w, h)` — coordonnées du cadre du visage
- Deuxième ligne : 5 points clés — œil gauche, œil droit, nez, commissure gauche des lèvres, commissure droite des lèvres

## Code

### Arduino : lire les coordonnées et piloter un servomoteur

```cpp
// Analyser le format $face:x,y,w,h#
if (nanoSerial.available()) {
    String line = nanoSerial.readStringUntil('\n');
    if (line.startsWith("$face:")) {
        int x = line.substring(6).toInt();
        int y = line.substring(line.indexOf(',')+1).toInt();
        servoX.write(map(x, 0, 320, 0, 180));
    }
}
```

### Lecture en Python

```python
ser = serial.Serial("COM3", 115200)
line = ser.readline().decode()
if line.startswith("$face:"):
    parts = line[6:-1].split(",")
    x, y, w, h = map(int, parts)
```

## Résultat

Un visage apparaît devant la caméra → l'image est annotée d'un cadre vert → les coordonnées sont envoyées sur le port série.
