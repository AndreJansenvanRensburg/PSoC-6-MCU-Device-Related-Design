/******************************************************************************
* File Name: main_cm4.c
*
* Version: 1.20
*
* Description: This example demonstrates the use cases of the watchdog timer 
* (WDT) in PSoC 6 MCU — WDT enabled to reset the device, WDT as a Deep Sleep wakeup 
* source, and WDT generating a periodic interrupt.
*
* Related Document: CE220060_PSoC6MCU_WatchdogTimer.pdf
*
* Hardware Dependency: CY8CKIT-062-BLE PSoC 6 BLE Pioneer kit
*
******************************************************************************
* Copyright (2017), Cypress Semiconductor Corporation.
******************************************************************************
* This software, including source code, documentation and related materials
* ("Software") is owned by Cypress Semiconductor Corporation (Cypress) and is
* protected by and subject to worldwide patent protection (United States and 
* foreign), United States copyright laws and international treaty provisions. 
* Cypress hereby grants to licensee a personal, non-exclusive, non-transferable
* license to copy, use, modify, create derivative works of, and compile the 
* Cypress source code and derivative works for the sole purpose of creating 
* custom software in support of licensee product, such licensee product to be
* used only in conjunction with Cypress's integrated circuit as specified in the
* applicable agreement. Any reproduction, modification, translation, compilation,
* or representation of this Software except as specified above is prohibited 
* without the express written permission of Cypress.
* 
* Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND, 
* EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED 
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
* Cypress reserves the right to make changes to the Software without notice. 
* Cypress does not assume any liability arising out of the application or use
* of Software or any product or circuit described in the Software. Cypress does
* not authorize its products for use as critical components in any products 
* where a malfunction or failure may reasonably be expected to result in 
* significant injury or death ("ACTIVE Risk Product"). By including Cypress's 
* product in a ACTIVE Risk Product, the manufacturer of such system or application
* assumes all risk of such use and in doing so indemnifies Cypress against all
* liability. Use of this Software may be limited by and subject to the applicable
* Cypress software license agreement.
*****************************************************************************/

#include "project.h"

/*******************************************************************************
*        Function Prototypes
*******************************************************************************/

/*******************************************************************************
*        Constants
*******************************************************************************/

#define WDT_IGNOREBITS      (3u)
#define LED_BLINK_5_TIMES   (10u)
#define LED_ON              (0x00u)
#define LED_OFF             (0x01u)

/*******************************************************************************
*        Global variables
*******************************************************************************/

/* Interrupt happened counter */
uint32_t interruptCnt;

/***************************************************************************//**
* Function Name: WdtIsrHandler
********************************************************************************
*
* Summary:
* The following functions are executed in WdtIsrHandler:
* 1. If interrupt cause is not WDT counter then return to main function.
* 2. If interrupt cause is WDT counter, then check if interruptCnt>=10. 
*    If interruptCnt>=10 (means that ten WDT interrupts are serviced following 
*    device startup) stop servicing WDT interrupt and return to main function.
* 3. If interruptCnt<10 then service WDT interrupt. Clear WDT interrupt, 
*    Toggle LED BLUE_LED_WDT_INT and increment interruptCnt.
*
* Parameters:
*  None
*
* Return:
*  None
*
* Side Effects:
*  None
*
*******************************************************************************/
void WdtIsrHandler(void)
{
    /* Check was the generated interrupt occurred from WDT */
    if(0u != (SRSS->SRSS_INTR & SRSS_SRSS_INTR_WDT_MATCH_Msk))
    {
        if (interruptCnt < LED_BLINK_5_TIMES)
        {
            /* Clear interrupts state */
            Cy_WDT_ClearInterrupt();
            
            /* Toggle BLUE_LED_WDT_INT LED */
            Cy_GPIO_Inv(BLUE_LED_WDT_INT_0_PORT, BLUE_LED_WDT_INT_0_NUM);
            
            /* Interrupt happened counter */
            interruptCnt++;
        }
        else
        {
            /* If interruptCnt>=10 (means that ten WDT interrupts are serviced
               following device startup) stop servicing WDT interrupt and return
               to main function.Note that the WDT interrupt is assumed to be 
               pending all the time. As a result, the control never 
               go out of the WdtIsrHandler.
               If Watchdog is used for code stall recovery purpose (reset), 
               it should be cleared in main function using Cy_WDT_ClearInterrupt()*/
        }
    }
    else
    {
        /* Do nothing because the other SRSS interrupt was occurred */
    }
}

/*******************************************************************************
* Function Name: main
********************************************************************************
*
* At the beginning the LED_Wdt (P0_3) pin is toggling five times. After 
* that the WDT resets the device. The LED_Wdt (P0_3) pin turns on for 0.7s
* to indicate the device WDT reset event was occurred.
*
*******************************************************************************/
int main(void)
{
    /* Flash GREEN_LED_WDT_RESET after WDT reset event. */
    if (CY_SYSLIB_RESET_HWWDT == Cy_SysLib_GetResetReason())
    {
        Cy_SysLib_Delay(300u);                
        Cy_GPIO_Write(GREEN_LED_WDT_RESET_0_PORT, GREEN_LED_WDT_RESET_0_NUM, LED_ON);        
        Cy_SysLib_Delay(700u);        
        Cy_GPIO_Write(GREEN_LED_WDT_RESET_0_PORT, GREEN_LED_WDT_RESET_0_NUM, LED_OFF);
    }
    /* Flash RED_LED_RESET after PowerUp or XRES pin press */
    else
    {
        Cy_SysLib_Delay(300u);
        Cy_GPIO_Write(RED_LED_RESET_0_PORT, RED_LED_RESET_0_NUM, LED_ON);
        Cy_SysLib_Delay(700u);
        Cy_GPIO_Write(RED_LED_RESET_0_PORT, RED_LED_RESET_0_NUM, LED_OFF);
    }

    /* Configure WDT interrupt  */
    Cy_SysInt_Init(&WDTIsr_cfg, WdtIsrHandler);
    NVIC_EnableIRQ(srss_interrupt_IRQn);
    
    /* Initialize and enable WDT */
    Cy_WDT_SetIgnoreBits(WDT_IGNOREBITS);
    Cy_WDT_UnmaskInterrupt();
    Cy_WDT_Enable();
    
    /* Enable global interrupts. */
    __enable_irq();

    while(1)
    {
        /* Puts CM4 into Deep Sleep. 
        WDT Interrupt wakes CM4 from Deep Sleep and executes WdtIsrHandler. */
        Cy_SysPm_DeepSleep(CY_SYSPM_WAIT_FOR_INTERRUPT);
    }
}

/* [] END OF FILE */

