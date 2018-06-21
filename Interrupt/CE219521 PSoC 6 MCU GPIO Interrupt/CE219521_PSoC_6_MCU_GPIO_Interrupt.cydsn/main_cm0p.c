/*******************************************************************************
* File Name: main_cm0p.c
*
* Version: 1.0
*
* Description:
* This is the source code for GPIO Interrupt example project. 
* The CPU (CM0+) blinks a green LED at an interval of 1 second for 4 seconds  
* to indicate the CPU is in active mode. Red LED then turns ON, to indicate 
* the CPU has entered Deep Sleep. A switch when pressed, wakes the CPU from 
* Deep Sleep. Red LED switches OFF and green LED starts blinking at an interval
* of 500ms for 4 seconds to indicate the CPU has entered active mode again. 
* With every interrupt and execution of ISR, the interval of blinking is alternated 
* between 1s and 500 ms. 
*
* Note:
*
* Owners:
*   jsln@cypress.com
*
* Related Documents:
*   AN217666 - PSoC 6 MCU Interrupts
*   CE219521 - PSoC 6 MCU - GPIO Interrupt 
*
* Hardware Dependency:
*  1. PSoC 6 MCU device
*  2. CY8CKIT-062-BLE Pioneer Kit
*
* Code Tested With:
*  1. PSoC Creator 4.2
*   a. ARM GCC
*   b. ARM MDK

*****************************************​*************************************
* Copyright (2017), Cypress Semiconductor Corporation.
*****************************************​*************************************
* This software is owned by Cypress Semiconductor Corporation (Cypress) and is
* protected by and subject to worldwide patent protection (United States and
* foreign), United States copyright laws and international treaty provisions.
* Cypress hereby grants to licensee a personal, non-exclusive, non-transferable
* license to copy, use, modify, create derivative works of, and compile the
* Cypress Source Code and derivative works for the sole purpose of creating
* custom software in support of licensee product to be used only in conjunction
* with a Cypress integrated circuit as specified in the applicable agreement.
* Any reproduction, modification, translation, compilation, or representation of
* this software except as specified above is prohibited without the express
* written permission of Cypress.
*
* Disclaimer: CYPRESS MAKES NO WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, WITH
* REGARD TO THIS MATERIAL, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
* Cypress reserves the right to make changes without further notice to the
* materials described herein. Cypress does not assume any liability arising out
* of the application or use of any product or circuit described herein. Cypress
* does not authorize its products for use as critical components in life-support
* systems where a malfunction or failure may reasonably be expected to result in
* significant injury to the user. The inclusion of Cypress' product in a life-
* support systems application implies that the manufacturer assumes all risk of
* such use and in doing so indemnifies Cypress against all charges. Use may be
* limited by and subject to the applicable Cypress software license agreement.
*****************************************​************************************/

#include "project.h"

/* Macros for configuring project functionality (LED blink periods) */
#define DELAY_SHORT             (250)   /* milliseconds */
#define DELAY_LONG              (500)   /* milliseconds */
#define NUM_BLINK_SECONDS       (4)

#define LED_ON                  (0)
#define LED_OFF                 (1)

/* Global variables for configuring project functionality (LED blink periods) */
uint16_t delayMs = DELAY_LONG;
uint16_t count = 0;
uint16_t blinkCounts = 0;

/*******************************************************************************
* Function Name: 
********************************************************************************
*
* Summary: 
* This function is executed when GPIO interrupt is triggered.
* This ISR 
         i. Clears the interrupt source
        ii. Sets the delay between green LED toggling on wake up. 
       iii. Switches OFF the red LED to indicate core is out of deepsleep.
* Parameters : None
* Return : None
*
*******************************************************************************/
void SwInterruptIsr()
{   
    Cy_GPIO_ClearInterrupt(Pin_Switch_0_PORT, Pin_Switch_0_NUM);
    
    if (delayMs == DELAY_LONG)
    {
        delayMs = DELAY_SHORT;
    }
    else delayMs = DELAY_LONG;
    
    count = 0;

	NVIC_ClearPendingIRQ(SysInt_SW_cfg.intrSrc);
}

/*******************************************************************************
* Function Name: main
********************************************************************************
*
* The main function for the Cortex-M0+ CPU does the following:
*  Initialization:
*   - Initialize and enable GPIO interrupt
*   
*  Do forever loop: 
*   - Set the number of blinks the green LED makes depending on the delayMs in 
*     4 seconds and blink the green LED.
*   - Turn ON RED LED to indicate Deep Sleep. 
*   - CM0+ is put to Deep Sleep
*
*******************************************************************************/
int main(void)
{ 
    /* Enable global interrupts. */
    __enable_irq(); 
    
    /* Initialize and enable GPIO interrupt assigned to CM0+ */
    /* SysInt_SW_cfg structure is defined in cyfitter_sysint_cfg.c based on
       parameters entered in the Interrupts tab of CYDWR. */
    Cy_SysInt_Init(&SysInt_SW_cfg, SwInterruptIsr); 
    NVIC_ClearPendingIRQ(SysInt_SW_cfg.intrSrc);
    NVIC_EnableIRQ((IRQn_Type)SysInt_SW_cfg.intrSrc);
    
    for(;;)
    {   
        /* Set number of blinks of green LED based on the delayMs */
        blinkCounts = ((DELAY_LONG * NUM_BLINK_SECONDS) / delayMs);
        
        /* Blink green LED at a frequency determined by delayMs 
            for 'blinkCounts' number of times*/
        while (count < blinkCounts)
        {
            Cy_GPIO_Write(Pin_CPU_Active_GreenLED_0_PORT, Pin_CPU_Active_GreenLED_0_NUM, LED_ON); 
            Cy_SysLib_Delay(delayMs);
            Cy_GPIO_Write(Pin_CPU_Active_GreenLED_0_PORT, Pin_CPU_Active_GreenLED_0_NUM, LED_OFF);  
            Cy_SysLib_Delay(delayMs);
            count++;
        }
       
        /* Turn ON red LED to indicate deepsleep */
        Cy_GPIO_Write(Pin_CPU_DeepSleep_RedLED_0_PORT, Pin_CPU_DeepSleep_RedLED_0_NUM, LED_ON);
        Cy_SysPm_DeepSleep(CY_SYSPM_WAIT_FOR_INTERRUPT);    
    
        /* Turn OFF red LED to indicate CPU wakeup */
        Cy_GPIO_Write(Pin_CPU_DeepSleep_RedLED_0_PORT, Pin_CPU_DeepSleep_RedLED_0_NUM, LED_OFF);
    }
}


/* [] END OF FILE */
