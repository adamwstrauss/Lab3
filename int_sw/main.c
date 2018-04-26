
//*****************************************************************************
//
// Application Name     - int_sw
// Application Overview - The objective of this application is to demonstrate
//							GPIO interrupts using SW2 and SW3.
//							NOTE: the switches are not debounced!
//
//*****************************************************************************

//****************************************************************************
//
//! \addtogroup int_sw
//! @{
//
//****************************************************************************

// Standard includes
#include <stdio.h>

// Driverlib includes
#include "hw_types.h"
#include "hw_ints.h"
#include "hw_memmap.h"
#include "hw_common_reg.h"
#include "interrupt.h"
#include "hw_apps_rcm.h"
#include "prcm.h"
#include "rom.h"
#include "rom_map.h"
#include "prcm.h"
#include "gpio.h"
#include "utils.h"
#include "timer.h"

// Common interface includes
#include "uart_if.h"
#include "timer_if.h"


#include "pin_mux_config.h"

unsigned long g_ulTimerInts;


//*****************************************************************************
//                 GLOBAL VARIABLES -- Start
//*****************************************************************************
extern void (* const g_pfnVectors[])(void);

volatile unsigned long SW2_intcount;
volatile unsigned long SW3_intcount;
volatile unsigned char SW2_intflag;
volatile unsigned char SW3_intflag;
static volatile unsigned long g_ulBase;


unsigned int timerValues[100];
int timerValuesLoc;


//*****************************************************************************
//                 GLOBAL VARIABLES -- End
//*****************************************************************************

// an example of how you can use structs to organize your pin settings for easier maintenance
typedef struct PinSetting {
    unsigned long port;
    unsigned int pin;
} PinSetting;

static PinSetting switch2 = { .port = GPIOA2_BASE, .pin = 0x40};

//*****************************************************************************
//                      LOCAL FUNCTION PROTOTYPES                           
//*****************************************************************************
static void BoardInit(void);

//*****************************************************************************
//                      LOCAL FUNCTION DEFINITIONS                         
//*****************************************************************************
static void GPIOA1IntHandler(void) { // SW3 handler
    unsigned long ulStatus;

    ulStatus = MAP_GPIOIntStatus (GPIOA1_BASE, true);
    MAP_GPIOIntClear(GPIOA1_BASE, ulStatus);		// clear interrupts on GPIOA1
    SW3_intcount++;
    SW3_intflag=1;
}



static void GPIOA2IntHandler(void) {	// SW2 handler
	unsigned long ulStatus;

    ulStatus = MAP_GPIOIntStatus (switch2.port, true);
    MAP_GPIOIntClear(switch2.port, ulStatus);		// clear interrupts on GPIOA2
    SW2_intcount++;
    SW2_intflag=1;
}


static void GPIOButtonIntHandler(void) { // SW3 handler
    unsigned long ulStatus;

    if(timerValuesLoc < 100)
        timerValues[timerValuesLoc++] = Timer_IF_GetCount(g_ulBase, TIMER_A);
    ulStatus = MAP_GPIOIntStatus (GPIOA2_BASE, true);
    MAP_GPIOIntClear(GPIOA2_BASE, ulStatus);        // clear interrupts on GPIOA1
    SW3_intcount++;
    SW3_intflag=1;
    Timer_IF_ReLoad(g_ulBase, TIMER_A,
                    500);
}
//*****************************************************************************
//
//! Board Initialization & Configuration
//!
//! \param  None
//!
//! \return None
//
//*****************************************************************************
static void
BoardInit(void) {
	MAP_IntVTableBaseSet((unsigned long)&g_pfnVectors[0]);
    
    // Enable Processor
    //
    MAP_IntMasterEnable();
    MAP_IntEnable(FAULT_SYSTICK);

    PRCMCC3200MCUInit();
}


//****************************************************************************
//
//! Main function
//!
//! \param none
//! 
//!
//! \return None.
//
//****************************************************************************
int main() {
	unsigned long ulStatus;
	timerValuesLoc = 0;

    BoardInit();
    
    PinMuxConfig();
    
    InitTerm();

    ClearTerm();

    //
    // Base address for first timer
    //
    g_ulBase = TIMERA0_BASE;

    //
    // Configuring the timers
    //
    Timer_IF_Init(PRCM_TIMERA0, g_ulBase, TIMER_CFG_ONE_SHOT_UP, TIMER_A, 0);

    //
    // Turn on the timers feeding values in mSec
    //
    Timer_IF_Start(g_ulBase, TIMER_A, 500);

    //
    // Register the interrupt handlers
    //
   // MAP_GPIOIntRegister(GPIOA1_BASE, GPIOA1IntHandler);
    //MAP_GPIOIntRegister(switch2.port, GPIOA2IntHandler);
    MAP_GPIOIntRegister(GPIOA2_BASE, GPIOButtonIntHandler);

    //
    // Configure rising edge interrupts on SW2 and SW3
    //
   //MAP_GPIOIntTypeSet(GPIOA1_BASE, 0x20, GPIO_RISING_EDGE);	// SW3
   // MAP_GPIOIntTypeSet(switch2.port, switch2.pin, GPIO_RISING_EDGE);	// SW2
    MAP_GPIOIntTypeSet(GPIOA2_BASE, 0x2, GPIO_FALLING_EDGE);

    //ulStatus = MAP_GPIOIntStatus (GPIOA1_BASE, false);
    //MAP_GPIOIntClear(GPIOA1_BASE, ulStatus);			// clear interrupts on GPIOA1
    //ulStatus = MAP_GPIOIntStatus (switch2.port, false);
    //MAP_GPIOIntClear(switch2.port, ulStatus);			// clear interrupts on GPIOA2

    ulStatus = MAP_GPIOIntStatus (GPIOA2_BASE, false);
    MAP_GPIOIntClear(GPIOA2_BASE, ulStatus);

    // clear global variables
    SW2_intcount=0;
    SW3_intcount=0;
    SW2_intflag=0;
    SW3_intflag=0;

    // Enable SW2 and SW3 interrupts
   // MAP_GPIOIntEnable(GPIOA1_BASE, 0x20);
    //MAP_GPIOIntEnable(switch2.port, switch2.pin);
    MAP_GPIOIntEnable(GPIOA2_BASE, 0x2);


    Message("\t\t****************************************************\n\r");
    Message("\t\t\tPush SW3 or SW2 to generate an interrupt\n\r");
    Message("\t\t ****************************************************\n\r");
    Message("\n\n\n\r");
	Report("SW2 ints = %d\tSW3 ints = %d\r\n",SW2_intcount,SW3_intcount);

    while (1) {
    	while ((SW2_intflag==0) && (SW3_intflag==0)) {;}
    	if (SW2_intflag) {
    		SW2_intflag=0;	// clear flag
    		Report("SW2 ints = %d\tSW3 ints = %d\r\n",SW2_intcount,SW3_intcount);
    	}
    	if (SW3_intflag) {
    		SW3_intflag=0;	// clear flag
    		Report("SW2 ints = %d\tSW3 ints = %d\r\n",SW2_intcount,SW3_intcount);
    	}
    }
}

//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************
