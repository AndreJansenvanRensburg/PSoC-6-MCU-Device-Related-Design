/*******************************************************************************
* File Name: main.c
*
* Version: 1.0
*
* Description:
*  This example shows how to use multiple DMAs to concatenate data from multiple
*  sources. Uses more than one descriptor and DMA to transfer data from the UART
*  RX buffer and the Real Time Clock to a memory array. Creating a timestamp
*  of the incoming data. After 4 packets are timestamped, echoes them back to
*  the UART.
*******************************************************************************
* Related Document: CE219940.pdf
*
* Hardware Dependency: CY8CKIT-062-BLE PSoC 6 BLE Pioneer Kit
*
******************************************************************************
* Copyright (2017), Cypress Semiconductor Corporation.
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
#include "stdio.h"      //snprintf
#include "string.h"     //strncpy

/* Defines for starting date */
#define START_SEC               (0u)    /* Value must be in range 0-59 */
#define START_MIN               (0u)    /* Value must be in range 0-59 */
#define START_HOUR              (12u)   /* Value must be in range 0-59 */
#define START_DAY               (30u)   /* Value must be in range 1-31 */
#define START_MONTH             (3u)    /* Value must be in range 1-12 */
#define START_YEAR              (17u)   /* Value must be in range 0-99 */

/* Defines for RTC Buffer Size */
#define RTC_BUFFER_SIZE         (20u)

/* Defines for Buffer Size */
#define PACKET_SIZE             (5u)
#define CON_BUFFER_SIZE         (PACKET_SIZE + RTC_BUFFER_SIZE)
#define MAX_LOG                 (4u)   /* Number of memory buffers in the array */

/* Buffers for DMA Operation */
uint8 ConcatenatedDataBuffer[CON_BUFFER_SIZE];
char RTCBuffer[RTC_BUFFER_SIZE];
uint8_t MemoryArray[MAX_LOG][CON_BUFFER_SIZE];

/* Local function declarations */
void ConfigureRTC(void);
void ConfigureRxDma(void);
void ConfigureMemoryDma(void);
void ConfigureTxDma(void);

/* ISR declarations */
void RxDma_Complete(void);
void MemoryDma_Complete(void);
void TxDma_Complete(void);
void UartErrors(void);

/*******************************************************************************
* Function Name: main
********************************************************************************
*
* The main function performs the following actions:
*  1. Sets up RX and TX DMAs to handle UART RX+TX direction and an extra DMA
*     to handle data concatenation into the memory array.
*  2. Sets up the RTC component and its interrupt.
*  3. UART sends text header into the serial terminal.
*  4. UART waits for the 4 packets to send them back to
*     the serial terminal with a timestamp.
*
*******************************************************************************/
int main(void)
{       
    /* Configures DMA Rx and Tx channels for operation. */
    ConfigureRxDma();
    ConfigureMemoryDma();
    ConfigureTxDma();
    
    /* Configures and starts the RTC to interrupt every second */
    /* Sets the start time of the RTC as 12:00 Mar 30. 2017 */
    ConfigureRTC();

    /* Configure UART Interrupt and start it. */  
    Cy_SysInt_Init(&UART_INT_cfg, &UartErrors);
    NVIC_EnableIRQ(UART_INT_cfg.intrSrc);
    UART_Start();
  
    //ANSI VT100 - Erases the screen with the background color and moves the cursor to home.
    //Comment out next line if not using VT100 terminals.
    UART_PutString("\x1b[2J\x1b[;H");/* <esc>[2J<esc>[;H */

    UART_PutString("\r\n****************************************************************\r\n");
    UART_PutString("This is CE219940_Multiple_DMA_Concatenation code example project\r\n");
    UART_PutString("Transmit a 5 characters packet to add a RTC timestamp.\r\n");   
    UART_PutString("After 4 packets the data will be echoed back.\r\n");   
    UART_PutString("Enter the 1st four packets: ");
    /* Enable global interrupts. */
    __enable_irq();
    
    for(;;)
    {
        //Do nothing.
        Cy_SysPm_Sleep(CY_SYSPM_WAIT_FOR_INTERRUPT);
    }
}

/*******************************************************************************
* Function Name: ConfigureRTC
********************************************************************************
*
* The ConfigureRTC function performs the following actions:
*  1. Sets up the Watch-Crystal Oscillator as the RTC Source Clock.
*  2. Sets up a Date & Time.
*  3. Configures Alarm 1 to interrupt every second.
*  4. Initializes the RTC Component.
*
*******************************************************************************/
void ConfigureRTC(void)
{
    /* Configures the Watch-Crystal Oscillator as the clock source for the RTC */
    Cy_SysClk_ClkBakSetSource(CY_SYSCLK_BAK_IN_WCO);
 
    /* Sets the start time of the RTC as 12:00 Mar 30. 2017 */
    /* Wait until operation is successful */
    while(RTC_SetDateAndTimeDirect(START_SEC, START_MIN, START_HOUR,
        START_DAY, START_MONTH, START_YEAR) != CY_RET_SUCCESS);
    
    /* Create an Interrupt every second to update string 
    * Setting every alarm enable to CY_RTC_ALARM_DISABLE except
    * master enable (ALM_EN to CY_RTC_ALARM_ENABLE) achieves this.
    * Alarm date will be ignored but must still be valid 
    */
    cy_stc_rtc_alarm_t AlarmConfig;
    AlarmConfig.sec = 0;
    AlarmConfig.secEn = CY_RTC_ALARM_DISABLE;
    AlarmConfig.min = 0;
    AlarmConfig.minEn = CY_RTC_ALARM_DISABLE;    
    AlarmConfig.hour = 12;
    AlarmConfig.hourEn = CY_RTC_ALARM_DISABLE;
    AlarmConfig.date = 1;
    AlarmConfig.dateEn = CY_RTC_ALARM_DISABLE;
    AlarmConfig.dayOfWeek = 1;
    AlarmConfig.dayOfWeekEn = CY_RTC_ALARM_DISABLE;
    AlarmConfig.month = 1;
    AlarmConfig.monthEn = CY_RTC_ALARM_DISABLE;
    AlarmConfig.almEn = CY_RTC_ALARM_ENABLE;

    /* Wait until operation is successful */
    while(Cy_RTC_SetAlarmDateAndTime(&AlarmConfig, CY_RTC_ALARM_1) != CY_RET_SUCCESS);
    Cy_RTC_SetInterruptMask(CY_RTC_INTR_ALARM1);
        
    /* Initialize the RTC */
    RTC_Start();
    return;
}

/*******************************************************************************
* Function Name: ConfigureRxDma
********************************************************************************
*
* The ConfigureRxDma function performs the following actions:
*  1. Configures the source address of the first descriptor as the date string.
*  2. Configures the source address of the second descriptor as the UART RX FIFO.
*  3. Configures both destination addresses to the concatenated buffer with the
*     respective offset.
*  4. Starts the DMA Component.
*
*******************************************************************************/
void ConfigureRxDma(void)
{
    /* Configure DMA Descriptors as 1D (Only X loops)*/
    RxDma_SetDescriptorType(&RxDma_Descriptor_1, CY_DMA_1D_TRANSFER);
    RxDma_SetDescriptorType(&RxDma_Descriptor_2, CY_DMA_1D_TRANSFER);
    
    /* Configure DMA channel per parameters from the customizer */
    RxDma_Init();

    /* Set sources and destination for descriptor 1. */
    RxDma_SetSrcAddress(&RxDma_Descriptor_1, (uint32_t *) RTCBuffer);
    RxDma_SetDstAddress(&RxDma_Descriptor_1, (uint32_t *) ConcatenatedDataBuffer);    
    
    /* Set sources and destination for descriptor 2. */
    RxDma_SetSrcAddress(&RxDma_Descriptor_2, (uint32_t *) &UART_HW->RX_FIFO_RD);
    RxDma_SetDstAddress(&RxDma_Descriptor_2, (uint32_t *) &ConcatenatedDataBuffer[RTC_BUFFER_SIZE]);

    /* Initialize and enable interrupt from RxDma. */
    Cy_SysInt_Init(&RX_DMA_INT_cfg, &RxDma_Complete);
    NVIC_EnableIRQ(RX_DMA_INT_cfg.intrSrc);

    /* Enable DMA interrupt source. */
    RxDma_SetInterruptMask(CY_DMA_INTR_MASK);
    RxDma_SetInterrupt();

    /* Enable channel and DMA block to start descriptors processing. */
    RxDma_ChannelEnable();
    Cy_DMA_Enable(RxDma_HW);
    return;
}

/*******************************************************************************
* Function Name: ConfigureMemoryDma
********************************************************************************
*
* The ConfigureMemoryDma function performs the following actions:
*  1. Configures the source address of the descriptor as the concatenated buffer.
*  2. Configures the destination address of the descriptor as the memory array.
*  3. Starts the DMA Component.
*
*******************************************************************************/
void ConfigureMemoryDma(void)
{
    /* Configure DMA Descriptors as 2D (X and Y loops)*/
    MemoryDma_SetDescriptorType(&MemoryDma_Descriptor_1, CY_DMA_2D_TRANSFER);
    
    /* Configure DMA channel per parameters from the customizer. */
    MemoryDma_Init();
    
    /* Set sources and destination for descriptor 1. */
    MemoryDma_SetSrcAddress(&MemoryDma_Descriptor_1, (uint32_t *) ConcatenatedDataBuffer);
    MemoryDma_SetDstAddress(&MemoryDma_Descriptor_1, (uint32_t *) MemoryArray);

    /* Initialize and enable interrupt from MemoryDma. */
    Cy_SysInt_Init(&MEMORY_DMA_INT_cfg, &MemoryDma_Complete);
    NVIC_EnableIRQ(MEMORY_DMA_INT_cfg.intrSrc);

    /* Enable DMA interrupt source. */
    MemoryDma_SetInterruptMask(CY_DMA_INTR_MASK);    
    
    /* Enable channel and DMA block to start descriptors processing. */
    MemoryDma_ChannelEnable();
    Cy_DMA_Enable(MemoryDma_HW);
    return;
}

/*******************************************************************************
* Function Name: ConfigureTxDma
********************************************************************************
*
* The ConfigureTxDma function performs the following actions:
*  1. Configures the source address of the descriptor as the memory array.
*  2. Configures the destination address of the descriptor as the UART TX FIFO.
*  3. Starts the DMA Component.
*
*******************************************************************************/
void ConfigureTxDma(void)
{
    /* Configure DMA Descriptors as 2D (X and Y loops)*/
    TxDma_SetDescriptorType(&TxDma_Descriptor_1, CY_DMA_2D_TRANSFER);

    /* Configure DMA channel per parameters from the customizer. */
    TxDma_Init();
    
    /* Set sources and destination for descriptor 1. */
    TxDma_SetSrcAddress(&TxDma_Descriptor_1, (uint32_t *) &(MemoryArray[0][0]));
    TxDma_SetDstAddress(&TxDma_Descriptor_1, (uint32_t *) &UART_HW->TX_FIFO_WR);

    /* Initialize and enable interrupt from TxDma. */
    Cy_SysInt_Init(&TX_DMA_INT_cfg, &TxDma_Complete);
    NVIC_EnableIRQ(TX_DMA_INT_cfg.intrSrc);

    /* Enable DMA interrupt source. */
    TxDma_SetInterruptMask(CY_DMA_INTR_MASK);    

    /* Enable the hardware but leave it disabled as it will be enable by another DMA. */
    TxDma_ChannelDisable();
    Cy_DMA_Enable(TxDma_HW);
    return;
}
/* Interrupt Service Routines */

/*******************************************************************************
* Function Name: Cy_RTC_Alarm1Interrupt
********************************************************************************
*
* The Cy_RTC_Alarm1Interrupt ISR performs the following actions:
*  1. Gets the current date & time.
*  2. Writes the current date & time as a string into the string buffer.
*
*******************************************************************************/
void Cy_RTC_Alarm1Interrupt(void)
{
    cy_stc_rtc_config_t Date;
    char tempString[RTC_BUFFER_SIZE + 1]; /* Null Termination */
    RTC_GetDateAndTime(&Date);
    snprintf(tempString, RTC_BUFFER_SIZE + 1,
        "\r\n%02d:%02d:%02d %02d/%02d/%02d ", 
        (int)Date.hour,(int)Date.min,(int)Date.sec,
        (int)Date.month,(int)Date.date,(int)Date.year);
    /* Remove Null termination */
    strncpy(RTCBuffer, tempString, RTC_BUFFER_SIZE);
}

/*******************************************************************************
* Function Name: RxDma_Complete
********************************************************************************
*
* The RxDma_Complete ISR performs the following actions:
*  1. Clears the Interrupt and resets the DMA to the first descriptor.
*  2. If there was an error stays in the DMA ISR forever.
*
*******************************************************************************/
void RxDma_Complete(void)
{
    /* Check interrupt cause to capture errors. */
    if (CY_DMA_INTR_CAUSE_COMPLETION != RxDma_GetInterruptStatus())
    {
        /* The DMA error must not occur while RX operations.
        * Go into infinite loop to identify that something is wrong with DMA.
        */
        for(;;)
        {
        }
    }
    /* Clear interrupt after receiving cause */    
    RxDma_ClearInterrupt();
    /* Reset DMA Descriptor chain */
    RxDma_SetDescriptor(&RxDma_Descriptor_1);
}

/*******************************************************************************
* Function Name: MemoryDma_Complete
********************************************************************************
*
* The MemoryDma_Complete ISR performs the following actions:
*  1. Clears the Interrupt.
*  2. If there was an error stays in the DMA ISR forever.
*  3. Resets the TxDma Descriptor and enables it to start sending the memory array.
*
*******************************************************************************/
void MemoryDma_Complete(void)
{
    /* Check interrupt cause to capture errors. */
    if (CY_DMA_INTR_CAUSE_COMPLETION != MemoryDma_GetInterruptStatus())
    {
        /* Go into infinite loop to identify that something is wrong with the DMA. */
        for(;;)
        {
        }
    }
    /* Clear interrupt after receiving cause */
    MemoryDma_ClearInterrupt();
    /* Reset DMA Descriptor chain and enable the DMA. */
    TxDma_SetDescriptor(&TxDma_Descriptor_1);
    TxDma_ChannelEnable();
}

/*******************************************************************************
* Function Name: TxDma_Complete
********************************************************************************
*
* The TxDma_Complete ISR performs the following actions:
*  1. Outputs a string to the UART for better Terminal visualization.
*
*******************************************************************************/
void TxDma_Complete(void)
{    
    /* Check interrupt cause to capture errors. */
    /* Having a null pointer on the descriptor is not an error on this DMA */
    switch(TxDma_GetInterruptStatus())
    {
    case CY_DMA_INTR_CAUSE_COMPLETION:
        UART_PutString("\r\nEnter the next four packets: ");
        //while(UART_GetNumInTxFifo() != 0) {}
        break;
    case CY_DMA_INTR_CAUSE_CURR_PTR_NULL:
        break;
    default:
        /* Go into infinite loop to identify that something is wrong with the DMA. */
        for(;;)
        {
        }
    }
    /* Disable DMA channel until the MemoryDMA enables it again */
    TxDma_ChannelDisable();
    /* Clear interrupt after receiving cause */
    TxDma_ClearInterrupt();
}

/*******************************************************************************
* Function Name: UartErrors
********************************************************************************
*
* The UartErrors ISR performs the following actions:
*  1. Gets the error cause (TX or RX FIFO Overflow).
*  2. Stays in the ISR forever.
*
*******************************************************************************/
void UartErrors(void)
{
    uint32 intrSrcRx;
    uint32 intrSrcTx;

    /* Get RX interrupt sources. */
    intrSrcRx = UART_GetRxFifoStatus();
    UART_ClearRxFifoStatus(intrSrcRx);

    /* Get TX interrupt sources. */
    intrSrcTx = UART_GetTxFifoStatus();
    UART_ClearTxFifoStatus(intrSrcTx);

    /* The TX/RX overflow must not occur while operation.
    * Go into infinite loop to identify that something went wrong.
    */
    for(;;)
    {
    }
}

/* [] END OF FILE */
