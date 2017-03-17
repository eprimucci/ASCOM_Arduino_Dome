//#define DEBUGGING


#include <digitalWriteFast.h>  // library for high performance reads and writes by jrraines
// see http://www.arduino.cc/cgi-bin/yabb2/YaBB.pl?num=1267553811/0
// and http://code.google.com/p/digitalwritefast/

#include <Encoder.h> // lib que usa interrupts para nmayor performance

#define RELAY_1 5 // activa motor hacia la izquierda
#define RELAY_2 6 // activa motor hacia la derecha

#define FULLROTATION 120000 // clicks del encoder

// posiciones de los sensores
#define SENSOR_HOME_POS 7000
#define SENSOR_1_POS 15200 // posicion en clicks del sensor 1
#define SENSOR_2_POS 28000 // posicion en clicks del sensor 2 
#define SENSOR_3_POS 46000 // posicion en clicks del sensor 3
#define SENSOR_4_POS 80000 // posicion en clicks del sensor 4
#define SENSOR_5_POS 100000 // posicion en clicks del sensor 5

// pines de los sensores
#define SENSOR_HOME 8 // Sensor de contacto para recalibrar posicion
#define SENSOR_1 9    // Sensor de contacto para recalibrar posicion
#define SENSOR_2 10   // Sensor de contacto para recalibrar posicion
#define SENSOR_3 11   // Sensor de contacto para recalibrar posicion
#define SENSOR_4 12   // Sensor de contacto para recalibrar posicion
#define SENSOR_5 13   // Sensor de contacto para recalibrar posicion

#define MODULO 50

#define DIR_NONE 0x00           // No complete step yet.
#define DIR_CW   0x10           // Clockwise step.
#define DIR_CCW  0x20           // Anti-clockwise step.

#define PIN_ENCODER_A 2             // pins connected to the encoder (digital_pin 2)
#define PIN_ENCODER_B 3             //              "                (digital_pin 3)

#include "Dome.h"
#include "Messenger.h"

Dome dome = Dome();
Messenger message = Messenger();
Encoder encoderDomo(PIN_ENCODER_A, PIN_ENCODER_B);

long posActual  = -200000;

void setup() {

  // pines de los sensores de contacto para feedback de la pos
  pinMode(SENSOR_HOME, INPUT_PULLUP);
  pinMode(SENSOR_1, INPUT_PULLUP);
  pinMode(SENSOR_2, INPUT_PULLUP);
  pinMode(SENSOR_3, INPUT_PULLUP);
  pinMode(SENSOR_4, INPUT_PULLUP);
  pinMode(SENSOR_5, INPUT_PULLUP);

  // pines del encoder ERROR! Se manejan en modo attach interrupt con RISING.. no manipular! dejar siempre comentado para acordarse!
  //    pinMode(2, INPUT );
  //    pinMode(3, INPUT );

  // reles
  pinMode(RELAY_1, OUTPUT);
  pinMode(RELAY_2, OUTPUT);
  digitalWrite(RELAY_1, LOW );
  digitalWrite(RELAY_2, LOW );

  Serial.begin(57600); // driver must match! si o si.. 
  Serial.flush();

  Serial.println("R ASCOM.Banderita.Dome");
  delay(50);
  Serial.flush();
  message.attach(messageCompleted);
}

void messageCompleted() {
  dome.interpretCommand(&message);
}


void listenToSensors() {
  // sensores de reposicionamiento
  if (digitalReadFast(SENSOR_HOME) == LOW) {
    #ifdef DEBUGGING
    Serial.println("HOME detectado");
    #endif
    encoderDomo.write(SENSOR_HOME_POS);
    if(dome.isFindingHome()) {
      #ifdef DEBUGGING
      Serial.println("El domo estaba buscanco HOME, avisando!");
      #endif
      dome.alertOnHome(SENSOR_HOME_POS);
    }
    
  }
  if (digitalReadFast(SENSOR_1) == LOW) {
    #ifdef DEBUGGING
    Serial.println("Sensor 1 detectado");
    #endif
    encoderDomo.write(SENSOR_1_POS);
  }
  if (digitalReadFast(SENSOR_2) == LOW) {
    #ifdef DEBUGGING
    Serial.println("Sensor 2 detectado");
    #endif
    encoderDomo.write(SENSOR_2_POS);
  }
  if (digitalReadFast(SENSOR_3) == LOW) {
    #ifdef DEBUGGING
    Serial.println("Sensor 3 detectado");
    #endif
    encoderDomo.write(SENSOR_3_POS);
  }
  if (digitalReadFast(SENSOR_4) == LOW) {
    #ifdef DEBUGGING
    Serial.println("Sensor 4 detectado");
    #endif
    encoderDomo.write(SENSOR_4_POS);
  }
  if (digitalReadFast(SENSOR_5) == LOW) {
    #ifdef DEBUGGING
    Serial.println("Sensor 5 detectado");
    #endif
    encoderDomo.write(SENSOR_5_POS);
  }
}


void loop() {

  listenToSensors();

  long nuevaPos;
  nuevaPos = encoderDomo.read();

  if(abs(nuevaPos)>FULLROTATION) {
    nuevaPos=0;
    encoderDomo.write(0);
  }
  else if(nuevaPos<0) {
    nuevaPos=FULLROTATION+nuevaPos; // sumo porque nuevaPos es negativo e.g. 120000 + (-300)
    encoderDomo.write(nuevaPos);
  }

  if (nuevaPos != posActual) {
    posActual = nuevaPos;
    if (abs(nuevaPos) % MODULO == 0) {
      dome.setPosition(nuevaPos);
      #ifdef DEBUGGING
      Serial.print("DEBUG: \tticks:\t");
      Serial.print(nuevaPos);
      Serial.print("\ttarget azimuth:\t");
      Serial.print(dome.getTargetAzimuth());
      Serial.print("\tgrados:\t");
      Serial.println(dome.curDegrees());
      #else
      dome.printAzimuth();
      #endif
            
    }
  }
  else {
    if (dome.isSlewing() && dome.getTargetAzimuth() == dome.curDegrees()) {
      // VERSION DE JULIO: (para encoders con menor resolución) if(dome.isSlewing() && abs(dome.getTargetAzimuth()-dome.curDegrees())<2 ) {
      dome.abortSlew();
    }
  }
  while (Serial.available()) message.process(Serial.read());
}

