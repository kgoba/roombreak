/*
 * Copyright (c) 2010, Kelvin Lawson. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. No personal names or organizations' names associated with the
 *    Atomthreads project may be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE ATOMTHREADS PROJECT AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE PROJECT OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdio.h>

#include <atom.h>
#include <atomtimer.h>
#include <atommutex.h>
#include <atomqueue.h>
#include <atomsem.h>
#include <atomport.h>

#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <avr/io.h>

#define ARRAY_SIZE(array)               (sizeof(array) / sizeof(array[0]))

/* Function prototypes */
void avrInitSystemTickTimer ( void );

/* Forward declarations */
static void main_thread_func (uint32_t data);
static void comm_thread_func (uint32_t data);

/* Constants */

/*
 * Idle thread stack size
 *
 * This needs to be large enough to handle any interrupt handlers
 * and callbacks called by interrupt handlers (e.g. user-created
 * timer callbacks) as well as the saving of all context when
 * switching away from this thread.
 *
 * In this case, the idle stack is allocated on the BSS via the
 * idle_thread_stack[] byte array.
 */
#define IDLE_STACK_SIZE_BYTES       128


/*
 * Main thread stack size
 *
 * Note that this is not a required OS kernel thread - you will replace
 * this with your own application thread.
 *
 * In this case the Main thread is responsible for calling out to the
 * test routines. Once a test routine has finished, the test status is
 * printed out on the UART and the thread remains running in a loop
 * flashing a LED.
 *
 * The Main thread stack generally needs to be larger than the idle
 * thread stack, as not only does it need to store interrupt handler
 * stack saves and context switch saves, but the application main thread
 * will generally be carrying out more nested function calls and require
 * stack for application code local variables etc.
 *
 * With all OS tests implemented to date on the AVR, the Main thread
 * stack has not exceeded 201 bytes. To allow all tests to run we set
 * a minimum main thread stack size of 204 bytes. This may increase in
 * future as the codebase changes but for the time being is enough to
 * cope with all of the automated tests.
 */
#define MAIN_STACK_SIZE_BYTES       204
#define COMM_STACK_SIZE_BYTES       204

/*
 * Startup code stack
 *
 * Some stack space is required at initial startup for running the main()
 * routine. This stack space is only temporarily required at first bootup
 * and is no longer required as soon as the OS is started. By default
 * GCC sets this to the top of RAM (RAMEND) and it grows down from there.
 * Because we only need this temporarily, though, it would be wasteful to
 * set aside a region at the top of RAM which is not used during runtime.
 *
 * What we do here is to reuse part of the idle thread's stack during
 * initial startup. As soon as we enter the main() routine we move the
 * stack pointer to half-way down the idle thread's stack. This is used
 * temporarily while calls are made to atomOSInit(), atomThreadCreate()
 * and atomOSStart(). Once the OS is started this stack area is no
 * longer required, and can be used for its original purpose (for the
 * idle thread's stack).
 *
 * This does mean, however, that we cannot monitor the stack usage of the
 * idle thread. Stack usage is monitored by prefilling the stack with a
 * known value, and we are obliterating some of that prefilled area by
 * using it as our startup stack, so we cannot use the stack-checking API
 * to get a true picture of idle thread stack usage. If you wish to 
 * monitor idle thread stack usage for your applications then you are
 * free to use a different region for the startup stack (e.g. set aside
 * an area permanently, or place it somewhere you know you can reuse
 * later in the application). For the time being, this method gives us a
 * simple way of reducing the memory consumption without having to add
 * any special AVR-specific considerations to the automated test
 * applications.
 *
 * This optimisation was required to allow some of the larger automated
 * test modules to run on devices with 1KB of RAM. You should avoid doing
 * this if you can afford to set aside 64 bytes or so, or if you are
 * writing your own applications in which you have further control over
 * where data is located.
 */


/* Local data */

#define MAIN_THREAD_PRIO            1
#define COMM_THREAD_PRIO            2

/* Application threads' TCBs */
static ATOM_TCB main_tcb;
static ATOM_TCB comm_tcb;

/* Main thread's stack area */
static uint8_t main_thread_stack[MAIN_STACK_SIZE_BYTES];
static uint8_t comm_thread_stack[COMM_STACK_SIZE_BYTES];

/* Idle thread's stack area */
static uint8_t idle_thread_stack[IDLE_STACK_SIZE_BYTES];

//extern "C" {
//    /* STDIO stream */
//    static FILE uart_stdout = FDEV_SETUP_STREAM(uart_putchar, NULL, _FDEV_SETUP_WRITE);    
//}



/**
 * \b main
 *
 * Program entry point.
 *
 * Sets up the AVR hardware resources (system tick timer interrupt) necessary
 * for the OS to be started. Creates an application thread and starts the OS.
 */

int main ( void )
{
    int8_t status;

    /**
     * Reuse part of the idle thread's stack for the stack required
     * during this startup function.
     */
    SP = (int)&idle_thread_stack[(IDLE_STACK_SIZE_BYTES/2) - 1];

    /**
     * Note: to protect OS structures and data during initialisation,
     * interrupts must remain disabled until the first thread
     * has been restored. They are reenabled at the very end of
     * the first thread restore, at which point it is safe for a
     * reschedule to take place.
     */

    /**
     * Initialise the OS before creating our threads.
     *
     * Note that we cannot enable stack-checking on the idle thread on
     * this platform because we are already using part of the idle
     * thread's stack now as our startup stack. Prefilling for stack
     * checking would overwrite our current stack.
     *
     * If you are not reusing the idle thread's stack during startup then
     * you are free to enable stack-checking here.
     */
    status = atomOSInit(&idle_thread_stack[0], IDLE_STACK_SIZE_BYTES, FALSE);
    if (status == ATOM_OK)
    {
        /* Enable the system tick timer */
        avrInitSystemTickTimer();

        /* Create an application thread */
        status = atomThreadCreate(&main_tcb,
                     MAIN_THREAD_PRIO, main_thread_func, 0,
                     &main_thread_stack[0],
                     MAIN_STACK_SIZE_BYTES,
                     TRUE);
    }
    if (status == ATOM_OK)
    {
        /* Create an application thread */
        status = atomThreadCreate(&comm_tcb,
                  COMM_THREAD_PRIO, comm_thread_func, 0,
                  &comm_thread_stack[0],
                  COMM_STACK_SIZE_BYTES,
                  TRUE);
    }
    if (status == ATOM_OK)
    {
        /**
         * First application thread successfully created. It is
         * now possible to start the OS. Execution will not return
         * from atomOSStart(), which will restore the context of
         * our application thread and start executing it.
         *
         * Note that interrupts are still disabled at this point.
         * They will be enabled as we restore and execute our first
         * thread in archFirstThreadRestore().
         */
        atomOSStart();
    }

    while (1)
        ;

    /* There was an error starting the OS if we reach here */
    return (0);
}


//static ATOM_TCB tcb[NUM_TEST_THREADS];

/**
 * \b main_thread_func
 *
 * Entry point for main application thread.
 *
 * This is the first thread that will be executed when the OS is started.
 *
 * @param[in] data Unused (optional thread entry parameter)
 *
 * @return None
 */
static void main_thread_func (uint32_t data)
{
    /*
    if (atomThreadCreate (&tcb[0], TEST_THREAD_PRIO + 1, test_thread_func, 0,
            &test_thread_stack[0][0],
            TEST_THREAD_STACK_SIZE, TRUE) != ATOM_OK)
    {
        
    }
     */           
    //printf_P (PSTR("Pass\n"));

    /* Flash LED once per second if passed, very quickly if failed */
    int sleep_ticks = SYSTEM_TICKS_PER_SEC;

    /* Test finished, flash slowly for pass, fast for fail */
    while (1)
    {
        /* Toggle a LED (STK500-specific) */
        //PORTB ^= (1 << 7);

        /* Sleep then toggle LED again */
        atomTimerDelay (sleep_ticks);
    }

}

typedef uint8_t byte;
typedef uint16_t word;

#define FIFO_DEFINE(name, type, size)     \
    static struct {                \
      ATOM_MUTEX lock;             \
      type buffer[size];           \
      byte head;                   \
      byte count;                  \
    } name;

#define FIFO_INIT(name)     \
    {                       \
        name.head = 0;      \
        name.count = 0;     \
        atomMutexCreate(&name.lock);       \
    }

#define FIFO_SIZE(name)     ARRAY_SIZE(name.buffer)
#define FIFO_EMPTY(name)    (name.count == 0)
#define FIFO_FULL(name)     (name.count == FIFO_SIZE(name))
#define FIFO_COUNT(name)    (name.count)
#define FIFO_HEAD(name)     ((name.count > 0) ? name.buffer[name.head] : 0)

#define FIFO_POP(name, lvalue, timeout)      \
    {                       \
        atomMutexGet(&name.lock, timeout);   \
        if (name.count > 0) {           \
          lvalue = name.buffer[name.head]; \
          name.head++;                  \
          if (name.head == FIFO_SIZE(name)) name.head = 0;      \
          name.count--;             \
        }                           \
        atomMutexPut(&name.lock);  \
    }

#define FIFO_PUSH(name, value, timeout)     \
    {                                   \
        atomMutexGet(&name.lock, timeout);     \
        if (!FIFO_FULL(name)) {                 \
          byte tail = name.head + name.count;   \
          if (tail >= FIFO_SIZE(name)) tail -= FIFO_SIZE(name);   \
          name.buffer[tail] = value;    \
          name.count++;             \
        }                           \
        atomMutexPut(&name.lock);  \
    }


#define RX_QUEUE_SIZE               32

FIFO_DEFINE(rxQueue, byte, RX_QUEUE_SIZE);


static void comm_thread_func (uint32_t data)
{
    int sleep_ticks = SYSTEM_TICKS_PER_SEC / 20;
    
    //atomQueueCreate(&rxQueue, rxQueueBuffer, 1, ARRAY_SIZE(rxQueueBuffer));
    FIFO_INIT(rxQueue);
    
    //atomSemCreate(&rxCount, 0);

    enum { STATE_INIT, STATE_PREFIX };
    uint8_t state = STATE_INIT;

    /* Test finished, flash slowly for pass, fast for fail */
    while (1)
    {
        uint8_t data;

        FIFO_POP(rxQueue, data, 0);
        
        switch (state) {
            case STATE_INIT:
            break;
        }
        /* Toggle a LED (STK500-specific) */
        //PORTB ^= (1 << 7);

        /* Sleep then toggle LED again */
        atomTimerDelay (sleep_ticks);
    }

}

void avrInitSystemTickTimer ( void )
{
    /* Set timer 1 compare match value for configured system tick,
     * with a prescaler of 256. We will get a compare match 1A
     * interrupt on every system tick, in which we must call the
     * OS's system tick handler. */
    OCR1A = (F_CPU / 256 / SYSTEM_TICKS_PER_SEC);

    /* Enable compare match 1A interrupt */
#ifdef TIMSK
    TIMSK = _BV(OCIE1A);
#else
    TIMSK1 = _BV(OCIE1A);
#endif

    /* Set prescaler 256 */
    TCCR1B = _BV(CS12) | _BV(WGM12);
}

ISR (TIMER1_COMPA_vect)
{
    /* Call the interrupt entry routine */
    atomIntEnter();

    /* Call the OS system tick handler */
    atomTimerTick();

    /* Call the interrupt exit routine */
    atomIntExit(TRUE);
}

ISR (USART_RX_vect)
{
    /* Call the interrupt entry routine */
    atomIntEnter();

    uint8_t data = UDR0;
    FIFO_PUSH(rxQueue, data, 0);
    //atomSemPut(&rxCount);
    
    /* Call the interrupt exit routine */
    atomIntExit(FALSE);
}
