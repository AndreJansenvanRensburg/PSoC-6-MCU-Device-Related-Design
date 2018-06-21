/***************************************************************************//**
* \file main_cm0p.c
* \version 1.20
*
* This file provides App0 Core0 example source.
* App0 Core0 firmware does the following:
* - Starts App0 Core1 firmware.
* - If required switches to App1.
*
********************************************************************************
* \copyright
* Copyright 2016-2017, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include "bootloader/cy_bootload.h"
#include "project.h"

#if CY_BOOTLOAD_OPT_CRYPTO_HW != 0
    #include "cy_crypto_config.h"
    cy_stc_crypto_server_context_t cryptoServerContext;
#endif

/*******************************************************************************
* Function Name: main
********************************************************************************
*
* Summary:
*  Main function of App#0 core0. Initializes core1 (CM4) and waits forever.
*
* Parameters:
*  None
* 
* Return:
*  None
*
*******************************************************************************/
int main(void)
{
    /* enable global interrupts */
    __enable_irq();
    
#if CY_BOOTLOAD_OPT_CRYPTO_HW != 0
    /* Start the Crypto Server */
    Cy_Crypto_Server_Start(&cryptoConfig, &cryptoServerContext);
#endif

    /* start up M4 core, with the CM4 core start address defined in the
       Bootloader SDK linker script */
    Cy_SysEnableCM4( (uint32_t)(&__cy_app_core1_start_addr) );

    for (;;)
    {
        /* empty */
    }
}

/*******************************************************************************
* Function Name: Cy_OnResetUser
********************************************************************************
*
* Summary:
*  This function is called at the start of Reset_Handler(). It is weak function
*  that may be redefined by user code.
*  Bootloader SDK requires it to call Cy_Bootload_OnResetApp0() is app#0 core0.
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
void Cy_OnResetUser(void)
{
    Cy_Bootload_OnResetApp0();
}

/* [] END OF FILE */
