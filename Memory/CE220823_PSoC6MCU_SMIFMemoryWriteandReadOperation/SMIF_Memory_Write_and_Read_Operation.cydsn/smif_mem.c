/******************************************************************************
* File Name: smif_mem.c
*
* Version: 1.20
*
* Description: Functions in this file implement routines to access SMIF memory
*
* Related Document: CE220823_PSoC6MCU_SMIFMemoryWriteandReadOperation.pdf
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

#include "smif_mem.h"
#include "stdio.h"
#include "project.h"

/*******************************************************************************
* Function Name: handle_error
********************************************************************************
*
* This function processes unrecoverable errors such as UART component 
* initialization error or SMIF initialization error etc. In case of such error 
* the system will Turn on RED_LED_ERROR and stay in a infinite loop of 
* this function.
*
* \param
*  None
*
* \return
*  None
*
*******************************************************************************/
void handle_error(void)
{
     /* Disable all interrupts */
    __disable_irq();
	
    /* Turn on error LED */
    Cy_GPIO_Write(ERROR_LED_0_PORT, ERROR_LED_0_NUM, LED_ON);
    while(1u) {}
}

/*******************************************************************************
* Function Name: WriteMemory
********************************************************************************
*
* This function writes data to the external memory in the quad mode. 
* The function sends the Quad Page Program: 0x38 command to the external memory. 
*
* \param baseaddr
* Holds the base address of the SMIF block registers.
*
* \param smifContext
* The internal SMIF context data.
*
* \param txBuffer 
* Data to write in the external memory.
* 
* \param txSize 
* The size of data.
* 
* \param address 
* The address to write data to.   
*
*******************************************************************************/
void WriteMemory(SMIF_Type *baseaddr,
                    cy_stc_smif_context_t *smifContext, 
                    uint8_t txBuffer[], 
                    uint32_t txSize, 
                    uint8_t *address)
{
    cy_en_smif_status_t smif_status;
    uint8_t rxBuffer_reg;
    cy_stc_smif_mem_device_cfg_t *device = smifMemConfigs[0]->deviceCfg;
    cy_stc_smif_mem_cmd_t *cmdreadStsRegQe = device->readStsRegQeCmd;
	cy_stc_smif_mem_cmd_t *cmdreadStsRegWip = device->readStsRegWipCmd;	

    /* Set QE */    
    smif_status = Cy_SMIF_Memslot_QuadEnable(SMIF_1_HW, (cy_stc_smif_mem_config_t*)smifMemConfigs[0], &SMIF_1_context);
    if(smif_status!=CY_SMIF_SUCCESS)
    {
        Cy_SCB_UART_PutString(UART_HW, "\r\n\r\nSMIF Cy_SMIF_Memslot_QuadEnable failed\r\n");
        handle_error();
    }
	
	/* Read data from the external memory configuration register */
	smif_status = Cy_SMIF_Memslot_CmdReadSts(baseaddr, smifMemConfigs[0], &rxBuffer_reg, (uint8_t)cmdreadStsRegQe->command , smifContext);
    if(smif_status!=CY_SMIF_SUCCESS)
    {
        Cy_SCB_UART_PutString(UART_HW, "\r\n\r\nSMIF Cy_SMIF_Memslot_CmdReadSts failed\r\n");
        handle_error();
    }
    printf("Received Data: 0x%X\r\n", (unsigned int) rxBuffer_reg);
	
    /* Send Write Enable to external memory */	
    smif_status = Cy_SMIF_Memslot_CmdWriteEnable(SMIF_1_HW, smifMemConfigs[0], &SMIF_1_context);
    if(smif_status!=CY_SMIF_SUCCESS)
    {
        Cy_SCB_UART_PutString(UART_HW, "\r\n\r\nSMIF Cy_SMIF_Memslot_CmdWriteEnable failed\r\n");
        handle_error();
    }
    
    printf("\r\nQuad Page Program (QPP 0x%0X) \r\n", 0x38);
	
	/* Quad Page Program command */       
    smif_status = Cy_SMIF_Memslot_CmdProgram(SMIF_1_HW, smifMemConfigs[0], address, txBuffer, txSize, &RxCmpltCallback, &SMIF_1_context);
    if(smif_status!=CY_SMIF_SUCCESS)
    {
        Cy_SCB_UART_PutString(UART_HW, "\r\n\r\nSMIF Cy_SMIF_Memslot_CmdProgram failed\r\n");
        handle_error();
    }	
    PrintWriteData(txBuffer, txSize);    
    
    while(Cy_SMIF_Memslot_IsBusy(SMIF_1_HW, (cy_stc_smif_mem_config_t*)smifMemConfigs[0], &SMIF_1_context))
    {
        /* Wait until the Erase operation is completed */
    }
	
	/* Read data from the external memory status register */
	smif_status = Cy_SMIF_Memslot_CmdReadSts(baseaddr, smifMemConfigs[0], &rxBuffer_reg, \
	                         (uint8_t)cmdreadStsRegWip->command , smifContext);    
    if(smif_status!=CY_SMIF_SUCCESS)
    {
        Cy_SCB_UART_PutString(UART_HW, "\r\n\r\nSMIF ReadStatusReg failed\r\n");
        handle_error();
    }
    printf("Received Data: 0x%X\r\n", (unsigned int) rxBuffer_reg);
}

/*******************************************************************************
* Function Name: ReadMemory
****************************************************************************//**
*
* This function reads data from the external memory in the quad mode. 
* The function sends the Quad I/O Read: 0xEB command to the external memory. 
*
* \param baseaddr
* Holds the base address of the SMIF block registers.
*
* \param smifContext
* The internal SMIF context data.
*
* \param rxBuffer 
* The buffer for read data.
* 
* \param rxSize 
* The size of data to read.
* 
* \param address 
* The address to read data from. 
*
*******************************************************************************/
void ReadMemory(SMIF_Type *baseaddr,
                            cy_stc_smif_context_t *smifContext, 
                            uint8_t rxBuffer[], 
                            uint32_t rxSize, 
                            uint8_t *address)
{   
    cy_en_smif_status_t smif_status;
    uint8_t rxBuffer_reg;
    cy_stc_smif_mem_device_cfg_t *device = smifMemConfigs[0]->deviceCfg;
    cy_stc_smif_mem_cmd_t *cmdreadStsRegQe = device->readStsRegQeCmd;

    /* Set QE */		    
    smif_status = Cy_SMIF_Memslot_QuadEnable(SMIF_1_HW, (cy_stc_smif_mem_config_t*)smifMemConfigs[0], &SMIF_1_context);
    if(smif_status!=CY_SMIF_SUCCESS)
    {
        Cy_SCB_UART_PutString(UART_HW, "\r\n\r\nSMIF Cy_SMIF_Memslot_QuadEnable failed\r\n");
        handle_error();
    }

	/* Read data from the external memory configuration register */
	smif_status = Cy_SMIF_Memslot_CmdReadSts(baseaddr, smifMemConfigs[0], &rxBuffer_reg, (uint8_t)cmdreadStsRegQe->command , smifContext);
    if(smif_status!=CY_SMIF_SUCCESS)
    {
        Cy_SCB_UART_PutString(UART_HW, "\r\n\r\nSMIF Cy_SMIF_Memslot_CmdReadSts failed\r\n");
        handle_error();
    }
    printf("Received Data: 0x%X\r\n", (unsigned int) rxBuffer_reg);	
    
    printf("\r\nQuad I/O Read (QIOR 0x%0X) \r\n", 0x38);
	
	/* The 4 Page program command */    
    smif_status = Cy_SMIF_Memslot_CmdRead(baseaddr, smifMemConfigs[0], address, rxBuffer, rxSize, &RxCmpltCallback, &SMIF_1_context);
    if(smif_status!=CY_SMIF_SUCCESS)
    {
        Cy_SCB_UART_PutString(UART_HW, "\r\n\r\nSMIF Cy_SMIF_Memslot_CmdRead failed\r\n");
        handle_error();
    }
    
    while(Cy_SMIF_BusyCheck(baseaddr))
    {
        /* Wait until the SMIF IP operation is completed. */
    }
    
    /* Send received data to the console */
    PrintReadData(rxBuffer, rxSize);
}

/*******************************************************************************
* Function Name: PrintReadData
****************************************************************************//**
*
* This function prints the content of the RX buffer to the UART console.
* 
* \param  rxBuffer - The buffer to the console output.
* 
* \param  size - The size of the buffer to the console output.
*
*******************************************************************************/
void PrintReadData(uint8_t * rxBuffer, uint32_t size)
{
    uint32_t index;
    
    printf("Received Data: ");

    for(index=0; index<size; index++)
    {
        printf("0x%02X ", (unsigned int) rxBuffer[index]);
    }
    
    printf("\r\n");  
    printf("=======================\r\n");
}

/*******************************************************************************
* Function Name: PrintWriteData
****************************************************************************//**
*
* This function prints the content of the TX buffer to the UART console.
* 
* \param  txBuffer - the buffer to output to the console
* 
* \param  size -  the size of the buffer to output to the console
*
*******************************************************************************/
void PrintWriteData(uint8_t * txBuffer, uint32_t size)
{
    uint32_t index;
    
    printf("Written Data: ");

    for(index=0; index<size; index++)
    {
        printf("0x%02X ", (unsigned int) txBuffer[index]);
    }
    printf("\r\n");  
}

/*******************************************************************************
* Function Name: InitBuffers
****************************************************************************//**
*
* This function initializes the transfer buffers.
*
* \param txBuffer - The buffer for Write data.
*
* \param rxBuffer - The buffer for Read data.
*
*******************************************************************************/
void InitBuffers(uint8_t txBuffer[],uint8_t rxBuffer[], uint32_t bufferSize)
{    
    uint32_t index;
    for(index=0; index<bufferSize; index++)
    {
        txBuffer[index] = (uint8_t) (index & 0xFF);
        rxBuffer[index] = 0;
    }					
}

/*******************************************************************************
* Function Name: TxRxEqualCheck
****************************************************************************//**
*
* This function checks if the transmitted and received arrays are equal
*
* \param txBuffer - The buffer for Write data.
*
* \param rxBuffer - The buffer for Read data.
*
*******************************************************************************/
bool TxRxEqualCheck(uint8_t txBuffer[],uint8_t rxBuffer[])
{
    uint8_t isEqual = TX_RX_EQUAL;
    uint32_t index;
    for(index=0; index<PACKET_SIZE; index++)
    {
        if (txBuffer[index] != rxBuffer[index])
        {
            isEqual = TX_RX_NOT_EQUAL;
            break;
        }
    }

    return(isEqual);
}

/*******************************************************************************
* Function Name: RxCmpltCallback
****************************************************************************//**
*
* The callback called after the transfer completion.
*
* \param event
* The event of the callback.
*  
*******************************************************************************/
void RxCmpltCallback (uint32_t event)
{
    if(0u == event)
    {
        /*The process event is 0*/
    }
}

/* [] END OF FILE */



