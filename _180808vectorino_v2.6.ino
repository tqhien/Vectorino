// Par JCM le 20/04/2018. version 1.0 Beta :
//à optimiser et fiabiliser :-)

// Par JCM le 04/05/2018. version 1.1 :
// MODIFS PROGRAMME
//ajout gestion de LED batterie faible.
//ajout limites dans le mode vector : km maxi 99km90 et temps maxi 99min pour éviter bugs d'affichage
//changement des noms. Vector'ino devient vectorino. Alfarduino devient alfarino.
// MODIFS HARDWARE
//ajout LED batterie faible +R270ohm surp pin D4
//ajout retour tension batterie sur pin A0
//ajout bouton luminosité jour/nuit. juste du cablage (resistance 270ohms sur pin led ecran bypassée par bouton)

// Par JCM le 9/05/2018. Version 1.2 :
//MODIFS PROGRAMME
//la modif prévue était intégration de carte sd pour sauvegarde des paramètres.
//impossible cause la gestion des fichiers d'écriture fait dépasser la taille mémoire du programme complet....
//il faudrait réécrire le programme plus rationnellement pour gagner de la place mais bon :-(
//cependant les modifs ont été apportées pour réserver les pins hardware dans une éventuelle future évolution (il fallait libérer D4) 
// MODIFS HARDWARE
// led batterie faible passe sur pin D5 pour libérer pin D4
//le BPRST passe sur pin A1 pour libérer D5...
//l'éventuel futur lecteur de carte SD serait alors connecté sur : SDCS-D4; SDMOSI-D11; SDMISO-D12; SDSCK-D13
//a partir de cette version, j'inverse aussi les jack : femelle coté vectorino et GND sur la tresse

// Par JCM le 9/05/2018. Version 1.3 :
//MODIFS PROGRAMME
// intégration des sauvegarde de paramètres par l'EEPROM (pas besoin de carte sd en fait!! c'est bien plus simple avec l'EEPROM)
// ajout de paramètre coefficient correcteur affichage vitesse
//MODIFS HARDWARE : aucune

// Par JCM le 10/05/2018. Version 2.0 :
//MODIFS PROGRAMME
// Ajout du mode "route" avec totalisateur mémorisé. Modif du menu d'accueil, des écrans de réglage..
//modif
//MODIFS HARDWARE : aucune

// Par JCM le 12/05/2018. Version 2.1 :
//MODIFS PROGRAMME
// Ajout mémorisation du partiel en mode route
//ajout mémorisation des chronos du mode piste
//ajout écran pour reset de tous les chronos
//modif des procédures de départ (ignorer le chrono de départ)
//MODIFS HARDWARE : aucune. prévoir de brancher le bouton inutilisé sur prise jack pour utilisation en bouton externe (feux additionnels par exemple).

// Par JCM le 12/06/2018. Version 2.2 :
//MODIFS PROGRAMME
//modification des incréments km en mode rallye : +/-20m au lieu de +/-10m
//correction de bug : affichage vitesse moyenne en mode rallye crée une erreur si reset en roulant. résolu.
//ajout limite maxi a la vmoyenne pour éviter les bugs d'affichage.
//MODIFS HARDWARE : aucune. prévoir de passer aux boutons "stricker". prévoir de supprimer le réglage luminosité.


// Par JCM le 14/06/2018. Version 2.3 :
//MODIFS PROGRAMME
//modification des incréments km en mode rallye : +/-50m au lieu de +/-20m
//MODIFS HARDWARE : aucune. prévoir de passer aux boutons "stricker". prévoir de supprimer le réglage luminosité.

// Par JCM le 21/06/2018. Version 2.4 :
//MODIFS PROGRAMME
//modification gestion vitesse instantanée pour précision et éviter les affichages bidons en appui UP ou DWN
//MODIFS HARDWARE : aucune. prévoir de passer aux boutons "stricker". prévoir de supprimer le réglage luminosité.

// Par JCM le 21/06/2018. Version 2.5 :
//MODIFS PROGRAMME
//modification gestion mode départ alfano : chrono reste a zero en mode départ
//MODIFS HARDWARE : aucune. prévoir de passer aux boutons "stricker". prévoir de supprimer le réglage luminosité.

// Par JCM le 08/08/2018. Version 2.6:
//MODIFS PROGRAMME
//résolution bug affichage vitesse max au moment du reset en mode alfano
//MODIFS HARDWARE : aucune. prévoir de passer aux boutons "stricker". prévoir de supprimer le réglage luminosité.

//principe du programme : 
//gestion des écrans de paramétrage dans le void setup
//et un booleen "modevector" pour choix dans le void loop du programme vector ou alfano
//les 2 capteurs sont gérés par des "interrupt" pour garantir la réactivité de lecture

//bibliothèque pour l'accès a la mémoire EEPROM (mémoire conservée à l'extinction de l'arduino)
#include <EEPROM.h>

//bibliothèques pour écran ST7735
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library
#include <SPI.h>
//affectation des pins pour l'écran ST7735
#define TFT_CS     10
#define TFT_RST    8
#define TFT_DC     9
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS,  TFT_DC, TFT_RST);
#define TFT_SCLK 13
#define TFT_MOSI 11  


//choix des pins de entrées TOR
const int up = 6; //bouton UP
const int dwn =7; //bouton DOWN
const int capteur = 2; //capteur de roue DOIT ETRE SUR PIN 2 CAR GERE PAR ROUTINE INTERRUPT
const int capteurbande = 3; //capteur de bande magnétique circuit DOIT ETRE SUR PIN 3 CAR GERE PAR ROUTINE INTERRUPT
//choix des pins entrée ANA
const char rst = A1; //bouton rst rentre sur un pin analogique utilisé comme un digital,  car plus de place en pin digital
const int voltbat = 0; //retour tension batterie sur pin A0
//choix des pins de sortie TOR
const int batfaible = 5; //pin de la LED batterie faible (devient 5 etait 4 dans version précédente)

//déclaration et initialisation des variables
long cm = 0; //distance parcourue en cm
unsigned int affm = 0; //pour affichage m
unsigned int affkm = 0; //pour affichage km
unsigned long tps = 0; //temps du chrono en centiemes
unsigned long tpsinit; //pour remise a zero du chrono. Sera précision en centième pour Alfano, et en secondes pour Vector
unsigned long topchrono = 0 ; //meilleur chrono absolu depuis le reset
unsigned int affmin = 0 ; //pour affichage min
unsigned int affsec = 0 ; //pour affichage sec
unsigned int affcent = 0 ; //pour affichage centiemes
unsigned int roue = 186 ; //diametre roue    VALEUR PAR DEFAUT (mesurée sur la ktm, sur roue avant en 17"). AJUSTEE PAR ECRAN D'ACCUEIL
unsigned long j = millis() ; //compteur pour calcul vitesse instantanée
unsigned long k = millis() ; //compteur pour calcul vitesse max
unsigned long cmavant = 0 ; //relevé de distance pour calcul vitesse
unsigned long vinstant = 0 ; //vitesse instantanée
unsigned int vmoy = 0 ; //vitesse moyenne
unsigned int vmax = 0 ; //vitesse maxi du tour
unsigned int vmaxabsolue = 0 ; //vitesse maxi sur toute la session
unsigned int tours = 1 ; //nombre de tours
char buffer[100]; //variable pour gestion de l'affichage (nécessaire pour afficher les chiffres au format 2 ou 3 digits via sprintf)
boolean modelecture = LOW ; //pour gérer mode lecture / mode enregistrement
unsigned long chrono = 0 ; //pour enregistrement des chronos des tours effectués
unsigned int afftour = 1 ; //pour choisir le tour affiché en mode lecture
unsigned int affvmax = 0; //pour enregistrement des vmax des tours effectués
unsigned int nbbandes = 1; //pour choix du nombre de bandes magnétiques sur le circuit
unsigned int passagebande = 0; //compte le nombre de passage sur la bande avant de valider le tour
unsigned long filtre = millis() ; //pour filtrer la lecture capteur de bande
boolean depart = HIGH; //pour empécher l'enregistrement du topchrono au premier passage ou après un reset
unsigned int modevector = 0; //pour choix entre alfano ou vector ou strino
unsigned int nbaimants = 2; //pour choix du nombre de bandes magnétiques sur le circuit
unsigned int correcv = 95; //coefficient correcteur de l'affichage vitesse
unsigned long totalis = 0 ; //pour mise a jour du totalisateur km
unsigned long totaliskm = 10000; //totalisateur km
unsigned int afftotalismkm = 0; //créé cause bug affichage avec les km qui dépassent 32700 (a cause de la fonction sprintf...)
unsigned int afftotaliskm = 0; //créé cause bug affichage avec les km qui dépassent 32700 (a cause de la fonction sprintf...)
boolean effachrono = LOW; //pour choix de reset chronos
unsigned int q = 1; //pour boucle effacement chronos

//NOTA Sur l'utilisation de "sprintf". j'aurais pu afficher directement la valeur avec tft.print(affmin) par exemple.
//mais dans ce cas ça n'affiche que les digits actifs. par exemple 3 et pas 03.
//ce qui pose un problème pour "effacer" les caractères suivants en réécrivant par dessus.
//pour résoudre ça je fais sprintf(buffer, "%02d", affmin); ce qui transforme le nombre entier affmin en une chaine de 2 caractères
//qui se range dans la variable "buffer", et il me reste a afficher "buffer" en faisant tft.print(buffer)

//NOTA penser à alléger A MORT les fonctions liées a l'écran dans void loop (toutes les fonctions type tft.qqchose)
//les appels à l'écran ralentissent GRAVE le programme

//NOTA y aurait grave besoin de faire le ménage pour limiter le nombre de variables et simplifier le programme. mais bon là...pas le temps && pas le courage

void setup() {
  
//déclaration des entrées TOR et de leur état de départ. NOTA : déclarée HIGH pour état stable qui demande une mise a LOW (mise a la masse, au 0v).
pinMode(up, INPUT); digitalWrite(up, HIGH);
pinMode(dwn, INPUT); digitalWrite(dwn, HIGH);
pinMode(rst, INPUT); digitalWrite(rst, HIGH);
pinMode(capteur, INPUT); digitalWrite(capteur, HIGH);
pinMode(capteurbande, INPUT); digitalWrite(capteurbande, HIGH);
//déclaration des sorties TOR et de leur état de départ
pinMode(batfaible, OUTPUT); digitalWrite(batfaible, LOW);

//initialisation de l'écran
Serial.begin(9600);
Serial.println("void setup lance");
tft.initR(INITR_BLACKTAB); // initialisation de l'écran 1.8
tft.fillScreen(ST7735_BLACK); // écran tout noir


//récupération des paramètres rentrés précédemments, qui sont stockés dans l'EEPROM
EEPROM.get(900,modevector);
EEPROM.get(910,roue);
EEPROM.get(920,nbaimants);
EEPROM.get(930,nbbandes);
EEPROM.get(940,correcv);
EEPROM.get(950,totaliskm);


//juste en mode codage pour voir les valeurs qui remontent via le moniteur serie
Serial.println("recup des données EEPROM lance");
Serial.println(modevector);
Serial.println(roue);
Serial.println(nbaimants);
Serial.println(nbbandes);
Serial.println(correcv);
Serial.println(totaliskm);




//ECRANS DE DEMARRAGE ET DE PARAMETRAGE :

if (digitalRead(up) == LOW) {  //ça permet de passer par les écrans de paramétrage si UP est maintenu au démarrage
  
//écran d'accueil :-) pour choix mode vectorino
tft.setTextColor(ST7735_MAGENTA,ST7735_BLACK);
tft.setTextSize(2);
tft.setCursor(0,0);
tft.println(" VECTORINO");
tft.println("   Mode?");
tft.setCursor(0,130);
tft.println(" puis 'OK'");
tft.setTextSize(1);
tft.setCursor(30,150);
tft.println("v2.6-08/2018");
tft.setTextSize(2);

while (digitalRead(rst) == HIGH) {

  if (modevector==1) {
tft.setTextColor(ST7735_YELLOW,ST7735_BLACK);
tft.setCursor(0,45);
tft.println(" ->Rallye");
tft.setTextColor(ST7735_MAGENTA,ST7735_BLACK);
tft.setCursor(0,75);
tft.println(" ->Piste");
tft.setCursor(0,105);
tft.println(" ->Route");

  }
     if (modevector==2) {
tft.setTextColor(ST7735_MAGENTA,ST7735_BLACK);
tft.setCursor(0,45);
tft.println(" ->Rallye");
tft.setTextColor(ST7735_YELLOW,ST7735_BLACK);
tft.setCursor(0,75);
tft.println(" ->Piste");
tft.setTextColor(ST7735_MAGENTA,ST7735_BLACK);
tft.setCursor(0,105);
tft.println(" ->Route");

  } 

     if (modevector==3) {
tft.setTextColor(ST7735_MAGENTA,ST7735_BLACK);
tft.setCursor(0,45);
tft.println(" ->Rallye");
tft.setCursor(0,75);
tft.println(" ->Piste");
tft.setTextColor(ST7735_YELLOW,ST7735_BLACK);
tft.setCursor(0,105);
tft.println(" ->Route");

  } 

  if (digitalRead(up) == LOW) { modevector--;}
  if (digitalRead(dwn) == LOW) { modevector++;}
  if (modevector<2) {modevector=1;}
  if (modevector>2) {modevector=3;}

}

EEPROM.put(900,modevector); //Stocke la valeur pour l'avoir au prochain redémarrage


tft.fillScreen(ST7735_BLACK); // écran tout noir

//écran choix périmetre de roue
tft.setTextColor(ST7735_MAGENTA,ST7735_BLACK);
tft.setCursor(0,10);
tft.println(" perimetre");
tft.println(" de roue?");
tft.setCursor(55,50);
tft.println(" cm");
tft.setCursor(0,75);
tft.println("  nombre");
tft.println("d'aimants?");
tft.setCursor(55,110);
tft.println(nbaimants);
tft.setCursor(0,130);
tft.println(" puis 'OK'");
tft.setTextSize(1);
tft.setCursor(30,150);
tft.println("v2.6-08/2018");
tft.setTextSize(2);

//gestion du diamètre de roue : affichage et ajustement sur l'écran d'accueil
tft.setTextColor(ST7735_YELLOW,ST7735_BLACK);
while (digitalRead(rst) == HIGH) { tft.setCursor(25,50); sprintf(buffer, "%03d", roue); tft.print(buffer); delay(100); //tant que j'appuie pas sur le bouton rst, je reste sur cet écran
  if (digitalRead(dwn) == LOW) {roue--;}
  if (roue<2) {roue=1;}
  if (digitalRead(up) == LOW) {roue++;}
  if (roue>300) {roue=300;}
}

EEPROM.put(910,roue); //Stocke la valeur pour l'avoir au prochain redémarrage


//gestion du choix nombre d'aimants
delay(300);
tft.setTextColor(ST7735_MAGENTA,ST7735_BLACK);
tft.setCursor(25,50); sprintf(buffer, "%03d", roue); tft.print(buffer);
tft.setTextColor(ST7735_YELLOW,ST7735_BLACK);
while (digitalRead(rst) == HIGH) { tft.setCursor(55,110); sprintf(buffer, "%01d", nbaimants); tft.print(buffer); delay(100); //tant que j'appuie pas sur le bouton rst, je reste sur cet écran
  if (digitalRead(dwn) == LOW) {nbaimants--;}
  if (nbaimants<2) {nbaimants=1;}
  if (digitalRead(up) == LOW) {nbaimants++;}
  if (nbaimants>9) {nbaimants=9;}
}

EEPROM.put(920,nbaimants); //Stocke la valeur pour l'avoir au prochain redémarrage


tft.fillScreen(ST7735_BLACK); // écran tout noir

//écran choix coefficient correcteur de vitesse instantanée et max
tft.setTextColor(ST7735_MAGENTA,ST7735_BLACK);
tft.setCursor(0,10);
tft.println("Correction");
tft.println("affichage");
tft.println(" vitesse?");
tft.setCursor(60,70);
tft.println(" %");
tft.setCursor(0,130);
tft.println(" puis 'OK'");
tft.setTextSize(1);
tft.setCursor(30,150);
tft.println("v2.6-08/2018");
tft.setTextSize(2);

//gestion du coef correcteur de vitesse : affichage et ajustement sur l'écran d'accueil
tft.setTextColor(ST7735_YELLOW,ST7735_BLACK);
while (digitalRead(rst) == HIGH) { tft.setCursor(30,70); sprintf(buffer, "%03d", correcv); tft.print(buffer); delay(100); //tant que j'appuie pas sur le bouton rst, je reste sur cet écran
  if (digitalRead(dwn) == LOW) {correcv--;}
  if (correcv<=50) {correcv=50;}
  if (digitalRead(up) == LOW) {correcv++;}
  if (correcv>=150) {correcv=150;}
}


EEPROM.put(940,correcv); //Stocke la valeur pour l'avoir au prochain redémarrage


if (modevector == 2) {
//écran d'accueil choix bandes magnétiques si mode alfano
tft.fillScreen(ST7735_BLACK); // écran tout noir
tft.setTextColor(ST7735_MAGENTA,ST7735_BLACK);
tft.setCursor(0,0);
tft.println();
tft.println("Mode piste");
tft.println();
tft.println(" combien");
tft.println("de bandes?");
tft.setCursor(30,100);
tft.println("bande(s)");
tft.setCursor(0,130);
tft.println(" puis 'OK'");
tft.setTextSize(1);
tft.setCursor(30,150);
tft.println("v2.6-08/2018");
tft.setTextSize(2);
//gestion du nombre de bandes : affichage et ajustement sur l'écran d'accueil
tft.setTextColor(ST7735_YELLOW,ST7735_BLACK);
while (digitalRead(rst) == HIGH) { tft.setCursor(10,100); tft.print(nbbandes); delay(100); //tant que j'appuie pas sur le bouton rst, je reste sur cet écran
  if (digitalRead(dwn) == LOW) {nbbandes--;}
  if (digitalRead(up) == LOW) {nbbandes++;}
  if (nbbandes<=1) {nbbandes=1;}
  if (nbbandes>=9) {nbbandes=9;}
}

EEPROM.put(930,nbbandes); //Stocke la valeur pour l'avoir au prochain redémarrage



//écran d'accueil reset de tous les chronos
tft.fillScreen(ST7735_BLACK); // écran tout noir
tft.setTextColor(ST7735_MAGENTA,ST7735_BLACK);
tft.setCursor(0,0);
tft.println();
tft.println("Mode piste");
tft.println();
tft.println(" Effacer");
tft.println(" chronos?");
tft.setCursor(45,85);
tft.println("OUI");
tft.setCursor(45,105);
tft.println("NON");
tft.setCursor(0,130);
tft.println(" puis 'OK'");
tft.setTextSize(1);
tft.setCursor(30,150);
tft.println("v2.6-08/2018");
tft.setTextSize(2);

while (digitalRead(rst) == HIGH) {

  if (effachrono==HIGH) {
  tft.setTextColor(ST7735_YELLOW,ST7735_BLACK);
  tft.setCursor(45,85);
  tft.println("OUI");
  tft.setTextColor(ST7735_MAGENTA,ST7735_BLACK);
  tft.setCursor(45,105);
  tft.println("NON");
  }

  if (effachrono==LOW) {
  tft.setTextColor(ST7735_MAGENTA,ST7735_BLACK);
  tft.setCursor(45,85);
  tft.println("OUI");
  tft.setTextColor(ST7735_YELLOW,ST7735_BLACK);
  tft.setCursor(45,105);
  tft.println("NON");
  }

  if (digitalRead(up) == LOW) { effachrono=HIGH;}
  if (digitalRead(dwn) == LOW) { effachrono=LOW;}
    
  }

//éffaceement de tous les chronos :
if (effachrono==HIGH) {
  q=1;
  while (q<81) {
    EEPROM.put(q*10,0);
    EEPROM.put((q*10+5),0);
    q++;
  }
  EEPROM.put(970,1);
  }
}


if (modevector == 3) {

//écran choix valeur totalisateur

tft.fillScreen(ST7735_BLACK); // écran tout noir

tft.setTextColor(ST7735_MAGENTA,ST7735_BLACK);
tft.setCursor(0,10);
tft.println("kilometres");
tft.println("totaux?");
tft.setCursor(85,70);
tft.println("km");
tft.setCursor(0,130);
tft.println(" puis 'OK'");
tft.setTextSize(1);
tft.setCursor(30,150);
tft.println("v2.6-08/2018");
tft.setTextSize(2);


//gestion du totalisateur : affichage et ajustement sur l'écran d'accueil
tft.setTextColor(ST7735_YELLOW,ST7735_BLACK);
while (digitalRead(rst) == HIGH) { //tant que j'appuie pas sur le bouton rst, je reste sur cet écran
  afftotalismkm = (totaliskm/1000);
  afftotaliskm = (totaliskm-afftotalismkm*1000);
    tft.setCursor(15,70); sprintf(buffer, "%02d", afftotalismkm); tft.print(buffer); sprintf(buffer, "%03d", afftotaliskm); tft.print(buffer); delay(100); 
  if (digitalRead(dwn) == LOW) {(totaliskm=(totaliskm-100));}
  if (totaliskm<100) {totaliskm=100;}
  if (digitalRead(up) == LOW) {(totaliskm=(totaliskm+100));}
  if (totaliskm>99900) {totaliskm=99900;}
}

EEPROM.put(950,totaliskm); //Stocke la valeur pour l'avoir au prochain redémarrage

}
}

//PREPARATION DES ECRANS DE FONCTIONNEMENT :


//mise en place de l'écran pour mode rallye

if (modevector == 1) {

tft.fillScreen(ST7735_BLACK); // écran tout noir
//affichage des inscriptions constantes
tft.setTextColor(ST7735_BLUE,ST7735_BLACK); // choix couleur écriture des inscriptions constantes
tft.setTextSize(2);
tft.setCursor(50,10);
tft.print("km");
tft.setCursor(50,47);
tft.print("mn");
tft.setCursor(75,84);
tft.print("kmh");
tft.setCursor(75,121);
tft.print("kmh~");
tft.setCursor(0,145);
tft.print("Max");
tft.setCursor(75,145);
tft.print("kmh");
tft.setTextSize(4);
tft.setTextColor(ST7735_YELLOW,ST7735_BLACK); // choix couleur écriture des inscriptions variables
//penser a choisir écriture sur fond de meme couleur que le fond de départ
}


//mise en place de l'écran pour mode piste

if (modevector == 2) {

tft.fillScreen(ST7735_BLACK); // écran tout noir

//affichage des inscriptions constantes
tft.setTextColor(ST7735_BLUE,ST7735_BLACK); // choix couleur écriture des inscriptions constantes
tft.setTextSize(2);
tft.setCursor(18,0);
tft.print("'");
tft.setCursor(70,0);
tft.print("''");
tft.setCursor(18,38);
tft.print("'");
tft.setCursor(70,38);
tft.print("'");
tft.setCursor(75,76);
tft.print("kmh");
tft.setCursor(75,114);
tft.print("kmh");
tft.setCursor(75,126);
tft.print("max");
tft.setCursor(0,144);
tft.print("tour #");
tft.setTextSize(4);
tft.setTextColor(ST7735_YELLOW,ST7735_BLACK); // choix couleur écriture des inscriptions variables
//penser a choisir écriture sur fond de meme couleur que le fond de départ

}


//mise en place de l'écran pour mode route

if (modevector == 3) {

tft.fillScreen(ST7735_BLACK); // écran tout noir

//affichage des inscriptions constantes
tft.setTextColor(ST7735_BLUE,ST7735_BLACK); // choix couleur écriture des inscriptions constantes
tft.setTextSize(2);
tft.setCursor(75,55);
tft.print("km/h");
tft.setCursor(62,80);
tft.print(".  km");
tft.setCursor(98,120);
tft.print("km");
tft.setCursor(80,100);
tft.setTextSize(1);
tft.print("partiel");
tft.setCursor(93,140);
tft.print("total");
tft.setTextSize(4);
tft.setTextColor(ST7735_YELLOW,ST7735_BLACK); // choix couleur écriture des inscriptions variables
//penser a choisir écriture sur fond de meme couleur que le fond de départ

}




//je réinitialise les variables basées sur le temps juste avant de commencer loop
if (modevector == 2) {tpsinit = (millis()/10);} //en mode alfano je suis précis au centième
if (modevector == 1) {tpsinit = (millis()/1000);} //en mode vector je suis précis à la seconde
j = millis() ;
k = millis();
filtre = millis() ;
passagebande = (nbbandes-1); //pour déclenchement d'enregistrement du 1er tour dès le passage sur la 1ere bande
if (modevector == 3) {EEPROM.get(960,cm); cmavant = cm;} //pour récupérer la valeur du trip précédente en mode route et initialisation du cmavant avec
EEPROM.get(970,tours); //pour repartir du bon numéro de tour (placé ici en aval du choix reset qui l'aurait éventuellement remis a 1

//Surveille état de capteur de roue pour lancer la routine "increment" sur front descendant 
attachInterrupt(digitalPinToInterrupt(capteur), increment, FALLING);

//Surveille état de capteur de bande magnétique pour lancer la routine "resettour" sur front descendant 
attachInterrupt(digitalPinToInterrupt(capteurbande), resettour, FALLING);

//affiche le chrono vierge au départ en mode alfano (sans ça l'affichage n'apparait pas tant qu'il y a pas eu de "capteur bande"
if (modevector == 2) {affiche_chrono_et_tour();}

Serial.println("fin de void setup");
}

//routine appelée par l'interruption déclenchée par le front montant de capteur de roue
void increment() {
  cm = (cm+(roue/nbaimants)); //incrémente la distance a chaque passage de l'aimant, selon le perimetre de roue et le nombre d'aimants
  totalis = (totalis+(roue/nbaimants)); //incrémente la distance a chaque passage de l'aimant, selon le perimetre de roue et le nombre d'aimants 
}

//routine appelée par l'interruption déclenchée par le front montant de capteur de bande magnétique
//pour affichage meilleur chrono et nombre de tours
void resettour() {
if (modevector == 2) {

  if ((millis()-2000) >filtre) { //pour ignorer le déclenchement s'il se fait dans les 2 sec du précédent (empèche les déclenchements multiples intempestifs)
  passagebande++;
  filtre = millis();}
  if (passagebande>=nbbandes) { //pour ne lancer que si on a passé toutes les bandes magnétiques
    //mise a jour du topchrono a chaque tour
    if (depart==LOW) { //je ne fais le processus incrémentation/enregistrement que si je ne suis pas en mode "départ" c"est a dire après un reset
    if ((topchrono<500) || (tps < topchrono)) { topchrono = tps;} //(on force la mise a jour si le chrono est irréaliste (moins de 5sec)
     EEPROM.put((tours*10),tps); //stockage du chrono dans l'EEPROM
     EEPROM.put(((tours*10)+5),vmax); //stocke la vmax dans l'EEPROM
     EEPROM.put(970,tours); //sotcke le numéro du tour en cours dans l'EEPROM
     tours++; //incrémente le nombre de tour
     if (tours >80) {tours = 1;}//on limite le nombre de tours a 80
     vmax = 0; //réinitialise la vmax du tour écoulé
     passagebande=0; //réinitialise le nombre de passage sur bande sur 1 tour
     tpsinit = (millis()/10); //reset le temps pour calcul chrono du tour
    } else {depart = LOW; vmax=0; passagebande=0; tpsinit = (millis()/10); } //si c'est juste après un reset, en mode "départ", je réinitialise mais je n'enregitre rien
  }
  affiche_chrono_et_tour();
}
}

void affiche_chrono_et_tour() { //pour afficher le top chrono en cours et le nombre de tours en cours. car pour le reste c'est rafraichi en permanence
  if (modevector == 2) {
  affmin = (topchrono/6000); //affmin étant un entier, la division ne donne que les minutes pleines
  affsec = ((topchrono-affmin*6000)/100); //pour n'avoir que les secondes, je prends le temps total moins les minutes pleines x 60.
  affcent = (topchrono-((affmin*6000)+(affsec*100))); //et rebelotte pour avoir les centiemes
  tft.setCursor(0,38); sprintf(buffer, "%01d", affmin); tft.print(buffer);
  tft.setCursor(27,38); sprintf(buffer, "%02d", affsec); tft.print(buffer);
  tft.setCursor(79,38); sprintf(buffer, "%02d", affcent); tft.print(buffer);
  tft.setTextSize(2);
  tft.setCursor(80,144); sprintf(buffer, "%02d", tours); tft.print(buffer);
  tft.setTextSize(4);
  
}
}



void loop() {

Serial.println("void loop lance");

//SURVEILLANCE DE LA TENSION DE LA BATTERIE POUR ALLUMAGE LED BATTERIE FAIBLE
if (analogRead(voltbat) < 760) { digitalWrite(batfaible, HIGH);}
else {digitalWrite(batfaible, LOW);}
//si tension est moins de 3,4v (5v=1023; suite a test 760environ équivalent à 3.4v) j'allume la led, sachant que la batterie chargée fait 3.7 a 3.8 et qu'elle coupe a ?(3.0v apparemment)


//INCREMENTATION DU TOTALISATEUR QUEL QUE SOIT LE MODE (pour que le totalisateur du mode route reste a jour meme si je roule dans un autre mode
if (totalis>100000) { // a chaque fois que j'ai parcouru 1km, j'incrémente le totalisateur et je réinitialise totalis
  totaliskm++;
  EEPROM.put(950,totaliskm);
  if (modevector == 3) {EEPROM.put(960,cm);} //nota je mémorise cm pour mémoriser le trip en mode route seulement
  totalis = 0;
  
  
}


//PROGRAMME DU MODE VECTORINO (MODE RALLYE) : 

if (modevector == 1) {

//gestion du reset
if (digitalRead(rst) == LOW) {cm = 0; cmavant = cm; vmoy = 0; tpsinit = (millis()/1000); vmax = 0;} //reset le compteur et les données pour calcul vmoy, vmax et le chrono

//gestion de l'ajustement du trip
if (digitalRead(up) == LOW) {cm = (cm+5000); cmavant = cm; j = millis(); } //corrige en plus et fige momentanément le calcul de Vitesse instantanée
if (digitalRead(dwn) == LOW) {cm = (cm-5000); cmavant = cm; j = millis(); } //corrige en moins et fige momentanément le calcul de Vitesse instantanée
if (cm <= 0) {cm = 0; cmavant = cm;} //pour que la correction en moins s'arrête a zero

//affichage distance
if (cm > 9990000) {cm = 0; cmavant = cm;} //la distance retombe a zéro au dela de 99km90. pour éviter les bugs d'affichage
affkm = cm/100000;
affm = (cm-affkm*100000)/1000;
tft.setCursor(0,0); sprintf(buffer, "%02d", affkm); tft.print(buffer);
tft.setCursor(78,0); sprintf(buffer, "%02d", affm); tft.print(buffer);

//affiche vitesse moyenne
if ((cm<2000)||(tps<2)) {vmoy = 0;} //maintient la vmoy a 0 avant les 20 premiers metres, ou 2sec après reset ou démarrage, pour éviter valeurs bidons.
else { vmoy = ((cm/((millis()/1000)-tpsinit))*3600/100000);} //basé sur l'afficheur de distance. se réinitialise en même temps.
if (vmoy > 400) {vmoy=400;} //maintient la vmoy a 400 si trop élevée pour éviter les valeurs bidon
tft.setCursor(0,111); sprintf(buffer, "%03d", vmoy); tft.print(buffer);

//affichage minuterie
tps = ((millis()/1000)-tpsinit); //temps depuis reset
if (tps > 5940) {tps = 0;} //si le temps dépasse 99min, il repasse a zero, c'est juste pour éviter les bugs d'affichage
affmin = (tps/60); //affmin étant un entier, la division ne donne que les minutes pleines
affsec = (tps-affmin*60); //pour n'avoir que les secondes, je prends le temps total moins les minutes pleines x 60.
tft.setCursor(0,37); sprintf(buffer, "%02d", affmin); tft.print(buffer);
tft.setCursor(78,37); sprintf(buffer, "%02d", affsec); tft.print(buffer);

//affichage vitesse instantanée
if ((millis())-2000<j) { //si délai inférieur a 2 sec, j'affiche la vitesse, si au dela, je recalcule la vitesse et réinitialise le temps
  tft.setCursor(0,74); sprintf(buffer, "%03d", vinstant); tft.print(buffer);}
else {
  vinstant = (cm*36*correcv-cmavant*36*correcv); //la formule était vinstant = ((((cm-cmavant)*3600)/100000)*correcv/100) mais modifiée pour faire la division après pour garder la précision
  vinstant = vinstant/200000; //ça équivaut à un calcul de moyenne sur les 2 dernières secondes
  if (vinstant > 400) {vinstant = 400;} //bridage de la vitesse a 400 pour éviter les bugs d'affichage
  j = (millis());
  cmavant = cm;}

//affichage vmax
  if (vinstant > vmax) {vmax = vinstant;} //la Vmax se met a jour quand la Vinstant la dépasse
  tft.setTextSize(2);
  tft.setCursor(35,145); sprintf(buffer, "%03d", vmax); tft.print(buffer);
  tft.setTextSize(4);


}


//PROGRAMME DU MODE ALFARINO (MODE PISTE) :

if (modevector == 2) {

//gestion du reset total
if (digitalRead(rst) == LOW) {
  cm=0;
  cmavant = cm;
  tpsinit=(millis()/10);
  vmax=0;
  vmaxabsolue=0;
  topchrono=0;
  passagebande = (nbbandes-1);
  depart = HIGH;
  affiche_chrono_et_tour();}


//affichage chrono en cours
if (depart == HIGH) { tps = 0;
tft.setCursor(0,0); tft.print("!"); tft.setCursor(27,0); tft.print("GO"); tft.setCursor(79,0); tft.print("!!");} //si on est en mode départ, on affiche "GO"
else {tps = ((millis()/10)-tpsinit); //si on est pas en mode départ, on calcule le temps depuis reset chrono (en centiemes de sec)
affmin = (tps/6000); //affmin étant un entier, la division ne donne que les minutes pleines
affsec = ((tps-affmin*6000)/100); //pour n'avoir que les secondes, je prends le temps total moins les minutes pleines x 60.
affcent = (tps-((affmin*6000)+(affsec*100))); //et rebelotte pour avoir les centiemes
tft.setCursor(0,0); sprintf(buffer, "%01d", affmin); tft.print(buffer);
tft.setCursor(27,0); sprintf(buffer, "%02d", affsec); tft.print(buffer);
tft.setCursor(79,0); sprintf(buffer, "%02d", affcent); tft.print(buffer);}

// la détection de fin de tour est faite par le signal "capteurbande" mais peut etre fait à la main par le bouton "up" grace a ça :
if (digitalRead(up) == LOW) { resettour(); }
// si le tour atteint 10 min, ça repart à zéro (pour éviter un bug d'affichage si on laisse tourner plus de 10minutes sans déclenchement)
if (tps > 59900) { tpsinit = (millis()/10); }


//affichage vitesse instantanée
if ((millis())-2000<j) { //si délai inférieur a 2 sec, j'affiche la vitesse, si au dela, je recalcule la vitesse et réinitialise le temps
  tft.setCursor(0,76); sprintf(buffer, "%03d", vinstant); tft.print(buffer);}
else {
  vinstant = (cm*36*correcv-cmavant*36*correcv); //la formule était vinstant = ((((cm-cmavant)*3600)/100000)*correcv/100) mais modifiée pour faire la division après pour garder la précision
  vinstant = vinstant/200000; //ça équivaut à un calcul de moyenne sur les 2 dernières secondes
  if (vinstant > 400) {vinstant = 400;} //bridage de la vitesse a 400 pour éviter les bugs d'affichage
  j = (millis());
  cmavant = cm;}


//affichage vmax
if (vinstant > vmax) {vmax = vinstant;} //la Vmax se met a jour quand la Vinstant la dépasse mais celle-ci est mise a zero avec "resettour"
if (vmax > vmaxabsolue) {vmaxabsolue = vmax;} //la Vmaxabsolue se met a jour quand la Vinstant la dépasse
tft.setCursor(0,114); sprintf(buffer, "%03d", vmaxabsolue); tft.print(buffer);

//passage en mode lecture
if (digitalRead(dwn) == LOW) { modelecture = HIGH; afftour = tours; }

while (modelecture == HIGH) {
  tft.setCursor(0,0); tft.print("-");
  tft.setCursor(27,0); tft.print("--");
  tft.setCursor(79,0); tft.print("--");
  tft.setCursor(0,76); tft.print("---");
  if (digitalRead(up) == LOW) { afftour++;}
  if (afftour>80) {afftour = 1;}
  if (digitalRead(dwn) == LOW) { afftour--;}
  if (afftour<1) {afftour = 80;}
  EEPROM.get(afftour*10,chrono);
  if ((chrono<2) || (chrono>59900)) {chrono=0;} //pour forcer l'affichage d'un chrono zero s'il est irréaliste
  affmin = (chrono/6000); //pour afficher le chrono correspondant au tour sélectionné
  affsec = ((chrono-affmin*6000)/100); //pour afficher le chrono correspondant au tour sélectionné
  affcent = (chrono-((affmin*6000)+(affsec*100))); //pour afficher le chrono correspondant au tour sélectionné
  EEPROM.get((afftour*10+5),affvmax);
  if ((affvmax<2) || (affvmax>400)) {affvmax=0;} //pour forcer l'affichage d'une vmax a zero si valeur irréaliste
  tft.setCursor(0,38); sprintf(buffer, "%01d", affmin); tft.print(buffer);
  tft.setCursor(27,38); sprintf(buffer, "%02d", affsec); tft.print(buffer);
  tft.setCursor(79,38); sprintf(buffer, "%02d", affcent); tft.print(buffer);
  tft.setTextSize(2);
  tft.setCursor(80,144); sprintf(buffer, "%02d", afftour); tft.print(buffer); //pour affichage du numéro du tour concerné
  tft.setTextSize(4);
  tft.setCursor(0,114); sprintf(buffer, "%03d", affvmax); tft.print(buffer); //pour affichage de la vmax correspondante a ce tour
    
  if (digitalRead(rst) == LOW) {
    delay(300); //délai pour éviter de sortir du mode lecture et faire un reset involontaire dans la foulée
    //pour remettre a jour l'affichage du top chrono et du tour en cours : (car ces affichages ont été changés par le mode lecture,
    //et ne seront à nouveau a jour qu'au nouveau "resettour" qui n'est pas forcément immédiat
    affiche_chrono_et_tour();
    //pour sortir du mode lecture :
    modelecture = LOW;
  }
}
}


//PROGRAMME DU MODE "STRINO" (MODE ROUTE) :

if (modevector == 3) {

tft.setTextSize(3);

//gestion du reset
if (digitalRead(rst) == LOW) {cm = 0; cmavant = cm;} //reset le trip

//affichage distance
if (cm > 99900000) {cm = 0;} //la distance retombe a zéro au dela de 999.0km. pour éviter les bugs d'affichage
affkm = cm/100000;
affm = (cm-affkm*100000)/10000;
tft.setCursor(10,75); sprintf(buffer, "%03d", affkm); tft.print(buffer);
tft.setCursor(75,75); sprintf(buffer, "%01d", affm); tft.print(buffer);

//affichage du totalisateur
afftotalismkm = (totaliskm/1000);
afftotaliskm = (totaliskm-afftotalismkm*1000);
tft.setCursor(5,115); sprintf(buffer, "%02d", afftotalismkm); tft.print(buffer); sprintf(buffer, "%03d", afftotaliskm); tft.print(buffer); //obligé de bidouiller car capacité buffer<32700km


//affichage vitesse instantanée
tft.setTextSize(7);
if ((millis())-2000<j) { //si délai inférieur a 2 sec, j'affiche la vitesse, si au dela, je recalcule la vitesse et réinitialise le temps
  tft.setCursor(0,0); sprintf(buffer, "%03d", vinstant); tft.print(buffer);}
else {
  vinstant = (cm*36*correcv-cmavant*36*correcv); //la formule était vinstant = ((((cm-cmavant)*3600)/100000)*correcv/100) mais modifiée pour faire la division après pour garder la précision
  vinstant = vinstant/200000; //ça équivaut à un calcul de moyenne sur les 2 dernières secondes
  if (vinstant > 400) {vinstant = 400;} //bridage de la vitesse a 400 pour éviter les bugs d'affichage
  j = (millis());
  cmavant = cm;}



}



}
