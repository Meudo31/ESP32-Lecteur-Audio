# Étape 3 : l'écran, la reprise et le minuteur de sommeil

Liste d'achat avec les liens : voir le [README principal](../README.md#liste-dachat-par-étape) (un écran OLED à 3-5 €).

Le lecteur gagne un petit écran et deux fonctions de vrai appareil :

- **Écran OLED** : mode (carte SD ou radio), numéro du morceau, titre qui défile, artiste, barre de progression, volume, et le volume en grand quand on tourne le potentiomètre.
- **Reprise après coupure** : débranche le lecteur en plein morceau, rebranche-le, il repart sur le même morceau au même endroit (ou sur la même radio). La position est notée toutes les 20 secondes et à chaque pause dans la mémoire de l'ESP32.
- **Minuteur de sommeil** : appui long sur « précédent » pour choisir 15, 30 ou 60 minutes (ou l'arrêt). Le son baisse doucement pendant les 20 dernières secondes, puis le lecteur se met en pause. Le minuteur est aussi sur la page web.

Le reste (webradios, page http://lecteur.local, boutons) est identique à l'étape 2. Le code compile mais n'a pas encore été testé sur une vraie carte.

## Câblage

Celui des étapes 1 et 2, plus l'écran :

| Écran OLED | ESP32 |
| --- | --- |
| VCC | 3V3 |
| GND | GND |
| SDA | GPIO21 |
| SCL | GPIO22 |

## Mise en route

1. Copie `include/config.example.h` en `include/config.h` et remplis le Wi-Fi (tu peux reprendre celui de l'étape 2). Les durées du minuteur sont réglables dans `MINUTEURS`.
2. `pio run -t upload`, puis `pio device monitor`. L'écran affiche « Lecteur » au démarrage puis le morceau en cours.

## Points à vérifier ensemble

- **Écran noir** : la plupart des modules sont à l'adresse 0x3C, certains à 0x3D (inscrite au dos). Change `ECRAN_ADRESSE` dans `src/ecran.h`.
- **Accents** : la police de l'écran ne les connaît pas, le code les remplace par des lettres simples (« Été » devient « Ete »).
- **Son haché pendant le rafraîchissement de l'écran** : peu probable, la mémoire tampon du son couvre environ 180 ms et l'écran prend 25 ms. Si ça arrive, passe la limite de `majEcran()` de 250 à 500 ms.
- **Usure de la mémoire** : une écriture toutes les 20 secondes est sans risque, la mémoire NVS de l'ESP32 répartit l'usure.
