/**
  Generated main.c file from MPLAB Code Configurator

  @Company
    Microchip Technology Inc.

  @File Name
    main.c

  @Summary
    This is the generated main.c using PIC24 / dsPIC33 / PIC32MM MCUs.

  @Description
    This source file provides main entry point for system initialization and application code development.
    Generation Information :
        Product Revision  :  PIC24 / dsPIC33 / PIC32MM MCUs - 1.169.0
        Device            :  PIC24FJ64GA002
    The generated drivers are tested against the following:
        Compiler          :  XC16 v1.50
        MPLAB 	          :  MPLAB X v5.40
*/

/*
    (c) 2020 Microchip Technology Inc. and its subsidiaries. You may use this
    software and any derivatives exclusively with Microchip products.

    THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER
    EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED
    WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A
    PARTICULAR PURPOSE, OR ITS INTERACTION WITH MICROCHIP PRODUCTS, COMBINATION
    WITH ANY OTHER PRODUCTS, OR USE IN ANY APPLICATION.

    IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE,
    INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND
    WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS
    BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO THE
    FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN
    ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
    THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.

    MICROCHIP PROVIDES THIS SOFTWARE CONDITIONALLY UPON YOUR ACCEPTANCE OF THESE
    TERMS.
*/

/**
  Section: Included Files
*/
#include <xc.h>
#include "mcc_generated_files/system.h"
#include "mcc_generated_files/pin_manager.h"
#include "mcc_generated_files/interrupt_manager.h"
#include "mcc_generated_files/ic1.h"
#include "mcc_generated_files/ic2.h"
#include "mcc_generated_files/oc1.h"
#include "mcc_generated_files/adc1.h"
#include "mcc_generated_files/i2c1.h"

// this emulates the slave device memory where data written to slave
// is placed and data read from slave is taken
static uint8_t EMULATE_EEPROM_Memory[8] =
        {
           0, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        };


/*
                         Main application
 */
#define CTL_KP      50.0
//#define CTL_KI      0.2
#define MIN_PWM     500
#define INTEG_MAX   1000.0
#define DEADBAND    5
#define HYST        ((stopped)?0:0)

int main(void)
{
#define motor_pos_adc   pos_adc[1]
#define default_pos_adc pos_adc[0]
    static uint16_t pos_adc[2];
    static float error, pos_cmd = 0, des_pos = 0;
#ifdef CTL_KI
    static float error_cum = 0;
#endif
    static int8_t direction, last_direction = 0, stopped = 1;
    static uint16_t effort;
        
    // initialize the device
    SYSTEM_Initialize();
        
    INTERRUPT_GlobalEnable();
    
    while (1)
    {
        pos_cmd = ((int8_t)EMULATE_EEPROM_Memory[0]) * (512.0 / 128.0);
        
        if (IFS0bits.AD1IF && ADC1_ConversionResultBufferGet(pos_adc) == 2) {
            IFS0bits.AD1IF = false;
            
            des_pos = ((float)pos_adc[0] + pos_cmd);
            if (des_pos < 50) {
                des_pos = 50;
            }
            if (des_pos > (0x3FF - 50))
            {
                des_pos = (0x3FF - 50);
            }
            error = (float)pos_adc[1] - des_pos;
            direction = 0;
            effort = 0;

            if (error < -(DEADBAND+HYST))
            {
                direction = 1;
                effort = (uint16_t)(-CTL_KP * error);
#ifdef CTL_KI
                error_cum += CTL_KI * (-error);
#endif
                stopped = 0;
            }
            else if (error > (DEADBAND+HYST))
            {
                direction = -1;
                effort = (uint16_t)(CTL_KP * error);
#ifdef CTL_KI
                error_cum += CTL_KI * (error);
#endif
                stopped = 0;
            }
            else 
            {
                stopped = 1;
            }

#ifdef CTL_KI
            if (error_cum > INTEG_MAX) {
                error_cum = INTEG_MAX;
            }
#endif            
            /* Output controller values */
            if (direction > 0)
            {
                MOT_IN1_SetHigh();
                MOT_IN2_SetLow();
            }
            else if (direction < 0)
            {
                MOT_IN1_SetLow();
                MOT_IN2_SetHigh();
            }
            else
            {
                // brake
                MOT_IN1_SetLow();
                MOT_IN2_SetLow();            
            }

            if (last_direction != direction)
            {
#ifdef CTL_KI
                error_cum = 0;
#endif
            }
            last_direction = direction;

#ifdef CTL_KI
            effort += (uint16_t)error_cum;
#endif
            if (effort >= 2000) {
                effort = 1999;
            }

            if (effort > MIN_PWM)
            {
                if (LIMIT_1_GetValue() == 0 && direction > 0) // CW Limit
                {
                    OC1_SecondaryValueSet(0);
                }
                else if (LIMIT_2_GetValue() == 0 && direction < 0) // CCW Limit
                {
                    OC1_SecondaryValueSet(0);
                }
                else
                    OC1_SecondaryValueSet(effort); // PWM value from 0 to 2000
            }
            else
            {
                OC1_SecondaryValueSet(0);
            }
        }
    }

    return 1;
}

static uint8_t i2c1_slaveWriteData = 0xAA;

bool I2C1_StatusCallback(I2C1_SLAVE_DRIVER_STATUS status)
{
    static uint16_t address, addrByteCount;
    static bool     addressState = true;

    switch (status)
    {
        case I2C1_SLAVE_TRANSMIT_REQUEST_DETECTED:

            // set up the slave driver buffer transmit pointer
            I2C1_ReadPointerSet(&EMULATE_EEPROM_Memory[address]);
            address++;

            break;

        case I2C1_SLAVE_RECEIVE_REQUEST_DETECTED:

            addrByteCount = 0;
            addressState = true;

            // set up the slave driver buffer receive pointer
            I2C1_WritePointerSet(&i2c1_slaveWriteData);

            break;

        case I2C1_SLAVE_RECEIVED_DATA_DETECTED:

            if (addressState == true)
            {
                // get the address of the memory being written
                if (addrByteCount == 0)
                {
                    address = (i2c1_slaveWriteData << 8) & 0xFF00;
                    addrByteCount++;
                }
                else if (addrByteCount == 1)
                {
                    address = address | i2c1_slaveWriteData;
                    addrByteCount = 0;
                    addressState = false;
                }
            }
            else // if (addressState == false)
            {
                // set the memory with the received data
                EMULATE_EEPROM_Memory[address >> 8] = address & 0x00FF;
            }

            break;

        case I2C1_SLAVE_10BIT_RECEIVE_REQUEST_DETECTED:

            // do something here when 10-bit address is detected

            // 10-bit address is detected

            break;

        default:
            break;

    }

    return true;
}

/**
 End of File
*/

