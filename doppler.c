
#include <lpc17xx.h>
#define PRESCALE (25-1)

char sevenseg[] = {0x3f,0x6,0x5b,0x4f,0x66,0x6d,0x7d,0x7,0x7f,0x6f};

int round_val(float num){
		float val = num - abs(num);
		if(val <= 0.5)
			return (int)num;
		else
			return (int)num + 1;
}



void initTimer0(void);
void startTimer0(void);
unsigned int stopTimer0(void);
void delayUS(unsigned int microseconds);
void delayMS(unsigned int milliseconds);


void initTimer0(void) //PCLK must be = 25Mhz!
{
	LPC_TIM0->CTCR = 0x0;
	LPC_TIM0->PR = PRESCALE; //Increment TC at every 24999+1 clock cycles
	LPC_TIM0->TCR = 0x02; //Reset Timer
}

void startTimer0(void)
{
	LPC_TIM0->TCR = 0x02; //Reset Timer
	LPC_TIM0->TCR = 0x01; //Enable timer
}

unsigned int stopTimer0(void)
{
	LPC_TIM0->TCR = 0x00; //Disable timer
	return LPC_TIM0->TC;
}

void delayUS(unsigned int microseconds) //Using Timer0
{
	LPC_TIM0->TCR = 0x02; //Reset Timer
	LPC_TIM0->TCR = 0x01; //Enable timer
	while(LPC_TIM0->TC < microseconds); //wait until timer counter reaches the desired delay
	LPC_TIM0->TCR = 0x00; //Disable timer
}

void delayMS(unsigned int milliseconds) //Using Timer0
{
	delayUS(milliseconds * 1000);
}

void delay_trigger(){
	LPC_GPIO0->FIOSET = (0x1<<15);
	delayUS(10);
	LPC_GPIO0->FIOCLR = (0x1<<15);
}

int echo_monitor(){
	float pulse_time = 0,distance=0;
	while((LPC_GPIO0->FIOPIN & (0x1<<16)) == 0x0);	//Wait till echo is low
	startTimer0();															//Initialize the echo timer
	while((LPC_GPIO0->FIOPIN & (0x1<<16)) == 0x1<<16);	//Wait till echo is high
	pulse_time = stopTimer0();										//Get count of echo timer
  distance = (0.0343*pulse_time)/2;
	return round_val(distance);
}

void display(int number,int displayTimeSeconds){
	int j,i,timeUS = 1000000*displayTimeSeconds,temp;
	startTimer0();
	while(LPC_TIM0->TC <timeUS){
		temp = number;
		for(j=0;j<4;j++){
				LPC_GPIO1->FIOPIN = j<<23;
				LPC_GPIO0->FIOPIN = sevenseg[(temp%10)]<<4;
			  for(i=0;i<1000;i++);
				temp /= 10;
			}
	}
	stopTimer0();
	LPC_GPIO1->FIOPIN = 00<<23;
	LPC_GPIO0->FIOPIN = 0xF9<<4; //Display a dot
}


int main(void){
	int distance;
	SystemInit(); //CLK = 12 MHz and PCLK = 25MHz
	SystemCoreClockUpdate();
		
	
	//Trigger, Echo, 7 segment
	LPC_PINCON->PINSEL0 = 0x0; //SET TO GPIO
	LPC_PINCON->PINSEL1 = 0x0;	//SET TO GPIO (For the echo pin) 
	LPC_GPIO0->FIODIR = 0x0FFF0;
	
	//Decoder
	LPC_PINCON->PINSEL3 = 0x0;		//Decoder	GPIO config 
	LPC_GPIO1->FIODIR = 0xF<<23;	//Decoder output config
	
	//Timer setup
	initTimer0();
	
	
	while(1){
		delay_trigger();
		distance = echo_monitor();
		printf("%d\n",distance);
		display(distance,1);
	}
}
