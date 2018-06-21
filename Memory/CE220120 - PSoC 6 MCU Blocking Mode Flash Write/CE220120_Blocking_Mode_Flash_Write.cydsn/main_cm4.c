/*******************************************************************************
* File Name: main_cm4.c
*
* Version: 1.10
*
* Description: Demonstrates writing into the flash using a Peripheral Driver 
*              Library (PDL) API. The flash write blocks the caller until
*              the flash write is complete.
*
* Related Document: CE220120.pdf
*
* Hardware Dependency: CY8CKIT-062-BLE PSoC 6 BLE Pioneer Kit
*
********************************************************************************
* Copyright (2017), Cypress Semiconductor Corporation.
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

/* Set this macro to '1' to observe the failure case of flash write. A RAM
 * address is passed as row address argument to the flash write function instead
 * of a flash address causing the function to return error.
 */
#define MAKE_FLASH_WRITE_FAIL       0u

/* This array reserves space in the flash for one row of size 
 * CY_FLASH_SIZEOF_ROW. Explicit initialization is required so that memory is
 * allocated in flash instead of RAM. CY_ALIGN ensures that the array address
 * is an integer multiple of the row size so that the array occupies one
 * complete row. 
 */
CY_ALIGN(CY_FLASH_SIZEOF_ROW) 
#if(MAKE_FLASH_WRITE_FAIL == 0)
    const uint8_t flashData[CY_FLASH_SIZEOF_ROW] = {0}; /* The array will be placed in Flash */
#else
    const uint8_t *flashData = (uint8_t *)CY_SRAM0_BASE; /* Make the address point to some RAM location */
#endif  /* #if(MAKE_FLASH_WRITE_FAIL == 0) */
    

/*******************************************************************************
* Function Name: main
********************************************************************************
*
* Summary: This is the main entry for Cortex-M4. This function does the 
* following. 
* 1. Initializes the data into RAM that will be later written into flash
* 2. Writes the data into flash
* 3. Verifies the data written into flash by comparing it with the RAM data
* 4. Turns the Green LED on if the flash write is successful otherwise turns the
*    Red LED on. 
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
    uint32_t i; 
    cy_en_flashdrv_status_t flashWriteStatus;
    uint8_t ramData[CY_FLASH_SIZEOF_ROW];
    bool compareStatus = true;
    
    __enable_irq(); /* Enable global interrupts. */

    /* Initialize the data in RAM that will be written into flash */
    for(i = 0; i < CY_FLASH_SIZEOF_ROW; i++)
    {
        ramData[i] = (uint8_t)i; 
    }
    
    /* Cy_Flash_WriteRow() is a blocking API that does not return until the
     * flash write is complete. The format of the rowAddr argument should be a 
     * 32-bit system address and any address within the row is a valid value.  
     */
    flashWriteStatus = Cy_Flash_WriteRow((uint32_t)flashData, (const uint32_t *)ramData);
    
    if(flashWriteStatus == CY_FLASH_DRV_SUCCESS)
    {
        /* Verify the data written into flash by comparing it with the RAM data */
        for(i = 0; i < CY_FLASH_SIZEOF_ROW; i++)
        {
            if(ramData[i] != flashData[i])
            {
                compareStatus = false;
                break;
            }
        }
    }
    else
    {
        compareStatus = false;
    }
    
    if(compareStatus)
    {
        /* Turn the Ok/Green LED on */
        Cy_GPIO_Clr(LED_OK_PORT, LED_OK_NUM);
    }
    else
    {
        /* Turn the Error/Red LED on */
        Cy_GPIO_Clr(LED_ERROR_PORT, LED_ERROR_NUM);
    }
    
    for(;;)
    {
        /* Put CPU into deep sleep mode to save power. This puts only CM4 into
         * deep sleep. Device enters deep sleep only when both the cores enter
         * deep sleep.
         */
        Cy_SysPm_DeepSleep(CY_SYSPM_WAIT_FOR_INTERRUPT);
    }
}

/* [] END OF FILE */
