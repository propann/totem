# Totem — Dépôt multi-firmware

Ce dépôt contient **deux cerveaux** distincts :

- **totem_slave** : firmware clavier/contrôleur (dossier existant).
- **totem_master** : firmware moteur audio/écran (nouveau dossier).

## Flash / Build (PlatformIO)

- Utiliser l’environnement `totem_slave` pour le **Clavier**.
- Utiliser l’environnement `totem_master` pour le **Moteur Audio**.

Exemples :

```bash
pio run -e totem_slave
pio run -e totem_master
```

## Structure

- `firmware_slave/` : firmware existant (ne pas modifier hors besoins spécifiques).
- `firmware_master/` : firmware audio (nouveau).
- `docs/` : documentation unifiée.
