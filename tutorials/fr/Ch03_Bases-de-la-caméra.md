# Chapitre 3 : Bases de la caméra

## Principe

Le NanoCam connecte la caméra via une interface DVP (vidéo numérique parallèle). Le capteur GC2145 émet des données de pixels parallèles 8 bits ; le périphérique LCD_CAM de l'ESP32-S3 les stocke directement en PSRAM par DMA, puis le serveur HTTP les diffuse au navigateur au format MJPEG.

### Concepts clés

- **DVP** : 8 lignes de données parallèles + 3 lignes de synchronisation (VSYNC/HREF/PCLK)
- **MJPEG** : chaque trame est une image JPEG indépendante ; le navigateur les charge en continu pour produire un effet vidéo
- **PSRAM** : 8MB de PSRAM servent de tampon de trames, avec une capacité de 2 à 4 trames

## Étapes

### 3.1 Afficher l'image par défaut

Après le flashage du firmware, le mode flux est actif par défaut ; ouvrez `http://<IP>` dans le navigateur pour voir l'image.

| Commande | Fonction |
|---|---|
| ai_mode:0\r | Transmission d'images |
| ai_mode:1\r | Détection de visage de chat |
| ai_mode:2\r | Détection de visage |
| ai_mode:3\r | Reconnaissance des couleurs |
| ai_mode:4\r | Reconnaissance faciale |
| ai_mode:5\r | Lecture de QR codes |
