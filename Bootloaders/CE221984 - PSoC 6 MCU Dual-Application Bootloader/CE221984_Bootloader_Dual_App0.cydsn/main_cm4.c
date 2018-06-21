/***************************************************************************//**
* \file main_cm4.c
* \version 1.00
*
* This file provides App0 Core1 example source for the bootloader dual-app code example.
* App0 Core1 firmware does the following:
* - If not started from software reset (SRES), tries to validate and switch to App1 or App2
*   - If in basic mode, tries App1 first
*   - If in factory default mode, tries App2 first
* - Bootloads an app from the host
*   - If in factory default mode and App1 is attempted to be bootloaded again,
*     turns on Red LED for five seconds and restarts bootloading
*   - Tries to validate and switch to the app just bootloaded, otherwise restarts bootloading
* - If button is pressed, tries to validate and switch to App1 or App2
*   - If in basic mode, tries App1 first
*   - If in factory default mode, tries App2 first
* - Blinks a Blue LED
* - Halts on timeout
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
#include <string.h>

/* Pin for user button SW2 */
#define PIN_SW2     GPIO_PRT0, 4u

/* Pin for the red LED in the kit RGB LED */
#define Red_LED     GPIO_PRT0, 3u

/* Pin for the blue LED in the kit RGB LED */
#define Blue_LED     GPIO_PRT11, 1u

#if CY_BOOTLOAD_OPT_CRYPTO_HW != 0
    #include "cy_crypto_config.h"
    cy_stc_crypto_context_t cryptoContext;
#endif /* CY_BOOTLOAD_OPT_CRYPTO_HW != 0 */


/*******************************************************************************
* Function Name: CopyRow
********************************************************************************
* Copies data from a "src" address to a flash row with the address "dest".
* If "src" data is the same as "dest" data then no copy is needed.
*
* Parameters:
*  dest     Destination address. Has to be an address of the start of flash row.
*  src      Source address. Has to be properly aligned.
*  rowSize  Size of flash row.
*
* Returns:
*  CY_BOOTLOAD_SUCCESS if operation is successful.
*  Error code in a case of failure.
*******************************************************************************/
cy_en_bootload_status_t CopyRow(uint32_t dest, uint32_t src, uint32_t rowSize, cy_stc_bootload_params_t * params)
{
    cy_en_bootload_status_t status;

    /* Save params->dataBuffer value */
    uint8_t *buffer = params->dataBuffer;

    /* Compare "dest" and "src" content */
    params->dataBuffer = (uint8_t *)src;
    status = Cy_Bootload_ReadData(dest, rowSize, CY_BOOTLOAD_IOCTL_COMPARE, params);

    /* Restore params->dataBuffer */
    params->dataBuffer = buffer;

    /* If "dest" differs from "src" then copy "src" to "dest" */
    if (status != CY_BOOTLOAD_SUCCESS)
    {
        (void) memcpy((void *) params->dataBuffer, (const void*)src, rowSize);
        status = Cy_Bootload_WriteData(dest, rowSize, CY_BOOTLOAD_IOCTL_WRITE, params);
    }
    /* Restore params->dataBuffer */
    params->dataBuffer = buffer;

    return (status);
}


/*******************************************************************************
* Function Name: HandleMetadata
********************************************************************************
* The goal of this function is to make Bootloader SDK metadata (MD) valid.
* The following algorithm is used (in C-like pseudocode):
* ---
* if (isValid(MD) == true)
* {   if (MDC != MD)
*         MDC = MD;
* } else
* {   if(isValid(MDC) )
*         MD = MDC;
*     else
*         MD = INITIAL_VALUE;
* }
* ---
* Here MD is metadata flash row, MDC is flash row with metadata copy,
* INITIAL_VALUE is known initial value.
*
* In this code example MDC is placed in the next flash row after the MD, and
* INITIAL_VALUE is MD with only CRC, App0 start and size initialized,
* all the other fields are not touched.
*
* Parameters:
*  params   A pointer to a Bootloader SDK parameters structure.
*
* Returns:
* - CY_BOOTLOAD_SUCCESS when finished normally.
* - Any other status code on error.
*******************************************************************************/
cy_en_bootload_status_t HandleMetadata(cy_stc_bootload_params_t *params)
{
    const uint32_t MD     = (uint32_t)(&__cy_boot_metadata_addr   ); /* MD address  */
    const uint32_t mdSize = (uint32_t)(&__cy_boot_metadata_length ); /* MD size, assumed to be one flash row */
    const uint32_t MDC    = MD + mdSize;                             /* MDC address */

    cy_en_bootload_status_t status = CY_BOOTLOAD_SUCCESS;
    
    status = Cy_Bootload_ValidateMetadata(MD, params);
    if (status == CY_BOOTLOAD_SUCCESS)
    {
        /* Checks if MDC equals to DC, if no then copies MD to MDC */
        status = CopyRow(MDC, MD, mdSize, params);
    }
    else
    {
        status = Cy_Bootload_ValidateMetadata(MDC, params);
        if (status == CY_BOOTLOAD_SUCCESS)
        {
            /* Copy MDC to MD */
            status = CopyRow(MD, MDC, mdSize, params);
        }
        if (status != CY_BOOTLOAD_SUCCESS)
        {
            const uint32_t elfStartAddress = 0x10000000;
            const uint32_t elfAppSize      = 0x8000;
            /* Set MD to INITIAL_VALUE */
            status = Cy_Bootload_SetAppMetadata(0u, elfStartAddress, elfAppSize, params);
        }
    }
    return (status);
}


/*******************************************************************************
* Function Name: counterTimeoutSeconds
********************************************************************************
* Returns number of counts that correspond to number of seconds passed as
* a parameter.
* E.g. comparing counter with 300 seconds is like this.
* ---
* uint32_t counter = 0u;
* for (;;)
* {
*     Cy_SysLib_Delay(UART_TIMEOUT);
*     ++count;
*     if (count >= counterTimeoutSeconds(seconds: 300u, timeout: UART_TIMEOUT))
*     {
*         count = 0u;
*         DoSomething();
*     }
* }
* ---
*
* Both parameters are required to be compile time constants,
* so this function gets optimized out to single constant value.
*
* Parameters:
*  seconds    Number of seconds to pass. Must be less that 4_294_967 seconds.
*  timeout    Timeout for Cy_Bootload_Continue() function, in milliseconds.
*             Must be greater than zero.
*             It is recommended to be a value that produces no reminder
*             for this function to be precise.
* Return:
*  See description.
*******************************************************************************/
static uint32_t counterTimeoutSeconds(uint32_t seconds, uint32_t timeout)
{
    return (seconds * 1000ul) / timeout;
}


/*******************************************************************************
* Function Name: LoadValidApp
********************************************************************************
* Validate and load the preferred application. Once it does not exist or invalid
* load the other valid application
* E.g. validating and loading application is like this.
* ---
* if (isValid(Prefrd_appId) == true)   
*	 Execute(Prefrd_appId);
* else
* {  isValid(Next_appId) == true )
*    Execute(Prefrd_appId);
* }
* ---
*
*
* Parameters:
*  Prefrd_appId    Preferred Application. Either Application 1 or Application 2
*  Next_appId      Application other than preferred application. Either Application 1 or Application 2
*  bootParamss     The pointer to a bootloader parameters structure.
*                  See \ref cy_stc_bootload_params_t .           
* Return:
*  See description.
*******************************************************************************/
void LoadValidApp(uint32_t Prefrd_appId, uint32_t Next_appId,cy_stc_bootload_params_t bootParamss)
{ 		
    /* Status codes for Bootloader SDK API */
    cy_en_bootload_status_t status;

    /* Validate and load preferred application*/
    status = Cy_Bootload_ValidateApp(Prefrd_appId, &bootParamss);

    if (status == CY_BOOTLOAD_SUCCESS)
    {
        /*
        * Clear the reset reason because Cy_Bootload_ExecuteApp() performs a 
        * software reset. Without clearing it, two reset reasons would be 
        * present.
        */
        do
        {
            Cy_SysLib_ClearResetReason();
        }while(Cy_SysLib_GetResetReason() != 0);

        Cy_Bootload_TransportStop();

        /* Never returns */
        Cy_Bootload_ExecuteApp(Prefrd_appId);
    }
	else
	{
        /* Validate and load the other application*/
	    status = Cy_Bootload_ValidateApp(Next_appId, &bootParamss);
		if (status == CY_BOOTLOAD_SUCCESS)
		{
			/*
			* Clear the reset reason because Cy_Bootload_ExecuteApp() performs a 
			* software reset. Without clearing it, two reset reasons would be 
			* present.
			*/
			do
			{
				Cy_SysLib_ClearResetReason();
			}while(Cy_SysLib_GetResetReason() != 0);
			
            Cy_Bootload_TransportStop();
            
			/* Never returns */
			Cy_Bootload_ExecuteApp(Next_appId);
		}
	}
}


/*******************************************************************************
* Function Name: main
********************************************************************************
*
* Summary:
*  Main function of the firmware application.
*  1. If application started from Non-Software reset it validates app #2
*  1.1. If app#2 is valid it switches to app#2, else goto #2.
*  2. If app#2 is not valid it validates app #1.
*  2.1. If app#1 is valid it switches to app#1, else goto #3.
*  3. Start bootloading communication.
*  4. If updated application has been received it validates this app.
*  5. If the new app is valid it switches to it, else wait for new application.
*  6. If 300 seconds has passed and no new application has been received
*     then validate app#2, if it is valid then switch to it, else validates app #1.
*  6.1. If app#1 is valid then switches to app#1, else freeze.


* Parameters:
*  seconds    Number of seconds to pass
*  timeout    Timeout for Cy_Bootload_Continue() function, in milliseconds
* 
* Return:
*  Counter value at which specified number of seconds has passed.
*
*******************************************************************************/
int main(void)
{
    /* timeout for Cy_Bootload_Continue(), in milliseconds */
    const uint32_t paramsTimeout = 20u;

    /* Bootloader params, used to configure bootloader */
    cy_stc_bootload_params_t bootParams;

    /* Status codes for Bootloader SDK API */
    cy_en_bootload_status_t status;

    /* 
    * Bootloading state, one of
    * - CY_BOOTLOAD_STATE_NONE
    * - CY_BOOTLOAD_STATE_BOOTLOADING
    * - CY_BOOTLOAD_STATE_FINISHED
    * - CY_BOOTLOAD_STATE_FAILED
    */
    uint32_t state;

    /*
    * Used to count seconds, to convert counts to seconds use
    * counterTimeoutSeconds(SECONDS, paramsTimeout)
    */
    uint32_t count = 0;

#if CY_BOOTLOAD_OPT_CRYPTO_HW != 0
    cy_en_crypto_status_t cryptoStatus;
#endif

    /* 
	* App0 can download and overwrite App1 or App2 at any time. 
	* Depending on Golden Image settings in bootload_user.h
	* define preferred AppID and next preferred AppID 
	* to operate in one of the two modes.
	*
	* (1) Basic mode:
	* Launching App1 is preferred unless it doesn’t validate,  
	* then App2 is launched. 
	*
	* (2) Factory default mode:
	* App0 can transfer control to app2 unless app2 
	* doesn't exist or is corrupted; then to app1.
	*/

#if CY_BOOTLOAD_OPT_GOLDEN_IMAGE != 0
    const uint32_t PreferredAppID = 2u;
    const uint32_t NextAppID = 1u; 
#else 
    const uint32_t PreferredAppID = 1u;
    const uint32_t NextAppID = 2u; 
      
#endif

#ifndef CY_BOOTLOAD_OPT_GOLDEN_IMAGE 
    uint32_t FirstPreferredApp = 1u;
    uint32_t SecondPreferredApp = 2u;
#endif /* CY_BOOTLOAD_OPT_GOLDEN_IMAGE */

    /* Buffer to store bootloader commands */
    CY_ALIGN(4) static uint8_t buffer[CY_BOOTLOAD_SIZEOF_DATA_BUFFER];

    /* Buffer for bootloader packets for Transport API */
    CY_ALIGN(4) static uint8_t packet[CY_BOOTLOAD_SIZEOF_CMD_BUFFER ];    

    /* Enable global interrupts */
    __enable_irq();

#if CY_BOOTLOAD_OPT_CRYPTO_HW != 0
    /* Initialize the Crypto Client code */
    cryptoStatus = Cy_Crypto_Init(&cryptoConfig, &cryptoContext);
    if (cryptoStatus != CY_CRYPTO_SUCCESS)
    {
        /* Crypto not initialized, debug what is the problem */
        Cy_SysLib_Halt(0x00u);
    }
#endif /* CY_BOOTLOAD_OPT_CRYPTO_HW != 0 */

    /* Initialize bootParams structure and Bootloader SDK state */
    bootParams.timeout          = paramsTimeout;
    bootParams.dataBuffer       = &buffer[0];
    bootParams.packetBuffer     = &packet[0];

    status = Cy_Bootload_Init(&state, &bootParams);

    /* Ensure Bootloader Metadata is valid */
    status = HandleMetadata(&bootParams);
    if (status != CY_BOOTLOAD_SUCCESS)
    {
        Cy_SysLib_Halt(0x00u);
    }

    /*
    * In the case of non-software reset check if there is a valid preferred application image.
    * If these is - switch to it. Else validate second preferred app and switch if present.
    */
    if (Cy_SysLib_GetResetReason() != CY_SYSLIB_RESET_SOFT)
    {
        LoadValidApp(PreferredAppID, NextAppID, bootParams);
    }

    /* Initialize bootloader communication */
    Cy_Bootload_TransportStart();

    for(;;)
    {
        status = Cy_Bootload_Continue(&state, &bootParams);
        ++count;

        if (state == CY_BOOTLOAD_STATE_FINISHED)
        {
            /* Finished bootloading the application image */
            /* Validate bootloaded application, if it is valid then switch to it */
            status = Cy_Bootload_ValidateApp(bootParams.appId, &bootParams);
            if (status == CY_BOOTLOAD_SUCCESS)
            {
                Cy_Bootload_TransportStop();
                Cy_Bootload_ExecuteApp(bootParams.appId);
            }
            else if (status == CY_BOOTLOAD_ERROR_VERIFY)
            {
                /*
                * Restarts Bootloading, an alternatives are to Halt MCU here
                * or switch to the other app if it is valid.
                * Error code may be handled here, i.e. print to debug UART.
                */
                status = Cy_Bootload_Init(&state, &bootParams);
                Cy_Bootload_TransportReset();
            }
        }
        else if (state == CY_BOOTLOAD_STATE_FAILED)
        {
            /* An error has happened during the bootloading process */
            /* Handle it here */
            /* In this Code Example just restart bootloading process */
            status = Cy_Bootload_Init(&state, &bootParams);
            Cy_Bootload_TransportReset();
        }
        else if (state == CY_BOOTLOAD_STATE_BOOTLOADING)
        {
            uint32_t passed5seconds = (count >= counterTimeoutSeconds(5u, paramsTimeout) ) ? 1u : 0u;
            /*
            * if no command has been received during 5 seconds when the bootloading
            * has started then restart bootloading.
            */
            if (status == CY_BOOTLOAD_SUCCESS)
            {
                count = 0u;
            }
            else if (status == CY_BOOTLOAD_ERROR_TIMEOUT)
            {
                if (passed5seconds != 0u)
                {
                    count = 0u;
                    Cy_Bootload_Init(&state, &bootParams);
                    Cy_Bootload_TransportReset();
                }
            }
            else
            {
                count = 0u;
                /* 
                * Tried to bootload golden image application.
                * Turns on Red LED for five seconds to denote an error. 
                */
                Cy_GPIO_Set(Blue_LED); /* turn off the LED */
                Cy_GPIO_Clr(Red_LED);  /* turn on the LED */
                Cy_SysLib_Delay(5000u);
                Cy_GPIO_Set(Red_LED);  /* turn off the LED */
                Cy_Bootload_Init(&state, &bootParams);
                Cy_Bootload_TransportReset();
            }
        }

        /* No image has been received in 300 seconds, try to load existing image, or sleep */
        if( (count >= counterTimeoutSeconds(300u, paramsTimeout) ) && (state == CY_BOOTLOAD_STATE_NONE) )
        {
            /* Stop bootloading communication */
            Cy_Bootload_TransportStop();

            /* Validate and switch to preferred App, if not validate and load to Next preferred App */
            LoadValidApp(PreferredAppID, NextAppID, bootParams);

            /* 300 seconds has passed and App is invalid. Handle that */
            Cy_SysLib_Halt(0x00u);
        }

        /* Blink once per two seconds */
        if ((count % counterTimeoutSeconds(1u, paramsTimeout)) == 0u) 
        {
            Cy_GPIO_Inv(Blue_LED); /* toggle the LED */
        }

        /* If Button clicked - Switch to App2 if it is valid */
        if (Cy_GPIO_Read(PIN_SW2) == 0u)
        {
            /* 50 ms delay for button debounce on button press */
            Cy_SysLib_Delay(50u);
            if (Cy_GPIO_Read(PIN_SW2) == 0u)
            {
                while (Cy_GPIO_Read(PIN_SW2) == 0u)
                {   /* 50 ms delay for button debounce on button release */
                    Cy_SysLib_Delay(50u);
                }

                /* Validate and switch to preferred app, if not validate and switch to next app */
                LoadValidApp(PreferredAppID, NextAppID, bootParams);
            }
        }
    }
}


/* [] END OF FILE */
