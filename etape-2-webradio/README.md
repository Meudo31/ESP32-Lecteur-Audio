# Étape 2 : les webradios et la page de commande

Liste d'achat : rien de plus qu'à l'étape 1, le Wi-Fi est intégré à l'ESP32.

Le lecteur se connecte au Wi-Fi et peut jouer des webradios en plus de la carte SD. Une page web, ouverte sur le téléphone à l'adresse **http://lecteur.local**, montre le morceau en cours et permet de tout commander : lecture, pause, morceau suivant, volume, choix d'un fichier de la carte ou d'une radio dans la liste.

Le câblage est celui de l'étape 1, sans changement. Le code compile mais n'a pas encore été testé sur une vraie carte.

## Utilisation

| Commande | Effet |
| --- | --- |
| Lecture/pause, appui court / long | Pause ou reprise / mode aléatoire (carte SD) |
| Suivant, appui court | Morceau ou radio suivante |
| Suivant, appui long | Bascule entre la carte SD et les webradios |
| Précédent | Morceau ou radio précédente |
| Potentiomètre | Volume. La page web règle aussi le volume ; le potentiomètre reprend la main dès qu'on le tourne |

Sans carte SD, le lecteur démarre directement sur la première radio. Sans Wi-Fi, la carte SD marche comme à l'étape 1.

## Mise en route

1. Copie `include/config.example.h` en `include/config.h`, puis remplis le Wi-Fi (2,4 GHz).
2. Ajoute ou retire des radios dans la liste `RADIOS` du même fichier. Les radios de Radio France sont déjà là ; pour en trouver d'autres, cherche leur adresse de flux MP3 sur [radio-browser.info](https://www.radio-browser.info).
3. `pio run -t upload`, puis `pio device monitor` : l'adresse IP et l'adresse http://lecteur.local s'affichent.
4. Ouvre http://lecteur.local sur le téléphone, connecté au même Wi-Fi. Si l'adresse ne répond pas (certains Android anciens ne connaissent pas les adresses en .local), tape l'adresse IP affichée dans le moniteur série.

## Points à vérifier ensemble

- **Coupures du son en webradio** : l'ESP32-WROOM n'a pas de mémoire PSRAM, donc la mémoire tampon est petite. Les flux à 128 kbit/s (les adresses « midfi » de Radio France) passent bien ; évite les flux à 320 kbit/s. Une carte ESP32-WROVER (avec PSRAM) supprime le problème sans changer le code.
- **Signal Wi-Fi** : le moniteur série affiche le signal en dBm ; en dessous de -75 dBm, rapproche le lecteur de la box.
- **Sécurité** : la page n'a pas de mot de passe. Elle n'est visible que sur ton réseau local, n'ouvre pas de port vers Internet sur ta box.
