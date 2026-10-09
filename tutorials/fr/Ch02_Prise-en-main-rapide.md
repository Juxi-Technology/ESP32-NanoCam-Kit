# Chapitre 2 : Prise en main rapide

## 2.1 Flashage du firmware

### Étapes

1. Décompresser le dossier → `nanocam_xxx.bin`
2. Ouvrir [esptool-js](https://espressif.github.io/esptool-js/)
3. Connecter le NanoCam en Type-C
4. Cliquer sur Connect → choisir le port série
5. Sélectionner le fichier firmware, saisir l'adresse `0x0`
6. Cliquer sur START → attendre la fin

### Vérification

Connecter un outil de port série (115200 8N1) au NanoCam, vous devez voir :

```
NanoCam Board Ver:0.3.0
```

---

## 2.2 Configuration WiFi

> Résultat : **le NanoCam se connecte au WiFi et obtient une IP**

### Méthode A : configuration par port série (la plus courante)

```
sta_ssid:VotreNomWiFi
sta_pd:VotreMotDePasseWiFi
```

Réception de `OK` → configuration réussie. L'appareil redémarre automatiquement après modification du mot de passe.

### Méthode B : connexion directe au point d'accès AP

Le NanoCam crée son propre point d'accès : `NanoCam-AP`, mot de passe `12345678`
Une fois le téléphone connecté, ouvrez `http://192.168.4.1` dans le navigateur

### Vérification

```
sta_ip
```

Retour : `sta_ip:192.168.x.x` ✅

---

## 2.3 Première image

> Résultat : **l'image en temps réel du NanoCam s'affiche dans le navigateur**
1. Saisir `http://<adresse IP>` dans le navigateur
2. L'image MJPEG en temps réel s'affiche
3. Envoyer `ai_mode:1` sur le port série → bascule en détection de visage de chat → un cadre de détection apparaît à l'image

### Description des endpoints

| URL | Utilisation |
|---|---|
| `http://<IP>/` | Image en temps réel (HTML) |
| `http://<IP>/stream` | Flux MJPEG brut (lisible par OpenCV/VLC) |
| `http://<IP>/status` | État de l'appareil en JSON |
| `http://<IP>/admin` | Interface d'administration web |
