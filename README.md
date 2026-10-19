# Lecteur audio MP3 et webradio

Un lecteur de musique fait maison : il joue les MP3 et WAV d'une carte micro SD, capte les webradios et se commande depuis le téléphone. Le projet avance par étapes, du plus simple au plus complet ; chaque étape est un projet PlatformIO autonome et peut faire l'objet d'une vidéo.

| Étape | Dossier | Ce que ça fait |
| --- | --- | --- |
| 1 | [etape-1-lecteur-sd](etape-1-lecteur-sd) | Lecteur MP3/WAV sur carte SD : lecture/pause, suivant, précédent, volume au potentiomètre, mode aléatoire |
| 2 | [etape-2-webradio](etape-2-webradio) | Ajoute les webradios (FIP, FIP Rock, France Inter…) et une page web http://lecteur.local pour tout commander du téléphone |
| 3 | [etape-3-ecran](etape-3-ecran) | Ajoute un écran OLED (titre qui défile, progression, volume), la reprise après coupure de courant et un minuteur de sommeil |
| Bonus | [bonus](bonus) | Stéréo, sortie casque ou chaîne hi-fi, batterie, réveil-matin, boîtier |

Toutes les étapes utilisent la même carte, **ESP32 DevKit V1 (ESP32-WROOM-32, 30 broches)**, et le même brochage : on ajoute des pièces sans rien recâbler. Le son passe par un ampli **MAX98357A** en I2S, qui reçoit le son en numérique et attaque directement un petit haut-parleur : pas de DAC ni d'ampli séparé à câbler.

Le décodage MP3 est fait par la bibliothèque [ESP32-audioI2S](https://github.com/schreibfaul1/ESP32-audioI2S) (version 2.0.6, téléchargée automatiquement par PlatformIO). Le code compile pour les trois étapes mais n'a pas encore été testé sur une vraie carte.

## Brochage commun

| Pièce | Broche ESP32 |
| --- | --- |
| MAX98357A : BCLK / LRC / DIN | GPIO26 / GPIO25 / GPIO27 |
| MAX98357A : VIN / GND | VIN (5 V de l'USB) / GND |
| Lecteur micro SD : CS / SCK / MOSI / MISO | GPIO5 / GPIO18 / GPIO23 / GPIO19 |
| Lecteur micro SD : VCC / GND | VIN (5 V) / GND |
| Bouton lecture/pause, suivant, précédent (autre patte à GND) | GPIO32 / GPIO33 / GPIO14 |
| Potentiomètre de volume : curseur (extrémités sur 3V3 et GND) | GPIO34 |
| Écran OLED (étape 3) : SDA / SCL, alimenté en 3V3 | GPIO21 / GPIO22 |

## Liste d'achat par étape

Prix approximatifs ; chaque étape réutilise ce qui a été acheté avant. Les liens ouvrent une recherche, car les fiches produits changent souvent : choisis un vendeur bien noté avec beaucoup de commandes. Amazon livre en 1 à 3 jours, AliExpress coûte environ moitié moins mais met 1 à 3 semaines.

### Étape 1 (environ 20 à 30 €)

| Pièce | Prix approx. | Où acheter |
| --- | --- | --- |
| ESP32 DevKit V1 (ESP32-WROOM-32, 30 broches) | 5 à 7 € | [Amazon](https://www.amazon.fr/s?k=esp32+devkit+v1) · [AliExpress](https://fr.aliexpress.com/w/wholesale-esp32-devkit-v1.html) |
| Ampli I2S MAX98357A | 3 à 5 € | [Amazon](https://www.amazon.fr/s?k=MAX98357A) · [AliExpress](https://fr.aliexpress.com/w/wholesale-MAX98357A.html) |
| Haut-parleur 4 ou 8 Ω, 3 W, 40 à 77 mm | 3 à 6 € | [Amazon](https://www.amazon.fr/s?k=haut+parleur+4+ohm+3W) · [AliExpress](https://fr.aliexpress.com/w/wholesale-speaker-4-ohm-3W.html) |
| Module lecteur micro SD en SPI (avec régulateur, alimenté en 5 V) | 1 à 2 € | [Amazon](https://www.amazon.fr/s?k=module+lecteur+micro+SD+SPI+arduino) · [AliExpress](https://fr.aliexpress.com/w/wholesale-micro-sd-card-module-spi.html) |
| Carte micro SD de 32 Go maximum, formatée en FAT32 | 5 € | [Amazon](https://www.amazon.fr/s?k=carte+micro+SD+32+Go+SanDisk) · en magasin aussi |
| Potentiomètre 10 kΩ linéaire (B10K) + bouton | 1 à 2 € | [Amazon](https://www.amazon.fr/s?k=potentiom%C3%A8tre+B10K+bouton) · [AliExpress](https://fr.aliexpress.com/w/wholesale-potentiometer-B10K-knob.html) |
| 3 boutons poussoirs 12 mm | 1 à 2 € | [Amazon](https://www.amazon.fr/s?k=bouton+poussoir+12mm+arduino) · [AliExpress](https://fr.aliexpress.com/w/wholesale-push-button-12mm.html) |
| Condensateur 470 à 1000 µF, 10 V ou plus (contre les grésillements) | moins d'1 € | [Amazon](https://www.amazon.fr/s?k=condensateur+1000uF+10V) · [AliExpress](https://fr.aliexpress.com/w/wholesale-1000uF-10V-capacitor.html) |
| Plaque d'essai et fils Dupont | 3 à 5 € | [Amazon](https://www.amazon.fr/s?k=breadboard+fils+dupont) · [AliExpress](https://fr.aliexpress.com/w/wholesale-breadboard-dupont-wire.html) |

### Étape 2 (rien à acheter)

Le Wi-Fi est intégré à l'ESP32. Il faut seulement une box Internet en 2,4 GHz.

### Étape 3 (environ 3 à 5 € en plus)

| Pièce | Prix approx. | Où acheter |
| --- | --- | --- |
| Écran OLED 0,96 pouce SSD1306 128 × 64, I2C (4 broches) | 3 à 5 € | [Amazon](https://www.amazon.fr/s?k=OLED+0.96+SSD1306+I2C) · [AliExpress](https://fr.aliexpress.com/w/wholesale-oled-0.96-ssd1306-i2c.html) |

### Bonus

| Pièce | Prix approx. | Où acheter |
| --- | --- | --- |
| Second MAX98357A + second haut-parleur (stéréo) | 6 à 10 € | [Amazon](https://www.amazon.fr/s?k=MAX98357A) · [AliExpress](https://fr.aliexpress.com/w/wholesale-MAX98357A.html) |
| DAC PCM5102A (sortie jack pour casque ou chaîne hi-fi) | 4 à 6 € | [Amazon](https://www.amazon.fr/s?k=PCM5102A) · [AliExpress](https://fr.aliexpress.com/w/wholesale-PCM5102A.html) |
| Batterie 18650 de marque + support, module TP4056 protégé, convertisseur 5 V MT3608 | 8 à 12 € | [Amazon](https://www.amazon.fr/s?k=batterie+18650+Samsung+25R) · [TP4056](https://www.amazon.fr/s?k=TP4056+USB-C+protection) · [MT3608](https://www.amazon.fr/s?k=MT3608) · [NKON](https://www.nkon.nl/fr/) |

Achète la batterie 18650 en France ou chez NKON : celles d'AliExpress affichent souvent une fausse capacité.

## Préparer la carte SD

- Formate-la en **FAT32** (32 Go maximum : au-delà, Windows propose exFAT, que la bibliothèque ne lit pas).
- Copie des **.mp3** (44,1 kHz, 128 à 320 kbit/s) ou des **.wav** (PCM 16 bits). Les sous-dossiers sont explorés sur 3 niveaux, jusqu'à 300 fichiers.
- L'ordre de lecture est l'ordre alphabétique du chemin : préfixe les fichiers par `01_`, `02_`… pour imposer un ordre.

## Mise en route

Chaque dossier a son propre README avec le câblage et la mise en route. L'étape 1 n'a aucun secret. Aux étapes 2 et 3 : copier `include/config.example.h` en `include/config.h`, remplir le Wi-Fi, puis `pio run -t upload` depuis le dossier de l'étape. Le fichier `config.h` est exclu du dépôt.
