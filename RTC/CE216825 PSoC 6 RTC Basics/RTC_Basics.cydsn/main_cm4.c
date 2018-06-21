/******************************************************************************
* File Name: main_cm4.c
* Version 1.10
*
* Description:
*  RTC Code Example. This code example demonstrates how to read and write the
*  current time using a RTC component. The UART interface is used to input a 
*  command and and print the result on the terminal.
* 
* Related Documents: CE216825_PSoC6_RTC_Basics.pdf
*
* Hardware Dependency:
*  1. PSoC 6 MCU device
*  2. CY8CKIT-062-BLE Pioneer Kit
*
*******************************************************************************
* Copyright (2017-2018), Cypress Semiconductor Corporation. All rights reserved.
*******************************************************************************
* This software, including source code, documentation and related materials
* (“Software”), is owned by Cypress Semiconductor Corporation or one of its
* subsidiaries (“Cypress”) and is protected by and subject to worldwide patent
* protection (United States and foreign), United States copyright laws and
* international treaty provisions. Therefore, you may use this Software only
* as provided in the license agreement accompanying the software package from
* which you obtained this Software (“EULA”).
*
* If no EULA applies, Cypress hereby grants you a personal, nonexclusive,
* non-transferable license to copy, modify, and compile the Software source
* code solely for use in connection with Cypress’s integrated circuit products.
* Any reproduction, modification, translation, compilation, or representation
* of this Software except as specified above is prohibited without the express
* written permission of Cypress.
*
* Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND, 
* EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED 
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. Cypress 
* reserves the right to make changes to the Software without notice. Cypress 
* does not assume any liability arising out of the application or use of the 
* Software or any product or circuit described in the Software. Cypress does 
* not authorize its products for use in any products where a malfunction or 
* failure of the Cypress product may reasonably be expected to result in 
* significant property damage, injury or death (“High Risk Product”). By 
* including Cypress’s product in a High Risk Product, the manufacturer of such 
* system or application assumes all risk of such use and in doing so agrees to 
* indemnify Cypress against all liability.
*******************************************************************************/
#include "project.h"
#include "stdio_user.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Macros used to store commands */
#define RTC_CMD_GET_DATE_TIME   ('1')
#define RTC_CMD_SET_DATE_TIME   ('2')

#define MAX_LENGTH              (4u)
#define VALID_DAY_LENGTH        (2u)
#define VALID_MONTH_LENGTH      (2u)
#define VALID_SHORT_YEAR_LENGTH (2u)
#define VALID_LONG_YEAR_LENGTH  (4u)

/*****************************************************/
/*              Function prototype                   */
/*****************************************************/
static void PrintDateTime(void);
static void SetDateTime(void);
static void PrintAvailableCommands(void);
static bool ValidateDateTime(uint32_t year, uint32_t month, uint32_t date, \
            uint32_t sec, uint32_t min, uint32_t hour);
static inline bool IsLeapYear(uint32_t );

/*******************************************************************************
* Function Name: main
********************************************************************************
*
* Summary: This is the system entrance point for Cortex-M4. This function 
*  initializes the UART and RTC component and process the input commands.
*
* Parameters:
*  None
*
* Return:
*  int
*
* Side Effects:
*  None  
*
*******************************************************************************/
int main(void)
{
    char cmd;
    
    /* Initialize with config set in component and enable the UART */
    Cy_SCB_UART_Init(UART_HW, &UART_config, &UART_context);
    Cy_SCB_UART_Enable(UART_HW);
    
    /* \x1b[2J\x1b[;H - ANSI ESC sequence for clear screen */
    printf("\x1b[2J\x1b[;H");
    /* Display starting message */
    printf("CE216825 - RTC Basics\r\n");
    
    /* Initialize RTC */
    if(Cy_RTC_Init(&RTC_config) != CY_RTC_SUCCESS)
    {
        printf("RTC initialization failed \r\n");
        CY_ASSERT(0); /* If RTC initialization failed */
    }
      
    __enable_irq(); /* Enable global interrupts. */
    
    /* Display available commands */
    PrintAvailableCommands();
    
    for(;;)
    {
        /* Read command from terminal */
        cmd = STDIO_GetChar();
        switch(cmd)
        {   
            /* Get current date and time request */
            case RTC_CMD_GET_DATE_TIME:
                PrintDateTime();
            break;
            
            /* Update current date and time request */
            case RTC_CMD_SET_DATE_TIME:
                SetDateTime();
                Cy_SCB_UART_ClearRxFifo(UART_HW);
            break;
                
            default:
                printf("\r\nUnknown command\r\n");
                Cy_SCB_UART_ClearRxFifo(UART_HW);
                PrintAvailableCommands();
            break;
        }
    }
}

/*******************************************************************************
* Function Name: PrintDateTime
********************************************************************************
* Summary:
*  This function prints current date and time to UART
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
static void PrintDateTime(void)
{
    /* Variable used to store date and time information */
    cy_stc_rtc_config_t dateTime;
    
    /*Get the current RTC time and date */
    Cy_RTC_GetDateAndTime(&dateTime);
    
    printf("\r\nCurrent date and time\r\n");
    printf("Date %02u/%02u/%02u\r\n",(uint16_t) dateTime.date, 
            (uint16_t) dateTime.month, (uint16_t) dateTime.year);
    printf("Time %02u:%02u:%02u\r\n", (uint16_t) dateTime.hour, 
            (uint16_t) dateTime.min, (uint16_t) dateTime.sec);
}

/*******************************************************************************
* Function Name: SetDateTime
********************************************************************************
* Summary:
*  This function sets new date and time.
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
static void SetDateTime(void)
{
    /* Variables used to store user input */
    char dateStr[MAX_LENGTH], monthStr[MAX_LENGTH], yearStr[MAX_LENGTH];
    char secStr[MAX_LENGTH], minStr[MAX_LENGTH], hourStr[MAX_LENGTH];
    
    /* Variables used to store date and time information */
    uint32_t date, month, year, sec, min, hour;
    
    printf("\r\nEnter new date (DD MM YY)\r\n");
    scanf("%s %s %s", dateStr, monthStr, yearStr);
    
    if(strlen(dateStr)<= VALID_DAY_LENGTH && strlen(monthStr)<= VALID_MONTH_LENGTH  && \
    (strlen(yearStr)<= VALID_SHORT_YEAR_LENGTH || strlen(yearStr)== VALID_LONG_YEAR_LENGTH ))
    {
        printf("\rEnter new time in 24-hour format (hh mm ss)\r\n");
        scanf("%s %s %s", hourStr, minStr, secStr);
        
        /* Convert string input to decimal */
        date    = atoi(dateStr);
        month   = atoi(monthStr);
        year    = atoi(yearStr);
        sec     = atoi(secStr);
        min     = atoi(minStr);
        hour    = atoi(hourStr);
        
        if(year > CY_RTC_MAX_YEAR) /* If user input 4 digits Year information, set 2 digits Year */
        {
            year = year % 100u;
        }
        
        if(ValidateDateTime(sec, min, hour, date, month, year))
        {
            /* Set date and time */
            if( Cy_RTC_SetDateAndTimeDirect(sec, min, hour, date, 
                month, year ) != CY_RTC_SUCCESS)
            {
                printf("Failed to update date and time\r\n");
                PrintAvailableCommands();
            }
            else
            {
                printf("\r\nDate and Time updated !!\r\n");
                PrintDateTime();
            }
        }
        else
        {
            printf("\r\nInvalid values! Please enter the values in specified format\r\n");
            PrintAvailableCommands();
        }
    }
    else
    {
        printf("\r\nInvalid values! Please enter the values in specified format\r\n");
        PrintAvailableCommands();
    }
}

/*******************************************************************************
* Function Name: PrintAvailableCommands
********************************************************************************
* Summary:
*  This function prints available commands to UART
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
static void PrintAvailableCommands(void)
{
    printf("\r\nAvailable commands \r\n");
    printf("1: Get current time and date\r\n");
    printf("2: Set new time and date\r\n\n");
}


/*******************************************************************************
* Function Name: ValidateDateTime
********************************************************************************
* Summary:
*  This function validates date and time value.

*
* Parameters:
*  sec:  The second valid range is [0-59].
*  min:  The minute valid range is [0-59].
*  hour: The hour valid range is [0-23].
*  date: The date valid range is [1-31], if the month of February is 
*        selected as the Month parameter, then the valid range is [0-29].
*  month: The month valid range is [1-12].
*  year: The year valid range is [0-99].
*
* Return:
*  false - invalid ; true - valid
*
*******************************************************************************/
static bool ValidateDateTime(uint32_t sec, uint32_t min, uint32_t hour, \
                      uint32_t date, uint32_t month, uint32_t year)
{
    /* Variable used to store days in months table */
    static uint8_t daysInMonthTable[CY_RTC_MONTHS_PER_YEAR] = {CY_RTC_DAYS_IN_JANUARY,
                                                            CY_RTC_DAYS_IN_FEBRUARY,
                                                            CY_RTC_DAYS_IN_MARCH,
                                                            CY_RTC_DAYS_IN_APRIL,
                                                            CY_RTC_DAYS_IN_MAY,
                                                            CY_RTC_DAYS_IN_JUNE,
                                                            CY_RTC_DAYS_IN_JULY,
                                                            CY_RTC_DAYS_IN_AUGUST,
                                                            CY_RTC_DAYS_IN_SEPTEMBER,
                                                            CY_RTC_DAYS_IN_OCTOBER,
                                                            CY_RTC_DAYS_IN_NOVEMBER,
                                                            CY_RTC_DAYS_IN_DECEMBER};
    uint8_t daysInMonth;
    bool status = true;
        
    status &= CY_RTC_IS_SEC_VALID(sec);
    status &= CY_RTC_IS_MIN_VALID(min);
    status &= CY_RTC_IS_HOUR_VALID(hour);
    status &= CY_RTC_IS_MONTH_VALID(month);
    status &= CY_RTC_IS_YEAR_SHORT_VALID(year);
    
    if(status)
    {
        daysInMonth = daysInMonthTable[month - 1];
        
        if(IsLeapYear(year + CY_RTC_TWO_THOUSAND_YEARS) && (month == CY_RTC_FEBRUARY))
        {        
            daysInMonth++;
        }
        status &= (date > 0U) && (date <= daysInMonth);
    }
    return status;
}

/*******************************************************************************
* Function Name: IsLeapYear
********************************************************************************
* Summary:
*  This function checks whether the year passed through the parameter is leap or
*  not.  Leap year is identified as a year that is a multiple of 4 or 400 but not 
*  100.
*
* Parameters:
*  year: The year to be checked
*
* Return:
*  false - The year is not leap; true - The year is leap.
*
*******************************************************************************/
static inline bool IsLeapYear(uint32_t year)
{
    return(((0U == (year % 4UL)) && (0U != (year % 100UL))) || (0U == (year % 400UL)));
}

/* [] END OF FILE */
