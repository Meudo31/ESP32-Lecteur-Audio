# Étape 1 : le lecteur MP3 sur carte SD

Liste d'achat avec les liens : voir le [README principal](../README.md#liste-dachat-par-étape).

L'ESP32 lit les MP3 et WAV de la carte micro SD et les joue sur le haut-parleur, à la suite et en boucle. Trois boutons et un potentiomètre suffisent pour le piloter, comme un baladeur.

Le code compile avec PlatformIO (ESP32 DevKit V1, framework Arduino) mais n'a pas encore été testé sur une vraie carte.

## Câblage

| De | Vers |
| --- | --- |
| MAX98357A VIN / GND | VIN (5 V) / GND de l'ESP32 |
| MAX98357A BCLK / LRC / DIN | GPIO26 / GPIO25 / GPIO27 |
| MAX98357A sorties + et - | Haut-parleur |
| Lecteur SD VCC / GND | VIN (5 V) / GND |
| Lecteur SD CS / SCK / MOSI / MISO | GPIO5 / GPIO18 / GPIO23 / GPIO19 |
| Bouton lecture/pause | GPIO32 et GND |
| Bouton suivant | GPIO33 et GND |
| Bouton précédent | GPIO14 et GND |
| Potentiomètre : extrémités | 3V3 et GND |
| Potentiomètre : curseur (patte du milieu) | GPIO34 |
| Condensateur 1000 µF (+ sur VIN, - sur GND) | Au plus près du MAX98357A |

Laisse les broches **GAIN** et **SD** du MAX98357A non branchées : gain de 9 dB et son mono (moyenne gauche + droite), ce qu'il faut pour un seul haut-parleur. Les sorties du haut-parleur ne doivent jamais toucher GND.

Les modules lecteurs SD bleus courants ont un régulateur et un adaptateur de niveaux : ils s'alimentent en 5 V. Si ton module n'a ni l'un ni l'autre (module « 3,3 V » sans composant), alimente-le en 3V3.

## Utilisation

| Commande | Effet |
| --- | --- |
| Lecture/pause, appui court | Pause ou reprise |
| Lecture/pause, appui long | Mode aléatoire activé ou désactivé |
| Suivant | Morceau suivant (au hasard en mode aléatoire) |
| Précédent | Retour au début du morceau, ou morceau précédent dans les 3 premières secondes |
| Potentiomètre | Volume de 0 à 21 |

## Mise en route

1. Prépare la carte SD (voir le [README principal](../README.md#préparer-la-carte-sd)) et insère-la.
2. Branche l'ESP32 en USB, lance `pio run -t upload`, puis `pio device monitor`. Si le téléversement échoue, maintiens le bouton BOOT au moment où « Connecting… » s'affiche.
3. Le moniteur série affiche le nombre de fichiers trouvés, puis le nom du morceau, le format et les balises ID3 (titre, artiste).

## Points à vérifier ensemble

- **Grésillements ou redémarrages au volume fort** : le port USB d'un PC fournit parfois trop peu de courant. Essaie un chargeur de téléphone 5 V / 2 A et ajoute le condensateur.
- **Son saturé** : un petit haut-parleur sature avant le volume 21. Baisse le volume, ou relie GAIN à GND par une résistance de 100 kΩ pour passer à 15 dB seulement si le son est trop faible.
- **« carte SD absente »** : vérifie le formatage FAT32 et le câblage MISO/MOSI, souvent inversés.
- **Fichiers ignorés** : les fichiers cachés commençant par un point (créés par macOS) sont volontairement sautés.
