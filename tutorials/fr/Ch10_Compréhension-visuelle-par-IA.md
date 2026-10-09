# Chapitre 10 : Compréhension visuelle par IA

## À propos de ce chapitre

La compréhension visuelle par IA est une fonctionnalité exclusive d'**ESP-Claw (mode 7)** ; elle n'est pas utilisée dans XiaoZhi AI (mode 6).
> Les outils `self.camera.take_photo` et `self.camera.inspect_image` utilisés dans ce chapitre : l'adresse de l'API d'analyse visuelle est fournie automatiquement par le serveur lors du handshake MCP via le champ `capabilities.vision`. Aucune URL d'API n'est à configurer manuellement côté firmware — la configuration de l'API se fait dans la console xiaozhi.me ou sur le serveur auto-hébergé ; voir pour cela le Chapitre 11 : Contrôle vocal ESP-Claw.

## Principe

Déroulement complet de l'analyse visuelle :

```
Commande vocale « regarde ce qu'il y a sur la table »
  → ASR (reconnaissance vocale)
  → Décision du LLM : une analyse par photo est nécessaire → appel de self.camera.take_photo ou self.camera.inspect_image
  → Firmware : esp_camera_fb_get() capture la trame (VGA RGB565)
  → Compression JPEG
  → Envoi via Explain() vers la Vision API fournie par le serveur
  → Le LLM multimodal renvoie une description textuelle
  → Annonce vocale TTS
```

### Différence entre les deux outils de photo

| Outil | Utilisation | Qui émet la Vision API |
|---|---|---|
| `self.camera.take_photo` | Prendre une photo puis la décrire via la capacité de vision intégrée du LLM | Serveur |
| `self.camera.inspect_image` (spécifique NanoCam) | Prendre une photo puis appeler `camera->Explain()` → HTTP POST vers une API multimodale indépendante | Firmware |

La différence entre les deux : `take_photo` passe par la vision LLM du serveur XiaoZhi (implémentation générique), tandis que `inspect_image` est l'implémentation dédiée de ce projet — le firmware appelle directement une API multimodale indépendante (dont l'adresse est fournie par le serveur).

## Étapes

### 11.1 S'assurer du mode ESP-Claw

```
ai_mode:7
```

Après redémarrage, l'appareil entre en mode ESP-Claw.

### 11.2 Photo + analyse IA

Après l'éveil, posez directement votre question :

```
💬 « Regarde ce qu'il y a ici »
💬 « Y a-t-il une tasse devant moi »
💬 « De quelle couleur est ce livre »
💬 « Combien de pommes y a-t-il sur la table »
💬 « Regarde ce qui est écrit sur cette feuille »
```

Le NanoCam prend une photo, l'envoie, l'analyse, puis répond vocalement.

### 11.3 Exemples de reconnaissance de scène

| Entrée vocale | Exemple de réponse de l'IA |
|---|---|
| « Qu'est-ce que c'est ? » | « C'est un ordinateur portable noir, avec une tasse de café blanche à côté » |
| « Y a-t-il des pommes ? » | « Je ne vois pas de pommes. Il y a deux livres et un stylo sur la table » |
| « Quelle couleur ? » | « Ce que tu montres est une tasse rouge » |
| « Combien de tasses ? » | « Il y a 2 tasses à l'image » |

## Code

### Callback principal de prise de vue + analyse

`nanocam_espclaw/main/boards/nanocam/nanocam_board.cc` — enregistrement de l'outil MCP :

```cpp
mcp.AddTool("self.camera.inspect_image",
    "Take a photo with the camera and send it to the vision AI for analysis.",
    PropertyList({ Property("prompt", kPropertyTypeString) }),
    [this](const PropertyList &props) -> ReturnValue {
        auto camera = GetCamera();
        if (!camera->Capture()) {
            return std::string("{\"error\":\"Camera capture failed\"}");
        }
        std::string prompt = props["prompt"].value<std::string>();
        return camera->Explain(prompt);
    });
```

`nanocam_espclaw/main/boards/common/esp32_camera.cc` — implémentation de Explain() :

```cpp
std::string Esp32Camera::Explain(const std::string &question) {
    // explain_url_ est fourni par le serveur lors du handshake MCP via capabilities.vision.url
    // capture de trame → compression JPEG → HTTP POST vers l'API multimodale
    // retourne le résultat d'analyse du LLM
}
```

### Handshake MCP côté serveur (fourniture de la Vision API)

```json
{
  "capabilities": {
    "vision": {
      "url": "https://api.openai.com/v1/chat/completions",
      "token": "sk-..."
    }
  }
}
```

À réception, le firmware appelle `camera->SetExplainUrl(url, token)` pour sauvegarder l'adresse de l'API, utilisée directement lors des appels `inspect_image` suivants.

## Modèles multimodaux pris en charge

En fournissant différentes `vision.url` côté serveur, vous pouvez utiliser n'importe quelle API compatible OpenAI :

| Modèle | Exemple d'adresse API | Cas d'usage |
|---|---|---|
| `gpt-4o` | `https://api.openai.com/v1/chat/completions` | Meilleures capacités générales |
| `gpt-4o-mini` | `https://api.openai.com/v1/chat/completions` | Excellent rapport qualité-prix |
| `qwen-vl-max` | `https://dashscope.aliyuncs.com/compatible-mode/v1/chat/completions` | Meilleure compréhension du chinois |
| `llava:13b` (Ollama) | `http://localhost:11434/v1/chat/completions` | Entièrement hors ligne |
| `claude-fable-5` | Nécessite la configuration d'un proxy | Descriptions de scène détaillées |

## Résultat

« Regarde ce qu'il y a ici » → photo et envoi → analyse de l'IA → annonce vocale « I see a red cup on a wooden table » — de véritables yeux pour l'IA.
