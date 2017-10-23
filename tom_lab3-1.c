/* Sample code for Lab 3.1. This code controls the ultrasonic ranger. */
#include <c8051_SDCC.h>
#include <stdio.h>
#include <stdlib.h>
//-----------------------------------------------------------------------------
// Function Prototypes
//-----------------------------------------------------------------------------
void Port_Init(void);
void PCA_Init (void);
void XBR0_Init();
void Set_Pulsewidth(void);
void PCA_ISR ( void ) __interrupt 9;

//-----------------------------------------------------------------------------
// Global Variables
//-----------------------------------------------------------------------------
unsigned int NEUTRAL_PW = 2150; //1.5 ms
unsigned int BACKWARD_PW_MAX = 1577; //1.1 ms
unsigned int FORWARD_PW_MAX = 2724; //1.9 ms
unsigned int PW = 0;
unsigned char PW_INCREMENT = 10;

//Period: 20 ms. PW: init 1.5 ms.

//-----------------------------------------------------------------------------
// Main Function
//-----------------------------------------------------------------------------
void main(void)
{
    // initialize board
    Sys_Init();
    putchar(' '); //the quotes in this line may not format correctly
    Port_Init();
    XBR0_Init();
    PCA_Init();

    //print beginning message
    printf("Embedded Control sspeeeEEEEEEEEEEEEEEEEEEEEd\r\n");
    // set the PCA output to a neutral setting
    PW = NEUTRAL_PW;
    while(1)
        Set_Pulsewidth();
}

//-----------------------------------------------------------------------------
// Port_Init
//-----------------------------------------------------------------------------
//
// Set up ports for input and output
//
void Port_Init()
{
    //P1MDOUT = ________;  //set output pin for CEX0 or CEX2 in push-pull mode
    P1MDOUT |= 0x03; //CEX2 in push-pull for ultrasonic ranger
}

//-----------------------------------------------------------------------------
// XBR0_Init
//-----------------------------------------------------------------------------
//
// Set up the crossbar
//
void XBR0_Init()
{
    XBR0 = 0X27;
    //As written in lab. Has UART, SPI, SMBus, and CEX channels as in worksheet.
}

//-----------------------------------------------------------------------------
// PCA_Init
//-----------------------------------------------------------------------------
//
// Set up Programmable Counter Array
//
void PCA_Init(void)
{
    // reference to the sample code in Example 4.5 -Pulse Width Modulation 
    // implemented using the PCA (Programmable Counter Array), p. 50 in Lab Manual.

    //Period: 20ms
    //PW min: 1.1 ms
    //PW max: 1.9 ms
    //PW ini: 1.5 ms

    PCA0MD = 0x81; //suspends PCA when system is idle (bit 7 = 1),
    // uses SYSCLK/12 (bits 1-3 = 0), and enables inerrupt (bit 0 = 1)
    PCA0CPM2 = 0xC2;
    //16bit (bit 7 = 1), compare mode (bit 6 = 1), enables PWM (bit 1 = 1)
    EA = 1; //enable interrupts
    EIE1 |= 0x08; //enable PCA0 interrupt

    //PCA_start = 28671; //start point so period is 20 ms
    PCA0CN |= ~0x40; //enable timer
}

//-----------------------------------------------------------------------------
// PCA_ISR
//-----------------------------------------------------------------------------
//
// Interrupt Service Routine for Programmable Counter Array Overflow Interrupt
//
void PCA_ISR ( void ) __interrupt 9
{
    // reference to the sample code in Example 4.5 -Pulse Width Modulation 
    // implemented using the PCA (Programmable Counter Array), p. 50 in Lab Manual.

    if (CF)
    {
        CF = 0; //reset interrupt flag
        PCA0H = 0x6F;
        PCA0L = 0xFF;
        //28671 split in high and low bits - makes 20 ms period.
    }
    PCA0CN &= 0x40; //clear CF bit, clear CCF bits 0-4.
    PCA0CN |= ~0x40; //enable timer
}

void Set_Pulsewidth()
{
    char input;
    //wait for a key to be pressed
    input = getchar();
    if(input == 'f')  // single character input to increase the pulsewidth
       // f for forward
    {
        /*
        if (PW > FORWARD_PW_MAX)
        {
            PW = FORWARD_PW_MAX; //Don't go beyond max
        }
        else
        {
            PW += PW_INCREMENT;
        }
        */
        if (PW < FORWARD_PW_MAX)
        {
            PW += PW_INCREMENT;
        }
    }
    else if(input == 's')  // single character input to decrease the pulsewidth
        //b for backward NOPE JK
        //s for 'slower'
    {
        /*
        if (PW < BACKWARD_PW_MAX)
        {
            PW = BACKWARD_PW_MAX;
        }
        else
        {
            PW -= PW_INCREMENT;
        }
        */
        if (PW > BACKWARD_PW_MAX)
        {
            PW -= PW_INCREMENT;
        }
    }
    printf("PW: %u\r\n", PW);
    PCA0CP1 = 0xFFFF - PW;

}
