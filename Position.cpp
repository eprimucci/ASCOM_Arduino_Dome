extern "C" {
  #include <stdlib.h>
}

#include "Arduino.h"
#include "Position.h"

Position::Position(long range) {
  this->stepperPosition = 0L;
  this->range = range;
}

void Position::sync(long azimuth) {
  this->stepperPosition = this->degreesToPos(azimuth);
}

long Position::getDegrees() {
  return this->posToDegrees(this->stepperPosition);
}

long Position::posToDegrees(long pos) {
  return map(pos, 0, this->range, 0, 360);
}

long Position::degreesToPos(int value) {
  return map(value, 0, 360, 0, this->range) + 1;
}

Position Position::operator++(int) {
  if((this->stepperPosition + 1) > this->range)   {
    this->stepperPosition += 1 - this->range;
  }
  else   {
    this->stepperPosition += 1;
  }
  
  return *this;
}

Position Position::operator--(int) {
  if(this->stepperPosition == 0 )   {
    this->stepperPosition = this->range;
  }
  else   {
    this->stepperPosition -= 1;
  }
  
  return *this;
}

Position Position::operator=(long pos) {
  this->stepperPosition = pos;
  return *this;
}

