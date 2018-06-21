/******************************************************************************
* File Name: main_cm4.c
* Version 1.0
*
* Description:
*   RTC Code Example. This code example demonstrates how to generate a daily 
*   alarm using RTC alarm interrupt.
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
#include "rtc_user.h"
#include "stdio_user.h"
#include <stdio.h>

/* Macros used to store LED states */
#define LED_OFF             (1u)

/* Daily mask compares the day of week after the alarm expires to find the next alarm */
#define INITIAL_ALARM_DELAY (3u)
#define ALARM_TIME_SEC      (RTC_INITIAL_DATE_SEC + INITIAL_ALARM_DELAY)

/* Daily mask compares the day of week after the alarm expires to find the next alarm */
/* | Bit7 | Bit6 | Bit5 | Bit4 | Bit3 | Bit2 | Bit1 | Bit0 | */
/* |------|------|------|------|------|------|------|------| */
/* | Sun  | Fri  | Thu  | Wed  | Tue  | Mon  | Sun  | X    | */
#define ALARM_ENABLE_SUNDAY     (0u) 
#define ALARM_ENABLE_MONDAY     (1u)
#define ALARM_ENABLE_TUESDAY    (1u)
#define ALARM_ENABLE_WEDNESDAY  (1u)
#define ALARM_ENABLE_THURSDAY   (1u)
#define ALARM_ENABLE_FRIDAY     (1u)
#define ALARM_ENABLE_SATURDAY   (0u)

/* Set Mon to Fri - The alarm will expire after INITIAL_ALARM_DELAY from start 
   time and then every Monday to Friday at initialized time */
uint8_t dailyAlarmMask = (((1u & ALARM_ENABLE_SUNDAY)   << CY_RTC_SUNDAY)   | \
                          ((1u & ALARM_ENABLE_MONDAY)   << CY_RTC_MONDAY)   | \
                          ((1u & ALARM_ENABLE_TUESDAY)  << CY_RTC_TUESDAY)  | \
                          ((1u & ALARM_ENABLE_WEDNESDAY)<< CY_RTC_WEDNESDAY)| \
                          ((1u & ALARM_ENABLE_THURSDAY) << CY_RTC_THURSDAY) | \
                          ((1u & ALARM_ENABLE_FRIDAY)   << CY_RTC_FRIDAY)   | \
                          ((1u & ALARM_ENABLE_SATURDAY) << CY_RTC_SATURDAY));

/* Initialize alarm time which is three seconds after system start */
cy_stc_rtc_alarm_t myRtcAlarmConfig = 
{
    .sec            = ALARM_TIME_SEC,
    .secEn          = CY_RTC_ALARM_ENABLE,
    .min            = RTC_INITIAL_DATE_MIN,
    .minEn          = CY_RTC_ALARM_ENABLE,
    .hour           = RTC_INITIAL_DATE_HOUR,
    .hourEn         = CY_RTC_ALARM_ENABLE,
    .dayOfWeek      = RTC_INITIAL_DATE_DOW,
    .dayOfWeekEn    = CY_RTC_ALARM_ENABLE,
    .date           = RTC_INITIAL_DATE_DOM,
    .dateEn         = CY_RTC_ALARM_DISABLE,
    .month          = RTC_INITIAL_DATE_MONTH,
    .monthEn        = CY_RTC_ALARM_DISABLE,
    .almEn          = CY_RTC_ALARM_ENABLE
};

/* Flag variables used for interrupt status */
bool alarmExpired   = false;
bool switchFlag     = false;

cy_stc_rtc_config_t dateTime;

/********************************/
/*      Function prototype      */
/********************************/
void RtcInterruptHandler(void);
void SwitchInterruptHandler(void);
void McwdtInterruptHandler(void);

/*******************************************************************************
* Function Name: main
********************************************************************************
* Summary: This is the system entrance point for Cortex-M4. This function 
* - initializes the PSoC Components and configures RTC alarm
* - handle RTC and switch interrupts
* - display UART messages
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
    Cy_SCB_UART_Init(UART_HW, &UART_config, &UART_context);
    Cy_SCB_UART_Enable(UART_HW);
    
    /* Initialize RTC */
    if(CY_RTC_SUCCESS != RtcInit())
    {
        printf("RTC initialization failed \r\n");
        /* If operation fails, halt */
        CY_ASSERT(0u);
    }
    else
    {
        /* Configure RTC alarm */
        if(CY_RTC_SUCCESS != RtcConfigAlarm(&myRtcAlarmConfig))
        {
            printf("RTC alarm configuration failed\r\n");
            /* If operation fails, halt */
            CY_ASSERT(0u);
        }
    }
    
    /* Enable RTC interrupt handler function */
    Cy_SysInt_Init(&RTC_RTC_IRQ_cfg, RtcInterruptHandler);
    NVIC_EnableIRQ(RTC_RTC_IRQ_cfg.intrSrc);
    
    /* Enable switch(SW2) interrupt */
    Cy_SysInt_Init(&IRQ_SW2_cfg, SwitchInterruptHandler);
    NVIC_EnableIRQ(IRQ_SW2_cfg.intrSrc);
    
    __enable_irq(); /* Enable global interrupts. */
    
    /* Turn off LED */
    Cy_GPIO_Write(LED_R_0_PORT, LED_R_0_NUM, LED_OFF);
    
    /* \x1b[2J\x1b[;H - ANSI ESC sequence for clear screen */
    printf("\x1b[2J\x1b[;H"); 
    /* Display starting message*/
    printf("CE218964 - RTC Daily Alarm\r\n\n");
    
    for(;;)
    {
        PrintCurrentDateTime();
        if(alarmExpired)
        {
            if(switchFlag) 
            {
                switchFlag = false;
                alarmExpired = false;
                /* Find next alarm date */
                FindNextAlarmDate(dailyAlarmMask);
                Cy_GPIO_Write(LED_R_0_PORT, LED_R_0_NUM, LED_OFF);
            }
            else
            {
                Cy_GPIO_Inv(LED_R_0_PORT, LED_R_0_NUM);
                printf("Alarm Expired !! Press SW2 to set up alarm for the next day\r\n");
            }
        }
        Cy_SysLib_Delay(1000); /* 1 sec delay */
    }
}

/*******************************************************************************
* Function Name: RtcInterruptHandler
****************************************************************************//**
* This is the handler of the RTC interrupt in CPU NVIC.
* 
* This function calls the RTC driver interrupt handler function Cy_RTC_Interrupt.
* The handler checks which RTC interrupt was asserted and calls the 
* respective RTC driver interrupt handler functions: Cy_RTC_Alarm1Interrupt(), 
* Cy_RTC_Alarm1Interrupt() or Cy_RTC_DstInterrupt(), and 
* Cy_RTC_CenturyInterrupt().
* 
* The order of the RTC handler functions execution is incremental. 
* Cy_RTC_Alarm1Interrupt() is run as the first one and Cy_RTC_CenturyInterrupt() 
* is called as the last one.
*
* Current project requires to implement weakly linked Cy_RTC_Alarm1Interrupt() 
* function. See below implementation of Cy_RTC_Alarm1Interrupt().
*
* Parameters:
*  None
*
* Returen:
*  None
*
*******************************************************************************/
void RtcInterruptHandler(void)
{
    Cy_RTC_Interrupt(&RTC_dstConfig, RTC_rtcDstStatus);
}

/*******************************************************************************
* Function Name: Cy_RTC_Alarm1Interrupt
********************************************************************************
*  This function is called when the CY_RTC_INTR_ALARM1 is expired.
*  It sets ALARM1_EXPIRED bit into myAlarmStatus variable to notify the main
*  processing routine. 
*  This function overrides the weakly linked Cy_RTC_Alarm1Interrupt() in cy_rtc.c
*
* Parameters:
*  None
*
* Returen:
*  None
*
*******************************************************************************/
void Cy_RTC_Alarm1Interrupt(void)
{
    alarmExpired = true;
}

/*******************************************************************************
* Function Name: SwitchInterruptHandler
********************************************************************************
*  Interrupt service routine for switch interrupt.
*
* Parameters:
*  None
*
* Returen:
*  None
*
*******************************************************************************/
void SwitchInterruptHandler(void)
{
    Cy_GPIO_ClearInterrupt(SW2_0_PORT, SW2_0_NUM);
    switchFlag = true;
    NVIC_ClearPendingIRQ(IRQ_SW2_cfg.intrSrc);
}

/* [] END OF FILE */
