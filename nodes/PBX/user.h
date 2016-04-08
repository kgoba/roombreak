#pragma once
#include "config.h"
#include "Common/util.h"
#include "line.h"

class Operator;

/// User (phone subscriber) base class
///
class User {
public:
  const char * getNumber() { return _number; }
  virtual void tick() = 0;

protected:
  const char * _number;
};

/// Physical user
///
class PUser : public User {
public:
  enum State {
    IDLE, OFFHOOK, DIAL, 
    CALL, TALK, BUSY, RING, RING_WAIT
  };

  PUser(PLine &line);

  State getState();

  void onLineOpen();
  void onLineClosed();
  void onTimer(int type);
  void onRing(bool isRinging);

  void tick();

  byte getLastDialled();

private:
  enum { 
    TIMER_PICKUP, TIMER_DIALSTART, TIMER_HANGUP, 
    TIMER_BREAK, TIMER_INTERDIGIT, TIMER_CALL,
    TIMER_RING,
    N_TIMERS 
  };

  enum {
    DIAL_SOLVE1,
    DIAL_SOLVE2,
    DIAL01,
    DIAL02,
    DIAL03,
    DIAL_ADMIN
  };

  PLine & _line;
  State _state;    
  int   _timerCounts[N_TIMERS];
  PLine::State _lastLineState;
  int   _currentDigit;
  int   _nDigits;
  char  _dialNumber[N_DIGITS];
  byte  _dialled;

  static const char  * _numbers [];

  void setState(State state);
  void startTimer(int type, int timeout);
  void stopTimer(int type);
};

/// Virtual user
///
class VUser : public User {
public:
  VUser(const char * number) { _number = number; }
  
  void tick();
  
private:
  
};

/// Operator
///
class Operator {
public:
  Operator(PLine &line1, PLine &line2) : _line1(line1), _line2(line2), _user1(line1) {
  }
  
  void onOffHook(User &u);
  void onDialDigit(User &u, int digit);

  void tick() {
    static word counter = 0;

    _user1.tick();
    _line2.update();

    if (_user1.getLastDialled() == 0) {
      //counter = 1000;
      //_line2.setRing(true);
    }

    /*
    if (counter > 0) {
      if (counter == 2)
        _line2.setRing(false);

      if (counter > 1)
        counter--;
    }
    */

    bool talk = false;
    bool call1 = false;
    bool call2 = false;
    
    static PLine::State lastState;
    
    PLine::State newState = _line2.getState();
    
    if (lastState == PLine::OPEN && newState == PLine::CLOSED) {
      _user1.onRing(true);
    }
    else if (lastState == PLine::CLOSED && newState == PLine::OPEN) {
      _user1.onRing(false);
    }
    lastState = newState;
    

    if (_line1.getState() == PLine::CLOSED && newState == PLine::CLOSED) {
      talk = true;
    }

    if (talk) pinWrite(PIN_TALK, HIGH);
    else pinWrite(PIN_TALK, LOW);
    
    //_line1.setRing(call1);
    //_line2.setRing(call2);
  }

  byte getLastDialled() {
    return _user1.getLastDialled();
  }
  
private:
  PLine   &_line1;
  PLine   &_line2;
  PUser   _user1;
};
