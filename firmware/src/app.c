#include "app.h"
#include "pt_harmony_1_0_0.h"

APP_DATA appData;
DRV_HANDLE tmr0_handle;
DRV_HANDLE oc0_handle;
DRV_HANDLE oc1_handle;

uint16_t tmr0_period=10000;

/* Initialise App */
void APP_Initialize ( void )
{
    /* Place the App state machine in its initial state. */
    appData.state = APP_STATE_INIT;

     /* Open and start the timer
     * 40MHz clk / 10,000 tick = 4 KHz
     * Period = 1/4,000 = 250 us
     */
    tmr0_handle = DRV_TMR_Open(DRV_TMR_INDEX_0, DRV_IO_INTENT_EXCLUSIVE);
    if ( DRV_TMR_CLIENT_STATUS_READY != DRV_TMR_ClientStatus(tmr0_handle) )
        return;
    /* Static driver does not map DRV_TMR_PeriodValueSet */
    DRV_TMR0_PeriodValueSet(tmr0_period);
    DRV_TMR_Start(tmr0_handle);

    /* Open and start OC0 (OC_ID_1) */
    oc0_handle = DRV_OC_Open(DRV_OC_INDEX_0, DRV_IO_INTENT_EXCLUSIVE);
    if ( DRV_OC_FaultHasOccurred(oc0_handle) )
        return;
   /* Don't need to enable the OC 
    * DRV_OC0_Enable();
    */
    /* Go high at 9999 and low at 5000 */
    DRV_OC_CompareValuesDualSet(oc0_handle, tmr0_period-1, tmr0_period>>1);
    DRV_OC_Start(oc0_handle, DRV_IO_INTENT_EXCLUSIVE);

    /* Open and start OC1 (OC_ID_2) */
    oc1_handle = DRV_OC_Open(DRV_OC_INDEX_1, DRV_IO_INTENT_EXCLUSIVE);
    if ( DRV_OC_FaultHasOccurred(oc1_handle) )
        return;
    /* Go high at 2500 low at 5000 */
    DRV_OC_CompareValuesDualSet(oc1_handle, tmr0_period>>2, tmr0_period>>1);
    DRV_OC_Start(oc1_handle, DRV_IO_INTENT_EXCLUSIVE);
    
    /* Setup Proto Threads */
    PT_setup();
}

/* App state machine */
void APP_Tasks ( void )
{

    /* Check the application's current state. */
    switch ( appData.state )
    {
        /* Application's initial state. */
        case APP_STATE_INIT:
        {
            /* Goto do nothing state */
            appData.state = APP_STATE_SERVICE_TASKS;
            break;
        }

        case APP_STATE_SERVICE_TASKS:
        {
            break;
        }
        /* The default state should never be executed. */
        default:
        {
            /* TODO: Handle error in application's state machine. */
            break;
        }
    }
}
