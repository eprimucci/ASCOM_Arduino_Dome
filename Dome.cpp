//#define DEBUGGING

extern "C" {
#include <stdlib.h>
}

#include "Arduino.h"
#include "Dome.h"

#define RELAY_1 5
#define RELAY_2 6
#define FULLROTATION 120000 // clicks del encoder en una vuelta completa del domo



long parkPosition = 0;
long findingHome = false;
Position position(FULLROTATION); // FULLROTATION es el range maximo, es decir, los clicks TOTALES que tiene el giro del domo;


Dome::Dome(void) {
  position = 0; // porque iniciamos la arduino con el domo en HOME (ojo que hay on overide del =, que en realidad es un setter de objeto Position
  this->targetAzimuth = 0; // donde nos manda el ASCOM (maxim, skymap, etc)
  this->slewing = false; // NO me estoy moviendo cuando inicio
}

/**
   Interpreta el command (idea de ejholmes)
   COMMAND <ARGUMENT> #" ejemplo para Slew a 145 grados es ": S 145 #" y un find home es ": F #"
*/
void Dome::interpretCommand(Messenger *message) {
  message->readChar(); // Lee el colon ":"
  char command = message->readChar(); // Read the command
  switch (command) {
    case 'P':
      park(message->readLong());
      break;
    case 'O':
      openCloseShutter(message->readInt());
      break;
    case 'S':
      slew(message->readLong());
      break;
    case 'H':
      abortSlew();
      break;
    case 'T':
      syncToAzimuth(message->readInt());
      break;
    case 'F':
      findHome();
      break;
  }
}

/**
 * Frena todo movimiento del domo
 */
void Dome::abortSlew() {
  digitalWrite(RELAY_1, LOW); // relay en OFF
  digitalWrite(RELAY_2, LOW); // relay en OFF

  // ponemos el target a lo que tenga ahora
  #ifdef DEBUGGING
  Serial.println("Parando el slew");
  #endif

  this->targetAzimuth = position.getDegrees();
  this->slewing = false;
  printAzimuth();
}



void Dome::findHome() {
  #ifdef DEBUGGING
  Serial.println("Buscando HOME dijo ET");
  #endif

  // ponemos el domo a buscar el grado -1000 que no existe
  this->targetAzimuth = -1000; // 
  this->slewing = true;
  this->findingHome = true;

  // por defult buscamos por este lado, cuando el sensor acuse 0 entonces paramos...
  digitalWrite(RELAY_2, LOW);
  digitalWrite(RELAY_1, HIGH);

}


void Dome::alertOnHome(long p) {
  #ifdef DEBUGGING
  Serial.println("Alertando HOME");
  #endif
  digitalWrite(RELAY_1, LOW); // relay en OFF
  digitalWrite(RELAY_2, LOW); // relay en OFF

  position = p;

  // ponemos el target a lo que tenga ahora
  this->targetAzimuth = position.getDegrees();
  this->slewing = false;
  this->findingHome = false;
  // y avisamos
  Serial.print("HOMED "); Serial.println(position.getDegrees());
}



/**
 * La posta del movimiento
 */
void Dome::slew(long azimuth) {
  if (position.getDegrees() == azimuth) {
    #ifdef DEBUGGING
    Serial.println("No hago slew porque ya estoy en pos");
    #endif
    this->slewing = false;
    return;
  }

  this->targetAzimuth = azimuth;
  this->slewing = true;

  long currentDegrees = position.getDegrees();
  int whereToTurn = this->calcSlewDirection(currentDegrees, azimuth);
  

  switch (whereToTurn) {

    case 1:
      digitalWrite(RELAY_2, LOW);
      digitalWrite(RELAY_1, HIGH);
      break;

    case -1:
      digitalWrite(RELAY_1, LOW);
      digitalWrite(RELAY_2, HIGH);
      break;

    default: // cero, por si ya estamos en posición
      digitalWrite(RELAY_1, LOW);
      digitalWrite(RELAY_2, LOW);
      break;

  }

  #ifdef DEBUGGING
  Serial.print("Iniciando slew desde: ");
  Serial.print(currentDegrees);
  Serial.print("\thacia: ");
  Serial.print(azimuth);
  Serial.print("\t girando: ");
  Serial.println(whereToTurn);
  #endif
}


void Dome::printAzimuth() {
  Serial.print("P "); Serial.println(position.getDegrees());
}

long Dome::curDegrees() {
  return position.getDegrees();
}

void Dome::setPosition(long p) {
  position = p;
}

long Dome::getTargetAzimuth() {
  return this->targetAzimuth;
}

void Dome::park(long val) {
  slew(val);
  delay(500);
  //this->slewing = false;
  Serial.println("PARKED");
}

void Dome::openCloseShutter(int targetStatus) {
  switch (targetStatus) {
    case 1:
      Serial.println("SHUTTER OPEN");
      break;
    case 0:
      Serial.println("SHUTTER CLOSED");
      break;
  }
}

void Dome::syncToAzimuth(long azimuth) {
  position.sync(azimuth);
  printAzimuth();
  delay(500);
  Serial.println("SYNCED");
}


bool Dome::isSlewing() {
  return this->slewing;
}

bool Dome::isFindingHome() {
  return this->findingHome;
}


/**
 * Calcula el sentido de rotación para llegar al nuevo heading en la forma más rápida
 */
int Dome::calcSlewDirection(long currentHeading, long targetHeading) { // should a new heading turn left ie. CCW?
  if (currentHeading == targetHeading) {
    return 0;
  }
  long diff = targetHeading - currentHeading;
  if (diff > 0) {
    if (diff > 180) {
      return 1;
    }
    else {
      return -1;
    }
  }
  else {
    if (diff >= -180) {
      return 1;
    }
    else {
      return -1;
    }
  }
}

