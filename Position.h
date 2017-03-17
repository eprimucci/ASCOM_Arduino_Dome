#ifndef Position_h
#define Position_h

#include <inttypes.h>
#include <avr/io.h>

class Position {
  public:
    Position(long pos);
    Position operator++(int);
    Position operator--(int);
    Position operator=(long pos);
    long getDegrees();
    long stepperPosition;
    void sync(long azimuth);
  private:
    long posToDegrees(long pos);
    long degreesToPos(int value);
    long range;
};

#endif
