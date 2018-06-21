/*******************************************************************************
* File Name: main_cm4.c
*
* Version: 1.0
*
* Description: Demonstrates writing into the flash using Peripheral Driver 
*              Library (PDL) non-blocking APIs. This is done using interrupt method.
*
* Related Document: CE221122_PSoC6MCU_NonBlockingFlashWrite.pdf
*
* Hardware Dependency: CY8CKIT-062-BLE PSoC 6 BLE Pioneer Kit
*
********************************************************************************
* Copyright (2018), Cypress Semiconductor Corporation.
********************************************************************************
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
*******************************************************************************/

#include "project.h"
#include <string.h>

/* Set this macro to '1' to observe the failure case of flash write. A RAM
 * address is passed as row address argument to the flash write function instead
 * of a flash address causing the function to return error.
 */
#define MAKE_FLASH_WRITE_FAIL 0u

/* Check TRM details to get this number. */
#define LAST_FLASH_ROW        2047u

/* This array reserves space in the flash for one row of size
 * CY_FLASH_SIZEOF_ROW. Explicit initialization is required so that memory is
* allocated in flash instead of RAM. */
#define CALCULATE_FLASH_ADDRESS(rowNum) (CY_FLASH_BASE + ((rowNum) * CY_FLASH_SIZEOF_ROW))

/* CY_ALIGN ensures that the array address is an integer multiple of the row 
   size so that the array occupies one complete row. */	
CY_ALIGN(CY_FLASH_SIZEOF_ROW)

#if(MAKE_FLASH_WRITE_FAIL == 0)
/* Make the address point to last user flash row */
    const uint8_t *flashData = (uint8_t *)CALCULATE_FLASH_ADDRESS(LAST_FLASH_ROW);
#else
    /* Make the address point to some RAM location */
    const uint8_t *flashData = (uint8_t *)CY_SRAM0_BASE;
#endif  /* #if(MAKE_FLASH_WRITE_FAIL == 0) */
	
/* The "Flash interrupt is set" flag */
bool flash_intr_set = false;


/*******************************************************************************
* Function Name: Flash_ISR
********************************************************************************
*
* Flash interrupt service routine. This function does the following: 
* 1. Sets the "Flash interrupt is set" flag
* 2. Clears the Flash interrupt
* This function is called twice. First time it is called when erase operation of
* flash is completed. Second time it is called when data is written to flash
* successfully.
*******************************************************************************/

void Flash_ISR (void)
{    /* Check if the generated interrupt occurred from Flash */
    
    if(0u != (FLASHC_FM_CTL->INTR & FLASHC_FM_CTL_INTR_TIMER_EXPIRED_Msk))
    {
        /* Set the "Flash interrupt is set" flag */
       
        flash_intr_set = true;
        
        /* Clear the Flash interrupt */
        FLASHC_FM_CTL->INTR = _VAL2FLD(FLASHC_FM_CTL_INTR_TIMER_EXPIRED, 1u);
        (void) FLASHC_FM_CTL->INTR;
    }
}

/*******************************************************************************
* Function Name: main
********************************************************************************
*
* This is the main entry for CM4. This function does the following. 
* 1. Initializes the data into RAM that will be later written into flash
* 2. Writes the data into flash
* 3. Verifies the data written into flash by comparing it with the RAM data
* 4. Turns the Green LED ON if the flash write is successful otherwise turns the
*    Red LED ON.
*
*******************************************************************************/

int main(void)
{
    uint32_t index; 
    cy_en_flashdrv_status_t flashWriteStatus;
    uint8_t ramData[CY_FLASH_SIZEOF_ROW];
    bool errorFlag = false;

    /* Initialize the data in RAM that will be written into flash */
    for(index = 0; index < CY_FLASH_SIZEOF_ROW; index++)
    {
        ramData[index] = (uint8_t)index;
    }
    
    /* Initialize and enable the Flash interrupt */
    Cy_SysInt_SetVector(cpuss_interrupt_fm_IRQn, Flash_ISR);
    NVIC_EnableIRQ(cpuss_interrupt_fm_IRQn);
    
    __enable_irq(); /* Enable global interrupts. */
    
    /* Non-blocking flash write */
    flashWriteStatus = Cy_Flash_StartWrite((uint32_t)flashData, (const uint32_t *)ramData);
    /* If success wait for flash erase interrupt. After flash erase is complete
        wait for flash write interrupt to occur. */
    if(flashWriteStatus == CY_FLASH_DRV_OPERATION_STARTED)
    {
        /* Set 100ms as timeout value */
        uint32_t timeout = 100u;
        
        /* Wait for flash erase interrupt */
        while((flash_intr_set == false) && (timeout > 0u))
        {
            timeout--;
            Cy_SysLib_Delay(1); /* delay one millisecond each iteration */
        }
        
        /* Interrupt has occurred. Clear the flag. */
        flash_intr_set = false;
        
        if(timeout == 0u)
        {
            errorFlag = true;
        }
        else
        {
            uint32_t timeout = 100u;
                      
            /* Wait for successful flash write */
            while((flash_intr_set == false) && (timeout > 0u))
            {
                timeout--;
                Cy_SysLib_Delay(1); /* delay one millisecond each iteration */
            }
            
             /* Interrupt has occurred. Clear the flag. */
            flash_intr_set = false;
            
            /* Check flash write is completed. */
            timeout = 100u;
                     
            while((Cy_Flash_IsOperationComplete() != CY_FLASH_DRV_SUCCESS)  && (timeout > 0u) )
            {
                timeout--;
                Cy_SysLib_Delay(1); /* delay one millisecond each iteration */
            }
                      
            
            if(timeout == 0u)
            {
                errorFlag = true;
            }
            else
            {
                /* Verify the data written into flash by comparing it with the RAM data */
                if( memcmp(ramData,flashData,CY_FLASH_SIZEOF_ROW) != 0u)
                {
                    errorFlag = true;    
                }    
            }
        }
    }
    else /* flash write operation did not start */
    {
        errorFlag = true;
    }
    
    if(errorFlag)
    {
        /* Turn the Error/Red LED ON */
        Cy_GPIO_Clr(LED_ERROR_PORT, LED_ERROR_NUM);
    }
    else
    {
        /* Turn the Ok/Green LED ON */
        Cy_GPIO_Clr(LED_OK_PORT, LED_OK_NUM);
    }
    
    for(;;)
    {
        /* Put the CPU into Deep Sleep mode to save power */
        Cy_SysPm_DeepSleep(CY_SYSPM_WAIT_FOR_INTERRUPT);
    }
}

/* [] END OF FILE */
