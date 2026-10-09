# Chapitre 1 : Configuration de l'environnement

## 1.1 Environnement de flashage du firmware

### Méthode A : sans environnement de développement (recommandé pour les débutants)

1. Installer le [pilote de port série CH340K](https://www.wch.cn/download/CH341SER_EXE.html)
2. Ouvrir le navigateur → [esptool-js](https://espressif.github.io/esptool-js/)
3. Connecter le NanoCam, choisir le port série, sélectionner le fichier firmware .bin
4. Cliquer sur Program pour flasher

### Méthode B : ligne de commande

```bash
pip install esptool
esptool.py --chip esp32s3 --port COM_x write_flash 0x0 nanocam_xxx.bin
```

### Méthode C : environnement de développement ESP-IDF (avancé)

1. Installer VSCode + l'extension ESP-IDF
2. F1 → `ESP-IDF: Configure ESP-IDF Extension`
3. Choisir ou installer ESP-IDF v5.4+
4. Compiler : `idf.py build flash monitor`

### Méthode D : installation via ESP-EIM-GUI

1. Télécharger sur le site officiel : https://dl.espressif.cn/dl/eim/
2. Après le téléchargement, double-cliquer pour ouvrir la page EIM ; le coin supérieur droit permet de basculer l'interface en chinois
3. Cliquer sur « Commencer l'installation »
4. À l'étape suivante, choisir l'installation personnalisée
5. Au préalable, `git` et `python3.12.x` doivent être installés (source de téléchargement de git pour la Chine continentale : [CNPM Binaries Mirror](https://registry.npmmirror.com/binary.html?path=git-for-windows/v2.55.0.windows.3/))
6. Choisir esp32s3 comme appareil cible
7. Pour la version d'ESP-IDF, cocher ici « afficher les anciennes versions stables », puis faire défiler vers le bas et sélectionner la version v5.4.1
8. Laisser la sélection du miroir de téléchargement inchangée, puis passer à l'étape suivante
9. Pour les fonctionnalités ESP-IDF, tout sélectionner est recommandé ; continuer vers l'étape suivante
10. À l'étape des outils, passer à la suite, puis choisir l'emplacement d'installation souhaité et lancer l'installation ; attendre la fin
Une fois l'installation terminée, cette version présente un problème de décompression : trouver le répertoire C:\Espressif\dist\xtensa-esp-elf-14.2.0_20241119-x86_64-w64-mingw32.zip, copier l'archive vers C:\Espressif\tools\xtensa-esp-elf, puis, après extraction, trouver le dossier xtensa-esp-elf et remplacer par les dossiers situés sous C:\Espressif\tools\xtensa-esp-elf\esp-14.2.0_20241119 pour que la compilation réussisse

## 1.2 Environnement serveur

### Service officiel xiaozhi.me (gratuit)

1. Se rendre sur [xiaozhi.me](https://xiaozhi.me) pour créer un compte
2. Accéder à la console
3. Une fois le module connecté au réseau, il annonce un code de vérification à 6 chiffres
4. Cliquer sur l'ajout d'appareil à droite de la section « Agent »
5. Saisir le code de vérification à 6 chiffres annoncé
6. Une fois l'appareil associé, la conversation peut commencer
