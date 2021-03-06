/******************************************************************************
* File Name: main_cm4.c
*
* Version: 1.20
*
* Description: This example shows how to use the MCWDT counters in free running 
* mode to measure the time between two presses of switch.
*
* Related Document: CE220498_PSoC6MCU_FreeRunningMultiCounterWatchdogTimer.pdf
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

#include "project.h"
#include "stdio.h"

/*******************************************************************************
*        Function Prototypes
*******************************************************************************/
void handle_error(void);
static uint32_t read_switch_status(void);

/*******************************************************************************
*        Constants
*******************************************************************************/
#define LED_ON               (0u)      /* Value to switch LED ON  */
#define LED_OFF              (!LED_ON) /* Value to switch LED OFF */

/* Switch debounce delay in milliseconds. */
#define SWITCH_DEBOUNCE_UNIT (1u)

/* Number of debounce units to count before consider that switch is pressed. */
#define SWITCH_PUSH          (80u)

/*******************************************************************************
*        Global variables
*******************************************************************************/


/*******************************************************************************
* Function Name: handle_error
********************************************************************************
* Summary:
* This function processes unrecoverable errors such as UART component 
* initialization error. In case of such error the system will Turn on ERROR_LED 
* and stay in an infinite loop of this function.
*
* Parameters:
*  None
*
* Return:
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
* Function Name: read_switch_status
********************************************************************************
* Summary:
*  Reads and returns the current status of the switch.
*
* Parameters:
*  None
*
* Return:
*  Returns non-zero value if switch is pressed and zero otherwise.
*
*******************************************************************************/
uint32_t read_switch_status(void)
{
    uint32_t helddown;
    uint32_t sw_status;

    sw_status = 0u; /* Switch is not active */
    helddown = 0u;  /* Reset debounce counter */

    /* Wait for debounce period before determining that the switch is pressed */
    while (0UL == Cy_GPIO_Read(SW2_0_PORT, SW2_0_NUM))
    {
        /* Count debounce period */
        Cy_SysLib_Delay(SWITCH_DEBOUNCE_UNIT);
        ++helddown;

        if (helddown > SWITCH_PUSH)
        {
            sw_status = 1u; /* Switch is pressed */
            break;
        }
    }

    return (sw_status);
}

/*******************************************************************************
* Function Name: main
********************************************************************************
*
* Summary:
*  This function initializes the Multi-Counter Watchdog component, interrupts 
*  and goes into Deep Sleep. MCWDT_0_Interrupt or MCWDT_1_Interrupt wakes CM4 
*  from Deep Sleep and executes corresponding ISRs.
*
* Parameters:
*  None
*
* Return:
*  None
*
* Side Effects:
*  None
*
*******************************************************************************/
int main(void)
{    
    /* UART initialization status */
    cy_en_scb_uart_status_t uart_init_status;
    cy_en_mcwdt_status_t mcwdt_init_status;
    
    /* Key press event count value */
	uint32_t event1_cnt, event2_cnt;
    uint32_t counter1_value, counter0_value;
    
    /* The time between two presses of switch */
    uint32_t timegap;
    
    /* Initialize UART operation */
    uart_init_status = Cy_SCB_UART_Init(UART_HW, &UART_config, &UART_context);
    if(uart_init_status!=CY_SCB_UART_SUCCESS)
    {
        handle_error();
    }
	
    Cy_SCB_UART_Enable(UART_HW);
	Cy_SCB_UART_PutString(UART_HW, "\r\n\r\nUART initialization complete\r\n");      
    
    /* Initialize the MCWDT_0 */
    mcwdt_init_status = Cy_MCWDT_Init(MCWDT_0_HW, &MCWDT_0_config);
    if(mcwdt_init_status!=CY_MCWDT_SUCCESS)
    {
        handle_error();
    }
    
    /* Enable the MCWDT_0 counters */
    Cy_MCWDT_Enable(MCWDT_0_HW, CY_MCWDT_CTR0|CY_MCWDT_CTR1|CY_MCWDT_CTR2, \
                    MCWDT_0_TWO_LF_CLK_CYCLES_DELAY);
   
    /* Enable the global interrupts. */
    __enable_irq();

    /* Initialize event count value */
    event1_cnt = 0;
    event2_cnt = 0;
    for(;;)
    {
        /* Check if switch is pressed */
        if (0UL != read_switch_status())
        {
            /* Consider previous key press as 1st key press event */
            event1_cnt = event2_cnt;
            
            /* Consider current key press as 2nd key press event */
            /* Get live counter value from MCWDT_0 */
            /* Note that MCWDT_0 Counter1 is cascaded from MCWDT_0 Counter0 */
            counter0_value = Cy_MCWDT_GetCount(MCWDT_0_HW, CY_MCWDT_COUNTER0);
            counter1_value = Cy_MCWDT_GetCount(MCWDT_0_HW, CY_MCWDT_COUNTER1);
            event2_cnt = ((counter1_value<<16) | (counter0_value<<0));            
            
            /* Calculate the time between two presses of switch and print on the terminal */
            /* MCWDT_0 Counter0 and Counter1 are clocked by LFClk = 32.768kHz +/- 0.015% */
            /* Update code once CDT#282377 fix is available to read cascaded count value */
            if(event2_cnt>event1_cnt)
            {
                timegap = (event2_cnt - event1_cnt)/32768;
            }
            else
            {
                timegap = 0;
            }
            printf("\r\nThe time between two presses of switch = %ds\r\n", (unsigned int)timegap);
        }
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

/* [] END OF FILE */

