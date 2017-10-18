/* Sample code for Lab 3.1. This program can be used to test the steering servo */
#include <c8051_SDCC.h>
#include <stdio.h>
#include <stdlib.h>
//-----------------------------------------------------------------------------
// 8051 Initialization Functions
//-----------------------------------------------------------------------------
void Port_Init(void);
void PCA_Init (void);
void XBR0_Init();
void Steering_Servo(void);
void Run_Adjustments(void);
void PCA_ISR ( void ) __interrupt 9;
unsigned int PW_CENTER = 0;			//Will eventually be set to a different value
unsigned int PW_LEFT = 1659;		//Initialized to .9 ms
unsigned int PW_RIGHT = 3871;		//Initialized to 2.1 ms
unsigned int SERVO_PW = 2765;		//Initialized to 1.5 ms
//-----------------------------------------------------------------------------
// Global Variables
//-----------------------------------------------------------------------------
char input;
unsigned char breaker;
//-----------------------------------------------------------------------------
// Main Function
//-----------------------------------------------------------------------------
void main(void)
{
	//initialize board
	Sys_Init();
	putchar(' '); //the quotes in this line may not format correctly
	Port_Init();
	XBR0_Init();
	PCA_Init();
	//print beginning message
	printf("Embedded Control Steering Calibration\r\n");
	//set initial value for steering (set to center)
	SERVO_PW = PW_CENTER;
	while(1)
	{
		Run_Adjustments();
	}
}
//-----------------------------------------------------------------------------
// Port_Init
//-----------------------------------------------------------------------------
//
// Set up ports for input and output
//
void Port_Init()
{
	P1MDOUT |= 0x01;		//set output pin for CEX0 in push-pull mode (P1.0)
}
//-----------------------------------------------------------------------------
// XBR0_Init
//-----------------------------------------------------------------------------
//
// Set up the crossbar
//
void XBR0_Init()
{
	XBR0 = 0x27 ;	//configure crossbar with UART, SPI, SMBus, and CEX channels as
					//in worksheet
}


//-----------------------------------------------------------------------------
// PCA_Init
//-----------------------------------------------------------------------------
//
// Set up Programmable Counter Array
//
void PCA_Init(void)
{
	PCA0MD = 0x89;		//Enable CF Interrupt
	PCA0CPM0 = 0xC2;	//CCM0 in 16-bit compare mode
	PCA0CN = 0x40;		//Enables PCA counter
	EIE1 |= 0x08;		//Enable PCA interrupt
	EA = 1;				//Enable global interrupt	
}
//-----------------------------------------------------------------------------
// PCA_ISR
//-----------------------------------------------------------------------------
//
// Interrupt Service Routine for Programmable Counter Array Overflow Interrupt
//
void PCA_ISR(void) __interrupt 9
{
	if (CF)
	{
		CF = 0;			//Reset CF interrupt flag
		PCA0 = 28672;	//Starting value for a 20 ms pulse when using SYSCLK/12 and 16-bit timer
	}
	
	PCA0CN &= 0x40;		//Handle other PCA interrupt sources
}
//-----------------------------------------------------------------------------
// Steering_Servo
//-----------------------------------------------------------------------------
// Note: This only functions as intended if PW_LEFT and PW_RIGHT are initialized
// to values that are smaller and larger (respectively) than their final experimental values;
// i.e., SERVO_PW < PW_RIGHT is only useful if PW_RIGHT is much greater than the value it
// will eventually be set to by manual adjustment of SERVO_PW.
void Steering_Servo()
{
	char input;
	//wait for a key to be pressed
	input = getchar();
	if(input == 'r') //if 'r' is pressed by the user
	{
		if(SERVO_PW < PW_RIGHT)
			SERVO_PW = SERVO_PW + 10; //increase the steering pulsewidth by 10
	}
	else if(input == 'l') //if 'l' is pressed by the user
	{
		if(SERVO_PW > PW_LEFT)
			SERVO_PW = SERVO_PW - 10; //decrease the steering pulsewidth by 10
	}
	else if (input == 'q')
	{
		breaker = 1;
	}
	printf("SERVO_PW: %u\r\n", SERVO_PW);
	PCA0CPL0 = 0xFFFF - SERVO_PW;
	PCA0CPH0 = (0xFFFF - SERVO_PW) >> 8;
}
//-----------------------------------------------------------------------------
// Run_Adjustments
//-----------------------------------------------------------------------------
// Contains all terminal outputs and sets the values of PW_CENTER, PW_LEFT, PW_RIGHT
//
//
void Run_Adjustments(void)
{
	printf("Please adjust the steering with the l and r keys until steering is centered.\r\n");
	printf("When you are finished, press q to move on to the next objective.\r\n");
	breaker = 0;
	while (!breaker)
	{
		Steering_Servo();
	}

	breaker = 0;
	PW_CENTER = SERVO_PW;
	printf("Please adjust steering to the leftmost position that does not strain the servo.\r\n");
	while (!breaker)
	{
		Steering_Servo();
	}

	breaker = 0;
	PW_LEFT = SERVO_PW;
	printf("Please adjust steering to the rightmost position that does not strain the servo.\r\n");
	while (!breaker)
	{
		Steering_Servo();
	}

	breaker = 0;
	PW_RIGHT = SERVO_PW;
	printf("You are now free to adjust steering within the determined constraints.\r\n");
	while (1)
	{
		Steering_Servo();
	}
}