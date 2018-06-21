/******************************************************************************
* File Name: main_cm4.c
* Version 1.0
*
* Description:
*   Code example implements a UART to memory buffer transfer using DMA.
*
* Hardware Dependency: CY8CKIT-062-BLE PSoC 6 BLE Pioneer Kit
*
******************************************************************************
* Copyright (C) 2017, Cypress Semiconductor Corporation.
******************************************************************************
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
*****************************************************************************/

#include "project.h"
#include <stdio.h>

/***************************************
*        Function Prototypes
***************************************/
    
void ConfigureTxDma(void);
void ConfigureRxDma(void);

void RxDmaCmplt(void);

/***************************************
*            Constants
****************************************/

#define BUFFER_SIZE         (10u)


uint8_t Buffer[BUFFER_SIZE];

int main(void)
{
    __enable_irq(); /* Enable global interrupts. */
    

    /* Configure UART. */
    /* Initialize UART with config set in component */
    Cy_SCB_UART_Init(UART_HW, &UART_config, &UART_context);
    /* Initialize and enable the UART interrupts */
    Cy_SysInt_Init(&UART_SCB_IRQ_cfg, &UART_Interrupt);
    NVIC_EnableIRQ(UART_SCB_IRQ_cfg.intrSrc);
    /* Enable the SCB block that implements the UART */
    Cy_SCB_UART_Enable(UART_HW);
    
    /* Print a message on UART */ 
    Cy_SCB_UART_PutString(UART_HW, "UART to Memory Buffer using DMA code example project\n");
    Cy_SCB_UART_PutString(UART_HW, "Transmit 10 characters to see an echo in the terminal.\n");
    
    /* Configures DMA Rx and Tx channels for operation. */
    ConfigureRxDma();
    ConfigureTxDma();

    /* Initialize and enable the RxDMA interrupt*/
    Cy_SysInt_Init  (&RxDMA_Cmplt_cfg, &RxDmaCmplt);
    NVIC_EnableIRQ(RxDMA_Cmplt_cfg.intrSrc);

    
    /* Place your initialization/startup code here (e.g. MyInst_Start()) */
    for(;;)
    {
        /* Place your application code here. */
    }
}

/*******************************************************************************
* Function Name: ConfigureRxDma
********************************************************************************
*
* Configures RxDMA channel for operation. This channel has a descriptor that is 
* configured to transfer 1 byte at a time when it becomes available at the UART
* RxFIFO. The Descriptor is configured to tranfer 10 bytes. When 10 byte transfer
* is completed, the RxDMA_Cmplt interrupt is triggered.
*
*******************************************************************************/
void ConfigureRxDma(void)
{
    cy_stc_dma_channel_config_t channelConfig;
    
    /* set the source and destination of transfer in the descriptor config structure */
    RxDMA_Descriptor_1_config.srcAddress = (uint32_t *) &UART_HW->RX_FIFO_RD;
    RxDMA_Descriptor_1_config.dstAddress = Buffer;
    
    /* Initialize the Descriptor for the RxDMA channel */
    Cy_DMA_Descriptor_Init(&RxDMA_Descriptor_1, &RxDMA_Descriptor_1_config);
    
    /* Set the channel config strucutrue with Descriptor pointer and other param */
    channelConfig.descriptor        = &RxDMA_Descriptor_1;
    channelConfig.preemptable       = RxDMA_PREEMPTABLE;
    channelConfig.priority          = RxDMA_PRIORITY;
    channelConfig.enable            = 0u;
    
    /* Initialize the DMA channel */
    Cy_DMA_Channel_Init(RxDMA_HW, RxDMA_DW_CHANNEL, &channelConfig);
    
    /* Enables the DMA channel interrupt mask */
    Cy_DMA_Channel_SetInterruptMask(RxDMA_HW, RxDMA_DW_CHANNEL, CY_DMA_INTR_MASK);
    
    /* Enables DMA channel */
    Cy_DMA_Channel_Enable(RxDMA_HW, RxDMA_DW_CHANNEL);
   
    /* Enable DMA hardware block */
    Cy_DMA_Enable(RxDMA_HW);
}


/*******************************************************************************
* Function Name: ConfigureTxDma
********************************************************************************
*
* Configures TxDMA which handles transmitting 10 bytes of data from the Buffer to 
* UART TxFIFO. At the end of the transfer the TxDMA is configured to be disabled. 
* 
*
*******************************************************************************/
void ConfigureTxDma(void)
{
    cy_stc_dma_channel_config_t channelConfig;
    
    /* set the source and destination of transfer in the descriptor config structure */    
    TxDMA_Descriptor_1_config.srcAddress = Buffer;
    TxDMA_Descriptor_1_config.dstAddress = (uint32_t *) &UART_HW->TX_FIFO_WR;
    
    /* Initialize the Descriptor for the DMA channel */
    Cy_DMA_Descriptor_Init(&TxDMA_Descriptor_1, &TxDMA_Descriptor_1_config);
    
    /* Set the channel config strucutrue with Descriptor pointer and other param */
    channelConfig.descriptor        = &TxDMA_Descriptor_1;
    channelConfig.preemptable       = TxDMA_PREEMPTABLE;
    channelConfig.priority          = TxDMA_PRIORITY;
    channelConfig.enable            = 0u;

    /* Initialize the DMA channel */
    Cy_DMA_Channel_Init(TxDMA_HW, TxDMA_DW_CHANNEL, &channelConfig);

}

/*******************************************************************************
* Function Name: RxDmaCmplt
********************************************************************************
*
* Interrupt that is triggered at the completion of the RxDMA transfer. 
* This interrupt reconfigures TxDMA for another 10 byte transfer
*
*******************************************************************************/
void RxDmaCmplt(void)
{
    /* Clears the interupt source in RxDMA channel */
    Cy_DMA_Channel_ClearInterrupt(RxDMA_HW, RxDMA_DW_CHANNEL);
    /* Reconfigure the descriptor for the TxDMA. */
    Cy_DMA_Channel_SetDescriptor(TxDMA_HW, TxDMA_DW_CHANNEL, &TxDMA_Descriptor_1);
    /* Enable the TxDMA channel. This channel is disabled automatically at the end of 
    *  the transfer */
    Cy_DMA_Channel_Enable(TxDMA_HW, TxDMA_DW_CHANNEL); 
}


/* [] END OF FILE */

