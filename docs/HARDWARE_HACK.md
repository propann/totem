# Hardware Hacks (Mémoire projet)

## Pin Bending validé (0, 1, 14)

Ce tableau référence le câblage confirmé pour éviter les erreurs futures.

| Pin Teensy | Action | Rôle / Câblage |
| --- | --- | --- |
| 0 (RX1) | Pin bending | Relié au lien série entre les cartes (RX). Ne pas connecter au header standard. |
| 1 (TX1) | Pin bending | Relié au lien série entre les cartes (TX). Ne pas connecter au header standard. |
| 14 (A0) | Pin bending | Ligne dédiée (ex: signal auxiliaire/contrôle). Ne pas connecter au header standard. |

> Remarque : ces pins sont **physiquement déviées** (bent) pour isoler le bus. Vérifier le routage avant soudure.
