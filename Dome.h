
#ifndef Dome_h
#define Dome_h

#include <inttypes.h>
#include <avr/io.h>
#include "Messenger.h"
#include "Position.h"

class Dome {
  public:
    Dome(void);
    void interpretCommand(Messenger *message);
    void park(long val);
    void openCloseShutter(int targetStatus);
    void slew(long val);
    void abortSlew();
    void syncToAzimuth(long azimuth);
    long curDegrees();
    void setPosition(long p);
    long getTargetAzimuth();
    void printAzimuth();
    bool isSlewing();
    int calcSlewDirection(long currentHeading, long targetHeading);
    long getAzimuth();
    long getPosition(long azimuth);
    void findHome();
    bool isFindingHome();
    void alertOnHome(long p);
  private:
    long parkPosition;
    long targetAzimuth;
    bool slewing;
    bool findingHome;
};

#endif
