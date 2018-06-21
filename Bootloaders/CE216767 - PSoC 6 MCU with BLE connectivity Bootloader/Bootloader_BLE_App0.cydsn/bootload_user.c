/***************************************************************************//**
* \file bootload_user.c
* \version 2.0
* 
* This file provides the custom API for a firmware application with 
* Bootloader SDK.
* - Cy_Bootload_ReadData (address, length, ctl, params) - to read  the NVM block 
* - Cy_Bootalod_WriteData(address, length, ctl, params) - to write the NVM block
*
* - Cy_Bootload_TransportStart() to start a communication interface
* - Cy_Bootload_TransportStop () to stop  a communication interface
* - Cy_Bootload_TransportReset() to reset a communication interface
* - Cy_Bootload_TransportRead (buffer, size, count, timeout)
* - Cy_Bootload_TransportWrite(buffer, size, count, timeout)
*
********************************************************************************
* \copyright
* Copyright 2016-2017, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include <string.h>
#include "transport_ble.h"
#include "syslib/cy_syslib.h"
#include "flash/cy_flash.h"
#include "bootloader/cy_bootload.h"


/*
* The Bootloader SDK metadata initial value is placed here
* Note: the number of elements equal to the number of the app multiplies by 2 
*       because of the two fields per app plus one element for the CRC-32C field.
*/
CY_SECTION(".cy_boot_metadata") __USED
static const uint32_t cy_bootload_metadata[CY_FLASH_SIZEOF_ROW / sizeof(uint32_t)] =
{
    CY_BOOTLOAD_APP0_VERIFY_START, CY_BOOTLOAD_APP0_VERIFY_LENGTH, /* The App0 base address and length */
    CY_BOOTLOAD_APP1_VERIFY_START, CY_BOOTLOAD_APP1_VERIFY_LENGTH, /* The App1 base address and length */
    0u                                                             /* The rest does not matter     */
};


static uint32_t IsMultipleOf(uint32_t value, uint32_t multiple);
static uint32_t IsMultipleOf(uint32_t value, uint32_t multiple)
{
    return ( ((value % multiple) == 0u)? 1ul : 0ul);
}


/*******************************************************************************
* Function Name: Cy_Bootload_WriteData
****************************************************************************//**
*
* This function documentation is part of the Bootloader SDK API, see the 
* cy_bootload.h file or Bootloader SDK API Reference Manual for details.
*
*******************************************************************************/
cy_en_bootload_status_t Cy_Bootload_WriteData (uint32_t address, uint32_t length, uint32_t ctl, 
                                               cy_stc_bootload_params_t *params)
{
    /* User Flash Limits */
    /* Note that App0 is out of range */
    const uint32_t minUFlashAddress = CY_FLASH_BASE + CY_BOOTLOAD_APP0_VERIFY_LENGTH;
    const uint32_t maxUFlashAddress = CY_FLASH_BASE + CY_FLASH_SIZE;
    /* EM_EEPROM Limits*/
    const uint32_t minEmEepromAddress = CY_EM_EEPROM_BASE;
    const uint32_t maxEmEepromAddress = CY_EM_EEPROM_BASE + CY_EM_EEPROM_SIZE;
    
    cy_en_bootload_status_t status = CY_BOOTLOAD_SUCCESS;
    
    /* Check if the length is valid */
    if ( (IsMultipleOf(length, CY_FLASH_SIZEOF_ROW) == 0u) && ((ctl & CY_BOOTLOAD_IOCTL_ERASE) == 0u) )
    {
        status = CY_BOOTLOAD_ERROR_LENGTH;   
    }
    
    /* Check if the address is inside the valid range */
    if ( ( (minUFlashAddress <= address) && (address < maxUFlashAddress) ) 
      || ( (minEmEepromAddress <= address) && (address < maxEmEepromAddress) )  )
    {   /* Do nothing, this is an allowed memory range to bootload to */
    }
    else
    {
        status = CY_BOOTLOAD_ERROR_ADDRESS;   
    }
    
    if (status == CY_BOOTLOAD_SUCCESS)
    {
        if ((ctl & CY_BOOTLOAD_IOCTL_ERASE) != 0u)
        {
            (void) memset(params->dataBuffer, 0, CY_FLASH_SIZEOF_ROW);
        }
        cy_en_flashdrv_status_t fstatus =  Cy_Flash_WriteRow(address, (uint32_t*)params->dataBuffer);
        status = (fstatus == CY_FLASH_DRV_SUCCESS) ? CY_BOOTLOAD_SUCCESS : CY_BOOTLOAD_ERROR_DATA;
    }
    return (status);
}


/*******************************************************************************
* Function Name: Cy_Bootload_ReadData
****************************************************************************//**
*
* This function documentation is part of the Bootloader SDK API, see the 
* cy_bootload.h file or Bootloader SDK API Reference Manual for details.
*
*******************************************************************************/
cy_en_bootload_status_t Cy_Bootload_ReadData (uint32_t address, uint32_t length, uint32_t ctl, 
                                              cy_stc_bootload_params_t *params)
{
    /* User Flash Limits */
    /* Note that App0 is out of range */
    const uint32_t minUFlashAddress = CY_FLASH_BASE + CY_BOOTLOAD_APP0_VERIFY_LENGTH;
    const uint32_t maxUFlashAddress = CY_FLASH_BASE + CY_FLASH_SIZE;
    /* EM_EEPROM Limits*/
    const uint32_t minEmEepromAddress = CY_EM_EEPROM_BASE;
    const uint32_t maxEmEepromAddress = CY_EM_EEPROM_BASE + CY_EM_EEPROM_SIZE;
    
    cy_en_bootload_status_t status = CY_BOOTLOAD_SUCCESS;

    /* Check if the length is valid */
    if (IsMultipleOf(length, CY_FLASH_SIZEOF_ROW) == 0u) 
    {
        status = CY_BOOTLOAD_ERROR_LENGTH;   
    }

    /* Check if the address is inside the valid range */
    if ( ( (minUFlashAddress <= address) && (address < maxUFlashAddress) ) 
      || ( (minEmEepromAddress <= address) && (address < maxEmEepromAddress) )  )
    {   /* Do nothing, this is an allowed memory range to bootload to */
    }
    else
    {
        status = CY_BOOTLOAD_ERROR_ADDRESS;   
    }

    /* Read or Compare */
    if (status == CY_BOOTLOAD_SUCCESS)
    {
        if ((ctl & CY_BOOTLOAD_IOCTL_COMPARE) == 0u)
        {
            (void) memcpy(params->dataBuffer, (const void *)address, length);
            status = CY_BOOTLOAD_SUCCESS;
        }
        else
        {
            status = ( memcmp(params->dataBuffer, (const void *)address, length) == 0 )
                     ? CY_BOOTLOAD_SUCCESS : CY_BOOTLOAD_ERROR_VERIFY;
        }
    }
    return (status);
}


/*******************************************************************************
* Function Name: Cy_Bootload_TransportRead
****************************************************************************//**
*
* This function documentation is part of the Bootloader SDK API, see the 
* cy_bootload.h file or Bootloader SDK API Reference Manual for details.
*
*******************************************************************************/
cy_en_bootload_status_t Cy_Bootload_TransportRead (uint8_t *buffer, uint32_t size, uint32_t *count, uint32_t timeout)
{
    return (CyBLE_CyBtldrCommRead(buffer, size, count, timeout));
}

/*******************************************************************************
* Function Name: Cy_Bootload_TransportWrite
****************************************************************************//**
*
* This function documentation is part of the Bootloader SDK API, see the 
* cy_bootload.h file or Bootloader SDK API Reference Manual for details.
*
*******************************************************************************/
cy_en_bootload_status_t Cy_Bootload_TransportWrite(uint8_t *buffer, uint32_t size, uint32_t *count, uint32_t timeout)
{
    return (CyBLE_CyBtldrCommWrite(buffer, size, count, timeout));
}

/*******************************************************************************
* Function Name: Cy_Bootload_TransportReset
****************************************************************************//**
*
* This function documentation is part of the Bootloader SDK API, see the 
* cy_bootload.h file or Bootloader SDK API Reference Manual for details.
*
*******************************************************************************/
void Cy_Bootload_TransportReset(void)
{
    CyBLE_CyBtldrCommReset();
}

/*******************************************************************************
* Function Name: Cy_Bootload_TransportStart
****************************************************************************//**
*
* This function documentation is part of the Bootloader SDK API, see the 
* cy_bootload.h file or Bootloader SDK API Reference Manual for details.
*
*******************************************************************************/
void Cy_Bootload_TransportStart(void)
{
    CyBLE_CyBtldrCommStart();
}

/*******************************************************************************
* Function Name: Cy_Bootload_TransportStop
****************************************************************************//**
*
* This function documentation is part of the Bootloader SDK API, see the 
* cy_bootload.h file or Bootloader SDK API Reference Manual for details.
*
*******************************************************************************/
void Cy_Bootload_TransportStop(void)
{
    CyBLE_CyBtldrCommStop();
}


/* [] END OF FILE */
