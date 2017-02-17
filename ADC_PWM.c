#include "TM4C123GE6PM.h"
#include "LCD.h"
void setup_ADC(void);
void setup_ADC2(void);
void setup_PWMA6(void);
void setup_timer(void);

int hardtemp = 0;
int count = 0;
int ADCin = 0;
int ADC = 0;
int Ypos = 0;
int Xpos = 0;

unsigned char on[] = {'P','Z',' '};
unsigned char on_len = 3;
unsigned int let[] = {80,100,90};
unsigned char let_size = 3;

void setup_ADC(void){ //setup of AIN5 GPIOD(2)
	SYSCTL->RCGCTIMER  |= 0x1;				//clock for timer 0
	SYSCTL->RCGCADC    |= 0x1;
	SYSCTL->RCGCGPIO   |= 0x8;
	GPIOD->LOCK 	  	= 0x4c4f434b;
	GPIOD->AFSEL 		= 0x4;		//Alternate Function
	GPIOD->DEN 			= 0x0;		//digital DISABLE
	GPIOD->AMSEL 		= 0x4;		//analog mode
	GPIOD->ADCCTL		= 0x4;
	ADC0->ACTSS 		= 0x0;		//Sample Sequencer DISABLED for config
	ADC0->EMUX 			= 0x5;		//use the timer to trigger a sample.	
	ADC0->SSMUX0 		= 0x5;		//Analog input 5 (PD2)
	ADC0->SSCTL0 		= 0x6;		//end sample 0x6
	ADC0->IM 				= 0x1;		//interupt mask
	ADC0->ACTSS 		= 0x1;		//Sample Sequencer DISABLED for config
	ADC0->ISC 	 		= 0x1;
	//------------------------------------------------
	TIMER0->CTL 		= 0x0; 		//using timer 1A
	TIMER0->CFG 		= 0x0;				
	TIMER0->TAMR 		= 0x2;				
	TIMER0->TAILR 	= 40000; 	//set to 1mS 10 makes 10mS
	TIMER0->TAMATCHR= 0x0;				
	TIMER0->CTL 		= 0x21;		//start timer1
	//------------------------------------------------
	NVIC->ISER[0] 			= 0x204000;
}

void ADC0Seq0_Handler (void){
	ADC0->ISC 			 |= 0x8;
	hardtemp = ADC0->SSFIFO0;	
	if(count==0){
		ADCin = hardtemp;	//reset ADCin when count is cleared.
		ADC0->ISC     	|= 0x8;
		count++;	
	}
	else if(count<10){
		ADCin+=hardtemp;	//get adc value and add to previous value
		ADC0->ISC |= 0x8;
		count++;			//increment count
		return;				//return
	}
	else {					//count is = to 250
		ADC	=	ADCin/count;//Average all the samples					
		count	=		0;	//Reset data for this handler.		
		return;																
	}
}

void setup_PWMA6(void){
	SYSCTL->RCGCPWM |= 0x2;
	SYSCTL->RCGC0 	|= 0x00100000; 	//Enable PWM Clock
	SYSCTL->RCGC2 	|= 0x1;			//clock to GPIO A
	GPIOA->AFSEL  	|= 0x40;
	GPIOA->PCTL   	|= 0x5000000;	//Select alternate function
	GPIOA->DEN    	|= 0x40;
	SYSCTL->RCC  		|= 0x1C0000;	//PWM0 DIV enable & Divisor
	PWM1->_1_CTL		= 0x00000000;		
	PWM1->_1_GENA 	= 0x0000008C;	//Drive High and bring Low when COMP match
	PWM1->_1_LOAD 	= 0x00004EC0; 	//20161 ticks
	PWM1->_1_CMPA 	= 0x00002760; 	//25% duty Cycle
	PWM1->_1_CTL  	= 0x00000001;	//Start timer in PWM1
	PWM1->ENABLE  	= 0x00000004; 	//Enable PWM2 output
}


void setup_timer(void){ //timer 1 set to run at 100mS or 1/10 sec
	SYSCTL->RCGCTIMER|= 0x2;
	TIMER1->CTL 			= 0x0; 		//using timer 1
	TIMER1->CFG 			= 0x0;				
	TIMER1->TAMR 			= 0x22;				
	TIMER1->TAILR 		= 0xA0900; //set to 100mS 250 makes 250mS
	TIMER1->TAMATCHR	= 0x0;				
	TIMER1->CTL 			= 0x1;		//start timer1
	TIMER1->IMR 			= 0x10;
	NVIC->ISER[0] 			|= 0x204000;
}

void Timer1A_Handler(void){ //100mS

	int p;
	int Sx, Fx, Dx;
	int Sy, Fy, Dy;
	TIMER1->ICR 	=0x10; 			//clear interrupt
	Xpos = ADC/12;
	if( Xpos > 20 )
	{
		//x position variables.
		Sx = Xpos;
		Fx = Xpos+1;
		Dx = abs(Fx - Sx);
		//y position variables.
		Sy = Ypos;
		Fy = Ypos+1;
		Dy = abs(6);	//Width of the Plot Line
	
		//set column
		writeCmd(0x2A);			//Write to X axis
		writeDat2(Sx); 			//start x
		writeDat2(Fx);			//end x
		//set page
		writeCmd(0x2B); 		//write to Y axis
		writeDat2(Sy);  		//start y
		writeDat2(Fy);  		//end y
	
		writeCmd(0x2C);
		for(p = 0; p < Dx*Dy; p++){
			writeDat2(red);
		}
			Ypos++;
		if (Ypos == 320){
			Ypos = 0;
			flash_screen(white);
			lines(black);
			//
			writeLetter(7,40,5); //Write space
			writeLetter(6,40,19); //Write a 5
			//
			writeLetter(0,90,5); //Write a 1
			writeLetter(7,90,19); //Write a 0
			
			//
			writeLetter(0,135,5); //Write a 1
			writeLetter(6,135,19); //Write a 5
			//
			writeLetter(8,185,5); //Write a 2
			writeLetter(7,185,19); //Write a 0
			//
			writeLetter(10,5,380); //Write a T
			writeLetter(11,5,394); //Write a I
			writeLetter(12,5,408); //Write a M
			writeLetter(13,5,422); //Write a E
			//
			
			writeSLetter(80,5,20); //P
			writeSLetter(83,15,20); //S
			writeSLetter(73,25,20); //I
		}
	}
}