// Lecteur audio - étape 1 : lecteur MP3/WAV sur carte SD
//
// Fonctionnement :
//  1. Au démarrage, l'ESP32 cherche tous les fichiers .mp3 et .wav de la carte SD
//     (racine et sous-dossiers) et les range par ordre alphabétique.
//  2. Il joue la liste en boucle sur le haut-parleur, via l'ampli I2S MAX98357A.
//  3. Trois boutons : lecture/pause, suivant, précédent. Un potentiomètre règle le volume.
//     Appui long sur lecture/pause : mode aléatoire activé ou désactivé.
//
// Câblage : voir le README de l'étape.

#include <Arduino.h>
#include <SD.h>
#include <SPI.h>
#include <vector>
#include <algorithm>
#include "Audio.h"

// ---------- Brochage (identique aux étapes 2 et 3) ----------
#define BROCHE_I2S_BCLK  26
#define BROCHE_I2S_LRC   25
#define BROCHE_I2S_DIN   27
#define BROCHE_SD_CS      5   // SCK 18, MISO 19, MOSI 23 (SPI matériel)
#define BROCHE_LECTURE   32
#define BROCHE_SUIVANT   33
#define BROCHE_PRECEDENT 14
#define BROCHE_VOLUME    34   // curseur du potentiomètre (entrée analogique seule)

#define VOLUME_MAX       21   // la bibliothèque audio va de 0 à 21
#define PROFONDEUR_MAX    3   // niveaux de sous-dossiers explorés
#define NB_FICHIERS_MAX 300   // au-delà, la mémoire de l'ESP32 sature

Audio audio;
std::vector<String> playlist;
int piste = 0;
bool enPause = false;
bool aleatoire = false;
int volume = -1;

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

// ---------- Lecture ----------
void jouer(int index) {
  if (playlist.empty()) return;
  piste = (index + playlist.size()) % playlist.size();
  enPause = false;
  Serial.printf("Lecture %d/%d : %s\n", piste + 1, (int)playlist.size(), playlist[piste].c_str());
  if (!audio.connecttoFS(SD, playlist[piste].c_str())) {
    Serial.println("Erreur : fichier illisible, passage au suivant");
  }
}

void suivant() {
  if (aleatoire && playlist.size() > 1) {
    int autre;
    do { autre = random(playlist.size()); } while (autre == piste);
    jouer(autre);
  } else {
    jouer(piste + 1);
  }
}

void precedent() {
  // Comme sur un baladeur : après 3 secondes, « précédent » revient au début du morceau
  if (audio.getAudioCurrentTime() > 3) jouer(piste);
  else jouer(piste - 1);
}

// Appelée par la bibliothèque à la fin d'un fichier
void audio_eof_mp3(const char* info) {
  suivant();
}

// Messages de la bibliothèque audio (format, débit, erreurs)
void audio_info(const char* info) {
  Serial.printf("[audio] %s\n", info);
}

// Titre et artiste lus dans les balises ID3 du MP3
void audio_id3data(const char* info) {
  Serial.printf("[ID3] %s\n", info);
}

// ---------- Volume ----------
void lireVolume() {
  static uint32_t derniereLecture = 0;
  static float moyenne = -1;
  if (millis() - derniereLecture < 50) return;
  derniereLecture = millis();
  int brut = analogRead(BROCHE_VOLUME);  // 0 à 4095
  moyenne = (moyenne < 0) ? brut : moyenne * 0.8f + brut * 0.2f;  // lisse le bruit de l'ADC
  // Hystérésis : on ne change que si le curseur s'éloigne nettement du cran actuel,
  // sinon le volume oscillerait entre deux valeurs
  const float pas = 4095.0f / VOLUME_MAX;
  if (volume < 0 || fabsf(moyenne - volume * pas) > pas * 0.7f) {
    volume = constrain((int)roundf(moyenne / pas), 0, VOLUME_MAX);
    audio.setVolume(volume);
    Serial.printf("Volume : %d/%d\n", volume, VOLUME_MAX);
  }
}

void setup() {
  Serial.begin(115200);
  delay(300);
  Serial.println("\nLecteur audio - étape 1");

  pinMode(BROCHE_LECTURE, INPUT_PULLUP);
  pinMode(BROCHE_SUIVANT, INPUT_PULLUP);
  pinMode(BROCHE_PRECEDENT, INPUT_PULLUP);
  analogReadResolution(12);
  randomSeed(esp_random());

  if (!SD.begin(BROCHE_SD_CS)) {
    Serial.println("Erreur : carte SD absente ou mal formatée (FAT32 obligatoire)");
    while (true) delay(1000);
  }
  explorer("/", 0);
  std::sort(playlist.begin(), playlist.end());
  Serial.printf("%d fichiers audio trouvés\n", (int)playlist.size());
  if (playlist.empty()) {
    Serial.println("Erreur : aucun .mp3 ou .wav sur la carte");
    while (true) delay(1000);
  }

  audio.setPinout(BROCHE_I2S_BCLK, BROCHE_I2S_LRC, BROCHE_I2S_DIN);
  lireVolume();
  jouer(0);
}

void loop() {
  audio.loop();
  lireVolume();

  switch (lireBouton(btnLecture)) {
    case COURT:
      audio.pauseResume();
      enPause = !enPause;
      Serial.println(enPause ? "Pause" : "Lecture");
      break;
    case LONG:
      aleatoire = !aleatoire;
      Serial.println(aleatoire ? "Mode aléatoire activé" : "Mode aléatoire désactivé");
      break;
    default: break;
  }
  if (lireBouton(btnSuivant) == COURT) suivant();
  if (lireBouton(btnPrecedent) == COURT) precedent();
}
