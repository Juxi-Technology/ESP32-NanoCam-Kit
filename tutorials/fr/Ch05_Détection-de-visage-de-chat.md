# Chapitre 5 : Détection de visage de chat

## Principe

La détection de visage de chat utilise le modèle CatFaceDetectMN03, entraîné et optimisé pour les caractéristiques du visage félin (oreilles triangulaires / grand écart interoculaire / museau). Entrée : une image 320x240 RGB565 ; sortie : une liste de boîtes englobantes de visages de chats. Elle partage le même format de sortie `print_detection_result` que la détection de visage du chapitre 4.

### Différences entre les modèles de détection de visage de chat et de visage humain

| Critère | Détection de visage (ai_mode:2) | Détection de visage de chat (ai_mode:1) |
|---|---|---|
| Modèle | Double cascade MSR01 + MNP01 | CatFaceDetectMN03 à un seul étage |
| Points clés | 10 (deux yeux / bout du nez / commissures des lèvres) | Aucun (non fournis par le modèle) |
| Seuil de confiance | MSR01=0.3, MNP01=0.4 | 0.4 |
| Tracé du cadre de détection | Rectangle vert creux + 5 points clés | Rectangle vert creux (sans points clés) |

## Étapes

### 5.1 Changer de mode

```
ai_mode:1
```

L'appareil redémarre automatiquement en mode détection de visage de chat

### 5.2 Observer le résultat

Placez un chat ou une photo de chat devant la caméra ; ouvrez `http://<IP>` dans le navigateur pour voir un cadre de détection vert marquer le visage du chat.

### 5.3 Sortie du port série

Lorsqu'un visage de chat est détecté, le port série affiche :

```
I (xxxxx) detection_result: [ 0]: ( 45,  30, 180, 210)
```

- Format : `[index] (x, y, w, h)` — coordonnées du coin supérieur gauche du cadre de visage de chat + largeur et hauteur
- Le modèle de visage de chat ne fournit pas de points clés (contrairement à la détection de visage humaine)

## Code

### Logique de détection principale

`components/modules/ai/who_cat_face_detection.cpp` :

```cpp
CatFaceDetectMN03 detector(0.4F, 0.3F, 10, 0.3F);
std::list<dl::detect::result_t> &detect_results = detector.infer(
    (uint16_t *)frame->buf, {(int)frame->height, (int)frame->width, 3});

if (detect_results.size() > 0) {
    draw_detection_result((uint16_t *)frame->buf, frame->height, frame->width, detect_results);
    print_detection_result(detect_results);  // Sortie des coordonnées sur le port série
}
```

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

### Lecture du port série en Python

```python
import serial
ser = serial.Serial("COM3", 115200)
while True:
    line = ser.readline().decode().strip()
    if "detection_result" in line:
        print(line)
```

## Résultat

Le chat apparaît → l'image est annotée d'un cadre vert → les coordonnées sont envoyées sur le port série. Elles peuvent être lues en Arduino/Python pour piloter un servomoteur et suivre le chat.
