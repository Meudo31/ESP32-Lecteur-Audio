// Lecteur audio - étape 2 : webradio et page web de commande
//
// Reprend le lecteur de l'étape 1 (carte SD, 3 boutons, potentiomètre) et ajoute :
//  - les webradios, listées dans config.h ;
//  - une page web sur http://lecteur.local pour tout commander depuis le téléphone :
//    lecture/pause, suivant, précédent, volume, choix d'un morceau ou d'une radio.
//
// Boutons :
//  - lecture/pause : appui court = pause, appui long = mode aléatoire (carte SD) ;
//  - suivant : appui court = morceau ou radio suivante, appui long = bascule carte SD / webradio ;
//  - précédent : morceau ou radio précédente.
// Sans Wi-Fi, le lecteur de carte SD marche quand même.

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <SD.h>
#include <SPI.h>
#include <vector>
#include <algorithm>
#include "Audio.h"
#include "config.h"
#include "page.h"

// ---------- Brochage (identique aux étapes 1 et 3) ----------
#define BROCHE_I2S_BCLK  26
#define BROCHE_I2S_LRC   25
#define BROCHE_I2S_DIN   27
#define BROCHE_SD_CS      5   // SCK 18, MISO 19, MOSI 23 (SPI matériel)
#define BROCHE_LECTURE   32
#define BROCHE_SUIVANT   33
#define BROCHE_PRECEDENT 14
#define BROCHE_VOLUME    34   // curseur du potentiomètre (entrée analogique seule)

#define VOLUME_MAX       21
#define PROFONDEUR_MAX    3
#define NB_FICHIERS_MAX 300
#define NB_RADIOS (sizeof(RADIOS) / sizeof(RADIOS[0]))

enum Mode { MODE_SD, MODE_RADIO };

Audio audio;
WebServer serveur(80);
std::vector<String> playlist;
Mode mode = MODE_SD;
int piste = 0;
int station = 0;
bool enPause = false;
bool aleatoire = false;
bool carteSdOk = false;
int volume = 10;
String titre;    // titre ID3 du morceau ou titre diffusé par la radio
String artiste;

// ---------- Boutons : appui court et appui long ----------
struct Bouton {
  uint8_t broche;
  explicit Bouton(uint8_t b) : broche(b) {}
  bool etatPrecedent = HIGH;
  uint32_t debutAppui = 0;
  bool longSignale = false;
  uint32_t dernierChangement = 0;
};

enum Appui { AUCUN, COURT, LONG };

Appui lireBouton(Bouton& b) {
  bool etat = digitalRead(b.broche);
  uint32_t maintenant = millis();
  Appui resultat = AUCUN;
  if (etat != b.etatPrecedent && maintenant - b.dernierChangement > 30) {  // anti-rebond
    b.dernierChangement = maintenant;
    b.etatPrecedent = etat;
    if (etat == LOW) {
      b.debutAppui = maintenant;
      b.longSignale = false;
    } else if (!b.longSignale) {
      resultat = COURT;
    }
  }
  if (etat == LOW && !b.longSignale && maintenant - b.debutAppui > 800) {
    b.longSignale = true;
    resultat = LONG;
  }
  return resultat;
}

Bouton btnLecture{BROCHE_LECTURE};
Bouton btnSuivant{BROCHE_SUIVANT};
Bouton btnPrecedent{BROCHE_PRECEDENT};

// ---------- Carte SD ----------
bool estAudio(const String& nom) {
  String n = nom;
  n.toLowerCase();
  return n.endsWith(".mp3") || n.endsWith(".wav");
}

void explorer(const char* dossier, int profondeur) {
  File rep = SD.open(dossier);
  if (!rep || !rep.isDirectory()) return;
  File f = rep.openNextFile();
  while (f && playlist.size() < NB_FICHIERS_MAX) {
    String chemin = f.path();
    String nom = f.name();
    if (!nom.startsWith(".")) {  // ignore les fichiers cachés (macOS crée des ._xxx.mp3)
      if (f.isDirectory()) {
        if (profondeur < PROFONDEUR_MAX) explorer(chemin.c_str(), profondeur + 1);
      } else if (estAudio(nom)) {
        playlist.push_back(chemin);
      }
    }
    f.close();
    f = rep.openNextFile();
  }
  rep.close();
}

// Nom affiché d'un fichier : chemin sans le « / » de tête
String nomFichier(int i) {
  return playlist[i].substring(1);
}

// ---------- Lecture ----------
void jouerFichier(int index) {
  if (playlist.empty()) return;
  mode = MODE_SD;
  piste = (index + playlist.size()) % playlist.size();
  enPause = false;
  titre = "";
  artiste = "";
  Serial.printf("Lecture %d/%d : %s\n", piste + 1, (int)playlist.size(), playlist[piste].c_str());
  if (!audio.connecttoFS(SD, playlist[piste].c_str())) {
    Serial.println("Erreur : fichier illisible");
  }
}

void jouerRadio(int index) {
  mode = MODE_RADIO;
  station = (index + NB_RADIOS) % NB_RADIOS;
  enPause = false;
  titre = "";
  artiste = "";
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Erreur : pas de Wi-Fi, webradio impossible");
    audio.stopSong();
    return;
  }
  Serial.printf("Webradio : %s\n", RADIOS[station].nom);
  if (!audio.connecttohost(RADIOS[station].url)) {
    Serial.println("Erreur : flux injoignable");
  }
}

void suivant() {
  if (mode == MODE_RADIO) { jouerRadio(station + 1); return; }
  if (aleatoire && playlist.size() > 1) {
    int autre;
    do { autre = random(playlist.size()); } while (autre == piste);
    jouerFichier(autre);
  } else {
    jouerFichier(piste + 1);
  }
}

void precedent() {
  if (mode == MODE_RADIO) { jouerRadio(station - 1); return; }
  // Comme sur un baladeur : après 3 secondes, « précédent » revient au début du morceau
  if (audio.getAudioCurrentTime() > 3) jouerFichier(piste);
  else jouerFichier(piste - 1);
}

void basculerLecture() {
  if (!audio.isRunning() && !enPause) {
    // Rien en cours (radio coupée, erreur) : on relance
    mode == MODE_RADIO ? jouerRadio(station) : jouerFichier(piste);
    return;
  }
  audio.pauseResume();
  enPause = !enPause;
  Serial.println(enPause ? "Pause" : "Lecture");
}

void changerMode() {
  if (mode == MODE_SD) jouerRadio(station);
  else if (carteSdOk && !playlist.empty()) jouerFichier(piste);
}

void reglerVolume(int v) {
  volume = constrain(v, 0, VOLUME_MAX);
  audio.setVolume(volume);
  Serial.printf("Volume : %d/%d\n", volume, VOLUME_MAX);
}

// ---------- Fonctions appelées par la bibliothèque audio ----------
void audio_eof_mp3(const char* info) {
  if (mode == MODE_SD) suivant();
}

void audio_eof_stream(const char* info) {
  Serial.println("Le flux de la radio s'est arrêté");
}

void audio_info(const char* info) {
  Serial.printf("[audio] %s\n", info);
}

// Balises ID3 du MP3, sous la forme « Title: ... » ou « Artist: ... »
void audio_id3data(const char* info) {
  String s = info;
  if (s.startsWith("Title: ")) titre = s.substring(7);
  else if (s.startsWith("Artist: ")) artiste = s.substring(8);
}

// Titre du morceau diffusé par la webradio
void audio_showstreamtitle(const char* info) {
  titre = info;
  Serial.printf("Radio : %s\n", info);
}

// ---------- Potentiomètre ----------
// Le potentiomètre ne commande le volume que lorsqu'on le tourne :
// un réglage fait depuis la page web n'est pas écrasé au tour de boucle suivant.
void lirePotentiometre() {
  static uint32_t derniereLecture = 0;
  static float moyenne = -1;
  static int cran = -1;
  if (millis() - derniereLecture < 50) return;
  derniereLecture = millis();
  int brut = analogRead(BROCHE_VOLUME);  // 0 à 4095
  moyenne = (moyenne < 0) ? brut : moyenne * 0.8f + brut * 0.2f;  // lisse le bruit de l'ADC
  const float pas = 4095.0f / VOLUME_MAX;
  // Hystérésis : on ne change que si le curseur s'éloigne nettement du cran actuel
  if (cran < 0 || fabsf(moyenne - cran * pas) > pas * 0.7f) {
    cran = constrain((int)roundf(moyenne / pas), 0, VOLUME_MAX);
    reglerVolume(cran);
  }
}

// ---------- Page web ----------
// Échappe une chaîne pour la placer entre guillemets dans du JSON
String json(const String& s) {
  String r = "\"";
  for (size_t i = 0; i < s.length(); i++) {
    char c = s[i];
    if (c == '"' || c == '\\') { r += '\\'; r += c; }
    else if ((uint8_t)c < 0x20) r += ' ';
    else r += c;
  }
  return r + "\"";
}

void pageAccueil() {
  serveur.send_P(200, "text/html; charset=utf-8", PAGE_HTML);
}

void pageEtat() {
  String nom = (mode == MODE_RADIO) ? String(RADIOS[station].nom)
             : (playlist.empty() ? String("") : nomFichier(piste));
  String affiche = titre;
  if (artiste.length() && titre.length()) affiche = artiste + " - " + titre;
  String r = "{\"mode\":";
  r += (mode == MODE_RADIO) ? "\"radio\"" : "\"sd\"";
  r += ",\"index\":" + String(mode == MODE_RADIO ? station : piste);
  r += ",\"nom\":" + json(nom);
  r += ",\"titre\":" + json(affiche);
  r += ",\"pause\":" + String(enPause ? "true" : "false");
  r += ",\"aleatoire\":" + String(aleatoire ? "true" : "false");
  r += ",\"volume\":" + String(volume);
  r += ",\"temps\":" + String(mode == MODE_SD ? audio.getAudioCurrentTime() : 0);
  r += ",\"duree\":" + String(mode == MODE_SD ? audio.getAudioFileDuration() : 0);
  r += "}";
  serveur.send(200, "application/json", r);
}

// La liste peut être longue : envoyée morceau par morceau pour ménager la mémoire
void pageListe() {
  serveur.setContentLength(CONTENT_LENGTH_UNKNOWN);
  serveur.send(200, "application/json", "");
  serveur.sendContent("[");
  for (size_t i = 0; i < playlist.size(); i++) {
    serveur.sendContent((i ? "," : "") + json(nomFichier(i)));
  }
  serveur.sendContent("]");
  serveur.sendContent("");  // fin de la réponse
}

void pageRadios() {
  String r = "[";
  for (size_t i = 0; i < NB_RADIOS; i++) {
    if (i) r += ",";
    r += json(RADIOS[i].nom);
  }
  serveur.send(200, "application/json", r + "]");
}

void pageCommande() {
  String a = serveur.arg("a");
  int n = serveur.arg("n").toInt();
  if (a == "lecture") basculerLecture();
  else if (a == "suivant") suivant();
  else if (a == "precedent") precedent();
  else if (a == "aleatoire") aleatoire = !aleatoire;
  else if (a == "volume") reglerVolume(serveur.arg("v").toInt());
  else if (a == "sd" && n >= 0 && n < (int)playlist.size()) jouerFichier(n);
  else if (a == "radio" && n >= 0 && n < (int)NB_RADIOS) jouerRadio(n);
  else { serveur.send(400, "text/plain", "commande inconnue"); return; }
  serveur.send(200, "text/plain", "ok");
}

// ---------- Démarrage ----------
void connecterWifi() {
  WiFi.mode(WIFI_STA);
  WiFi.setSleep(false);  // évite les coupures du son en webradio
  WiFi.begin(WIFI_SSID, WIFI_MOTDEPASSE);
  Serial.print("Connexion au Wi-Fi");
  uint32_t debut = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - debut < 15000) {
    delay(250);
    Serial.print(".");
  }
  Serial.println();
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Erreur : Wi-Fi injoignable, seule la carte SD sera disponible");
    return;
  }
  Serial.printf("Wi-Fi connecté : %s (signal %d dBm)\n", WiFi.localIP().toString().c_str(), WiFi.RSSI());
  if (MDNS.begin(NOM_RESEAU)) {
    MDNS.addService("http", "tcp", 80);
    Serial.printf("Page de commande : http://%s.local\n", NOM_RESEAU);
  }
}

void setup() {
  Serial.begin(115200);
  delay(300);
  Serial.println("\nLecteur audio - étape 2");

  pinMode(BROCHE_LECTURE, INPUT_PULLUP);
  pinMode(BROCHE_SUIVANT, INPUT_PULLUP);
  pinMode(BROCHE_PRECEDENT, INPUT_PULLUP);
  analogReadResolution(12);
  randomSeed(esp_random());

  carteSdOk = SD.begin(BROCHE_SD_CS);
  if (carteSdOk) {
    explorer("/", 0);
    std::sort(playlist.begin(), playlist.end());
    Serial.printf("%d fichiers audio trouvés sur la carte SD\n", (int)playlist.size());
  } else {
    Serial.println("Pas de carte SD : mode webradio uniquement");
  }

  connecterWifi();
  serveur.on("/", pageAccueil);
  serveur.on("/etat", pageEtat);
  serveur.on("/liste", pageListe);
  serveur.on("/radios", pageRadios);
  serveur.on("/cmd", pageCommande);
  serveur.begin();

  audio.setPinout(BROCHE_I2S_BCLK, BROCHE_I2S_LRC, BROCHE_I2S_DIN);
  lirePotentiometre();
  if (!playlist.empty()) jouerFichier(0);
  else jouerRadio(0);
}

void loop() {
  audio.loop();
  serveur.handleClient();
  lirePotentiometre();

  switch (lireBouton(btnLecture)) {
    case COURT: basculerLecture(); break;
    case LONG:
      aleatoire = !aleatoire;
      Serial.println(aleatoire ? "Mode aléatoire activé" : "Mode aléatoire désactivé");
      break;
    default: break;
  }
  switch (lireBouton(btnSuivant)) {
    case COURT: suivant(); break;
    case LONG: changerMode(); break;
    default: break;
  }
  if (lireBouton(btnPrecedent) == COURT) precedent();
}
