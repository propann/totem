# 🗿 PROJET TOTEM : Station de travail hybride à double Teensy

**Dépôt :** https://github.com/propann/totem  
**Projet de base :** https://github.com/FundamentalFrequency/LMN-3

## 📖 Aperçu

Project Totem est une ré‑ingénierie radicale de la station LMN‑3. Il remplace le Raspberry Pi (SBC) par un **Teensy 4.1** secondaire, transformant le système en « cluster bare‑metal ». Cela supprime la latence de l’OS, les temps de démarrage et le jitter, en dédiant un CPU à l’interface/synthèse et l’autre aux E/S/contrôles.

### Architecture

* **Nœud A (maître) :** Teensy 4.1 exécutant MicroDexed-touch + Peanut‑GB (émulateur GameBoy). Gère le moteur audio, l’interface utilisateur et le mix.
* **Nœud B (esclave) :** Teensy 4.1 (sur PCB LMN‑3). Gère la matrice clavier, le joystick, les encodeurs et l’audio auxiliaire.

---

## ⚠️ MODIFICATION MATÉRIELLE REQUISE (CRITIQUE)

**NE PAS FLASHER LE FIRMWARE ESCLAVE SUR UN PCB LMN‑3 D’ORIGINE SANS MODIFICATION.**
Le PCB LMN‑3 d’origine route les colonnes de matrice vers les broches 0, 1 et 14. Project Totem nécessite ces broches pour l’UART haut débit et le S/PDIF.

### Le hack de « pliage de broches » (unité esclave)

Pour construire l’unité esclave, vous devez isoler des broches spécifiques du PCB et les réacheminer via des fils volants.

1.  **Isoler les broches :** Pendant l’assemblage, pliez **les broches 0, 1 et 14** vers l’extérieur (horizontalement). Ne les soudez pas aux en‑têtes du PCB.
2.  **Le pont (connexion inter‑Teensy) :**
    * **UART :** Broche 0 esclave (RX) <-> Broche 1 maître (TX)
    * **UART :** Broche 1 esclave (TX) <-> Broche 0 maître (RX)
    * **Audio :** Broche 14 esclave (SPDIF OUT) -> Broche 15 maître (SPDIF IN)
    * **Masse :** une masse commune est obligatoire.
3.  **Le réacheminement (fils volants) :**
    Restaurez les connexions de matrice rompues en soudant des fils des trous du PCB vers des broches SD inutilisées du Teensy :
    * Trou PCB 0 -> Broche Teensy **29**
    * Trou PCB 1 -> Broche Teensy **33**
    * Trou PCB 14 -> Broche Teensy **37**

---

## 🎹 Modules firmware

### 1. Firmware esclave (contrôleur)
Situé dans `/firmware_slave`.
* **Basé sur :** firmware LMN‑3.
* **Modifications :**
    * Suppression de l’USB‑MIDI / ajout du MIDI série haut débit (2 Mbps).
    * Ajout de `AudioSynthWaveform` + `AudioOutputSPDIF3` (broche 14).
    * Implémentation d’un protocole binaire brut pour le joystick (X/Y/Bouton) via série.
    * Remappage des colonnes de matrice dans `config.h` pour correspondre au hack matériel.

### 2. Firmware maître (moteur)
Situé dans `/firmware_master`.
* **Basé sur :** MicroDexed-touch.
* **Fonctionnalités :**
    * `AsyncAudioInputSPDIF3` pour synchroniser l’audio esclave (ASRC).
    * Intégration de Peanut‑GB pour l’émulation.
    * Mixeur global (synthé FM + émulateur + entrée auxiliaire).

---

## 🛠️ Protocoles

**Communication série (broches 0/1) :**
* **Débit :** 2 000 000 (2 Mbps).
* **Format :**
    * Messages MIDI standard (NoteOn, CC) via `Control_Surface`.
    * Paquet binaire personnalisé pour le joystick : `[0xFF, X, Y, Btn]`.

**Transport audio :**
* S/PDIF sur TTL (3,3 V) via Broche 14->15.
* Fréquence d’échantillonnage : 44,1 kHz.

---

## Crédits
* **LMN‑3 :** Fundamental Frequency
* **Teensy Audio Library :** Paul Stoffregen
* **Control Surface :** tttapa
* **MicroDexed-touch :** Holger Wirtz
* **Peanut‑GB :** deltabeard
