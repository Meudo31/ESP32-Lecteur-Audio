// Écran OLED SSD1306 128 x 64 en I2C (SDA GPIO21, SCL GPIO22)
//
// La police intégrée de la bibliothèque Adafruit ne connaît pas les accents :
// les titres sont convertis en lettres simples (é -> e, ç -> c...) avant l'affichage.
#pragma once
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define ECRAN_LARGEUR 128
#define ECRAN_HAUTEUR  64
#define ECRAN_ADRESSE 0x3C   // 0x3D sur certains modules : regarde l'étiquette au dos

Adafruit_SSD1306 ecran(ECRAN_LARGEUR, ECRAN_HAUTEUR, &Wire, -1);
bool ecranOk = false;

// Ce que l'écran doit montrer, rempli par main.cpp
struct Affichage {
  String entete;     // « SD 3/42 » ou « RADIO FIP »
  String ligne1;     // titre
  String ligne2;     // artiste ou nom du fichier
  uint32_t temps = 0, duree = 0;
  int volume = 0;
  bool pause = false, aleatoire = false;
  int minuteur = 0;  // minutes restantes, 0 = pas de minuteur
  uint32_t volumeChangeA = 0;  // millis() du dernier changement de volume
};

// Remplace les lettres accentuées UTF-8 les plus courantes par leur version sans accent
String sansAccents(const String& s) {
  static const char* avec[]  = { "à","â","ä","á","é","è","ê","ë","î","ï","í","ô","ö","ó","ù","û","ü","ú","ç","ñ",
                                 "À","Â","Ä","É","È","Ê","Ë","Î","Ï","Ô","Ö","Ù","Û","Ü","Ç","’","œ","Œ","–","—" };
  static const char* sans[]  = { "a","a","a","a","e","e","e","e","i","i","i","o","o","o","u","u","u","u","c","n",
                                 "A","A","A","E","E","E","E","I","I","O","O","U","U","U","C","'","oe","OE","-","-" };
  String r = s;
  for (size_t i = 0; i < sizeof(avec) / sizeof(avec[0]); i++) r.replace(avec[i], sans[i]);
  // Tout autre caractère non ASCII devient « ? » (octets de tête) ou disparaît (octets de suite)
  String propre;
  for (size_t i = 0; i < r.length(); i++) {
    uint8_t c = r[i];
    if (c < 0x80) propre += (char)c;
    else if (c >= 0xC0) propre += '?';
  }
  return propre;
}

bool initEcran() {
  Wire.begin(21, 22);
  Wire.setClock(400000);
  ecranOk = ecran.begin(SSD1306_SWITCHCAPVCC, ECRAN_ADRESSE);
  if (!ecranOk) {
    Serial.println("Erreur : écran OLED introuvable (vérifie le câblage et l'adresse 0x3C/0x3D)");
    return false;
  }
  ecran.setTextWrap(false);
  ecran.setTextColor(SSD1306_WHITE);
  ecran.clearDisplay();
  ecran.setTextSize(2);
  ecran.setCursor(4, 20);
  ecran.print("Lecteur");
  ecran.display();
  return true;
}

String minSec(uint32_t s) {
  char tampon[12];
  snprintf(tampon, sizeof(tampon), "%lu:%02lu", (unsigned long)(s / 60), (unsigned long)(s % 60));
  return tampon;
}

// Fait défiler un texte trop long pour l'écran (21 caractères en taille 1)
String defiler(const String& texte, uint32_t instant) {
  const int largeur = 21;
  if ((int)texte.length() <= largeur) return texte;
  String boucle = texte + "   ";
  int decalage = (instant / 300) % boucle.length();
  return (boucle + boucle).substring(decalage, decalage + largeur);
}

void dessinerEcran(const Affichage& a) {
  if (!ecranOk) return;
  uint32_t maintenant = millis();
  ecran.clearDisplay();
  ecran.setTextSize(1);

  // Volume en grand pendant 1,5 s après un changement
  if (maintenant - a.volumeChangeA < 1500) {
    ecran.setCursor(28, 8);
    ecran.print("Volume");
    ecran.setTextSize(3);
    ecran.setCursor(a.volume < 10 ? 56 : 46, 24);
    ecran.print(a.volume);
    ecran.drawRect(4, 54, 120, 8, SSD1306_WHITE);
    ecran.fillRect(4, 54, 120 * a.volume / 21, 8, SSD1306_WHITE);
    ecran.display();
    return;
  }

  // En-tête : mode, puis pictogrammes à droite
  ecran.setCursor(0, 0);
  ecran.print(sansAccents(a.entete).substring(0, 13));
  String droite;
  if (a.minuteur) droite += "Z" + String(a.minuteur) + " ";
  if (a.aleatoire) droite += "A ";
  droite += a.pause ? "||" : ">";
  ecran.setCursor(ECRAN_LARGEUR - droite.length() * 6, 0);
  ecran.print(droite);
  ecran.drawFastHLine(0, 10, ECRAN_LARGEUR, SSD1306_WHITE);

  ecran.setCursor(0, 16);
  ecran.print(defiler(sansAccents(a.ligne1), maintenant));
  ecran.setCursor(0, 28);
  ecran.print(defiler(sansAccents(a.ligne2), maintenant));

  // Barre de progression (carte SD seulement : une radio n'a pas de durée)
  if (a.duree > 0) {
    ecran.drawRect(0, 42, ECRAN_LARGEUR, 6, SSD1306_WHITE);
    ecran.fillRect(0, 42, min<uint32_t>(ECRAN_LARGEUR, ECRAN_LARGEUR * a.temps / a.duree), 6, SSD1306_WHITE);
    ecran.setCursor(0, 54);
    ecran.print(minSec(a.temps) + " / " + minSec(a.duree));
  }
  String vol = "Vol " + String(a.volume);
  ecran.setCursor(ECRAN_LARGEUR - vol.length() * 6, 54);
  ecran.print(vol);
  ecran.display();
}
