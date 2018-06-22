/******************************************************************************
* File Name: main_cm4.c
*
* Version: 1.20
*
* Description: This example writes and reads 256 bytes of data to external 
* memory using SMIF quad mode. The example checks the integrity of 
* read data against written data.
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
*******************************************************************************/

#include "smif_mem.h"

/*******************************************************************************
*            Function Prototypes
*******************************************************************************/

/* SMIF interrupt function */
void SMIF_Interrupt_User(void);

/*******************************************************************************
*            Constants
*******************************************************************************/

/*******************************************************************************
*            Global variables
*******************************************************************************/

/*******************************************************************************
* Function Name: main
********************************************************************************
*
* Summary:
*  The main function performs the following actions:
*  1. Turns OFF the ERROR_LED and SUCCESS_LED.
*  2. Initializes the UART operation. If the UART initialization fails, then 
*     turns on the ERROR_LED and stays in an infinite loop.
*  3. Enables the global interrupt.
*  4. Initializes the SMIF interrupt. If the SMIF interrupt initialization 
*     fails, then turns on the ERROR_LED and stays in an infinite loop.
*  5. Initializes the SMIF. If the SMIF initialization fails, then turns on the 
*     ERROR_LED and stays in an infinite loop.
*  6. Erases 256-bytes of memory
*  7. Writes 256-bytes of data to memory in quad mode
*  8. Reads 256-bytes of data from memory in quad mode and compares with the 
*     written data.
*  9. Indicates Pass or Fail LED status
*
* Parameters:
*  None
*
* Return:
*  int
*
*******************************************************************************/
int main(void)
{
    /* UART initialization status */
    cy_en_scb_uart_status_t uart_init_status;

    /* SMIF interrupt initialization status */
    cy_en_sysint_status_t intr_init_status;

    /* SMIF status */
    cy_en_smif_status_t smif_status;

    /* Variables */
    uint8_t txBuffer[PACKET_SIZE] = {0};
    uint8_t rxBuffer[PACKET_SIZE] = {0};  
    uint8_t extMemAddress[ADDRESS_SIZE] = {0x00, 0x00, 0x00};

    /* Turn OFF Error and status LEDs */
    Cy_GPIO_Write(ERROR_LED_0_PORT, ERROR_LED_0_NUM, LED_OFF);
    Cy_GPIO_Write(SUCCESS_LED_0_PORT, SUCCESS_LED_0_NUM, LED_OFF);
    
    /* Initialize UART operation. */
    uart_init_status = Cy_SCB_UART_Init(UART_HW, &UART_config, &UART_context);
    if(uart_init_status!=CY_SCB_UART_SUCCESS)
    {
        handle_error();
    }
	
    Cy_SCB_UART_Enable(UART_HW);
	  Cy_SCB_UART_PutString(UART_HW, "\r\n\r\nUART initialization complete\r\n");  
	
	/* Enable global interrupts */
    __enable_irq();

    Cy_SCB_UART_PutString(UART_HW, "\r\n\r\nSMIF code example started\r\n");
    
    /* Configure SMIF interrupt */
    cy_stc_sysint_t smifIntConfig =
    {
        .intrSrc = smif_interrupt_IRQn,     /* SMIF interrupt */
        .intrPriority = SMIF_PRIORITY       /* SMIF interrupt priority */
    };    
    intr_init_status = Cy_SysInt_Init(&smifIntConfig, SMIF_Interrupt_User);
    if(intr_init_status!=CY_SYSINT_SUCCESS)
    {
        Cy_SCB_UART_PutString(UART_HW, "\r\n\r\nSMIF interrupt initialization failed\r\n");
        handle_error();
    }    
    Cy_SCB_UART_PutString(UART_HW, "SMIF interrupt initialization complete\r\n");
	
    /* Initialize SMIF */
    smif_status = Cy_SMIF_Init(SMIF_1_HW, &SMIF_1_config, TIMEOUT_1_MS, &SMIF_1_context);
    if(smif_status!=CY_SMIF_SUCCESS)
    {
        Cy_SCB_UART_PutString(UART_HW, "\r\n\r\nSMIF initialization failed\r\n");
        handle_error();
    }
    
    /* Configures data select */ //Comment
    Cy_SMIF_SetDataSelect(SMIF_1_HW, CY_SMIF_SLAVE_SELECT_0, CY_SMIF_DATA_SEL0);
    Cy_SMIF_Enable(SMIF_1_HW, &SMIF_1_context);
    Cy_SCB_UART_PutString(UART_HW, "SMIF initialization complete\r\n");
    
	/* Initialize the transfer buffers */
    InitBuffers(txBuffer, rxBuffer, PACKET_SIZE);    

    /* Enable the SMIF interrupt */
    NVIC_EnableIRQ(smif_interrupt_IRQn);
    Cy_SCB_UART_PutString(UART_HW, "=========================================================\r\n");
    Cy_SCB_UART_PutString(UART_HW, "\r\nSMIF operation in quad mode\r\n");    
        
	/* Erase before next Write */   
    smif_status = Cy_SMIF_Memslot_CmdWriteEnable(SMIF_1_HW, smifMemConfigs[0], &SMIF_1_context);
    if(smif_status!=CY_SMIF_SUCCESS)
    {
        Cy_SCB_UART_PutString(UART_HW, "\r\n\r\nSMIF Cy_SMIF_Memslot_CmdWriteEnable failed\r\n");
        handle_error();
    }
    smif_status = Cy_SMIF_Memslot_CmdSectorErase(SMIF_1_HW, (cy_stc_smif_mem_config_t*)smifMemConfigs[0], extMemAddress,&SMIF_1_context);
    if(smif_status!=CY_SMIF_SUCCESS)
    {
        Cy_SCB_UART_PutString(UART_HW, "\r\n\r\nSMIF Cy_SMIF_Memslot_CmdSectorErase failed\r\n");
        handle_error();
    }
    
    /* Wait until the memory is erased */
    while(Cy_SMIF_Memslot_IsBusy(SMIF_1_HW, (cy_stc_smif_mem_config_t*)smifMemConfigs[0], &SMIF_1_context))
    {
        /* Wait until the Erase operation is completed */
    }

	/* Read the PACKET_SIZE bytes in the single mode from the address extMemAddress
     * and send the packet to the console.
     */
	/* Should be 0xFF, the memory was just erased */
    ReadMemory(SMIF_1_HW, &SMIF_1_context, rxBuffer, PACKET_SIZE, extMemAddress);

    /* Reinitialize the buffers */
    InitBuffers(txBuffer, rxBuffer, PACKET_SIZE);

	/* Write the PACKET_SIZE bytes in the single mode to the address extMemAddress
     * and send the packet to the console.
     */    
    WriteMemory(SMIF_1_HW, &SMIF_1_context, txBuffer, PACKET_SIZE, extMemAddress);

	/* Read the PACKET_SIZE bytes in the single mode from the address extMemAddress
     * and send the packet to the console.
     */
	/* The value should be txBuffer */
	ReadMemory(SMIF_1_HW, &SMIF_1_context, rxBuffer, PACKET_SIZE, extMemAddress);

    /* Check if the transmitted and received arrays are equal */
    if (TxRxEqualCheck(txBuffer, rxBuffer))
    {
		Cy_GPIO_Write(SUCCESS_LED_0_PORT, SUCCESS_LED_0_NUM, LED_ON);
		Cy_SCB_UART_PutString(UART_HW, "\r\nRead data matches with written data in quad mode\r\n");
		Cy_SCB_UART_PutString(UART_HW, "\r\nSMIF operation is successful in quad mode\r\n");
    }
	else
	{
		Cy_GPIO_Write(ERROR_LED_0_PORT, ERROR_LED_0_NUM, LED_ON);
		Cy_SCB_UART_PutString(UART_HW, "\r\nRead data does not match with written data in quad mode\r\n");
		Cy_SCB_UART_PutString(UART_HW, "\r\nSMIF operation is failed in quad mode\r\n");		
	}

    for(;;)
    {  
        /* CM4 does nothing after SMIF operation is complete. */
    }
}

/*******************************************************************************
* Function Name: _write
*******************************************************************************/
#if defined (__GNUC__)
    /* Add an explicit reference to the floating point printf library to allow
    the usage of the floating point conversion specifier. */
    /* For the GCC compiler, revise the  _write() function for the printf functionality */
    int _write(int file, char *ptr, int len)
    {
        int nChars = 0;

        /* Suppress the compiler warning about an unused variable. */
        if (0 != file)
        {
        }
        
        for (/* Empty */; len != 0; --len)
        {
            /* Block until there is space in the TX FIFO or buffer. */
            while (0UL == Cy_SCB_UART_Put(UART_HW, *ptr))
            {
            }
            
            ++nChars;
            ++ptr;
        }
        
        return (nChars);
    }
#elif defined(__ARMCC_VERSION)
    /* For the MDK/RVDS compiler, revise the fputc() function for the printf functionality */
    struct __FILE
    {
        int handle;
    };
    
    enum
    {
        STDIN_HANDLE,
        STDOUT_HANDLE,
        STDERR_HANDLE
    };
    
    FILE __stdin = {STDIN_HANDLE};
    FILE __stdout = {STDOUT_HANDLE};
    FILE __stderr = {STDERR_HANDLE};
    
    int fputc(int ch, FILE *file)
    {
        int ret = EOF;
        switch(file->handle)
        {
            case STDOUT_HANDLE:
                while (0UL == Cy_SCB_UART_Put(UART_HW, ch))
                {
                }
                ret = ch;
            break;
                
            case STDERR_HANDLE:
                ret = ch;
            break;
                
            default:
                file = file;
            break;
        }
        return(ret);
    }
    
#elif defined (__ICCARM__)
    /* For the IAR compiler, revise the  __write() function for the printf functionality */
    size_t __write(int handle, const unsigned char * buffer, size_t size)
    {
        size_t nChars = 0;
        
        for (/* Empty */; size != 0; --size)
        {
            Cy_SCB_UART_Put(UART_HW, *buffer++);
            ++nChars;
        }
    return (nChars);
}

#endif /* (__GNUC__) */

/*******************************************************************************
* Function Name: SMIF_Interrupt_User
****************************************************************************//**
*
* Summary:
* The ISR for the SMIF interrupt. All Read/Write transfers to/from the external 
* memory are processed inside the SMIF ISR.
*  
*******************************************************************************/
void SMIF_Interrupt_User(void)
{
    Cy_SMIF_Interrupt(SMIF_1_HW, &SMIF_1_context);
}

/* [] END OF FILE */



