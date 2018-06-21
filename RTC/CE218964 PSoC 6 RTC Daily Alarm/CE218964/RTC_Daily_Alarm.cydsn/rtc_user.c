/******************************************************************************
* File Name: rtc_user.c
* Version 1.0
*
* Description:
*  This file contains RTC related functions.
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
#include "rtc_user.h"

char_t days[CY_RTC_DAYS_PER_WEEK][4] = {"SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT"};

/*******************************************************************************
* Function Name: RtcInit
********************************************************************************
* Summary: This function configures the RTC registers using the cy_stc_rtc_config_t 
* structure.
*
* Parameters:
*  cy_en_rtc_status_t
*
* Returen:
*  None
*******************************************************************************/
cy_en_rtc_status_t RtcInit(void)
{
    uint32_t initTimeout = MY_RTC_ACCESS_RETRY;
    cy_en_rtc_status_t initRtc;
    
    /* RTC block doesn't allow to access, when synchronizing the user registers
       and the internal actual RTC registers.It will return RTC_BUSY value, if it 
       is not available to update the configuration values. Needs to retry, if it
       doesn't return CY_RTC_SUCCESS */
    do
    {
        initRtc = Cy_RTC_Init(&RTC_config);
        initTimeout--;
        
        Cy_SysLib_Delay(MY_RTC_RETRY_DELAY);
    } while(( initRtc != CY_RTC_SUCCESS) && (initTimeout != 0u));
    
	return (initRtc);
}

/*******************************************************************************
* Function Name: RtcConfigAlarm
********************************************************************************
* Summary: This function configures the RTC alarm.
*
* Parameters:
*  cy_stc_rtc_alarm_t
*
* Returen:
*  cy_en_rtc_status_t
*
*******************************************************************************/
cy_en_rtc_status_t RtcConfigAlarm(cy_stc_rtc_alarm_t *myRtcAlarmConfig)
{
    uint32_t timeout = MY_RTC_ACCESS_RETRY;
    cy_en_rtc_status_t retStatus;
    
    /* RTC block doesn't allow to access, when synchronizing the user registers
       and the internal actual RTC registers.It will return RTC_BUSY value, if it 
       is not available to update the configuration values. Needs to retry, if it
       doesn't return CY_RTC_SUCCESS */
    do
	{
		retStatus = Cy_RTC_SetAlarmDateAndTime(myRtcAlarmConfig, CY_RTC_ALARM_1);
		timeout--;
        
		Cy_SysLib_Delay(MY_RTC_RETRY_DELAY);
    } while(( retStatus != CY_RTC_SUCCESS) && (timeout != 0u));
    
	if(retStatus == CY_RTC_SUCCESS)
    {
        /* Set interrupt for next custom alarm */
		Cy_RTC_SetInterruptMask(CY_RTC_INTR_ALARM1);
    }
    
	return (retStatus);
}

/*******************************************************************************
* Function Name: FindNextAlarmDate
********************************************************************************
* Summary:
*  This function finds the next alarm day and configures the RTC alarm.
*
* Parameters:
*  uint8_t: Daily alarm mask variable 
*
* Returen:
*  None
*
*******************************************************************************/
void FindNextAlarmDate(uint8_t dailyAlarmMask)
{
    cy_stc_rtc_alarm_t alarm;
    uint32_t alarmDay;
    uint8_t i;
    
    /* Get current date and time */
    Cy_RTC_GetAlarmDateAndTime(&alarm,CY_RTC_ALARM_1);
    
    alarmDay = alarm.dayOfWeek;
    
    /* Find next alarm day */
    for(i = 0; i < CY_RTC_DAYS_PER_WEEK; i++)
    {
        alarmDay = (alarmDay % CY_RTC_DAYS_PER_WEEK) + 1;
        if(dailyAlarmMask & (1 << alarmDay))
        {
            alarm.dayOfWeek = alarmDay;
            break;
        }
    }
    
    /* Configure RTC alarm */
    if( RtcConfigAlarm(&alarm) != CY_RTC_SUCCESS)
    {
        printf("Alarm configuration failed\r\n");
        CY_ASSERT(0u);
    }
    else
    {
        printf("Next alarm day configured for: %s\r\n", days[alarmDay - 1]);
    }
}

/*******************************************************************************
* Function Name: PrintCurrentDateTime
********************************************************************************
* Summary:
*  This function prints current time to UART
*
* Parameters:
*  None
*
* Returen:
*  None
*******************************************************************************/
void PrintCurrentDateTime(void)
{
    cy_stc_rtc_config_t dateTime;
    
    /* Get current date and time */ 
    Cy_RTC_GetDateAndTime(&dateTime);
     
    printf("%02u/%02u/%02u %s %02u:%02u:%02u\r\n", \
        (uint16_t)dateTime.date, (uint16_t)dateTime.month, (uint16_t)dateTime.year, days[dateTime.dayOfWeek-1], \
        (uint16_t)dateTime.hour, (uint16_t)dateTime.min, (uint16_t)dateTime.sec);
}
/* [] END OF FILE */
