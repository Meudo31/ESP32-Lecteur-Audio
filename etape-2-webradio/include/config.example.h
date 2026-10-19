// Copier ce fichier en config.h et remplir les valeurs.
// config.h ne doit jamais être publié (il contient le mot de passe Wi-Fi).
#pragma once

#define WIFI_SSID       "NomDeTonWifi"
#define WIFI_MOTDEPASSE "MotDePasseWifi"

// Nom sur le réseau : la page de commande sera sur http://lecteur.local
#define NOM_RESEAU      "lecteur"

// Webradios : nom affiché, adresse du flux MP3 (les flux AAC marchent aussi).
// Les adresses changent parfois : si une radio ne démarre pas, cherche
// « nom de la radio stream url mp3 » ou regarde sur https://www.radio-browser.info
struct Radio { const char* nom; const char* url; };
static const Radio RADIOS[] = {
  { "FIP",            "http://icecast.radiofrance.fr/fip-midfi.mp3" },
  { "FIP Rock",       "http://icecast.radiofrance.fr/fiprock-midfi.mp3" },
  { "FIP Jazz",       "http://icecast.radiofrance.fr/fipjazz-midfi.mp3" },
  { "France Inter",   "http://icecast.radiofrance.fr/franceinter-midfi.mp3" },
  { "franceinfo",     "http://icecast.radiofrance.fr/franceinfo-midfi.mp3" },
  { "Mouv'",          "http://icecast.radiofrance.fr/mouv-midfi.mp3" },
  { "France Musique", "http://icecast.radiofrance.fr/francemusique-midfi.mp3" },
};
