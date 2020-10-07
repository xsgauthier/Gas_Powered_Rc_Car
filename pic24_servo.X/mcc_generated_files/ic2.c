/**
  IC2 Generated Driver API Source File

  @Company
    Microchip Technology Inc.

  @File Name
    ic2.c

  @Summary
    This is the generated source file for the IC2 driver using PIC24 / dsPIC33 / PIC32MM MCUs

  @Description
    This source file provides APIs for driver for IC2.
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

#include "ic2.h"

/**
  IC Mode.

  @Summary
    Defines the IC Mode.

  @Description
    This data type defines the IC Mode of operation.

*/

static uint16_t         gIC2Mode;

/**
  Section: Driver Interface
*/

void IC2_Initialize (void)
{
    // ICSIDL disabled; ICM Simple Capture mode: Falling; ICTMR TMR2; ICI Every; 
    IC2CON = 0x82;
    
    gIC2Mode = IC2CONbits.ICM;
    
    IFS0bits.IC2IF = false;
    IEC0bits.IC2IE = true;
}


void __attribute__ ((weak)) IC2_CallBack(void)
{
    // Add your custom callback code here
}

void __attribute__ ( ( interrupt, no_auto_psv ) ) _ISR _IC2Interrupt( void )
{	
    if(IFS0bits.IC2IF)
    {
		// IC2 callback function 
		IC2_CallBack();
        IFS0bits.IC2IF = 0;
    }
}
void IC2_Start( void )
{
    IC2CONbits.ICM = gIC2Mode;
}

void IC2_Stop( void )
{
    IC2CONbits.ICM = 0;
}

uint16_t IC2_CaptureDataRead( void )
{
    return(IC2BUF);
}

bool IC2_HasCaptureBufferOverflowed( void )
{
    return( IC2CONbits.ICOV );
}


bool IC2_IsCaptureBufferEmpty( void )
{
    return( ! IC2CONbits.ICBNE );
}

/**
 End of File
*/
