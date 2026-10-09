# Chapitre 11 : Contrôle vocal ESP-Claw

## À propos de ce chapitre

Lorsque l'appareil bascule en `ai_mode:7`, NanoCam entre en mode ESP-Claw. Il **partage le même firmware** (`nanocam_espclaw/`) que XiaoZhi AI (`ai_mode:6`) ; la seule différence : en plus du dialogue vocal, le mode ESP-Claw enregistre 5 outils de contrôle matériel supplémentaires.

| Critère | XiaoZhi AI (mode 6) | ESP-Claw (mode 7) |
|---|---|---|
| Dialogue vocal | ✅ ASR→LLM→TTS | ✅ même pipeline vocal |
| Contrôle LED | ❌ | ✅ couleur / marche-arrêt à la voix |
| Changement de mode IA | ❌ | ✅ changement à la voix |
| Photo + analyse visuelle IA | ❌ | ✅ prise de vue + compréhension multimodale de l'image |

## Principe

Au-dessus de la pipeline vocale, le mode ESP-Claw enregistre 5 outils dédiés au NanoCam via `RegisterMcpTools()` :

```
Commande vocale « mets la lampe en bleu »
  → ASR (reconnaissance vocale, cloud)
  → Le LLM comprend l'intention → appel de self.led.set_color({"r":0, "g":0, "b":255})
  → Le LED WS2812 du NanoCam devient bleu
  → TTS : « D'accord, la lampe est réglée en bleu »
```

## Étapes

### 12.1 Flasher le firmware

ESP-Claw utilise le projet de firmware indépendant `nanocam_espclaw/` :

```bash
cd nanocam_espclaw
idf.py set-target esp32s3
idf.py build
idf.py -p COMx flash
```

### 12.2 Changer de mode

Après le démarrage, réglez le mode ESP-Claw :

```
ai_mode:7
```

L'appareil redémarre automatiquement dans ce mode. La commande `ai_mode:6` permet de revenir au mode XiaoZhi AI.

### 12.3 Exemples de contrôle vocal

Après l'éveil, énoncez directement votre besoin :

```
💬 « Allume la lampe »                    → le WS2812 s'allume en blanc
💬 « Mets la lampe en bleu »              → le LED devient bleu
💬 « Éteins la lampe »                    → le LED s'éteint
💬 « Passe en mode détection de visage »  → sauvegarde NVS ai_mode:2 + redémarrage
💬 « Regarde ce qu'il y a ici »           → photo + envoi à l'analyse IA multimodale
💬 « Y a-t-il une tasse devant moi »      → reconnaissance multimodale de l'image
```

### 12.4 Photo + analyse visuelle IA

Lorsque l'utilisateur dit « regarde... », le firmware capture une trame VGA RGB565, la compresse en JPEG et l'envoie à l'API multimodale configurée côté serveur pour analyse ; le résultat est annoncé vocalement via TTS.
> L'URL et le token de l'API multimodale sont fournis automatiquement par le serveur lors du handshake de connexion ; aucune commande de configuration manuelle sur le port série n'est nécessaire.

## Les 5 outils dédiés au NanoCam

| Outil | Fonction | Paramètres |
|---|---|---|
| `self.led.set_color` | Définir la couleur de la LED RGB WS2812 (GPIO18) | `r,g,b`: 0-255 |
| `self.led.turn_off` | Éteindre la LED | Aucun |
| `self.camera.set_ai_mode` | Changer de mode IA (sauvegarde NVS + redémarrage) | `mode`: 0-7 |
| `self.camera.inspect_image` | Photo + analyse visuelle par LLM multimodal | `prompt`: description de la question |
| `self.get_device_info` | Informations sur l'appareil en JSON | Aucun |

## Fichiers de configuration

| Contenu | Chemin |
|---|---|
| Enregistrement des outils MCP | `nanocam_espclaw/main/boards/nanocam/nanocam_board.cc` |
| Logique d'envoi vers la Vision API | `nanocam_espclaw/main/boards/common/esp32_camera.cc` |
| Configuration SDK par défaut | `nanocam_espclaw/sdkconfig.defaults` |

> Le firmware ESP-Claw est un projet indépendant, qui ne partage aucun code avec `nanocam_vision`. Les deux firmwares doivent être compilés et flashés séparément.

## Comment choisir

| Votre besoin | Mode recommandé |
|---|---|
| Discuter et poser des questions à la voix uniquement | mode 6 (XiaoZhi) |
| Contrôler la LED à la voix | mode 7 (ESP-Claw) |
| Photographier + « faire voir » l'image à l'IA | mode 7 (ESP-Claw) |
| Changer de mode de détection IA à la voix | mode 7 (ESP-Claw) |

> L'utilisation complète d'ESP-Claw (configuration du serveur, développement d'outils MCP personnalisés, etc.) est encore en cours d'exploration ; la documentation sera mise à jour au fil des recherches.
