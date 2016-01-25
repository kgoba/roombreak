#define clkpin 13
#define sdapin 12

#define digitVal 40*255/100
#define digitOff 0
#define ledVal 5*255/100
#define ledOff 0

#define ledCount 54

void set_leds(unsigned int indata, byte duty = ledVal);
void set_dots(byte indata, byte duty = digitVal);
void set_segments(byte index, byte mask, byte duty = digitVal);
void set_digit(byte index, byte value, byte duty = digitVal);
void set_digits(byte mTens,byte mOnes,byte sTens,byte sOnes, byte duty = digitVal);
void update_display();

void setup() {
  pinMode(clkpin,OUTPUT);
  pinMode(sdapin,OUTPUT);
}

class Action {
public:
  virtual void reset() = 0;
  virtual bool update() = 0;

private:
  Action * _next;
};

class SegmentSequence : public Action {
public:
  SegmentSequence(long long * sequence, int count, int start = 0) 
    : _sequence(sequence), _count(count), _last(0), _last2(0), _start(start) 
  {
    reset();
  }
  void reset() {
    _counter = _start;
    _last = 0;
    _last2 = 0;
  }
  bool update() {
    if (_counter == _count) return true;

    set_segments(0, _sequence[_counter] | _last | _last2);
    set_segments(1, (_sequence[_counter] | _last | _last2) >> 7);
    set_segments(2, (_sequence[_counter] | _last | _last2) >> 14);
    set_segments(3, (_sequence[_counter] | _last | _last2) >> 21);
    _last2 = _last;
    _last = _sequence[_counter];

    _counter++;
    return (_counter == _count);
  }

private:
  long long * _sequence;
  long long _last, _last2;
  int _count;
  int _start;
  int _counter;
};

class LedSequence : public Action {
public:
  LedSequence(unsigned int * sequence, int count, int start = 1) 
    : _sequence(sequence), _count(count), _start(start) 
  {
    reset();
  }
  void reset() {
    _counter = 0;
    _counter2 = 0;
    //_last = 0;
  }
  bool update() {
    if (_counter == _count) return true;

    _counter2++;
    if (_counter2 > _start) {
      set_leds(_sequence[_counter]);
      _counter++;
      _counter2 = 0;
    }
    return (_counter == _count);
  }

private:
  unsigned int * _sequence;
  int _count;
  int _start;
  int _counter;
  int _counter2;
};

class FadeDigit : public Action {
public:
  enum Dir { DIR_IN, DIR_OUT };
  FadeDigit(byte digit_index, byte value, Dir dir) {
    _digit_index = digit_index;
    _value = value;
    _dir = dir;
    reset();
  }
  void reset() {
    _counter = 0;
  }
  bool update() {
    if (_counter == 9) return true;
    
    if (_dir == DIR_IN) {
      set_digit(_digit_index, _value, (1 << _counter) - 1);
    }
    else {
      set_digit(_digit_index, _value, (1 << (8 - _counter)) - 1);
    }
    _counter++;
    return (_counter == 9);
  }

private:
  byte _digit_index;
  byte _value;
  Dir _dir;

  byte _counter;
};

class Parallel : public Action {
public:
  Parallel(Action ** list, int count) : _list(list), _count(count) {
  }
  void reset() {
    for (int i = 0; i < _count; i++)
      _list[i]->reset();
  }
  bool update() {
    bool allDone = true;
    for (int i = 0; i < _count; i++)
      if (!_list[i]->update())
        allDone = false;
    return allDone;
  }

private:
  Action ** _list;
  int      _count;
};

Action * parallel1[] = {
  new FadeDigit(0, 2, FadeDigit::DIR_IN),
  new FadeDigit(1, 0, FadeDigit::DIR_IN)
};

Action * parallel2[] = {
  new FadeDigit(0, 2, FadeDigit::DIR_OUT),
  new FadeDigit(1, 0, FadeDigit::DIR_OUT)
};

#define SEG_A   0x40ul
#define SEG_B   0x20ul
#define SEG_C   0x10ul
#define SEG_D   0x08ul
#define SEG_E   0x04ul
#define SEG_F   0x02ul
#define SEG_G   0x01ul

#define LEN(x)    sizeof(x)/sizeof(x[0])

long long segments1[] = {
  SEG_A,
  SEG_A << 7,
  SEG_A << 14,
  SEG_A << 21,
  SEG_B << 21,
  SEG_C << 21,
  SEG_D << 21,
  SEG_D << 14,
  SEG_D << 7,
  /*
  SEG_D,
  SEG_E,
  SEG_G,
  SEG_G << 7,
  SEG_B << 7,
  SEG_A << 14,
  SEG_A << 21,
  SEG_B << 21,
  SEG_G << 21,
  SEG_E << 21,
  SEG_D << 14,
  SEG_D << 7,
  */
  SEG_D,
  SEG_E,
  SEG_F
};

unsigned int leds1[] = {
  /*
  0x0001,
  0x0002,
  0x0004,
  0x0008,
  0x0010,
  0x0020,
  0x0040,
  0x0080,
  0x0100,
  0x0200
  */
  0x0001,
  0x0003,
  0x0007,
  0x000F,
  0x001F,
  0x003F,
  0x007F,
  0x00FF,
  0x01FF,
  0x03FF
};

Action * parallel3[] = {
  new SegmentSequence(segments1, LEN(segments1)),
  new LedSequence(leds1, LEN(leds1), 0)
};

struct SequenceEntry {
  Action *  action;
  int       delay;
};

SequenceEntry sequence[] = {
  { .action = new Parallel(parallel3, 2), .delay = 100 },
  { .action = new FadeDigit(0, 2, FadeDigit::DIR_IN), .delay = 40 },
  { .action = new FadeDigit(1, 0, FadeDigit::DIR_IN), .delay = 40 },
  { .action = new FadeDigit(2, 1, FadeDigit::DIR_IN), .delay = 40 },
  { .action = new FadeDigit(3, 6, FadeDigit::DIR_IN), .delay = 40 },
  { .action = new FadeDigit(0, 2, FadeDigit::DIR_OUT), .delay = 40 },
  { .action = new FadeDigit(1, 0, FadeDigit::DIR_OUT), .delay = 40 },
  { .action = new FadeDigit(2, 1, FadeDigit::DIR_OUT), .delay = 40 },
  { .action = new FadeDigit(3, 6, FadeDigit::DIR_OUT), .delay = 40 }
};

/*
Action * sequence[] = {
//  new Parallel(parallel1, 2),
//  new Parallel(parallel2, 2)
  new Parallel(parallel3, 2),
  new FadeDigit(0, 2, FadeDigit::DIR_IN),
  new FadeDigit(1, 0, FadeDigit::DIR_IN),
  new FadeDigit(2, 1, FadeDigit::DIR_IN),
  new FadeDigit(3, 6, FadeDigit::DIR_IN),
  new FadeDigit(0, 2, FadeDigit::DIR_OUT),
  new FadeDigit(1, 0, FadeDigit::DIR_OUT),
  new FadeDigit(2, 1, FadeDigit::DIR_OUT),
  new FadeDigit(3, 6, FadeDigit::DIR_OUT)
};
*/

int counter;

void loop() {

  Action * action = sequence[counter].action;

  bool done = action->update();
  
  update_display();
  delay(sequence[counter].delay);

  if (done) {
    action->reset();
    counter++;
    if (counter == LEN(sequence)) counter = 0;
  }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Service routines
// 

const byte onesmap[]={16,17,0,1,2,14,13,15};
const byte tensmap[]={11,12,5,6,7,9,8,10};
const byte ledmap[]={17,15,14,13,12,11,10,9,8,7,1,16};

#define dig0 0b01111110
#define dig1 0b00110000
#define dig2 0b01101101
#define dig3 0b01111001
#define dig4 0b00110011
#define dig5 0b01011011
#define dig6 0b01011111
#define dig7 0b01110000
#define dig8 0b01111111
#define dig9 0b01111011
#define digOff 0b00000000

const byte digits[11]={dig0,dig1,dig2,dig3,dig4,dig5,dig6,dig7,dig8,dig9,digOff};

byte pwm_out[ledCount];

void set_leds(unsigned int indata, byte duty) {
  for (byte i=0;i<12;i++){
    pwm_out[ledmap[i]+18]=(indata & (1 << i))?duty:ledOff;
  }
}

void set_dots(byte indata, byte duty) {
  pwm_out[tensmap[7]]=(indata & (1 << 3))?duty:digitOff; //mTens
  pwm_out[onesmap[7]]=(indata & (1 << 2))?duty:digitOff; //mOnes
  pwm_out[tensmap[7]+36]=(indata & (1 << 1))?duty:digitOff; //sTens
  pwm_out[onesmap[7]+36]=(indata & (1 << 0))?duty:digitOff; //sOnes
}

void set_digit(byte index, byte value, byte duty) {
  set_segments(index, digits[value], duty);
}

void set_segments(byte index, byte mask, byte duty) {
  for (byte i = 0; i < 7; i++) {
    byte bit_idx;
    switch (index) {
    case 0:
      bit_idx = tensmap[i];
      break;
    case 1:
      bit_idx = onesmap[i];
      break; 
    case 2:
      bit_idx = tensmap[i]+36;
      break;
    case 3:
      bit_idx = onesmap[i]+36;
      break;
    }
    pwm_out[bit_idx]=(mask & (1 << (6-i)))?duty:digitOff;
  }
}

void set_digits(byte mTens,byte mOnes,byte sTens,byte sOnes, byte duty) {
  byte i;
  for (i=0;i<7;i++){
    pwm_out[tensmap[i]]=(digits[mTens] & (1 << (6-i)))?duty:digitOff;
    pwm_out[onesmap[i]]=(digits[mOnes] & (1 << (6-i)))?duty:digitOff;
    pwm_out[tensmap[i]+36]=(digits[sTens] & (1 << (6-i)))?duty:digitOff;
    pwm_out[onesmap[i]+36]=(digits[sOnes] & (1 << (6-i)))?duty:digitOff;
  }
}

void update_display() {
  //delayMicroseconds(650);
  byte i;
  byte j;
  for (i=0;i<ledCount;i++){
    for (j = 0; j < 8; j++)  {
      digitalWrite(sdapin, !!(pwm_out[i] & (1 << (7 - j))));
      digitalWrite(clkpin, HIGH);
      digitalWrite(clkpin, LOW);        
    }
  }
}
