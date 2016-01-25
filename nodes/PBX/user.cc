#include "user.h"


void PUser::setState(State state)
{
  switch (state) {
    case IDLE:
    {
      _line.setTone(PLine::TONE_OFF);
      break;
    }
    case OFFHOOK:
    {
      startTimer(TIMER_DIALSTART, 8000);      // time until dialing has to start
      _line.setTone(PLine::TONE_DIAL);
      _currentDigit = 0;
      _nDigits = 0;
      break;
    }
    case BUSY:
    {
      _line.setTone(PLine::TONE_BUSY);
      break;
    }
    case DIAL:
    {
      stopTimer(TIMER_DIALSTART);
      _line.setTone(PLine::TONE_OFF);
      break;
    }
    case CALL:
    {
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
      startTimer(TIMER_INTERDIGIT, 100); 
      break;
    }
  }
}

void PUser::onLineOpen()
{
  startTimer(TIMER_HANGUP, 200);          // set idle timeout 200 ms
  startTimer(TIMER_BREAK, 30);
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
      setState(BUSY);
      break;
    }
    case TIMER_HANGUP:
    {
      setState(IDLE);
      break;
    }
    case TIMER_PICKUP:
    {
      setState(OFFHOOK);
      break;
    }
    case TIMER_BREAK:
    {
      _currentDigit++;
      if (_currentDigit > 10) {
        setState(BUSY);
      }
      break;
    }
    case TIMER_INTERDIGIT:
    {
      // digit finished
      // check with operator
      //_dialNumber[_nDigits] = (_currentDigit < 10) : '0' + _currentDigit : '0';
      _nDigits++;
      if (_nDigits == 4) {
        setState(CALL);
      }
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
  for (byte type = 0; N_TIMERS < 3; type++) {
    if (_timerCounts[type] > 0) {
      if (--_timerCounts[type] == 0) {
        onTimer(type);
      }
    }
  }
  // process line state
  PLine::State newLineState = _line.getState();
  if (newLineState != _lastLineState) {
    _lastLineState = newLineState;
    if (newLineState == PLine::OPEN)     
      onLineOpen();
    else if (newLineState == PLine::CLOSED) {
      onLineClosed();      
    }
  }
}

void VUser::tick()
{
  
}