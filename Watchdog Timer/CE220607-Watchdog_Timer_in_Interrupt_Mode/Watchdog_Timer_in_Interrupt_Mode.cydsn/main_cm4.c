/******************************************************************************
* File Name: main_cm4.c
*
* Version: 1.10
*
* Description: This example demonstrates how to use the WDT in interrupt mode, 
* blinks a kit LED using the WDT interrupt.
*
* Related Document: CE220607_PSoC6MCU_WatchdogTimerinInterruptMode.pdf
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
void WDTIsr(void);
void handle_error(void);

/*******************************************************************************
*        Constants
*******************************************************************************/

#define WDT_IGNOREBITS      (2u)
#define LED_ON              (0x00u)
#define LED_OFF             (!LED_ON)

/*******************************************************************************
*        Global variables
*******************************************************************************/

/*******************************************************************************
* Function Name: handle_error
********************************************************************************
* Summary:
* This function processes unrecoverable errors such as MCWDT component 
* initialization error. In case of such error the system will Turn on ERROR_LED 
* and stay in an infinite loop of this function.
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
void handle_error(void)
{
     /* Disable all interrupts */
    __disable_irq();
	
    /* Turn on error LED */
    Cy_GPIO_Write(RED_LED_0_PORT, RED_LED_0_NUM, LED_ON);
    while(1u) {}
}

/***************************************************************************//**
* Function Name: WDTIsr
********************************************************************************
*
* Summary:
* The following functions are executed in WDTIsr:
* 1. If the interrupt cause is not WDT counter then return to main function.
* 2. If the interrupt cause is WDT counter then clear WDT interrupt and toggle 
*    RED_LED 
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
void WDTIsr(void)
{
    /* Check, was the generated interrupt occurred from WDT */
    if(0u != (SRSS->SRSS_INTR & SRSS_SRSS_INTR_WDT_MATCH_Msk))
    {
        /* Clear interrupt */
        Cy_WDT_ClearInterrupt();
           
        /* Toggle RED_LED */
        Cy_GPIO_Inv(GREEN_LED_0_PORT, GREEN_LED_0_NUM);
    }
    else
    {
        /* Do nothing, because the other SRSS interrupt was occurred */
    }
}

/*******************************************************************************
* Function Name: main
********************************************************************************
*
* Summary:
* The following functions are executed in main:
* 1. Configure the WDT interrupt
* 2. Initialize and enable the WDT
* 3. Enable global interrupts
* 4. Put CM4 into Deep Sleep and wait for WDT interrupt
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
int main(void)
{
    cy_en_sysint_status_t sysint_status;
    
    /* Configure the WDT interrupt  */
    sysint_status = Cy_SysInt_Init(&WDTIsr_cfg, WDTIsr);
    if(sysint_status!=CY_SYSINT_SUCCESS)
    {
        handle_error();
    }
    
    NVIC_EnableIRQ(srss_interrupt_IRQn);
    
    /* Initialize and enable the WDT */
    /* WDT interrupt period can be varied by configuring WDT_IGNOREBITS */
    Cy_WDT_SetIgnoreBits(WDT_IGNOREBITS);
    Cy_WDT_UnmaskInterrupt();
    Cy_WDT_Enable();
    
    /* Enable global interrupts. */
    __enable_irq();

    for(;;)
    {
        /* Puts CM4 into Deep Sleep. */
        /* WDT interrupt wakes the CM4 from Deep Sleep and executes WDTIsr */
        Cy_SysPm_DeepSleep(CY_SYSPM_WAIT_FOR_INTERRUPT);
    }
}

/* [] END OF FILE */

