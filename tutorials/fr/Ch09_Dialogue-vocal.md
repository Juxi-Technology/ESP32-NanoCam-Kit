# Chapitre 9 : Dialogue vocal (XiaoZhi AI)

## À propos de ce chapitre

Ce chapitre concerne le mode XiaoZhi AI (`ai_mode:6`). **Important** : les modes 6 (dialogue vocal) et 7 (ESP-Claw) **partagent le même firmware** (`nanocam_espclaw/`) ; au démarrage, l'appareil charge simplement un ensemble d'outils MCP différent selon la valeur `ai_mode` stockée en NVS.

| Critère | XiaoZhi AI (mode 6) | ESP-Claw (mode 7) |
|---|---|---|
| Dialogue vocal | ✅ ASR→LLM→TTS | ✅ même pipeline vocal |
| Outils MCP | Outils génériques (volume / photo, etc.) | **Outils génériques + 5 outils matériels dédiés** |
| Compréhension visuelle | `self.camera.take_photo` | **`self.camera.inspect_image`** (vision multimodale) |
| Contrôle LED | ❌ | ✅ couleur réglée à la voix |
| Cas d'usage | Dialogue IA général, éducation pour enfants | Contrôle matériel, inspection visuelle, domotique |

> Ce chapitre se concentre sur le dialogue vocal du **XiaoZhi AI (mode 6)**. Pour découvrir les capacités de contrôle matériel d'ESP-Claw, lire le Chapitre 11 : Contrôle vocal ESP-Claw.

## Principe

NanoCam intègre le framework open source XiaoZhi AI et se connecte au serveur LLM via les protocoles WebSocket / MQTT pour réaliser une pipeline d'interaction vocale complète :

```
L'utilisateur parle → capture par le microphone ES8311 → encodage Opus
  → WebSocket → ASR cloud (reconnaissance vocale)
  → Le grand modèle LLM génère la réponse
  → Synthèse vocale TTS → décodage Opus
  → Amplificateur NS4150B → lecture sur le haut-parleur
```

Conception full-duplex : l'utilisateur peut interrompre l'IA pendant qu'elle parle (barge-in), pour une expérience proche d'une conversation humaine.

## Matériel requis

Ce chapitre fait appel aux fonctions audio et nécessite le matériel suivant :
- Carte principale NanoCam (avec codec ES8311 + microphone AP2718AT)
- Carte de base NanoCam (avec amplificateur NS4150B + CH340K)
- Haut-parleur (à relier au connecteur haut-parleur de la carte de base, VON/VOP)
> La carte principale seule permet aussi de tester l'audio (écoute via la sortie casque de l'ES8311). Le microphone est un MEMS analogique AP2718AT, relié à MIC1P de l'ES8311 via le condensateur de liaison C26.

## Étapes

### 10.1 Flasher le firmware XiaoZhi AI

XiaoZhi AI utilise le projet de firmware indépendant `nanocam_espclaw/` :

```bash
cd nanocam_espclaw
idf.py set-target esp32s3
idf.py build
idf.py -p COMx flash monitor
```

Le mode XiaoZhi AI est actif par défaut au démarrage.

### 10.2 Se connecter au service cloud xiaozhi.me

Le NanoCam se connecte par défaut au service cloud officiel [xiaozhi.me](https://xiaozhi.me) (gratuit) ; aucun serveur auto-hébergé n'est nécessaire.
1. Créer un compte sur [xiaozhi.me](https://xiaozhi.me)
2. À la mise sous tension, l'appareil annonce automatiquement un code d'activation à 6 chiffres
3. Saisir le code d'activation dans la console xiaozhi.me → associer l'appareil
4. Choisir le modèle LLM dans la console (Qwen / DeepSeek, etc.)
L'activation n'est nécessaire qu'une seule fois ; ensuite, l'appareil se reconnecte automatiquement à chaque mise sous tension.

### 10.3 Premier dialogue

Après le signal sonore, vous pouvez dialoguer :

```
Vous : « 你好小智, quel temps fait-il aujourd'hui ? »
NanoCam : « Je vais regarder la météo pour toi... »
```

Le mot d'éveil est **« 你好小智 »** (par défaut).

### 10.4 Scénarios de dialogue courants

```
💬 « Raconte une blague »                 → réponse vocale de l'IA
💬 « Mets-moi un réveil dans 5 minutes »  → fonction réveil
💬 « Quelle heure est-il »                → annonce de l'heure
💬 « Joue une musique douce »             → lecture de musique en ligne
💬 « C'est quoi un trou noir »            → questions de connaissances
```

## Serveur auto-hébergé (facultatif)

Si vous avez des exigences de confidentialité, ou si vous souhaitez utiliser votre propre LLM, vous pouvez déployer le serveur open source XiaoZhi AI :

```bash
git clone https://github.com/xinnan-tech/xiaozhi-esp32-server
cd xiaozhi-esp32-server
pip install -r requirements.txt
python app.py
```

L'adresse du serveur du firmware est fournie via le système OTA (`CONFIG_OTA_URL` dans sdkconfig) ; l'appareil demande automatiquement cette adresse à la mise sous tension.
> XiaoZhi AI utilise le serveur open source XiaoZhi AI (protocole privé WebSocket + pipeline ASR/LLM/TTS). En mode ESP-Claw, en complément, la fonction d'analyse visuelle : le serveur fournit l'URL et le token de la Vision API au firmware lors du handshake MCP, qui n'a rien à configurer lui-même.

## Dépannage

| Symptôme | Cause possible | Solution |
|---|---|---|
| Aucun son | Haut-parleur non connecté | Vérifier le connecteur haut-parleur de la carte de base |
| Reconnaissance vocale imprécise | Bruit ambiant trop fort | Parler près du microphone (distance < 1m) |
| Connexion impossible | WiFi non configuré | Configurer d'abord le réseau via le port série `sta_ssid:xxx` |
| Pas de code d'activation | Premier démarrage non terminé | Attendre 30 secondes ; l'appareil annonce automatiquement le code |
| Réponses très lentes | Latence du serveur LLM | Choisir un modèle plus rapide sur xiaozhi.me, ou auto-héberger le serveur |
