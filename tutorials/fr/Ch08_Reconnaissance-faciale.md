# Chapitre 8 : Reconnaissance faciale

## Principe

La reconnaissance faciale = **détection de visage** (pipeline à deux étages MSR01+MNP01) + **extraction de caractéristiques** (réseau de neurones MFN FaceRecognition112V1S8) + **comparaison par similarité cosinus**.

```
Trame RGB565 de la caméra
  → Détection grossière MSR01 (320×240, seuil 0.3F)
  → Détection fine MNP01 (sur les candidats grossiers, seuil 0.4F)
  → Extraction de 10 points clés du visage (yeux / nez / commissures des lèvres)
  → Alignement sur les points clés → recadrage du visage 112×112
  → Réseau convolutif MFN → vecteur de caractéristiques 512 dimensions
  → Normalisation L2
  → Calcul de la distance cosinus avec chaque vecteur d'ID enregistré en Flash
  → Similarité cosinus maximale > seuil (0.55) → correspondance → sortie de l'ID
  → Toutes les similarités < seuil → personne inconnue → sortie « who? »
```

### Optimisation des performances

L'extraction de caractéristiques MFN et la comparaison sur l'ensemble de la base sont coûteuses ; les exécuter à chaque trame ferait saccader l'image. L'implémentation actuelle adopte une **stratégie de saut de trames** : la détection de visage s'exécute à chaque trame (peu coûteuse), la reconnaissance MFN une fois toutes les 10 trames (coûteuse), et l'étiquette affiche en continu le dernier résultat de reconnaissance. L'image reste ainsi fluide et l'étiquette d'ID ne clignote pas.

### Stockage des caractéristiques faciales

Les caractéristiques faciales enregistrées (id + embedding 512 dimensions) sont stockées de façon persistante dans la partition `fr` de la Flash (96 KB, jusqu'à 47 identifiants faciaux). Elles survivent à une coupure d'alimentation.

## Matériel requis

- Carte principale NanoCam + carte de base
- Câble USB-C (alimentation + port série vers le PC)
- Terminal série (débit 115200)

## Étapes

### 8.1 Entrer en mode reconnaissance faciale

```
ai_mode:4
```

L'appareil redémarre automatiquement en mode FaceID ; la LED RGB WS2812 (GPIO18 DIN, alimentation VDD50) s'allume en violet. Après le redémarrage, le port série doit afficher :

```
I (5526) MFN: fr partition size: 98304 bytes, maxminum 47 IDs can be stored
I (5526) MFN: No face ID in flash
```

`No face ID in flash` signifie qu'aucun visage n'a encore été enregistré ; c'est normal.

### 8.2 Enregistrer un visage

Placez le visage bien face à la caméra (distance 30-50cm, éclairage uniforme) et assurez-vous qu'**un seul visage** apparaît à l'image. Envoyez sur le port série :

```
face_eril
```

L'appareil détecte le visage, en extrait automatiquement les caractéristiques et l'enregistre en Flash :

```
I (xxxx) ENROLL: ID 1 is enrolled
```

L'image superpose le texte bleu `Enroll: ID 1`, qui disparaît après environ 0.5 seconde.
> **Attention** : la commande est `face_eril` (abréviation d'enroll), pas `face_enroll`. Si vous voyez `fail: unknown command`, vérifiez l'orthographe.

### 8.3 Reconnaître un visage

Une fois l'enregistrement terminé, envoyez la commande de reconnaissance :

```
face_rz
```

Le système passe en mode reconnaissance continue. Le visage présent est comparé à tous les identifiants enregistrés en Flash :
- **Correspondance** : le port série affiche `Similarity: 0.85, Match ID: 1`, l'image superpose en continu `ID: 1` en vert
- **Personne inconnue** : le port série affiche `Similarity: 0.32, Match ID: 0`, l'image superpose en continu `who?` en rouge
> L'étiquette **reste affichée** et ne disparaît pas. Pour quitter le mode reconnaissance, envoyez `face_detect` pour revenir en mode détection pure.

### 8.4 Supprimer un visage

```
face_del
```

Supprime le dernier identifiant facial enregistré ; le port série renvoie `N IDs left` et l'image affiche brièvement le nombre d'identifiants restants. Les caractéristiques en Flash sont supprimées en même temps.

### 8.5 Quitter le mode reconnaissance

```
face_detect
```

Retour au mode détection de visage pure (cadre + points clés uniquement, sans reconnaissance) ; les étiquettes d'ID sont effacées.
> **À propos du mode DETECT** : sur l'ESP32-S3, l'impression des coordonnées sur le port série en mode détection pure est désactivée (`#if !CONFIG_IDF_TARGET_ESP32S3`), afin d'éviter de saturer le port série avec les journaux de détection. Les journaux de coordonnées `detection_result` n'apparaissent qu'après être entré en mode reconnaissance (`face_rz`).

## Aide-mémoire complet des commandes

| Commande | Fonction | Comportement de l'étiquette | Persistante |
|---|---|---|---|
| `face_eril` | Enregistrer le visage actuellement détecté | Bleue "Enroll: ID N" | Flash 0.5s |
| `face_rz` | Entrer en mode reconnaissance continue | Verte "ID: N" / rouge "who?" | ✅ Continue |
| `face_del` | Supprimer le dernier ID enregistré | Rouge "N IDs left" | Flash 0.5s |
| `face_detect` | Quitter la reconnaissance, revenir à la détection pure | Efface toutes les étiquettes | — |

## Exemple de déroulement

```
ai_mode:4                          # Entrer en mode reconnaissance faciale
[redémarrage de l'appareil, LED violet]

face_eril                          # Enregistrer le premier visage (Alice)
→ ID 1 is enrolled

face_eril                          # Enregistrer le second visage (Bob)
→ ID 2 is enrolled

face_rz                            # Démarrer la reconnaissance continue
→ Alice devant la caméra : l'image affiche « ID: 1 » en continu
→ Bob devant la caméra : l'image affiche « ID: 2 » en continu
→ Une personne inconnue devant la caméra : l'image affiche « who? » en continu

face_detect                        # Quitter le mode reconnaissance
→ les étiquettes disparaissent, seuls les cadres de détection restent

face_del                           # Supprimer Bob (ID 2)
→ 1 IDs left

face_rz                            # Reconnaître à nouveau
→ Alice devant la caméra : « ID: 1 »
→ Bob devant la caméra : « who? » (supprimé)
```

> Le mode reconnaissance faciale consomme beaucoup de mémoire (modèle MFN + double modèle de détection de visage) ; le port série Type-C (UART0) fonctionne normalement. Si le port série ne répond pas, vérifiez d'abord que le débit est bien de 115200.

## Code

### Logique de reconnaissance principale

`components/modules/ai/who_human_face_recognition.cpp` — stratégie de saut de trames :

```cpp
case RECOGNIZE:
{
    // Saut de trames : 1 reconnaissance MFN toutes les 10 détections
    static int recog_skip = 0;
    if (recog_skip <= 0) {
        recognize_result = recognizer->recognize(
            (uint16_t *)frame->buf,
            {(int)frame->height, (int)frame->width, 3},
            detect_results.front().keypoint);
        recog_skip = 10;
    }
    recog_skip--;
    frame_show_state = SHOW_STATE_RECOGNIZE;
    break;
}
```

## Dépannage

| Symptôme | Cause possible | Solution |
|---|---|---|
| `No face ID in flash` | Normal, aucun visage encore enregistré | Envoyer `face_eril` pour enregistrer |
| Résultat toujours `who?` | Éclairage insuffisant / angle défavorable / similarité sous le seuil | Réenregistrer, face à la caméra, éclairage uniforme |
| Aucune réaction lors de l'enregistrement | Nombre de visages à l'image ≠ 1 | S'assurer qu'il n'y a qu'un seul visage, à 30-50cm |
| Image saccadée pendant la reconnaissance | Normal, l'inférence MFN prend du temps | Déjà optimisé par saut de trames, une exécution toutes les 10 trames |
| Étiquette clignotante | — | Corrigé, l'étiquette reste affichée en continu |
| `fail: unknown command` | Faute d'orthographe dans la commande | Vérifier la commande : `face_eril` et non `face_enroll` |

## Résultat

Enregistrer un visage → reconnaissance continue avec affichage de l'ID → sortie des résultats en I2C/port série → commande de relais/servomoteur : une solution de contrôle d'accès complète.
