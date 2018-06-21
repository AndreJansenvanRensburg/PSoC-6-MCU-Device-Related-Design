/*****************************************************************************
* File Name: main_cm0.c
*
* Description:  This file contains the implementation of the main function of
* the CM0+.
*
* Related Document: Code example CE219881
*
* Hardware Dependency: See code example CE219881
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

/* Low power counter configuration (Copied from Counter.c), 
   but with a different clockPrescaler. */
cy_stc_tcpwm_counter_config_t const CounterLp_config =
{
        .period = 1000000000UL,
        .clockPrescaler = 0UL,
        .runMode = 0UL,
        .countDirection = 0UL,
        .compareOrCapture = 2UL,
        .compare0 = 16384UL,
        .compare1 = 16384UL,
        .enableCompareSwap = false,
        .interruptSources = 0UL,
        .captureInputMode = 3UL,
        .captureInput = CY_TCPWM_INPUT_CREATOR,
        .reloadInputMode = 3UL,
        .reloadInput = CY_TCPWM_INPUT_CREATOR,
        .startInputMode = 3UL,
        .startInput = CY_TCPWM_INPUT_CREATOR,
        .stopInputMode = 3UL,
        .stopInput = CY_TCPWM_INPUT_CREATOR,
        .countInputMode = 3UL,
        .countInput = CY_TCPWM_INPUT_CREATOR,
};

/* Project Constants */
#define SWITCH_NO_EVENT     0u
#define SWITCH_QUICK_PRESS  1u
#define SWITCH_SHORT_PRESS  2u
#define SWITCH_LONG_PRESS   3u

#define LED_FULL_BRIGHT     99u
#define LED_LOW_BRIGHT      5u

#define FLL_CLOCK_TIMEOUT   200000u

/* Callback parameter linked to the PWM */
cy_stc_syspm_callback_params_t callbackParams;

/* Auxiliary Prototype functions */
uint32_t GetSwitchEvent(void);

/* Callback Prototypes */
cy_en_syspm_status_t TCPWM_SleepCallback(cy_stc_syspm_callback_params_t *callbackParams);
cy_en_syspm_status_t TCPWM_DeepSleepCallback(cy_stc_syspm_callback_params_t *callbackParams);
cy_en_syspm_status_t TCPWM_EnterLowPowerCallback(cy_stc_syspm_callback_params_t *callbackParams);
cy_en_syspm_status_t TCPWM_ExitLowPowerCallback(cy_stc_syspm_callback_params_t *callbackParams);
cy_en_syspm_status_t Clock_EnterLowPowerCallback(cy_stc_syspm_callback_params_t *callbackParams);
cy_en_syspm_status_t Clock_ExitLowPowerCallback(cy_stc_syspm_callback_params_t *callbackParams);

/*******************************************************************************
* Function Name: main
****************************************************************************//**
*
* The main function for the Cortex-M0+ CPU does the following:
*  Initialization:
*  - Register sleep callbacks.
*  - Initialize the PWM block that controls the LED brightness.
*  Do forever loop:
*  - Check if SW2 was pressed and for how long.
*  - If short pressed, go to sleep.
*  - If long pressed, go to deep sleep.
*  
*******************************************************************************/
int main(void)
{
    /* Callback declaration for Power Modes */
    cy_stc_syspm_callback_t PwmSleepCb = {TCPWM_SleepCallback,      /* Callback function */
                                          CY_SYSPM_SLEEP,           /* Callback type */
                                          CY_SYSPM_SKIP_CHECK_READY | 
                                          CY_SYSPM_SKIP_CHECK_FAIL, /* Skip mode */
                                          &callbackParams,          /* Callback params */
                                          NULL, NULL};
    cy_stc_syspm_callback_t PwmDeepSleepCb = {TCPWM_DeepSleepCallback,  /* Callback function */
                                              CY_SYSPM_DEEPSLEEP,       /* Callback type */
                                              CY_SYSPM_SKIP_CHECK_READY |
                                              CY_SYSPM_SKIP_CHECK_FAIL, /* Skip mode */
                                              &callbackParams,          /* Callback params */
                                              NULL, NULL};
    cy_stc_syspm_callback_t PwmEnterLpCb = {TCPWM_EnterLowPowerCallback,    /* Callback function */
                                            CY_SYSPM_ENTER_LP_MODE,         /* Callback type */
                                            CY_SYSPM_SKIP_CHECK_READY |
                                            CY_SYSPM_SKIP_CHECK_FAIL |
                                            CY_SYSPM_SKIP_BEFORE_TRANSITION, /* Skip mode */
                                            &callbackParams,                /* Callback params */
                                            NULL, NULL};    
    cy_stc_syspm_callback_t PwmExitLpCb = {TCPWM_ExitLowPowerCallback,      /* Callback function */
                                           CY_SYSPM_EXIT_LP_MODE,           /* Callback type */
                                           CY_SYSPM_SKIP_CHECK_READY |
                                           CY_SYSPM_SKIP_CHECK_FAIL |
                                           CY_SYSPM_SKIP_BEFORE_TRANSITION, /* Skip mode */
                                           &callbackParams,                 /* Callback params */
                                           NULL, NULL};    
    cy_stc_syspm_callback_t ClkEnterLpCb = {Clock_EnterLowPowerCallback,    /* Callback function */
                                            CY_SYSPM_ENTER_LP_MODE,         /* Callback type */
                                            CY_SYSPM_SKIP_CHECK_READY |
                                            CY_SYSPM_SKIP_CHECK_FAIL |
                                            CY_SYSPM_SKIP_AFTER_TRANSITION, /* Skip mode */
                                            &callbackParams,                /* Callback params */
                                            NULL, NULL};    
    cy_stc_syspm_callback_t ClkExitLpCb = {Clock_ExitLowPowerCallback,      /* Callback function */
                                           CY_SYSPM_EXIT_LP_MODE,           /* Callback type */
                                           CY_SYSPM_SKIP_CHECK_READY |
                                           CY_SYSPM_SKIP_CHECK_FAIL |
                                           CY_SYSPM_SKIP_AFTER_TRANSITION,  /* Skip mode */
                                           &callbackParams,                 /* Callback params */
                                           NULL, NULL};  
    
    /* Enable CM4.  CY_CORTEX_M4_APPL_ADDR must be updated if CM4 memory layout is changed. */
    Cy_SysEnableCM4(CY_CORTEX_M4_APPL_ADDR); 
    
    /* Register SysPm callbacks */
    Cy_SysPm_RegisterCallback(&PwmSleepCb);
    Cy_SysPm_RegisterCallback(&PwmDeepSleepCb);
    Cy_SysPm_RegisterCallback(&PwmEnterLpCb);
    Cy_SysPm_RegisterCallback(&PwmExitLpCb);
    Cy_SysPm_RegisterCallback(&ClkEnterLpCb);
    Cy_SysPm_RegisterCallback(&ClkExitLpCb);
    
    /* Initialize the TCPWM blocks */
    Cy_TCPWM_PWM_Init(PWM_R_HW, PWM_R_CNT_NUM, &PWM_R_config);
    Cy_TCPWM_PWM_Init(PWM_B_HW, PWM_B_CNT_NUM, &PWM_B_config);
    Cy_TCPWM_Counter_Init(Counter_HW, Counter_CNT_NUM, &Counter_config);
    
    /* Enable the TCPWM blocks */
    Cy_TCPWM_PWM_Enable(PWM_R_HW, PWM_R_CNT_NUM);
    Cy_TCPWM_TriggerStart(PWM_R_HW, PWM_R_CNT_MASK);
    Cy_TCPWM_PWM_Enable(PWM_B_HW, PWM_B_CNT_NUM);
    Cy_TCPWM_TriggerStart(PWM_B_HW, PWM_B_CNT_MASK);
    
    /* Start the RED PWM at 100% duty cycle */
    Cy_TCPWM_PWM_SetCompare0(PWM_R_HW, PWM_R_CNT_NUM, LED_FULL_BRIGHT);   
    
    for(;;)
    {
        switch (GetSwitchEvent())
        {
            case SWITCH_QUICK_PRESS:
                        
                /* Check if the device is in Low Power mode */
                if (Cy_SysPm_Cm0IsLowPower())
                {
                    /* Switch to normal active mode */
                    Cy_SysPm_ExitLpMode();
                }
                else
                {
                    /* Switch to low power active mode */
                    Cy_SysPm_EnterLpMode();
                }            
                break;
            
            case SWITCH_SHORT_PRESS:

                /* Go to sleep */
                Cy_SysPm_Sleep(CY_SYSPM_WAIT_FOR_EVENT);     
                
                break;
            case SWITCH_LONG_PRESS:
    
                /* Go to deep sleep */
                Cy_SysPm_DeepSleep(CY_SYSPM_WAIT_FOR_EVENT);

                break;

            default:
                break;
        }
        
    }
}

/* Constants to define LONG and SHORT presses on SW2 */
#define QUICK_PRESS_COUNT     9999u /* < 500 milliseconds */
#define SHORT_PRESS_COUNT   250000u /* ~ 1 second */
#define LONG_PRESS_COUNT    500000u /* > 2 seconds */

/*******************************************************************************
* Function Name: GetSwitchEvent
****************************************************************************//**
*
* Returns how the SW2 was pressed:
* - SWITCH_NO_EVENT: No press or very quick press
* - SWITCH_SHORT_PRESS: Short press was detected
* - SWITCH_LONG_PRESS: Long press was detected
*  
*******************************************************************************/
uint32_t GetSwitchEvent(void)
{
    uint32_t pressCount;
    uint32_t event = SWITCH_NO_EVENT;
    
    /* Check if SW2 is pressed */
    if (0u == Cy_GPIO_Read(SW2_PORT, SW2_NUM))
    {
        /* Check if the counter is not running */
        if (0u == ( Cy_TCPWM_Counter_GetStatus(Counter_HW, Counter_CNT_NUM) &
                   CY_TCPWM_COUNTER_STATUS_COUNTER_RUNNING))
        {
            /* Enable and trigger the counter */
            Cy_TCPWM_Counter_Enable(Counter_HW, Counter_CNT_NUM);
            Cy_TCPWM_TriggerStart(Counter_HW, Counter_CNT_MASK);
            
            /* Reset the switch counter */
            Cy_TCPWM_Counter_SetCounter(Counter_HW, Counter_CNT_NUM, 0u);
        }
    }
    else
    {
        pressCount = Cy_TCPWM_Counter_GetCounter(Counter_HW, Counter_CNT_NUM);
        
        /* Check if SW2 was pressed for a long time */
        if (pressCount > LONG_PRESS_COUNT)
        {
            event = SWITCH_LONG_PRESS;
        }
        /* Check if SW2 was pressed for a short time */
        else if (pressCount > SHORT_PRESS_COUNT)
        {
            event = SWITCH_SHORT_PRESS;
        }
        else if (pressCount > QUICK_PRESS_COUNT)
        {
            event = SWITCH_QUICK_PRESS;
        }
        
        /* Disable the switch counter */
        Cy_TCPWM_Counter_Disable(Counter_HW, Counter_CNT_NUM);
    }
    
    return event;
}

/*******************************************************************************
* Function Name: TCPWM_SleepCallback
****************************************************************************//**
*
* Sleep callback implementation. It reduces the LED brightness before going to
* sleep power mode. After waking up, it sets the LED brightness to full.
* Note that the LED brightness is controlled using the PWM block. There are 
* two LEDs in the design, but only one can be active, depending on the power 
* mode. 
* - Normal Mode : RED 
* - Low Power Mode : BLUE
*  
*******************************************************************************/
cy_en_syspm_status_t TCPWM_SleepCallback(
    cy_stc_syspm_callback_params_t *callbackParams)
{
    cy_en_syspm_status_t retVal = CY_SYSPM_FAIL;
    
    if (callbackParams != NULL)
    {       
        switch (callbackParams->mode)
        {            
            case CY_SYSPM_BEFORE_TRANSITION:
                
                /* Check if the device is in LowPower mode */
                if (Cy_SysPm_Cm0IsLowPower())
                {
                    /* Before going to sleep mode, decrease Blue LED brightness */
                    Cy_TCPWM_PWM_SetCompare0(PWM_B_HW, PWM_B_CNT_NUM, LED_LOW_BRIGHT);
                }
                else
                {
                    /* Before going to sleep mode, decrease Red LED brightness */
                    Cy_TCPWM_PWM_SetCompare0(PWM_R_HW, PWM_R_CNT_NUM, LED_LOW_BRIGHT);
                }
                    
                /* Disable switch Counter */
                Cy_TCPWM_Counter_Disable(Counter_HW, Counter_CNT_NUM);
                
                retVal = CY_SYSPM_SUCCESS;
                break;
                
            case CY_SYSPM_AFTER_TRANSITION:
                    
                /* Check if the device is in LowPower mode */
                if (Cy_SysPm_Cm0IsLowPower())
                {
                    /* After waking up, set the Blue LED to full brightness */
                    Cy_TCPWM_PWM_SetCompare0(PWM_B_HW, PWM_B_CNT_NUM, LED_FULL_BRIGHT);
                }
                else
                {
                    /* After waking up, set the Red LED to full brightness */
                    Cy_TCPWM_PWM_SetCompare0(PWM_R_HW, PWM_R_CNT_NUM, LED_FULL_BRIGHT);
                }
                
                /* Re-enable switch Counter */
                Cy_TCPWM_Counter_Enable(Counter_HW, Counter_CNT_NUM);
                Cy_TCPWM_TriggerStart(Counter_HW, Counter_CNT_MASK);
                
                /* Reset switch counter */
                Cy_TCPWM_Counter_SetCounter(Counter_HW, Counter_CNT_NUM, 0u);
                
                retVal = CY_SYSPM_SUCCESS;
                break;
                
            default: 
                /* Don't do anything in the other modes */
                retVal = CY_SYSPM_SUCCESS;
                break;
        }
    }
                    
    return retVal;
}

/*******************************************************************************
* Function Name: TCPWM_DeepSleepCallback
****************************************************************************//**
*
* Deep Sleep callback implementation. It turns the LED off before going to deep
* sleep power mode. After waking up, it sets the LED brightness to full.
* Note that the PWM block needs to be re-enabled after waking up, since the 
* clock feeding the PWM is disabled in deep sleep. There are two LEDs in the 
* design, but only one can be active, depending on the power mode. 
* - Normal Mode : RED 
* - Low Power Mode : BLUE
*  
*******************************************************************************/
cy_en_syspm_status_t TCPWM_DeepSleepCallback(
    cy_stc_syspm_callback_params_t *callbackParams)
{
    cy_en_syspm_status_t retVal = CY_SYSPM_FAIL;
    
    if (callbackParams != NULL)
    {
        switch (callbackParams->mode)
        {            
            case CY_SYSPM_BEFORE_TRANSITION:
                
                /* Before going to sleep mode, turn off the LEDs */
                Cy_TCPWM_PWM_Disable(PWM_R_HW, PWM_R_CNT_NUM);
                Cy_TCPWM_PWM_Disable(PWM_B_HW, PWM_B_CNT_NUM);
                
                /* Disable the switch counter */
                Cy_TCPWM_Counter_Disable(Counter_HW, Counter_CNT_NUM);
                
                retVal = CY_SYSPM_SUCCESS;
                break;
                
            case CY_SYSPM_AFTER_TRANSITION:
                                                            
                /* Re-enable PWM */
                Cy_TCPWM_PWM_Enable(PWM_R_HW, PWM_R_CNT_NUM);
                Cy_TCPWM_TriggerStart(PWM_R_HW, PWM_R_CNT_MASK);
                Cy_TCPWM_PWM_Enable(PWM_B_HW, PWM_B_CNT_NUM);
                Cy_TCPWM_TriggerStart(PWM_B_HW, PWM_B_CNT_MASK);               
                
                /* Check if the device is in LowPower mode */
                if (Cy_SysPm_Cm0IsLowPower())
                {
                    /* After waking up, set the Blue LED to full brightness */
                    Cy_TCPWM_PWM_SetCompare0(PWM_B_HW, PWM_B_CNT_NUM, LED_FULL_BRIGHT);
                }
                else
                {
                    /* After waking up, set the Red LED to full brightness */
                    Cy_TCPWM_PWM_SetCompare0(PWM_R_HW, PWM_R_CNT_NUM, LED_FULL_BRIGHT);                    
                }
                
                /* Re-enable the switch counter */
                Cy_TCPWM_Counter_Enable(Counter_HW, Counter_CNT_NUM);
                Cy_TCPWM_TriggerStart(Counter_HW, Counter_CNT_MASK);
                
                /* Reset switch counter */
                Cy_TCPWM_Counter_SetCounter(Counter_HW, Counter_CNT_NUM, 0u);
                
                retVal = CY_SYSPM_SUCCESS;
                break;
                
            default: 
                /* Don't do anything in the other modes */
                retVal = CY_SYSPM_SUCCESS;
                break;
        }
    }
                    
    return retVal;
}

/*******************************************************************************
* Function Name: TCPWM_EnterLowPowerCallback
****************************************************************************//**
*
* Enter Low Power Mode callback implementation. It changes the color of the LED
* (from RED to BLUE). Reconfigure the switch counter to compensate the change
* of the device clock frequency.
*  
*******************************************************************************/
cy_en_syspm_status_t TCPWM_EnterLowPowerCallback(cy_stc_syspm_callback_params_t *callbackParams)
{
    cy_en_syspm_status_t retVal = CY_SYSPM_FAIL;
    
    if (callbackParams != NULL)
    {
        switch (callbackParams->mode)
        {                            
            case CY_SYSPM_AFTER_TRANSITION:
                                             
                /* Set Blue LED to full brightness  */
                Cy_TCPWM_PWM_SetCompare0(PWM_B_HW, PWM_B_CNT_NUM, LED_FULL_BRIGHT);
                
                /* Turned off the Red LED */
                Cy_TCPWM_PWM_SetCompare0(PWM_R_HW, PWM_R_CNT_NUM, 0);
                
                /* Re-initalize the counter clock pre-scaler to compensate the drop in the FLL */
                Cy_TCPWM_Counter_Init(Counter_HW, Counter_CNT_NUM, &CounterLp_config);
                Cy_TCPWM_Counter_Enable(Counter_HW, Counter_CNT_NUM);
                Cy_TCPWM_TriggerStart(Counter_HW, Counter_CNT_MASK);
                
                
                retVal = CY_SYSPM_SUCCESS;
                break;
                
            default: 
                /* Don't do anything in the other modes */
                retVal = CY_SYSPM_SUCCESS;
                break;
        }
    }
                    
    return retVal;
}

/*******************************************************************************
* Function Name: TCPWM_ExitLowPowerCallback
****************************************************************************//**
*
* Exit Low Power Mode callback implementation. It changes the color of the LED
* (from BLUE to RED). Reconfigure the switch counter to compensate the change
* of the device clock frequency.
*  
*******************************************************************************/
cy_en_syspm_status_t TCPWM_ExitLowPowerCallback(cy_stc_syspm_callback_params_t *callbackParams)
{
    cy_en_syspm_status_t retVal = CY_SYSPM_FAIL;
    
    if (callbackParams != NULL)
    {
        switch (callbackParams->mode)
        {                            
            case CY_SYSPM_AFTER_TRANSITION:
                                             
                /* Set Red LED to full brightness  */
                Cy_TCPWM_PWM_SetCompare0(PWM_R_HW, PWM_R_CNT_NUM, LED_FULL_BRIGHT);
                
                /* Turned off the Blue LED */
                Cy_TCPWM_PWM_SetCompare0(PWM_B_HW, PWM_B_CNT_NUM, 0);
                
                /* Re-initalize the counter clock pre-scaler to the original value */
                Cy_TCPWM_Counter_Init(Counter_HW, Counter_CNT_NUM, &Counter_config);
                Cy_TCPWM_Counter_Enable(Counter_HW, Counter_CNT_NUM);
                Cy_TCPWM_TriggerStart(Counter_HW, Counter_CNT_MASK);
                
                retVal = CY_SYSPM_SUCCESS;
                break;
                
            default: 
                /* Don't do anything in the other modes */
                retVal = CY_SYSPM_SUCCESS;
                break;
        }
    }
                    
    return retVal;
}

/*******************************************************************************
* Function Name: Clock_EnterLowPowerCallback
****************************************************************************//**
*
* Enter Low Power Mode callback implementation. It reduces the FLL frequency by 
* half and reduce the internal LDO voltage to 0.9 Volts.
*  
*******************************************************************************/
cy_en_syspm_status_t Clock_EnterLowPowerCallback(cy_stc_syspm_callback_params_t *callbackParams)
{
    cy_en_syspm_status_t retVal = CY_SYSPM_FAIL;
    
    if (callbackParams != NULL)
    {
        switch (callbackParams->mode)
        {                            
            case CY_SYSPM_BEFORE_TRANSITION:
                                             
                /* Disable the FLL */
                Cy_SysClk_FllDisable();
                
                /* Reconfigure the FLL to be half of the original frequency */
                Cy_SysClk_FllConfigure(CYDEV_CLK_IMO__HZ, CYDEV_CLK_FLL__HZ/2, CY_SYSCLK_FLLPLL_OUTPUT_AUTO);
                
                /* Re-enable the FLL */
                Cy_SysClk_FllEnable(FLL_CLOCK_TIMEOUT);
                
                /* Drop-the internal LDO voltage to 0.9V */
                Cy_SysPm_LdoSetVoltage(CY_SYSPM_LDO_VOLTAGE_0_9V);
                
                retVal = CY_SYSPM_SUCCESS;
                break;
                
            default: 
                /* Don't do anything in the other modes */
                retVal = CY_SYSPM_SUCCESS;
                break;
        }
    }
                    
    return retVal;
}

/*******************************************************************************
* Function Name: Clock_ExitLowPowerCallback
****************************************************************************//**
*
* Exit Low Power Mode callback implementation. It sets the original FLL 
* frequency for the device and LDO voltage.
*  
*******************************************************************************/
cy_en_syspm_status_t Clock_ExitLowPowerCallback(cy_stc_syspm_callback_params_t *callbackParams)
{
    cy_en_syspm_status_t retVal = CY_SYSPM_FAIL;
    
    if (callbackParams != NULL)
    {
        switch (callbackParams->mode)
        {                            
            case CY_SYSPM_BEFORE_TRANSITION:
                             
                /* Increase the LDO Voltage to 1.1 V */
                Cy_SysPm_LdoSetVoltage(CY_SYSPM_LDO_VOLTAGE_1_1V);
            
                /* Disable the FLL */
                Cy_SysClk_FllDisable();
                
                /* Reconfigure the FLL to be the original frequency */
                Cy_SysClk_FllConfigure(CYDEV_CLK_IMO__HZ, CYDEV_CLK_FLL__HZ, CY_SYSCLK_FLLPLL_OUTPUT_AUTO);
                
                /* Re-enable the FLL */
                Cy_SysClk_FllEnable(FLL_CLOCK_TIMEOUT);
                
                retVal = CY_SYSPM_SUCCESS;
                break;
                
            default: 
                /* Don't do anything in the other modes */
                retVal = CY_SYSPM_SUCCESS;
                break;
        }
    }
                    
    return retVal;
}

/* [] END OF FILE */
