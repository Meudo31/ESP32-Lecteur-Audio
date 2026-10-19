# Bonus : idées pour aller plus loin

Liste d'achat avec les liens : voir le [README principal](../README.md#bonus). Ces idées ne sont pas codées ; chacune peut faire une vidéo courte ou une partie de vidéo.

## Stéréo avec deux MAX98357A

Branche un second MAX98357A sur les mêmes BCLK, LRC et DIN. Chaque module choisit son canal selon la tension sur sa broche SD (fiche technique du MAX98357A) : plus de 1,4 V = canal gauche, entre 0,77 et 1,4 V = canal droit, entre 0,36 et 0,77 V = mono (réglage d'usine des modules courants). Pour le gauche, relie SD directement à VIN. Pour le droit, mets une résistance d'environ 470 kΩ entre SD et VIN, puis mesure SD au multimètre et ajuste pour tomber vers 1 V : la valeur exacte dépend des résistances déjà soudées sur ton module. Pas une ligne de code à changer.

## Sortie casque ou chaîne hi-fi avec un PCM5102A

Le PCM5102A est un DAC I2S qui sort le son sur une prise jack, à brancher sur un casque, une enceinte amplifiée ou une chaîne. Câblage : BCK sur GPIO26, LCK sur GPIO25, DIN sur GPIO27, SCK à GND. Le code reste le même ; le son est nettement meilleur qu'avec le MAX98357A et un petit haut-parleur.

## Lecteur sur batterie

Batterie 18650, module de charge TP4056 protégé, puis convertisseur MT3608 réglé à 5 V vers la broche VIN. Ajoute un pont diviseur (deux résistances de 100 kΩ) entre la batterie et GPIO35 pour afficher la charge sur l'écran de l'étape 3.

## Réveil-matin

L'ESP32 récupère l'heure sur Internet (`configTzTime` avec le serveur `pool.ntp.org`). Ajoute une heure de réveil sur la page web et, à l'heure dite, lance la radio préférée avec le volume qui monte doucement : c'est le minuteur de sommeil de l'étape 3 à l'envers.

## Boîtier

Un boîtier imprimé en 3D ou une petite caisse en bois, avec une ouverture pour le haut-parleur, fait passer le montage d'essai à un vrai objet. Une caisse fermée améliore beaucoup les basses d'un petit haut-parleur.
