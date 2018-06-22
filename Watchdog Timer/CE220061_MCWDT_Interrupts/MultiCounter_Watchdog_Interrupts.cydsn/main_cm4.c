/******************************************************************************
* File Name: main_cm4.c
*
* Version: 1.30
*
* Description: This example uses two MCWDT PSoC Creator Components available in
* the PSoC 6 MCUs to generate periodic events. These periodic events are used 
* to drive GPIO pins.
*
* Related Document: CE220061_PSoC6MCU_MultiCounterWatchdogInterrupts.pdf
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

void MCWDT_0_Interrupt(void);
void MCWDT_1_Interrupt(void);

/*******************************************************************************
*        Constants
*******************************************************************************/

/*******************************************************************************
*        Global variables
*******************************************************************************/

/***************************************************************************//**
* Function Name: MCWDT_0_Interrupt
********************************************************************************
*
* Summary:
*  This function is registered to be called when MCWDT_0 interrupt happens.
*  Three sub-counter interrupt outputs are combined into a single interrupt
*  request. There is local masking before combining, so each sub-counter 
*  interrupt may be individually blocked or passed to the combined interrupt 
*  output.
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
void MCWDT_0_Interrupt(void)
{
    uint32 mcwdtIsrMask;

    mcwdtIsrMask = Cy_MCWDT_GetInterruptStatus(MCWDT_0_HW);

    /* Toggle the Output Counter00_Out if interrupt cause is MCWDT_0 Counter0*/
    if (0u != (CY_MCWDT_CTR0 & mcwdtIsrMask))
    {
        Cy_MCWDT_ClearInterrupt(MCWDT_0_HW, CY_MCWDT_CTR0);
        Cy_GPIO_Inv(Counter00_Out_0_PORT, Counter00_Out_0_NUM);  
    }

    /* Toggle the Output Counter01_Out if interrupt cause is MCWDT_0 Counter1*/
    if (0u != (CY_MCWDT_CTR1 & mcwdtIsrMask))
    {
        Cy_MCWDT_ClearInterrupt(MCWDT_0_HW, CY_MCWDT_CTR1);
        Cy_GPIO_Inv(Counter01_Out_0_PORT, Counter01_Out_0_NUM);  
    }

    /* Toggle the Output Counter02_Out if interrupt cause is MCWDT_0 Counter2*/
    if (0u != (CY_MCWDT_CTR2 & mcwdtIsrMask))
    {
        Cy_MCWDT_ClearInterrupt(MCWDT_0_HW, CY_MCWDT_CTR2);
        Cy_GPIO_Inv(Counter02_Out_0_PORT, Counter02_Out_0_NUM);
    }  
}

/***************************************************************************//**
* Function Name: MCWDT_1_Interrupt
********************************************************************************
*
* Summary:
*  This function is registered to be called when MCWDT_1 interrupt happens.
*  Three sub-counter interrupt outputs are combined into a single interrupt
*  request. There is local masking before combining, so each sub-counter 
*  interrupt may be individually blocked or passed to the combined interrupt 
*  output.
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
void MCWDT_1_Interrupt(void)
{
    uint32 mcwdtIsrMask;

    mcwdtIsrMask = Cy_MCWDT_GetInterruptStatus(MCWDT_1_HW);
	
    /* Toggle the Output Counter10_Out if interrupt cause is MCWDT_1 Counter0*/
    if (0u != (CY_MCWDT_CTR0 & mcwdtIsrMask))
    {
        Cy_MCWDT_ClearInterrupt(MCWDT_1_HW, CY_MCWDT_CTR0);
        Cy_GPIO_Inv(Counter10_Out_0_PORT, Counter10_Out_0_NUM);
    }

    /* Toggle the Output Counter11_Out if interrupt cause is MCWDT_1 Counter1*/
    if (0u != (CY_MCWDT_CTR1 & mcwdtIsrMask))
    {
        Cy_MCWDT_ClearInterrupt(MCWDT_1_HW, CY_MCWDT_CTR1);
        Cy_GPIO_Inv(Counter11_Out_0_PORT, Counter11_Out_0_NUM);
    }

    /* Toggle the Output Counter12_Out if interrupt cause is MCWDT_1 Counter2*/
    if (0u != (CY_MCWDT_CTR2 & mcwdtIsrMask))
    {
        Cy_MCWDT_ClearInterrupt(MCWDT_1_HW, CY_MCWDT_CTR2);
        Cy_GPIO_Inv(Counter12_Out_0_PORT, Counter12_Out_0_NUM);
    }    
}

/*******************************************************************************
* Function Name: main
********************************************************************************
*
* Summary:
*  This function initializes the Multi-Counter Watchdog component, interrupts 
*  and goes into Deep Sleep. MCWDT_0_Interrupt or MCWDT_1_Interrupt wakes CM4 
*  from Deep Sleep and executes corresponding ISRs.
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
    /* Initialize MCWDT_0 interrupt */
    Cy_SysInt_Init(&MCWDT0_ISR_cfg, MCWDT_0_Interrupt);
    NVIC_EnableIRQ(srss_interrupt_mcwdt_0_IRQn);

	/* Initialize MCWDT_1 interrupt */
    Cy_SysInt_Init(&MCWDT1_ISR_cfg, MCWDT_1_Interrupt);
    NVIC_EnableIRQ(srss_interrupt_mcwdt_1_IRQn);
  
    /* Set interrupt mask for MCWDT_0 */
	Cy_MCWDT_SetInterruptMask(MCWDT_0_HW, (CY_MCWDT_CTR0|CY_MCWDT_CTR1| \
	                                       CY_MCWDT_CTR2));
	
    /* Set interrupt mask for MCWDT_1 */
	Cy_MCWDT_SetInterruptMask(MCWDT_1_HW, (CY_MCWDT_CTR0|CY_MCWDT_CTR1| \
	                                       CY_MCWDT_CTR2));
    
    /* Start MCWDT_0 */
    Cy_MCWDT_Init(MCWDT_0_HW, &MCWDT_0_config);
    Cy_MCWDT_Enable(MCWDT_0_HW, CY_MCWDT_CTR0|CY_MCWDT_CTR1|CY_MCWDT_CTR2, \
	                MCWDT_0_TWO_LF_CLK_CYCLES_DELAY);
    
    /* Start MCWDT_1 */
    Cy_MCWDT_Init(MCWDT_1_HW, &MCWDT_1_config);
    Cy_MCWDT_Enable(MCWDT_1_HW, CY_MCWDT_CTR0|CY_MCWDT_CTR1|CY_MCWDT_CTR2, \
	                MCWDT_1_TWO_LF_CLK_CYCLES_DELAY);
    
    /* Enable global interrupts. */
    __enable_irq();

    for(;;)
    { 
        /* Puts CM4 into Deep Sleep. 
        MCWDT_0_Interrupt or MCWDT_1_Interrupt wakes CM4 from Deep Sleep 
        and executes corresponding ISRs. */
        Cy_SysPm_DeepSleep(CY_SYSPM_WAIT_FOR_INTERRUPT);
    }
}

/* [] END OF FILE */

