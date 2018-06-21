/***************************************************************************//**
* \file main_cm0p.c
* \version 1.00
*
* This file provides App1 Core0 example source for the bootloader dual-app code example.
* App1 Core0 firmware does the following:
* - Starts App1 Core1 firmware
*
********************************************************************************
* \copyright
* Copyright 2018, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include "bootloader/cy_bootload.h"
#include "project.h"


/*******************************************************************************
* Function Name: main
********************************************************************************
*
* Summary:
*  Main function of App#1 core0. Initializes core1 (CM4) and waits forever.
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
    /* start up M4 core */
    Cy_SysEnableCM4( (uint32_t)(&__cy_app_core1_start_addr) );

    for (;;)
    {
        /* empty */
    }
}


/* [] END OF FILE */
