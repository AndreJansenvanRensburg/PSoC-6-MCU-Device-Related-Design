

/******************************************************************************
* File Name: main_cm4.c
* Version 1.0
*
* Description:
*   CE218553- PWM trigger a DMA channel. Code example implements a PWM to DMA 
*	trigger connection using PSoC Creator. The Code example also explains how 
*	the PSoC Creator's auto routing, implements this trigger route by connecting 
* 	appropriate trigger multiplexers.
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

#define ARRAY_SIZE               (256u)
#define BREATHING_CHANGE         (512u)

/* This array holds all the compare values that will be loaded onto the PWM 
*  to create the breathing pattern */
uint32 compareVal[ARRAY_SIZE];

void arrayInit(void);

int main(void)
{
    cy_stc_dma_channel_config_t channelConfig;
    __enable_irq(); /* Enable global interrupts. */
    
    /*initialize the compareVal array */
    arrayInit();
    
    /* Initialize, enable and trigger the  PWM */
    Cy_TCPWM_PWM_Init(PWM_HW, PWM_TCPWM__CNT_IDX, &PWM_config);
    Cy_TCPWM_Enable_Multiple(PWM_HW, PWM_CNT_MASK);
    Cy_TCPWM_TriggerStart(PWM_HW, PWM_CNT_MASK);
    
    /* Configure DMA Descriptor to change the PWM compare value per breathingLed array */
    DMA_Descriptor_1_config.srcAddress = &compareVal;
    DMA_Descriptor_1_config.dstAddress = (void *) &(PWM_HW->CNT[PWM_TCPWM__CNT_IDX].CC);
    Cy_DMA_Descriptor_Init(&DMA_Descriptor_1, &DMA_Descriptor_1_config);
    
    /* Configure DMA Channel with the descriptor configured above */
    channelConfig.descriptor    = &DMA_Descriptor_1;
    channelConfig.preemptable       = DMA_PREEMPTABLE;
    channelConfig.priority          = DMA_PRIORITY;
    channelConfig.enable            = 0u;

    Cy_DMA_Channel_Init(DMA_DW__BLOCK_HW, DMA_DW_CHANNEL, &channelConfig);

    Cy_DMA_Channel_Enable(DMA_DW__BLOCK_HW, DMA_DW__CHANNEL_NUMBER);
   
    /* Enable DMA hardware block */
    Cy_DMA_Enable(DMA_DW__BLOCK_HW);


    for(;;)
    {

    }
    
}


/* Function Name: arrayInit
*  Description: This function is used to initialize the array compareVal[]. The array is initialized with
*  list of compare values. The array contents from index 0-127 are in a increasing order, and the rest 
*  are in decreasing order, thus generating a breathing pattern when these values are streamed into the 
*  PWM's compare register
* 
*  Return: void
*/

void arrayInit(void)
{
    uint32_t i=0u;
    /* Creating the array with increasing and decreasing compare value to generate the breathing */
    compareVal[i] = 0;
    
    for(i=0u; i < ARRAY_SIZE>>1; i++)
    {
        /* Increase compare values */
        compareVal[i] = compareVal[i-1u] + BREATHING_CHANGE;
    }
    for(i=(ARRAY_SIZE>>1); i < ARRAY_SIZE; i++)
    {
        /* Decrease compare value */
        compareVal[i] = compareVal[i-1] - BREATHING_CHANGE;
    }
}
