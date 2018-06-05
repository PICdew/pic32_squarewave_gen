/*
 * Generates 2 square wave at a fixed frequency - With no code. Just setup the Timer and OC
 * First wave is 50% +duty cycle
 * The other is 25% +duty cycle
 * Also flashes the LED - RA0 Pin2
 * 
 * Uses Proto thread library
 *    - Uses TMR0 as a 1ms timer. Refer PT_setup()
 *      TMR0 ISR calls PT_inc_timer()
 * Uses TMR1 as direct source to OC0 and OC1
 *    - TMR1 ISR does nothing.
 *      Timer 2 Interrupts are disabled in APP_Initialize (after the DRV_TMR1_Start(), which enables interrupts)
 * OC0 outputs the 50% duty cycle - RB7 Pin 16
 * OC1 outputs the 25% duty cycle - RB5 Pin 14
 * 
 * Use HMC to setup:
 *   - Drivers
 *     - Timer (Static) - +Interrupts
 *       - TMR0 - Timer 5
 *       - TMR1 - Timer 2
 *     - OC (Static)
 *       - OC0 - OC1
 *       - OC1 - OC2
 *   - Pin Settings
 *     - Pin 2 - RA0 - LED_AL
 *     - Pin 14 - RB5 - OC2
 *     - Pin 16 - RB7 - OC1
 */
#include "app.h"
#include "pt_harmony_1_0_0.h"

APP_DATA appData;

#define TMR1_HZ 4000
uint16_t tmr1_period;

// Millisecs between LED pin toggle
#define LED_TOGGLE_MS 500
static struct pt pt_led;

/* Protothread routine to toggle LED pin
 * LED Pin RA0 Pin2, needs to be setup in HMC as LED_AL
 * Jumper on Microstick needs to be closed.
 */
static PT_THREAD (protothread_led(struct pt *pt))
{
    PT_BEGIN(pt);
    while(1) {
        PT_YIELD_TIME_msec(LED_TOGGLE_MS) ;
        /* Toggle LED Pin RA0 Pin2 */
        SYS_PORTS_PinToggle(PORTS_ID_0, PORT_CHANNEL_A, PORTS_BIT_POS_0);
    }
    PT_END(pt);
}

/* Initialise App */
void APP_Initialize ( void )
{
    /* Place the App state machine in its initial state. */
    appData.state = APP_STATE_INIT;

     /* Setup and start TMR1
      * TMR1_HZ = 4 KHz
      * PB CLK = 40 MHz
      * tmr1_period = 40,000,000 / 4,000 = 10,000
      * Period = 1/4,000 = 250 us
      */
    tmr1_period=DRV_TMR1_CounterFrequencyGet()/TMR1_HZ;
    DRV_TMR1_ClockSet(DRV_TMR_CLKSOURCE_INTERNAL, TMR_PRESCALE_VALUE_1);
    DRV_TMR1_PeriodValueSet(tmr1_period);
    DRV_TMR1_Start();
    /* Disable interrupts on the TMR1 timer. Which is Timer 2*/
    PLIB_INT_SourceDisable(INT_ID_0, INT_SOURCE_TIMER_2);

    /* Set and start OC0 (OC_ID_1) */
   /* Don't need to enable the OC 
    * DRV_OC0_Enable();
    */
    /* Go high at 9999 and low at 5000 */
    DRV_OC0_CompareValuesDualSet(tmr1_period-1, tmr1_period>>1);
    DRV_OC0_Start();

    /* Set and start OC1 (OC_ID_2) */
    /* Go high at 2500 low at 5000 */
    DRV_OC1_CompareValuesDualSet(tmr1_period>>2, tmr1_period>>1);
    DRV_OC1_Start();
    
    /* Setup Proto Threads
     * Initialises TMR0 as a 1ms timer. ISR calls PT_inc_timer 
     */
    PT_setup();
    
    /* Initialise LED flasher thread*/
    PT_INIT(&pt_led);
}

/* App state machine */
void APP_Tasks ( void )
{
    PT_SCHEDULE(protothread_led(&pt_led));
}