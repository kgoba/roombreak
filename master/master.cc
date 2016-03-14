//#include <wiringSerial.h>
#include <wiringPi.h>

#include <stdio.h>   /* Standard input/output definitions */
#include <string.h>  /* String function definitions */
#include <unistd.h>  /* UNIX standard function definitions */
#include <fcntl.h>   /* File control definitions */
#include <errno.h>   /* Error number definitions */
#include <termios.h> /* POSIX terminal control definitions */
#include <sys/ioctl.h>   

const char *kDevice = "/dev/ttyAMA0";
const int kBaudrate = 19200;
const int kPinTXE = 0;
const int kPinRXD = 3;

class Serial {
public:
  Serial(int pinTXE, int pinRXD);
  ~Serial();
  void close();
  bool open(const char *device, int baudrate);
  void write(const char *data, int count);
  int read(char *data, int count);
  void flush();

private:
  int   _fd;
  int   _pinTXE;
  int   _pinRXD;
};

Serial::Serial(int pinTXE, int pinRXD) 
{
  _fd = -1;
  _pinTXE = pinTXE;
  _pinRXD = pinRXD;
  pinMode(pinTXE, OUTPUT);
  pinMode(pinRXD, OUTPUT);
}

Serial::~Serial()
{
  close();
}

void Serial::close()
{
  ::close(_fd);
  _fd = -1;
}

bool Serial::open(const char *device, int baudrate)
{
  //_fd = serialOpen(device, baudrate); 
  _fd = ::open(device, O_RDWR | O_NOCTTY | O_NDELAY);
  printf("File descriptor %d\n", _fd);
  if (_fd == -1) return false;

  //::fcntl(_fd, F_SETFL, 0);   // blocking read
  ::fcntl(_fd, F_SETFL, FNDELAY);   // non-blocking read
  
  struct termios options;
  ::tcgetattr(_fd, &options);
  
  ::cfsetispeed(&options, B19200);
  ::cfsetospeed(&options, B19200);
  options.c_cflag |= (CLOCAL | CREAD);

  options.c_cflag &= ~PARENB;
  options.c_cflag &= ~CSTOPB;
  options.c_cflag &= ~CSIZE; /* Mask the character size bits */
  options.c_cflag |= CS8;    /* Select 8 data bits */

  ::tcsetattr(_fd, TCSAFLUSH, &options);
  return true;
}

void Serial::write(const char *data, int count)
{
  // enable RS485 transmitter
//  digitalWrite(_pinRXD, HIGH);
  digitalWrite(_pinTXE, HIGH);
  ::write(_fd, data, count);
  ::tcflush(_fd, TCOFLUSH);
}

int Serial::read(char *data, int count)
{
  return ::read(_fd, data, count);
}

void Serial::flush()
{
  //serialFlush(_fd);
  //::ioctl(_fd, TCFLSH);
  // disable RS485 transmitter
  digitalWrite(_pinTXE, LOW);
//  digitalWrite(_pinRXD, LOW);
}

int main()
{
  wiringPiSetup();

  Serial serial(kPinTXE, kPinRXD);
  bool status = serial.open(kDevice, kBaudrate);
  if (!status) return 1;

  for (int i = 0; i < 1000; i++) {
    serial.write("test", 4);
    usleep(10 * 1000);
    //serial.flush();
    usleep(500 * 1000);

    int recv;
    char buf[32];
    do {
      recv = serial.read(buf, 32);
      if (recv > 0) {
        printf("Received %d chars: ", recv);
        for (int j = 0; j < recv; j++) {
          printf("%d ", buf[recv]);
        }
        //write(STDOUT_FILENO, buf, recv);
        putchar('\n');
      }
    } while (recv > 0);
  }

  return 0;
}
