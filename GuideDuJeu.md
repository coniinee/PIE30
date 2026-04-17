# Voix — Manuel d’utilisation clinique et guide technique

## Objet du document

Ce manuel a été rédigé comme un **guide d’utilisation réel** pour la version finale de **Voix**.

Il s’adresse à deux publics :
- **les enseignants / évaluateurs**, qui doivent comprendre la logique de conception et d’implémentation ;
- **les orthophonistes / thérapeutes**, qui ont besoin d’un guide pratique pour utiliser l’application avec un enfant.

Le document combine donc un **manuel utilisateur**, un **guide de prise en main clinique** et une **explication simplifiée du traitement du signal et de la logique de jeu**.

---

## 1. Qu’est-ce que Voix ?

**Voix** est une application de bureau développée avec **JUCE** pour l’**entraînement à la production vocale chez des enfants sourds ou malentendants**, sous supervision thérapeutique.

Le logiciel transforme l’entrée microphone en retour visuel immédiat à travers trois mini-jeux :

1. **Guirlande du sapin** — présence vocale et phonation continue
2. **Moulin à vent** — intensité vocale
3. **Vol du papillon** — hauteur de la voix

L’application n’est ni un dispositif médical ni un outil diagnostique. C’est un **prototype de soutien thérapeutique** destiné à rendre l’entraînement vocal plus motivant, plus visuel et plus démonstratif.

---

## 2. Objectifs cliniques des jeux

### Guirlande du sapin
Objectif principal : produire et maintenir un son stable.

### Moulin à vent
Objectif principal : moduler l’intensité vocale.

### Vol du papillon
Objectif principal : contrôler la hauteur vocale.

---

## 3. Démarrage rapide

1. Assurez-vous que JUCE est disponible localement pour la compilation.
2. Vérifiez le microphone et les autorisations système.
3. Lancez l’application.
4. Choisissez un jeu, un mode et une difficulté.
5. Appuyez sur **Démarrer**.
6. Laissez la calibration se faire dans le silence.
7. Après la calibration et le compte à rebours éventuel, l’enfant peut jouer.

---

## 4. Vue d’ensemble de l’écran

### Partie gauche : zone enfant
- sélection visuelle du jeu ;
- animation principale ;
- cartes d’état ;
- effets de progression et de célébration.

### Partie droite : zone thérapeute
- jeu, mode et difficulté ;
- bouton principal d’action ;
- bouton de réinitialisation ;
- diagnostics audio ;
- mesures du tour ;
- informations de session.

---

## 5. Préparation avant une séance

Avant de commencer, vérifiez :
- que la pièce est relativement calme ;
- que le microphone est suffisamment proche ;
- que la calibration se fait dans le silence ;
- que le mode et la difficulté sont adaptés.

Pour une première utilisation, il est conseillé de commencer par :
- **Guirlande du sapin** ;
- **Mode Entraînement** ;
- **Difficulté Facile**.

---

## 6. Détail des commandes

### Bouton principal d’action
Le texte du bouton change selon l’état courant :
- **Démarrer l’entraînement / Démarrer le programme** ;
- **Recalibrer** ;
- **Étape suivante** ;
- **Rejouer** ;
- **Redémarrer le programme**.

### Bouton Réinitialiser
Ramène la session à l’état initial. À utiliser pour interrompre un tour, changer de configuration ou repartir proprement.

### Mode
- **Entraînement** : un seul tour indépendant.
- **Programme guidé** : trois étapes successives.

### Difficulté
- **Facile**
- **Standard**
- **Avancé**

### Compte à rebours
Le **compte à rebours de 3 s** après calibration facilite la transition entre préparation et action.

### Décalage du seuil
**Décalage du seuil (dB, plus bas = plus sensible)**

- valeur plus basse → détection plus sensible ;
- valeur plus élevée → détection moins sensible.

---

## 7. Diagnostics audio

Le panneau de droite affiche plusieurs mesures :

- **Niveau brut (dBFS)**
- **Niveau lissé (dBFS)**
- **Contrôle d’intensité (%)**
- **Hauteur estimée (Hz)**
- **Bruit de fond (dBFS)**
- **Seuil d’ouverture (dBFS)**
- **Seuil de fermeture (dBFS)**

Ces informations aident le thérapeute à comprendre ce que le système détecte et à ajuster la sensibilité si nécessaire.

---

## 8. Mesures du tour

Le logiciel suit notamment :
- **Série actuelle** ;
- **Meilleure série** ;
- **Temps vocalisé** ;
- **Stabilité** ;
- **Contrôle d’intensité** ;
- **Étoiles**.

Les étoiles sont une récompense de motivation, pas un score clinique.

---

## 9. Comment jouer à chaque jeu

### Guirlande du sapin
Demander à l’enfant de produire un son stable et de le maintenir pour faire monter les lumières.

### Moulin à vent
Demander à l’enfant de produire une voix stable et suffisamment forte pour garder le moulin en mouvement.

### Vol du papillon
Demander à l’enfant de faire varier la hauteur de sa voix pour monter ou descendre le papillon et le maintenir près de la fleur cible.

---

## 10. Programme guidé

Chaque jeu possède **trois étapes**.

### Guirlande du sapin
- Étape 1 : Échauffement
- Étape 2 : Garder la stabilité
- Étape 3 : Défi du sapin complet

### Moulin à vent
- Étape 1 : Brise légère
- Étape 2 : Vent régulier
- Étape 3 : Défi du vent fort

### Vol du papillon
- Étape 1 : Fleur basse
- Étape 2 : Fleur centrale
- Étape 3 : Fleur haute

---

## 11. Déroulé thérapeutique conseillé

Ordre recommandé pour une première séance :
1. Guirlande du sapin / Entraînement / Facile
2. Guirlande du sapin / Programme guidé / Standard
3. Moulin à vent / Entraînement / Facile ou Standard
4. Vol du papillon / Entraînement / Facile
5. Vol du papillon / Programme guidé / Standard si l’enfant est à l’aise

---

## 12. Dépannage

### Le jeu ne réagit pas
Vérifiez le silence pendant la calibration, l’état du microphone, la distance au micro et le décalage du seuil.

### Le jeu réagit au bruit
Recalibrez dans un environnement plus calme et augmentez légèrement le décalage du seuil.

### Le papillon paraît trop difficile
C’est normal : le contrôle de hauteur est le plus exigeant. Commencez en **Facile**.

---

## 13. Notes techniques simplifiées

- la présence vocale repose sur une mesure RMS convertie en dBFS ;
- le niveau est lissé pour stabiliser l’interface ;
- deux seuils sont utilisés pour l’hystérésis ;
- une calibration automatique ajuste les seuils avant chaque tour ;
- chaque jeu utilise ensuite ces informations différemment : présence, intensité ou hauteur.

---

## 14. Bonnes pratiques

- faire la calibration dans le silence ;
- garder une distance micro assez constante ;
- commencer par des consignes simples ;
- ne pas modifier trop de paramètres à la fois ;
- réserver le papillon aux enfants déjà à l’aise avec le principe du système.

---

## 15. Conclusion

**Voix** fournit une base cohérente, fonctionnelle et démontrable pour un projet de rééducation vocale assistée par retour visuel, avec trois modes d’entraînement distincts, un panneau thérapeute utile et une architecture claire.
