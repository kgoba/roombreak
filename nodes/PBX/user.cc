#include "user.h"

#include <Common/serial.h>

void PUser::setState(State state)
{
  switch (state) {
    case IDLE:
    {      
      Serial::println("State -> IDLE");
      stopTimer(TIMER_CALL);
      stopTimer(TIMER_DIALSTART);
      _line.setTone(PLine::TONE_OFF);
      break;
    }
    case OFFHOOK:
    {
      Serial::println("State -> OFFHOOK");
      startTimer(TIMER_DIALSTART, 8000);      // time until dialing has to start
      _line.setTone(PLine::TONE_DIAL);
      _currentDigit = 0;
      _nDigits = 0;
      break;
    }
    case BUSY:
    {
      Serial::println("State -> BUSY");
      _line.setTone(PLine::TONE_BUSY);
      break;
    }
    case DIAL:
    {
      Serial::println("State -> DIAL");
      stopTimer(TIMER_DIALSTART);
      _line.setTone(PLine::TONE_OFF);
      break;
    }
    case CALL:
    {
      Serial::println("State -> CALL");
      startTimer(TIMER_CALL, 12000);
      _line.setTone(PLine::TONE_CALL);
      break;
    }
    default:
      break;
  }
  _state = state;
}

void PUser::onLineClosed()
{
  stopTimer(TIMER_HANGUP);
  switch (_state) {
    case IDLE:
    {
      startTimer(TIMER_PICKUP, 200);      // time until offhook is detected
      break;
    }
    case OFFHOOK:
    {
      break;
    }
    case DIAL:
    {
      stopTimer(TIMER_BREAK);
      startTimer(TIMER_INTERDIGIT, 200); 
      break;
    }
  }
}

void PUser::onLineOpen()
{
  startTimer(TIMER_HANGUP, 200);          // set idle timeout 200 ms
  startTimer(TIMER_BREAK, 20);
  switch (_state) {
    case IDLE:
    {
      stopTimer(TIMER_PICKUP);
      break;
    }
    case OFFHOOK:
    {
      setState(DIAL);
      break;
    }
    case DIAL:
    {
      break;
    }
  }
}

void PUser::onTimer(int type)
{
  switch (type) {
    case TIMER_DIALSTART:
    {
      Serial::println("Timer DIALSTART");
      setState(BUSY);
      break;
    }
    case TIMER_HANGUP:
    {
      //Serial::println("Timer HANGUP");
      setState(IDLE);
      break;
    }
    case TIMER_PICKUP:
    {
      //Serial::println("Timer PICKUP");
      setState(OFFHOOK);
      break;
    }
    case TIMER_BREAK:
    {
      //Serial::println("Timer BREAK");
      _currentDigit++;
      if (_currentDigit > 10) {
        Serial::println("Dial Error");
        setState(BUSY);
      }
      break;
    }
    case TIMER_INTERDIGIT:
    {
      if (_currentDigit == 10) _currentDigit = 0;
      //Serial::println("Timer INTERDIGIT");
      Serial::print("Dial ");
      Serial::putChar(_currentDigit + '0');
      Serial::println("");
      
      // digit finished
      // check with operator
      //_dialNumber[_nDigits] = (_currentDigit < 10) : '0' + _currentDigit : '0';
      _nDigits++;
      _currentDigit = 0;
      if (_nDigits == 4) {
        setState(CALL);
      }
      break;
    }
    case TIMER_CALL:
    {
      Serial::println("Timer CALL");
      //setState(BUSY);
      setState(IDLE);
      _line.playCustom(5);
      break;
    }
  }
}

void PUser::startTimer(int type, int timeout)
{
  _timerCounts[type] = timeout;
}

void PUser::stopTimer(int type)
{
  _timerCounts[type] = 0;
}

void PUser::tick()
{
  // process timeouts
  for (byte type = 0; type < N_TIMERS; type++) {
    if (_timerCounts[type] > 0) {
      if (--_timerCounts[type] == 0) {
        onTimer(type);
      }
    }
  }
  
  _line.update();
  // process line state
  PLine::State newLineState = _line.getState();
  if (newLineState != _lastLineState) {
    _lastLineState = newLineState;
    if (newLineState == PLine::OPEN) {
      onLineOpen();
    }
    else if (newLineState == PLine::CLOSED) {
      onLineClosed();
    }
  }
}

void VUser::tick()
{
  
}