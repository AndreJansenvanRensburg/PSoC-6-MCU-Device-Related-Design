/*****************************************************************************
* File Name: main_cm4.c
*
* Description:  This file contains the implementation of the main function of
* the CM4.
*
* Related Document: Code example CE219881
*
* Hardware Dependency: See code example CE219881
*
******************************************************************************
* Copyright (2017), Cypress Semiconductor Corporation.
******************************************************************************
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
*****************************************************************************/
#include "project.h"

/* Wake-up Interrupt pin config structure (P0[4]) */
cy_stc_sysint_t WakeupIsrPin =
{
    /* Wake-up pin is located in Port 0 */
    .intrSrc = ioss_interrupts_gpio_0_IRQn,
    .intrPriority = 0,
};

/* Auxiliary Prototype functions */
void WakeupInterruptHandler(void);

/*******************************************************************************
* Function Name: main
****************************************************************************//**
*
* The main function for the Cortex-M4 CPU does the following:
*  Initialization:
*  - Initialize the interrupt pin.
*  Do forever loop:
*  - Go to deep sleep.
*  - When awake, check if CM0 sleeps. If yes, send event to wake it up.
*  
*******************************************************************************/
int main(void)
{
    /* Enable global interrupts. */
    __enable_irq(); 
    
    /* Initialize the Wake-up Interrupt */
    Cy_SysInt_Init(&WakeupIsrPin, WakeupInterruptHandler);
    
    /* Configure pin interrupt */
    Cy_GPIO_SetInterruptMask(SW2_PORT, SW2_NUM, 0x01);
    Cy_GPIO_SetInterruptEdge(SW2_PORT, SW2_NUM, CY_GPIO_INTR_RISING);
    
    /* Enable ISR to wakeup pin */
    NVIC_EnableIRQ(WakeupIsrPin.intrSrc);
    
    for(;;)
    {
        /* Go to deep sleep till the SW2 is pressed */
        Cy_SysPm_DeepSleep(CY_SYSPM_WAIT_FOR_INTERRUPT);
        
        /* If awake, check if CM0 is sleeping, then wake it up */
        if (Cy_SysPm_Cm0IsSleep() || Cy_SysPm_Cm0IsDeepSleep())
        {
            __SEV();
        }
    }
}

/*******************************************************************************
* Function Name: WakeupInterruptHandler
****************************************************************************//**
*
* Wake-up pin interrupt handler. Clear the interrupt only.
*  
*******************************************************************************/
void WakeupInterruptHandler(void)
{
    /* Clear any pending interrupt */
    if (0u != Cy_GPIO_GetInterruptStatusMasked(SW2_PORT, SW2_NUM))
    {
        Cy_GPIO_ClearInterrupt(SW2_PORT, SW2_NUM);
    }
}
    
/* [] END OF FILE */
