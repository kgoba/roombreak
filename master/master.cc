//#include <wiringSerial.h>
//#include <wiringPi.h>

#include "gpio.h"

#include <stdio.h>   /* Standard input/output definitions */
#include <string.h>  /* String function definitions */
#include <unistd.h>  /* UNIX standard function definitions */
#include <fcntl.h>   /* File control definitions */
#include <errno.h>   /* Error number definitions */
#include <termios.h> /* POSIX terminal control definitions */
#include <sys/ioctl.h>   

const char *kDevice = "/dev/ttyAMA0";
const int kBaudrate = 19200;
const int kPinTXE = 22;
const int kPinRXD = 17;

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
  GPIOExport(pinTXE);
  GPIOExport(pinRXD);
  usleep(10 * 1000);
  GPIODirection(pinTXE, OUT);
  GPIODirection(pinRXD, OUT);
}

Serial::~Serial()
{
  GPIOUnexport(_pinRXD);
  GPIOUnexport(_pinTXE);
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

  ::fcntl(_fd, F_SETFL, 0);   // blocking read
  //::fcntl(_fd, F_SETFL, FNDELAY);   // non-blocking read
  
  struct termios options;
  ::tcgetattr(_fd, &options);
  
  ::cfsetispeed(&options, B19200);
  ::cfsetospeed(&options, B19200);
  options.c_cflag |= (CLOCAL | CREAD);

  options.c_cflag &= ~PARENB;
  options.c_cflag &= ~CSTOPB;
  options.c_cflag &= ~CSIZE; /* Mask the character size bits */
  options.c_cflag |= CS8;    /* Select 8 data bits */

  ::tcflush(_fd, TCIFLUSH);
  ::tcsetattr(_fd, TCSAFLUSH, &options);
  return true;
}

void Serial::write(const char *data, int count)
{
  // enable RS485 transmitter
  GPIOWrite(_pinRXD, HIGH);
  GPIOWrite(_pinTXE, HIGH);
  //usleep(1 * 1000);
  ::write(_fd, data, count);
}

int Serial::read(char *data, int count)
{
  // Initialize file descriptor sets
  fd_set read_fds, write_fds, except_fds;
  FD_ZERO(&read_fds);
  FD_ZERO(&write_fds);
  FD_ZERO(&except_fds);
  FD_SET(_fd, &read_fds);

  // Set timeout to 5 ms
  struct timeval timeout;
  timeout.tv_sec = 1;
  timeout.tv_usec = 100000;

  // Wait for input to become ready or until the time out; the first parameter is
  // 1 more than the largest file descriptor in any of the sets
  if (select(_fd + 1, &read_fds, &write_fds, &except_fds, &timeout) == 1)
  {
    // fd is ready for reading
    return ::read(_fd, data, count);
  }
  else
  {
    printf("Timeout\n");
    // timeout or error
    return ::read(_fd, data, count);
    return 0;
  }
}

void Serial::flush()
{
  // disable RS485 transmitter
  ::tcflush(_fd, TCOFLUSH);
  usleep(10 * 1000);
  GPIOWrite(_pinTXE, LOW);
  GPIOWrite(_pinRXD, LOW);
}

int main()
{
  Serial serial(kPinTXE, kPinRXD);
  bool status = serial.open(kDevice, kBaudrate);
  if (!status) return 1;

  char msg1[] = { 0xAF, 0x6A, 0xDE, 0x17, 0x7F, 0x00, 0xCE };
  //char msg2[] = { 0xAF, 0x6A, 0xDE, 0x1B, 0x00, 0x00, 0xF2 };
  char msg2[] = { 0xAF, 0x6A, 0xDE, 0x11, 0x00, 0x00, 0xC7 };
   for (int i = 0; i < 5000; i++) {
    serial.write(msg2, 7);
    //usleep(10 * 1000);
    serial.flush();
    usleep(1 * 1000);

    int recv;
    char buf[32];
    do {
      recv = serial.read(buf, 7);
      if (recv > 0) {
        printf("Received %d chars: ", recv);
        for (int j = 0; j < recv; j++) {
          printf("%02x ", buf[j]);
        }
        //write(STDOUT_FILENO, buf, recv);
        putchar('\n');
      }
    } while (recv > 0);
  }

  return 0;
}
