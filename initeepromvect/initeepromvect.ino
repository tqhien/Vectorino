//programme qui permet d'initialiser les bonnes valeurs dans l'EEPROM
//correspond au programme vectorino V2.1

#include <EEPROM.h>

byte modevector = 1; //mode rallye par défaut
unsigned int roue = 186; 
byte nbaimants = 1;
byte nbbandes = 1;
byte correcv = 95;
unsigned long totaliskm = 10000;
unsigned long tps = 0;
unsigned int vmax = 0;
long cm = 0;
byte tours = 1; 

byte q=1;

void setup() {
  // put your setup code here, to run once:

Serial.begin(9600);
Serial.println("preparation du reset eeprom");

EEPROM.put(900,modevector);
Serial.println("modevector fait");

EEPROM.put(910,roue);
Serial.println("roue fait");

EEPROM.put(920,nbaimants);
Serial.println("nbaimants fait");

EEPROM.put(930,nbbandes); 
Serial.println("nbbandes fait");

EEPROM.put(940,correcv);
Serial.println("correcv fait");

EEPROM.put(950,totaliskm);
Serial.println("totaliskm fait");

EEPROM.put(960,cm);
Serial.println("cm fait");

EEPROM.put(970,tours);
Serial.println("tours fait");


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
