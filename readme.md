# TOTEM Workstation

**TOTEM** est une workstation audio open-source basée sur l'architecture "Dual-Teensy", dérivée du projet LMN-3.
**Based on the LMN-3 Project by fundamental.frequency.**
Ce dépôt contient le code source unifié pour le processeur Maître (Audio) et le processeur Esclave (Contrôle/UI).

## ⚠️ AVERTISSEMENT MATÉRIEL CRITIQUE ⚠️

Ce firmware est conçu pour une version **modifiée** du PCB LMN-3. Ne pas flasher sur un PCB LMN-3 stock sans lire ceci :

### Architecture Dual-Teensy 4.1
1.  **MASTER (Moteur Audio)** : Teensy 4.1 standard.
2.  **SLAVE (Contrôleur UI)** : Teensy 4.1 avec modifications physiques irréversibles.

### Modifications du Teensy Esclave (Hardware Hack)
Pour libérer les bus de communication UART et S/PDIF, les pins suivantes du Teensy Esclave doivent être **PLIÉES** (isolées du PCB) et connectées via des fils volants :

* **Pin 0 (RX1)** : Connectée au Master TX.
* **Pin 1 (TX1)** : Connectée au Master RX.
* **Pin 14 (S/PDIF)** : Connectée au Master Pin 15.

### Réparation de la Matrice (Fly-wires)
L'isolation des pins 0, 1 et 14 coupe des colonnes de la matrice clavier. Elles sont reroutées ainsi :
* Colonne 7 (Anciennement Pin 0) ➔ **Pin 33**
* Colonne 6 (Anciennement Pin 1) ➔ **Pin 37**
* Colonne 9 (Anciennement Pin 14) ➔ **Pin 38**

---

## 🎹 Mapping des Touches (Mode Groovebox)

L'interface est divisée en zones fonctionnelles :
* **Zone Rythme (Haut Gauche 4x4)** : Déclencheurs MIDI (Canal 10).
* **Zone Mélodie (2 Rangées du bas)** : Clavier chromatique (Canal 1).
* **Zone Commandes (L-Shape Droite)** : Octave +/-, Shift, Mode, Menus, Transport.
* **Encodeurs** :
    * 1-3 : Paramètres de performance (Cutoff, Reso, FX).
    * 4 (Droite) : Navigation Système (Rotation = Scroll, Click = Enter).

## 🛠 Installation

Le projet utilise **PlatformIO**.
1.  Ouvrir le dossier dans VSCode avec l'extension PlatformIO.
2.  Sélectionner l'environnement :
    * `env:totem_master` pour le Teensy Audio.
    * `env:totem_slave` pour le Teensy Contrôleur.
3.  Upload via USB (En cas de bug Esclave, maintenir le bouton Encodeur 4 au démarrage pour le mode Diagnostic).
