# ESP32-NanoCam — Manuel du protocole série AT

> Débit : 115200 | Bits de données : 8 | Parité : aucune | Bits d'arrêt : 1 | Contrôle de flux : aucun
> Compatible avec le jeu de commandes AT des principaux modules caméra, avec en plus les commandes étendues NanoCam.

---

## 1. Règles générales

- Les commandes sont **insensibles à la casse** (`STA_SSID` = `sta_ssid`)
- Les commandes doivent être suivies de **n'importe quel signe de ponctuation ASCII** (`,` `.` `:` `;` etc.) comme caractère de fin
- Certaines commandes **redémarrent automatiquement** après modification
- Chaque commande se termine par `\r\n` (généralement ajouté automatiquement par le terminal série)

## 2. Configuration WiFi

### Mode STA (connexion au routeur)

| Commande | Description | Exemple | Valeur de retour |
|---|---|---|---|
| `sta_ssid:nom` | Définit le nom du WiFi | `sta_ssid:MyWiFi` | `OK` |
| `sta_pd:mot_de_passe` | Définit le mot de passe WiFi (redémarrage après modification) | `sta_pd:12345678` | `OK` (redémarrage) |

> Le nom et le mot de passe WiFi ne doivent pas dépasser 30 caractères ; les caractères chinois ne sont pas pris en charge.

### Mode AP (point d'accès autonome)

| Commande | Description | Exemple | Valeur de retour |
|---|---|---|---|
| `ap_ssid:nom` | Définit le nom du point d'accès | `ap_ssid:NanoCam-AP` | `OK` |
| `ap_pd:mot_de_passe` | Définit le mot de passe du point d'accès (redémarrage après modification) | `ap_pd:12345678` | `OK` (redémarrage) |

### Mode WiFi

| Commande | Description | Paramètre | Valeur de retour |
|---|---|---|---|
| `wifi_mode:X` | Change de mode | 0=AP 1=STA 2=AP+STA | `OK` (redémarrage en cas de changement) |

---

## 3. Changement de mode IA

| Commande | Mode | Description | Redémarrage |
|---|---|---|---|
| `ai_mode:0` | Normal | Transmission MJPEG, sans IA | ✅ |
| `ai_mode:1` | Détection de visage de chat | Cadre de visage de chat en temps réel + confiance | ✅ |
| `ai_mode:2` | Détection de visage | Cadre de visage en temps réel + coordonnées | ✅ |
| `ai_mode:3` | Reconnaissance des couleurs | Sélection par cadre → détection en temps réel | ✅ |
| `ai_mode:4` | Reconnaissance faciale | Enregistrer → identifier → supprimer | ✅ |
| `ai_mode:5` | QR code | Décodage en temps réel → sortie sur le port série | ✅ |
| `ai_mode:6` | Agent LLM | Dialogue vocal XiaoZhi AI + vision IA | ✅ |
| `ai_mode:7` | ESP-Claw | Contrôle vocal + analyse visuelle par photo + OpenAI Vision | ✅ |

> Valeurs valides de `ai_mode` : 0-7. Hors plage, la valeur revient à 0 par défaut. Redémarrage automatique après modification ; le nouveau mode prend effet après le redémarrage.

---

## 4. Requêtes d'informations

| Commande | Description | Exemple de valeur de retour |
|---|---|---|
| `sta_ip` | Interroge l'IP STA | `sta_ip:192.168.1.100` |
| `ap_ip` | Interroge l'IP AP | `ap_ip:192.168.4.1` |
| `wifi_ver` | Interroge la version du firmware | `NanoCam Board Ver:0.2.0` |

---

## 5. Contrôle système

| Commande | Description | Valeur de retour |
|---|---|---|
| `wifi_reset` | Restaure les paramètres d'usine (redémarrage) | `Reset_OK` |
| `nano_reboot` | Réinitialisation logicielle | `Rebooting...` |
| `nano_info` | Informations complètes sur l'appareil (JSON) | Voir ci-dessous |

### Exemple de retour de nano_info

```json
{
  "device": "NanoCam",
  "ver": "0.2.0",
  "chip": "ESP32-S3",
  "flash": "16MB",
  "psram": "8MB",
  "ai_mode": 1,
  "wifi_mode": 2,
  "sta_ip": "192.168.1.100",
  "free_heap": 245760
}
```

---

## 6. Commandes dédiées à la reconnaissance faciale

> Valides uniquement en ai_mode:4 (mode reconnaissance faciale).

| Commande | Description | Comportement de l'étiquette | Exemple de retour |
|---|---|---|---|
| `face_eril` | Enregistre le visage détecté dans l'image actuelle | Bleue "Enroll: ID N", flash 0.5s | `>>> face enroll triggered` |
| `face_rz` | Entre en mode reconnaissance faciale continue | Verte "ID: N" / rouge "who?", **affichage persistant sans disparition** | `>>> face recognize triggered` |
| `face_del` | Supprime le dernier ID de visage enregistré | Rouge "N IDs left", flash 0.5s | `>>> face delete triggered` |
| `face_detect` | Quitte le mode reconnaissance, retour à la détection de visage seule | Efface toutes les étiquettes | `>>> face detect mode` |

### Procédure d'utilisation de la reconnaissance faciale

```
ai_mode:4          # Entrer en mode reconnaissance faciale (redémarrage automatique de l'appareil)
face_eril          # Enregistrer un visage (s'assurer qu'un seul visage est dans l'image)
face_rz            # Démarrer la reconnaissance continue — l'étiquette reste affichée sans disparaître
face_detect        # Quitter le mode reconnaissance — effacement des étiquettes
face_del           # Supprimer le dernier visage enregistré
```

### Points d'attention pour la reconnaissance faciale

1. Lors de l'enregistrement, s'assurer qu'**un seul visage** est présent à l'image, à une distance de 30-50cm
2. En mode reconnaissance (`face_rz`), l'étiquette **reste affichée** et ne disparaît pas au bout de 0.5 seconde — c'est le nouveau comportement de la version 0.3.0
3. Pour quitter le mode reconnaissance, envoyer `face_detect`, sinon l'étiquette reste affichée en permanence
4. Les caractéristiques faciales sont stockées dans la partition Flash `fr`, conservées hors tension, jusqu'à 47 ID maximum
5. La reconnaissance utilise une stratégie de saut de trames (inférence MFN une fois toutes les 10 trames)

---

## 7. Commandes étendues (exclusives NanoCam)

| Commande | Description | État |
|---|---|---|
| `nano_server:url` | Définit l'adresse du serveur LLM (sauvegardée en NVS) | ✅ |
| `nano_api_key:key` | Définit la clé API LLM (sauvegardée en NVS) | ✅ |
| `nano_mqtt:broker,port,topic` | Configure le serveur MQTT | 🔨 |
| `nano_led:R,G,B` | Définit la LED RGB (WS2812, GPIO18 DIN) | 📋 |
| `nano_snap` | Prise de photo et stockage (SPIFFS) | ✅ |
| `nano_stream:on/off` | Démarre/arrête la transmission d'images | 📋 |

### nano_server / nano_api_key

| Commande | Description | Exemple | Valeur de retour |
|---|---|---|---|
| `nano_server:URL` | Définit l'adresse du serveur LLM | `nano_server:https://api.openai.com` | `OK server=https://api.openai.com` |
| `nano_api_key:KEY` | Définit la clé API | `nano_api_key:sk-xxxx` | `OK` |

> Prend en charge toute API compatible OpenAI (vLLM / Ollama / modèles locaux).
Le mode ESP-Claw (ai_mode:7) prend en charge `nano_server` ; XiaoZhi AI (ai_mode:6) utilise une configuration serveur dédiée.

---

## 8. Remarques

1. `sta_pd` / `ap_pd` redémarrent automatiquement après modification ; le nouveau mot de passe prend effet après redémarrage
2. `ai_mode` redémarre automatiquement après modification (uniquement si le mode change)
3. En mode reconnaissance faciale (mode 4), la configuration via le port série Type-C peut ne plus fonctionner (mémoire insuffisante)
4. Le nom / mot de passe WiFi ne doit pas dépasser 30 caractères et ne doit pas contenir de caractères chinois
5. Les commandes doivent être suivies d'un signe de ponctuation comme caractère de fin
