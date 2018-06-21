/*******************************************************************************
* File Name: main_cm4.c
*
* Version: 1.20
*
* Description:
* This is the source code for the example project of dual CPU interrupts 
* involving Multi-Counter Watchdog and Real Time Clock interrupts. 
* The project has an RTC component,that is executed by code in main_cm4.c
*
* Note: The main.c files for both of the CPUs in the PSoC 6 are part of the
* same PSoC Creator project. They are both compiled at project build time.
* The resultant .hex file has the code for both CPUs. When the project
* executes, the code in main_cm0p.c starts executing first. The Cortex-M0+
* code then turns on the Cortex-M4 CPU, and the code in main_cm4.c starts
* executing.
** Note:
*
* Owners:
*   jsln@cypress.com
*   arvi@cypress.com
*
* Related Documents:
*   CE219339 – PSOC 6 MCU - MCWDT and RTC Interrupts (Dual CPU)
*   AN217666 - PSoC 6 MCU Interrupts
*
* Hardware Dependency:
*  1. PSoC 6 MCU device
*  2. CY8CKIT-062-BLE Pioneer Kit
*
* Code Tested With:
*  1. PSoC Creator 4.2
*   a. ARM GCC
*   b. ARM MDK
*
********************************************************************************
* Copyright 2017, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include "project.h"

/*******************************************************************************
* Macro Definitions for project functionality
*******************************************************************************/

/* Time in seconds between two successive RTC interrupts */
#define RTC_INTR_TIME_SECONDS   (5u) 

#define NUM_SEC_IN_MINUTE       (60u)
#define ALARM1_ENABLED          (1u)
#define ALARM1_DISABLED         (0u)

/*******************************************************************************
* Global variables
*******************************************************************************/

/* RTC alarm status */
static uint8_t alarmStatus = 0;

/* Structure variable to store RTC alarm info */
cy_stc_rtc_alarm_t rtcAlarm;

/*******************************************************************************
* Function Name: RTC_UpdateAlarm
****************************************************************************//**
* This function fetches the current date/time from RTC and
* sets the alarm to trigger an interrupt 'RTC_INTR_TIME_SECONDS' 
* in the future for the first time. All the alarm fields should have valid data
*******************************************************************************/
static inline void RTC_SetAlarm()
{
    /* Structure variable to store RTC date and time info */
    cy_stc_rtc_config_t currentDateTime;
    
    /* Get current date and time from RTC registers */
    Cy_RTC_GetDateAndTime(&currentDateTime);
    
    /* Structure variable to store RTC alarm info */

    rtcAlarm.sec = ((currentDateTime.sec + RTC_INTR_TIME_SECONDS) % NUM_SEC_IN_MINUTE),
    rtcAlarm.secEn = CY_RTC_ALARM_ENABLE ;
    rtcAlarm.min = RTC_CM4_INITIAL_DATE_SEC;
    rtcAlarm.minEn = CY_RTC_ALARM_DISABLE;
    rtcAlarm.hour = RTC_CM4_INITIAL_DATE_HOUR;
    rtcAlarm.hourEn = CY_RTC_ALARM_DISABLE;
    rtcAlarm.dayOfWeek = RTC_CM4_INITIAL_DATE_DOW;
    rtcAlarm.dayOfWeekEn = CY_RTC_ALARM_DISABLE;
    rtcAlarm.date = RTC_CM4_INITIAL_DATE_DOM ;
    rtcAlarm.dateEn = CY_RTC_ALARM_DISABLE;
    rtcAlarm.month = RTC_CM4_INITIAL_DATE_MONTH;
    rtcAlarm.monthEn = CY_RTC_ALARM_DISABLE;
    rtcAlarm.almEn = CY_RTC_ALARM_ENABLE;
    
    /* Set Alarm Date and time to RTC registers and wait for successful operation */
    while(Cy_RTC_SetAlarmDateAndTime(&rtcAlarm, CY_RTC_ALARM_1) != CY_RET_SUCCESS);
}

/*******************************************************************************
* Function Name: RTC_UpdateAlarm
****************************************************************************//**
* This function fetches the current date/time from RTC and
* sets the alarm to trigger an interrupt 'RTC_INTR_TIME_SECONDS' 
* in the future.
*******************************************************************************/

static inline void RTC_UpdateAlarm()
{
    /* Structure variable to store RTC date and time info */
    cy_stc_rtc_config_t currentDateTime;
    
    /* Get current date and time from RTC registers  */
    Cy_RTC_GetDateAndTime(&currentDateTime);
    
    /* set alarm for RTC_INTR_TIME_SECONDS seconds from the current date and time */
    rtcAlarm.sec = ((currentDateTime.sec + RTC_INTR_TIME_SECONDS) % NUM_SEC_IN_MINUTE);
    
    /* Set Alarm time to RTC registers and wait for successful operation */
    while(Cy_RTC_SetAlarmDateAndTime(&rtcAlarm, CY_RTC_ALARM_1) != CY_RET_SUCCESS);
}
/*******************************************************************************
* Function Name: Cy_RTC_Alarm1Interrupt
****************************************************************************//**
* This function is the handler for the RTC Alarm1 Interrupt.  
* It clears the interrupt source and toggles a RED LED. 
* This function is declared as a blank weak function in RTC_CM4.h. 
*******************************************************************************/

void Cy_RTC_Alarm1Interrupt()
{   
    alarmStatus = ALARM1_ENABLED;
}

/*******************************************************************************
* Function Name: main
****************************************************************************//**
*
* The main function for the Cortex-M4 CPU does the following:
*  Initialization:
*   - Initializes RTC component
*   - Initializes and enables RTC interrupt
*   - Set initial RTC Alarm date and time
*  Do forever loop:
*   - Check the status of alarm 
*   - Update alarm and toggle red LED is alarm status is set
*******************************************************************************/

int main(void)
{
    __enable_irq(); /* Enable global interrupts. */
    
    /* Start RTC and enable interrupt */
    Cy_RTC_Init(&RTC_CM4_config);
    Cy_SysInt_Init(&RTC_CM4_RTC_IRQ_cfg, &RTC_CM4_Interrupt);
    NVIC_EnableIRQ(RTC_CM4_RTC_IRQ_cfg.intrSrc);
    
    /* Configures the source to trigger an interrupt */
    Cy_RTC_SetInterruptMask(CY_RTC_INTR_ALARM1);
    
    /* Set the initial RTC alarm condition */
    RTC_SetAlarm(); 

    for(;;)
    {   
        if (alarmStatus)
        {
            alarmStatus = ALARM1_DISABLED;
            
            /* Update the alarm  alarm to trigger an interrupt 
            'RTC_INTR_TIME_SECONDS' from the current time */
            RTC_UpdateAlarm();
            
            Cy_GPIO_Inv(Pin_CM4_RTC_RedLED_0_PORT, Pin_CM4_RTC_RedLED_0_NUM); 
        }
    }  
}

/* [] END OF FILE */
