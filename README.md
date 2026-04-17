# Voix

> **Note sur les dépendances de compilation**  
> Ce dépôt **n’est pas une archive JUCE entièrement autonome**. Le code source, le fichier `CMakeLists.txt` et le projet `.jucer` sont inclus, mais **le framework JUCE lui-même n’est pas intégré dans cette archive**.  
> Pour compiler le projet, vous devez soit :
> - installer JUCE localement et pointer la compilation vers cette installation ;
> - soit ajouter JUCE comme dépendance ou sous-module dans votre environnement local.

**Voix** est une application de mini-jeux thérapeutiques développée avec **JUCE**, conçue pour l’**entraînement vocal d’enfants sourds ou malentendants** sous la supervision d’un thérapeute ou d’un orthophoniste.

L’application transforme l’entrée microphone en temps réel en retour visuel immédiat à travers **trois jeux pilotés par la voix** :

- **Guirlande du sapin** — travaille la **présence vocale** et la **phonation continue**
- **Moulin à vent** — travaille l’**intensité vocale**
- **Vol du papillon** — travaille la **hauteur de la voix**

Le projet a été conçu comme une application de bureau autonome, compacte mais complète : capture audio en temps réel, analyse du signal, calibration adaptative, commandes thérapeute, progression par étapes, système d’étoiles et scènes rendues entièrement en C++ avec JUCE.

---

## Table des matières

1. [Vue d’ensemble du projet](#vue-densemble-du-projet)
2. [Jeux thérapeutiques implémentés](#jeux-thérapeutiques-implémentés)
3. [Fonctionnalités principales](#fonctionnalités-principales)
4. [Architecture de l’application](#architecture-de-lapplication)
5. [Déroulement à l’exécution](#déroulement-à-lexécution)
6. [Algorithmes principaux](#algorithmes-principaux)
7. [Logique de jeu et progression](#logique-de-jeu-et-progression)
8. [Architecture de l’interface](#architecture-de-linterface)
9. [Structure du dépôt](#structure-du-dépôt)
10. [Compilation](#compilation)
11. [Notes de plateforme](#notes-de-plateforme)
12. [Dépannage](#dépannage)
13. [Limites actuelles](#limites-actuelles)
14. [Extensions possibles](#extensions-possibles)

---

## Vue d’ensemble du projet

L’objectif de **Voix** est de transformer de simples tâches de production vocale en interactions visuelles motivantes. Plutôt que d’afficher uniquement des valeurs numériques, l’application convertit les informations acoustiques en états de jeu ludiques et faciles à comprendre.

Le projet s’appuie sur trois dimensions cliniquement intuitives :

1. **Présence sonore / phonation tenue** : l’enfant comprend que, s’il produit un son stable, quelque chose se passe à l’écran.
2. **Intensité vocale** : l’enfant apprend à moduler la force de sa voix.
3. **Hauteur de la voix** : l’enfant apprend à monter ou baisser sa hauteur vocale et à la maintenir dans une zone cible.

Cette version a été pensée comme un **projet final démontrable et autonome**, et non comme un outil de recherche en analyse vocale. L’accent est mis sur :

- la stabilité en temps réel ;
- une organisation claire du code ;
- une logique de jeu compréhensible ;
- des paramètres ajustables par le thérapeute ;
- un flux de compilation reproductible.

---

## Jeux thérapeutiques implémentés

### 1) Guirlande du sapin
**Compétence ciblée :** présence vocale + phonation continue

Lorsqu’un son stable est détecté, la guirlande s’allume progressivement du bas vers le haut. Quand le son s’arrête, la progression redescend lentement. Le joueur réussit en atteignant l’objectif et en maintenant la voix assez longtemps.

### 2) Moulin à vent
**Compétence ciblée :** contrôle de l’intensité

Quand une voix stable est détectée, le moulin commence à tourner. Plus la voix est forte, plus la progression augmente vite. Le jeu dépend donc non seulement de la présence d’un son, mais aussi de sa puissance.

### 3) Vol du papillon
**Compétence ciblée :** contrôle de la hauteur

Le papillon se déplace verticalement selon la hauteur estimée de la voix. L’enfant doit monter ou baisser la hauteur vocale pour placer le papillon près d’une fleur cible et le maintenir dans la bonne zone.

---

## Fonctionnalités principales

- Entrée microphone temps réel avec `juce::AudioAppComponent`
- Calibration automatique avant chaque tour
- Analyse RMS en dBFS
- Lissage du niveau sonore
- Détection de voix avec hystérésis
- Temps minimum de maintien pour les décisions d’ouverture / fermeture
- Normalisation de l’intensité pour le contrôle de jeu
- Estimation de la hauteur pour le jeu du papillon
- Mode **Entraînement** et mode **Programme guidé** en 3 étapes
- Difficultés **Facile / Standard / Avancé**
- Compte à rebours avant le début du jeu
- Suivi de progression, durée tenue, stabilité et étoiles
- Panneau thérapeute unifié
- Rendu graphique entièrement personnalisé avec JUCE
- Prise en charge des workflows **Projucer** et **CMake**

---

## Architecture de l’application

À haut niveau, le projet est structuré en quatre couches :

```text
Entrée microphone
      |
      v
AudioAnalyzer
  - RMS / dBFS
  - lissage
  - calibration
  - seuils
  - normalisation d’intensité
  - estimation de hauteur
      |
      v
GameSession
  - machine d’états
  - objectifs par étape
  - mise à jour de progression
  - validation de réussite
  - attribution d’étoiles
      |
      v
Scenes + SettingsPanel
  - rendu du sapin
  - rendu du moulin
  - rendu du papillon
  - commandes thérapeute
  - statistiques / diagnostics / HUD
```

### Classes principales

#### `MainComponent`
Composant principal de l’application.

Responsabilités :
- gérer le cycle de vie de l’audio ;
- recevoir les blocs microphone ;
- piloter le timer d’interface ;
- connecter `AudioAnalyzer` à `GameSession` ;
- transmettre les données de scène au jeu actif ;
- coordonner le panneau de contrôle.

#### `AudioAnalyzer`
Convertit les échantillons microphone en valeurs exploitables par le jeu : niveau brut, niveau lissé, bruit de fond, seuils, détection vocale, intensité normalisée et hauteur estimée.

#### `GameSession`
Machine d’états du gameplay : mode, difficulté, jeu actif, objectifs par étape, progression, validation de réussite et attribution d’étoiles.

#### `SettingsPanel`
Panneau de contrôle destiné au thérapeute : démarrage, recalibration, réinitialisation, diagnostics audio et statistiques de tour.

#### `TreeScene`, `WindmillScene`, `ButterflyScene`
Scènes graphiques personnalisées recevant des données simplifiées depuis `MainComponent`.

#### `SceneChrome`
Couche utilitaire partagée pour les cartes, badges, barres de progression et overlays.

#### `Utf8Text`
Petit utilitaire de texte UTF-8 utilisé pour garantir l’affichage correct des chaînes accentuées et des caractères multilingues dans l’interface.

---

## Déroulement à l’exécution

1. `Main.cpp` lance l’application JUCE.
2. `MainComponent` initialise l’audio et l’interface.
3. Lorsqu’on appuie sur **Démarrer**, une calibration d’environ **1,5 s** commence.
4. Les seuils sont ensuite recalculés à partir du bruit de fond estimé.
5. Le jeu passe soit au compte à rebours, soit directement à l’état de jeu.
6. Le thread audio calcule le RMS, la détection, l’intensité et la hauteur.
7. Le timer d’interface met à jour `GameSession`, le panneau de contrôle et la scène active.

---

## Algorithmes principaux

### RMS et dBFS
Pour chaque bloc audio :

```text
RMS = sqrt( sum(x[n]^2) / N )
```

Puis conversion en décibels :

```text
dBFS = 20 * log10(RMS)
```

### Lissage exponentiel
Le niveau est lissé pour éviter les réactions trop brusques :

```text
smooth[n] = alpha * smooth[n-1] + (1 - alpha) * current[n]
```

### Hystérésis
Deux seuils sont utilisés :
- un seuil d’ouverture ;
- un seuil de fermeture.

Cela stabilise la détection de voix et évite les oscillations autour d’une valeur unique.

### Calibration adaptative
Avant chaque tour, le système mesure le bruit de fond puis ajuste les seuils automatiquement. Le thérapeute peut ensuite affiner la sensibilité avec le curseur dédié.

### Contrôle spécifique selon le jeu
- **Guirlande du sapin** : dépend surtout de la présence d’une voix stable.
- **Moulin à vent** : dépend de la présence de voix et de l’intensité.
- **Vol du papillon** : dépend de la présence de voix et de la hauteur estimée.

---

## Logique de jeu et progression

### Guirlande du sapin
- voix stable → progression vers le haut ;
- silence ou instabilité → régression progressive ;
- objectif atteint + maintien suffisant → réussite.

### Moulin à vent
- voix stable → rotation active ;
- intensité plus forte → progression plus rapide ;
- intensité insuffisante ou silence → perte de progression.

### Vol du papillon
- hauteur plus grave → position plus basse ;
- hauteur plus aiguë → position plus haute ;
- proximité de la cible + stabilité → validation progressive.

### Modes de jeu
- **Entraînement** : un seul tour indépendant.
- **Programme guidé** : trois étapes successives avec difficulté croissante.

### Étoiles
Le jeu attribue de **1 à 3 étoiles** selon la qualité de la performance. Il s’agit d’un retour motivationnel, pas d’un score clinique.

---

## Architecture de l’interface

L’interface est divisée en deux zones :

### Zone gauche
- sélection visuelle du jeu ;
- scène active ;
- overlays de calibration, compte à rebours et célébration ;
- cartes d’état visibles par l’enfant.

### Zone droite
- panneau thérapeute ;
- sélecteurs de jeu, mode et difficulté ;
- diagnostics audio ;
- statistiques du tour ;
- actions principales.

---

## Structure du dépôt

```text
PIE30/
├── .gitignore
├── CMakeLists.txt
├── GuideDuJeu.md
├── README.md
├── Voix.jucer
└── Source/
    ├── AudioAnalyzer.cpp
    ├── AudioAnalyzer.h
    ├── ButterflyScene.cpp
    ├── ButterflyScene.h
    ├── GameSession.h
    ├── Main.cpp
    ├── MainComponent.cpp
    ├── MainComponent.h
    ├── SceneChrome.h
    ├── SettingsPanel.cpp
    ├── SettingsPanel.h
    ├── TreeScene.cpp
    ├── TreeScene.h
    ├── Utf8Text.h
    ├── WindmillScene.cpp
    └── WindmillScene.h
```

---

## Compilation

### Avec Projucer
1. Installez JUCE localement.
2. Ouvrez `Voix.jucer` dans Projucer.
3. Vérifiez la configuration des modules JUCE et des exporters.
4. Générez puis ouvrez le projet dans l’IDE correspondant.

**Remarque importante :**
- le fichier `.jucer` fourni contient déjà un exporter **Xcode (macOS)** ;
- si vous souhaitez utiliser **Projucer sous Windows**, vous devrez **ajouter manuellement un exporter Visual Studio** avant de générer le projet ;
- sur Windows, vous pouvez aussi utiliser directement la méthode **CMake** ci-dessous.

5. Compilez la cible de l’application.

### Avec CMake
macOS / Linux :

```bash
mkdir build
cd build
cmake .. -DJUCE_DIR=/chemin/vers/JUCE
cmake --build .
```

Windows :

```bash
mkdir build
cd build
cmake .. -DJUCE_DIR=C:/chemin/vers/JUCE
cmake --build . --config Release
```

### Remarques
- JUCE doit être présent localement.
- Les permissions microphone doivent être autorisées sur la machine cible.

---

## Notes de plateforme

### macOS
- Autorisez l’accès micro dans les Réglages système.
- Si Xcode affiche un ancien nom, regénérez les fichiers via Projucer ou nettoyez le dossier de build.

### Windows
- Vérifiez le périphérique d’entrée par défaut.
- Si vous utilisez Projucer, pensez à ajouter un exporter Visual Studio.
- Utilisez une version de Visual Studio compatible avec JUCE.

### Linux
- Vérifiez PulseAudio / PipeWire / ALSA selon l’environnement local.

---

## Dépannage

### L’application ne réagit pas à la voix
Vérifier :
- l’accès microphone ;
- le niveau d’entrée du système ;
- la distance au micro ;
- la valeur du décalage de seuil ;
- le silence pendant la calibration.

### Le jeu réagit trop facilement au bruit
Essayer :
- de recalibrer dans une pièce plus calme ;
- d’augmenter légèrement le décalage de seuil ;
- d’éloigner un peu le micro ;
- de réduire le bruit ambiant.

### Le papillon semble plus difficile
C’est normal : le contrôle de hauteur est le plus exigeant des trois modes.

---

## Limites actuelles

- L’application est un prototype pédagogique / thérapeutique, pas un dispositif médical.
- L’estimation de hauteur est plus sensible à la qualité du signal que la simple détection de présence vocale.
- Le comportement dépend du micro et de l’environnement sonore.
- JUCE n’est pas inclus dans l’archive finale.

---

## Extensions possibles

- ajouter d’autres mini-jeux basés sur le voisement ;
- enrichir les diagnostics cliniques exportables ;
- personnaliser les profils enfant / séance ;
- sauvegarder les résultats entre les sessions ;
- améliorer encore l’estimation de hauteur pour les voix enfantines.

---

## Résumé

**Voix** propose une démonstration complète et cohérente d’un mini-système de rééducation vocale fondé sur JUCE, avec capture audio temps réel, calibration adaptative, analyse de présence vocale, d’intensité et de hauteur, trois jeux distincts et un panneau thérapeute exploitable.
