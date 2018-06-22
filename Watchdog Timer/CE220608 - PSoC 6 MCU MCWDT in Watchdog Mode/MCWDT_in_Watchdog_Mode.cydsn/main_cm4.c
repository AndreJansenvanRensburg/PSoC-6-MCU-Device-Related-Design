/******************************************************************************
* File Name: main_cm4.c
*
* Version: 1.20
*
* The example starts an MCWDT counter in watchdog mode, clears the watchdog 
* until a button is pressed, then the device is put into Deep Sleep mode. After 
* the watchdog reset, an LED is flashed to confirm that the watchdog reset has 
* occurred.
*
* Related Document: CE220608_PSoC6MCU_MultiCounterWatchdogTimerinWatchdogMode.pdf
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
void Watchdog_ISR(void);
void handle_error(void);

/*******************************************************************************
*        Constants
*******************************************************************************/
#define LED_ON              (0x00u)
#define LED_OFF             (!LED_ON)

/*******************************************************************************
*        Global variables
*******************************************************************************/

/* Intialize the switch status to not pressed state */
uint32_t switch_pressed = 0;

/*******************************************************************************
* Function Name: handle_error
********************************************************************************
* Summary:
* This function processes unrecoverable errors such as MCWDT component 
* initialization error. In case of such error the system will Turn on 
* ERROR_REDLED and stay in an infinite loop of this function.
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
	
    /* Turn on the error LED */
    Cy_GPIO_Write(ERROR_REDLED_0_PORT, ERROR_REDLED_0_NUM, LED_ON);
    while(1u) {}
}

/***************************************************************************//**
* Function Name: Watchdog_ISR
********************************************************************************
*
* Summary:
* The following functions are executed in Watchdog_ISR:
* 1. If the switch is pressed then do not feed the watchdog.
* 2. If the switch is not pressed then feed the watchdog.
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
void Watchdog_ISR(void)
{
    uint32 mcwdtIsrMask;

    if(switch_pressed == 1)
    {
        /* If the switch is pressed then do not feed the watchdog */
    }
    else
    {
        /* If the switch is not pressed then feed the watchdog */
        mcwdtIsrMask = Cy_MCWDT_GetInterruptStatus(MCWDT_0_HW);
        if (0u != (CY_MCWDT_CTR0 & mcwdtIsrMask))
        {
            Cy_MCWDT_ClearInterrupt(MCWDT_0_HW, CY_MCWDT_CTR0);
            
            /* Toggle the INTR_BLUELED to indicate that we have fed the 
               watchdog */
            Cy_GPIO_Inv(INTR_BLUELED_0_PORT, INTR_BLUELED_0_NUM);  
        }
        
    }
}

/*******************************************************************************
* Function Name: main
********************************************************************************
*
* Summary:
* The following functions are executed in main:
* 1. Flash the RESET_GREENLED to indicate reset has occurred due to POR or XRES 
*    or Watchdog reset 
* 2. Initialize the MCWDT_0
* 3. Enable the global interrupts
* 4. Wait for the SW2 switch press. During this wait, if MCWDT_0 Counter0 
*    interrupt occurs, it will be serviced
* 5. If SW2 is pressed, then CM4 into Deep Sleep and wait for MCWDT_0_interrupt
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
    cy_en_mcwdt_status_t mcwdt_init_status;
    
    /* Flash the RESET_GREENLED to indicate reset has occurred due to POR or 
       XRES or Watchdog reset */
    switch_pressed = 0;
    Cy_SysLib_Delay(500u);
    Cy_GPIO_Write(RESET_GREENLED_0_PORT, RESET_GREENLED_0_NUM, LED_ON);
    Cy_SysLib_Delay(500u);    
    Cy_GPIO_Write(RESET_GREENLED_0_PORT, RESET_GREENLED_0_NUM, LED_OFF);
    Cy_SysLib_Delay(500u);
    
    /* Initialize the MCWDT_0 interrupt */
    Cy_SysInt_Init(&Watchdog_ISR_cfg, Watchdog_ISR);
    NVIC_EnableIRQ(srss_interrupt_mcwdt_0_IRQn);
    
    /* Set the interrupt mask for the MCWDT_0 */
	Cy_MCWDT_SetInterruptMask(MCWDT_0_HW, (CY_MCWDT_CTR0));
    
    /* Initialize the MCWDT_0 */
    mcwdt_init_status = Cy_MCWDT_Init(MCWDT_0_HW, &MCWDT_0_config);
    if(mcwdt_init_status!=CY_MCWDT_SUCCESS)
    {
        handle_error();
    }
    
    /* Enable the MCWDT_0 counters */
    Cy_MCWDT_Enable(MCWDT_0_HW, CY_MCWDT_CTR0, \
                    MCWDT_0_TWO_LF_CLK_CYCLES_DELAY);
   
    /* Enable the global interrupts. */
    __enable_irq();

    /* Wait for the SW2 switch press. During this wait, if MCWDT_0 Counter0 
       interrupt occurs, interrupt will be serviced */
    while (0UL != Cy_GPIO_Read(SW2_0_PORT, SW2_0_NUM));
    
    /* Flag that the switch is pressed */
    switch_pressed = 1;

    for(;;)
    { 
        /* Switch is pressed, put the CM4 to Deep Sleep. The watchdog is not fed, and
           a reset happens */
        Cy_SysPm_DeepSleep(CY_SYSPM_WAIT_FOR_INTERRUPT);
    }
}

/* [] END OF FILE */

