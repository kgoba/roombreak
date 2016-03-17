/*

Python module for Raspberry Pi, implementing half-duplex communication with flow control
e.g. RS485 driver (MAX485) connected to UART RX/TX and 2 GPIO pins for flow control.

 */

#include <Python.h>

//#include <wiringSerial.h>
#include "gpio.h"

#include <stdio.h>   /* Standard input/output definitions */
#include <string.h>  /* String function definitions */
#include <unistd.h>  /* UNIX standard function definitions */
#include <fcntl.h>   /* File control definitions */
#include <errno.h>   /* Error number definitions */
#include <termios.h> /* POSIX terminal control definitions */
#include <sys/ioctl.h>   

static int pinTXE;
static int pinRXD;

static int impl_init(void);
static int impl_open(const char *device, int baudrate, int blocking);
static int impl_close(int fd);
static int impl_write(int fd, const char *data, int count);
static int impl_read(int fd, char *data, int count);
static int impl_flush(int fd);

static PyObject *
xfserial_init(PyObject *self, PyObject *args)
{
    int rc;

    //if (!PyArg_ParseTuple(args, "ii", &pinTXE, &pinRXD))
    //    return NULL;
    
    rc = impl_init();
    
    return Py_BuildValue("i", rc);
}

static PyObject *
xfserial_open(PyObject *self, PyObject *args)
{
    const char *device;
    int baudrate;
    int blocking;
    int fd;

    if (!PyArg_ParseTuple(args, "sii", &device, &baudrate, &blocking))
        return NULL;
    
    fd = impl_open(device, baudrate, blocking);

    return Py_BuildValue("i", fd);
}

static PyObject *
xfserial_write(PyObject *self, PyObject *args)
{
    const char *data;
    int count;
    int fd;
    int written;

    if (!PyArg_ParseTuple(args, "is#", &fd, &data, &count))
        return NULL;
    
    written = impl_write(fd, data, count);

    return Py_BuildValue("i", written);
}

static PyObject *
xfserial_flush(PyObject *self, PyObject *args)
{
    int fd;
    int rc;

    if (!PyArg_ParseTuple(args, "i", &fd))
        return NULL;
    
    rc = impl_flush(fd);
    
    return Py_BuildValue("i", rc);
}


static PyMethodDef xfserialMethods[] = {
    {"init",  xfserial_init, METH_VARARGS, "Initialize flow control pins."},
    {"open",  xfserial_open, METH_VARARGS, "Open a serial device and set baudrate."},
    {"write", xfserial_write, METH_VARARGS, "Start transmitting and queue data (string)."},
    {"flush", xfserial_flush, METH_VARARGS, "Flush transmit buffer."},
    {NULL, NULL, 0, NULL}        /* Sentinel */
};

PyMODINIT_FUNC
initxfserial(void)
{
    (void) Py_InitModule("xfserial", xfserialMethods);
}

int
main(int argc, char *argv[])
{
    /* Pass argv[0] to the Python interpreter */
    Py_SetProgramName(argv[0]);

    /* Initialize the Python interpreter.  Required. */
    Py_Initialize();

    /* Add a static module */
    initxfserial();
    
    return 0;
}

static int impl_init(void)
{
  int rc;
  return 0;
}
  
static int impl_open(const char *device, int baudrate, int blocking)
{
  if (GPIOExport(pinTXE) || GPIOExport(pinRXD)) return -1;
  if (GPIODirection(pinTXE, OUT) || GPIODirection(pinRXD, OUT)) return -2;
  if (GPIOWrite(pinTXE, LOW) || GPIOWrite(pinRXD, LOW)) return -3;
  
  int _fd = open(device, O_RDWR | O_NOCTTY | O_NDELAY);
  if (_fd == -1) return -4;
  //printf("File descriptor %d\n", _fd);
  
  struct termios options;

  if (blocking) {
    fcntl(_fd, F_SETFL, 0);   // blocking read
  }
  else {
    fcntl(_fd, F_SETFL, FNDELAY);   // non-blocking read
  }

  tcgetattr(_fd, &options);
  cfsetispeed(&options, B19200);
  cfsetospeed(&options, B19200);
  options.c_cflag |= (CLOCAL | CREAD);
  options.c_cflag &= ~PARENB;
  options.c_cflag &= ~CSTOPB;
  options.c_cflag &= ~CSIZE; /* Mask the character size bits */
  options.c_cflag |= CS8;    /* Select 8 data bits */
  tcsetattr(_fd, TCSAFLUSH, &options);      
  
  return _fd;
}

static int impl_close(int fd) {
  close(fd);
  if (GPIOUnexport(pinTXE) || GPIOUnexport(pinRXD)) return -1;
  return 0;
}

static int impl_write(int fd, const char *data, int count)
{
  // enable RS485 transmitter
  GPIOWrite(pinRXD, HIGH);
  GPIOWrite(pinTXE, HIGH);
  return write(fd, data, count);
}

static int impl_read(int fd, char *data, int count)
{
  // Initialize file descriptor sets
  fd_set read_fds, write_fds, except_fds;
  FD_ZERO(&read_fds);
  FD_ZERO(&write_fds);
  FD_ZERO(&except_fds);
  FD_SET(fd, &read_fds);

  // Set timeout to 5 ms
  struct timeval timeout;
  timeout.tv_sec = 0;
  timeout.tv_usec = 5000;

  // Wait for input to become ready or until the time out; the first parameter is
  // 1 more than the largest file descriptor in any of the sets
  if (select(fd + 1, &read_fds, &write_fds, &except_fds, &timeout) == 1)
  {
    // fd is ready for reading
    return read(fd, data, count);
  }
  else
  {
    // timeout or error
    return 0;
  }
}

static int impl_flush(int fd)
{
  // disable RS485 transmitter
  tcflush(fd, TCOFLUSH);
  GPIOWrite(pinTXE, LOW);
  GPIOWrite(pinRXD, LOW);
  return 0;
}
