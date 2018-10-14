//programme qui permet d'initialiser les bonnes valeurs dans l'EEPROM
//correspond au programme vectorino V2.1

#include <EEPROM.h>

/** Nombre magique : si l'eeprom ne commence pas par ce nombre, elle n'a pas été initialisée. Il faut donc prendre les valeurs pas défaut **/
static const unsigned long STRUCT_MAGIC = 123456789;
/** Numéro de version : permet d'indiquer les variables sont utilisables et lesquelles n'existent pas **/
static const byte STRUCT_VERSION = 1 ;

/** La structure qui contient les données **/
struct VectorinoStructure {
  // Magic number et struct version
  unsigned long magic;
  byte struct_version;
  
  byte modevector; //mode rallye par défaut
  unsigned int roue; 
  byte nbaimants;
  byte nbbandes;
  byte correcv;
  unsigned long totaliskm;
  long cm;
  byte tours;
} ;

VectorinoStructure vs ;

unsigned long tps = 0;
unsigned int vmax = 0;
byte q=1;

void setup() {
  // put your setup code here, to run once:
  chargeEEPROM() ;

  Serial.begin(9600);
  // Affiche les données dans la structure
  Serial.print("modevector = ");
  Serial.println(vs.modevector);

  Serial.print("roue = ");
  Serial.println(vs.roue);

  Serial.print("nbaimants = ");
  Serial.println(vs.nbaimants);

  Serial.print("nbbandes = ");
  Serial.println(vs.nbbandes);

  Serial.print("correcv = ");
  Serial.println(vs.correcv);

  Serial.print("totaliskm = ");
  Serial.println(vs.totaliskm);

  Serial.print("cm = ");
  Serial.println(vs.cm);

  Serial.print("tours = ");
  Serial.println(vs.tours);


  while (q<81) {
    EEPROM.put(q*10,tps);
    Serial.print("chrono fait");
    Serial.println(q*10);
    EEPROM.put((q*10+5),vmax);
    Serial.print("vmax fait");
    Serial.println(q*10+5);
    q++;
  }

  Serial.println("mise a jour EEPROM terminee");

}

void loop() {
  // put your main code here, to run repeatedly:

}

/** Sauvegarde en mémoire EEPROM le contenu actuel de la structure */
void sauvegardeEEPROM() {
  // Met à jour le nombre magic et le numéro de version avant l'écriture
  vs.magic = STRUCT_MAGIC;
  vs.struct_version =  STRUCT_VERSION;
  EEPROM.put(900, vs);
}

void chargeEEPROM () {
  // Lit la mémoire EEPROM
  EEPROM.get(900, vs);
  
  Serial.println("Vérification eeprom");
  // Détection d'une mémoire non initialisée
  byte erreur = vs.magic != STRUCT_MAGIC;

  // Valeurs par défaut struct_version == 0
  if (erreur) {

    Serial.println("eeprom non initialisée, Sauvegarde des valeurs par défaut");
    // Valeurs par défaut pour les variables de la version 0
    vs.modevector = 1;
    vs.roue = 186 ;
    vs.nbaimants = 1 ;
    vs.nbbandes = 1 ;
    vs.correcv = 95 ;
    vs.totaliskm = 10000 ;
    vs.cm = 1 ;
    vs.tours = 1 ;
  }


  // Valeurs par défaut struct_version == 1
  if (vs.struct_version < 1 || erreur) {
    // Valeurs par défaut pour les variables de la version 1
    //vs.valeur_2 = 13.37;
  }

  // Valeurs par défaut pour struct_version == 2
  if (vs.struct_version < 2 || erreur) {
    // Valeurs par défaut pour les variables de la version 2
    //strcpy(vs.valeur_3, "Hello World!");
  }
  // Sauvegarde les nouvelles données
  sauvegardeEEPROM();
}


