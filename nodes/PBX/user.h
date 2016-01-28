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
    CALL, TALK, BUSY, RING
  };
  
  PUser(PLine &line) : _state(IDLE), _line(line) {
    _lastLineState = _line.getState();
  }
  
  State getState();
    
  void onLineOpen();
  void onLineClosed();
  void onTimer(int type);
  void onRing();
  
  void tick();
  
private:
  enum { 
    TIMER_PICKUP, TIMER_DIALSTART, TIMER_HANGUP, 
    TIMER_BREAK, TIMER_INTERDIGIT, TIMER_CALL,
    N_TIMERS 
  };

  PLine _line;
  State _state;    
  int   _timerCounts[N_TIMERS];
  PLine::State _lastLineState;
  int   _currentDigit;
  int   _nDigits;
  char  _dialNumber[N_DIGITS];

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
  void onOffHook(User &u);
  void onDialDigit(User &u, int digit);

  void tick();
};
