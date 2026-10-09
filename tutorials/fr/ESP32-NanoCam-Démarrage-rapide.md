# ESP32-NanoCam — Guide de démarrage rapide

---

## Préparation

- Carte principale NanoCam + carte de base (ESP32-S3 N16R8 + CH340K)
- Câble USB Type-C (prise en charge du transfert de données)
- Ordinateur (Windows / Mac / Linux)
- Module caméra GC2145 (connecté en usine)

---

## Étape 1 : flasher le firmware (3 minutes)

### Méthode A : sans environnement de développement (recommandée)

1. Ouvrir le navigateur et accéder à [esptool-js](https://espressif.github.io/esptool-js/)
2. Relier le NanoCam à l'ordinateur avec un câble Type-C
3. Sélectionner le port série, débit 115200
4. Repérer le fichier firmware `nanocam_xxx.bin` dans l'archive décompressée
5. Sélectionner le fichier firmware `nanocam_xxx.bin`, adresse `0x0`
6. Cliquer sur « START », puis attendre la fin

### Méthode B : ligne de commande (avancé)

```bash
pip install esptool
esptool.py --chip esp32s3 --port COM3 write_flash 0x0 nanocam.bin
```

---

## Étape 2 : connexion WiFi (2 minutes)

Le NanoCam fonctionne par défaut en **double mode AP+STA simultané**, sans commutation :
- Le **point d'accès AP** reste toujours actif : connectez directement le téléphone à `NanoCam-AP` (mot de passe `12345678`), puis ouvrez `http://192.168.4.1` dans le navigateur
- La **connexion STA au routeur** nécessite une configuration WiFi unique :
Avec un outil de port série (débit **115200 8N1**), connectez-vous au port Type-C du NanoCam :

```
sta_ssid:votre_nom_WiFi
sta_pd:votre_mot_de_passe_WiFi
```

> Réception de `OK` = configuration réussie. L'appareil redémarre automatiquement après modification du mot de passe.
Pour changer de mode WiFi (généralement inutile) :

| Commande | Mode | Description |
|---|---|---|
| `wifi_mode:0` | AP seul | Désactive le STA, seul le point d'accès reste actif |
| `wifi_mode:1` | STA seul | Désactive le point d'accès, connexion au routeur uniquement |
| `wifi_mode:2` | AP+STA | Par défaut, les deux fonctionnent simultanément |

---

## Étape 3 : afficher l'image (1 minute)

1. Envoyer `sta_ip` sur le port série pour obtenir l'IP STA
2. Saisir `http://<adresse IP>` dans le navigateur (ou `http://192.168.4.1` en mode AP)
3. La page web affiche l'image en temps réel

---

## Étape 4 : maîtriser l'IA (2 minutes)

Envoyez les commandes suivantes sur le port série pour changer de mode :

| Commande | Mode | Effet |
|---|---|---|
| `ai_mode:0` | Transmission standard | Image MJPEG en temps réel |
| `ai_mode:1` | Détection de visage de chat | Un cadre de détection de visage de chat apparaît à l'image |
| `ai_mode:2` | Détection de visage | Un cadre de détection de visage apparaît à l'image |
| `ai_mode:3` | Reconnaissance des couleurs | Sélection par cadre → suivi en temps réel |
| `ai_mode:4` | Reconnaissance faciale | Enregistrer → identifier → supprimer |
| `ai_mode:5` | Scan de QR codes | Viser un QR code → sortie du contenu sur le port série |
| `ai_mode:6` | Agent LLM | Réveil vocal « 你好小智 » (XiaoZhi AI) |
| `ai_mode:7` | ESP-Claw | Agent IA ESP-Claw (framework officiel Espressif) |

> Chaque changement de mode nécessite un redémarrage manuel : vous pouvez redémarrer en appuyant sur le bouton RST du module ; le nouveau mode prend effet après le redémarrage.

---

## Étape 5 : intégrer dans votre projet

### Contrôle Arduino

```cpp
Serial.begin(115200);
Serial.print("ai_mode:2");  // basculer en détection de visage
```

### Contrôle Python

```python
import serial
ser = serial.Serial("COM3", 115200)
ser.write(b"ai_mode:1\r\n")  # basculer en détection de visage de chat
```

### Voir toutes les commandes

→ Manuel du protocole série AT

---

## Questions fréquentes

| Problème | Solution |
|---|---|
| Échec du flashage | Vérifier que le câble Type-C prend en charge les données ; maintenir S2 (BOOT) de la carte de base enfoncé, puis remettre sous tension |
| Aucune image | Envoyer `sta_ip` sur le port série pour confirmer l'IP ; vérifier que les appareils sont sur le même sous-réseau |
| Caméra inactive | Vérifier que la nappe FPC est insérée fermement, contacts métalliques vers le bas ; contrôler PWDN (IO12) / RESET (IO14) |
| WiFi inaccessible | Envoyer `wifi_reset` pour restaurer les paramètres d'usine, puis reconfigurer |

Plus de questions → [FAQ](https://FAQ.md)

---

## Étapes suivantes

- 📖 Manuel du protocole série — référence complète des commandes AT
- 🎓 Plan du tutoriel — tutoriel progressif en 28 chapitres
- 🔧 Spécifications matérielles — mappage complet des broches GPIO
- 🤖 Guide d'intégration ROS2 — tutoriel micro-ROS
