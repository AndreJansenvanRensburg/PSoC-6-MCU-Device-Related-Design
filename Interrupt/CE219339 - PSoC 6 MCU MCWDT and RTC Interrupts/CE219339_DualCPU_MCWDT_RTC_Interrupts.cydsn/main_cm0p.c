/*******************************************************************************
* File Name: main_cm0p.c
*
* Version: 1.20
*
* Description:
* This is the source code for the example project of dual CPU interrupts 
* involving Multi-Counter Watchdog Timer (MCWDT) and Real Time Clock (RTC) interrupts. 
* MCWDT is assigned to CM0+ and executed by the code in main_cm0p.c
*
* Note:
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

/* Macros for configuring project functionality */

#define LED_ON  (0u)
#define LED_OFF (1u)

/***************************************************************************//**
* Function Name: Mcwdt_Handler
********************************************************************************
*
*  This function is executed when MCWDT interrupt is triggered
*  This ISR clears the interrupt source and toggles an Orange LED 
*
*******************************************************************************/
void Mcwdt_Handler()
{
    uint32 mcwdtIsrMask;
    mcwdtIsrMask = Cy_MCWDT_GetInterruptStatus(MCWDT_CM0p_HW);
    if(0u != (CY_MCWDT_CTR0 & mcwdtIsrMask))
    {
        Cy_MCWDT_ClearInterrupt(MCWDT_CM0p_HW, CY_MCWDT_CTR0);  
        Cy_GPIO_Inv(Pin_CM0p_MCWDT_OrangeLED_0_PORT, Pin_CM0p_MCWDT_OrangeLED_0_NUM);
    }  
}  
/*******************************************************************************
* Function Name: main
****************************************************************************//**
*
* The main function for the Cortex-M0+ CPU does the following:
*  Initialization:
*   - Enables the other CPU.
*   - Initializes and enables MCWDT interrupts
*   - Initializes and enables MCWDT component
*   
*  Do forever loop: 
*
*******************************************************************************/

int main(void)
{
    __enable_irq(); /* Enable global interrupts. */
    
    /* Enable CM4 */
    Cy_SysEnableCM4(CY_CORTEX_M4_APPL_ADDR); 
    
    /* Initialize and enable MCWDT interrupts */
    Cy_SysInt_Init(&MCWDT_Int_cfg, Mcwdt_Handler);
    NVIC_ClearPendingIRQ((IRQn_Type) MCWDT_Int_cfg.intrSrc);
    NVIC_EnableIRQ((IRQn_Type) MCWDT_Int_cfg.intrSrc);
   
    /* Start MCWDT component*/
    Cy_MCWDT_Init(MCWDT_CM0p_HW, &MCWDT_CM0p_config);
    Cy_MCWDT_Enable(MCWDT_CM0p_HW, MCWDT_CM0p_ENABLED_CTRS_MASK, 0u);
  
    /* Set MCWDT interrupt mask*/
    Cy_MCWDT_SetInterruptMask(MCWDT_CM0p_HW, CY_MCWDT_CTR0);

    for(;;)
    {
       
    }
}


/* [] END OF FILE */
