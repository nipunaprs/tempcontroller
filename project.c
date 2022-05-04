#define ADC_BASE               0xFF204000
//#define LED_BASE	           0xFF200000
#define SW_BASE               0xFF200040
#define JP1_BASE              0xFF200060
#define KEY_BASE              0xFF200050
#define HEX3_HEX0_BASE        0xFF200020
#define HEX5_HEX4_BASE        0xFF200030
#define TIMER_A9_BASE 0xFFFEC600
#define JP1_BASE              0xFF200060

/*
NOTES

TO DO LIST:
// Need to implement set time to auto start
        Set a default time on program start

Switch 3 --> Set time value to auto start (needs to be done)




// Set a control for the auto mode (DONE)
// Program the auto mode calculation based on outside (DONE - can make more complex)
	    Have a lookup table for optimal temp range (Didn't do)


Pick either option 1 (switch 1) or option 2 (switch 2)
Switch 1 --> Set the desired temperature
Switch 2 --> Set the outside temperature (turns on auto mode, no need for desired temperature)

Switch 3 --> Set time value to auto start (needs to be done)


Key 1/2 --> Used to increment and decrement temp for desired/outside when not using GIO
Key 3 --> (not working) Reset the timer Press down to see the current time (or diff in time)


*/





//Define LED base
//volatile int* const led = (int *) LED_BASE;

//Define switch pointer
volatile int * SW_ptr = (int *)SW_BASE;

//Define GPIO pointer
volatile int * GP_ptr = (int *)JP1_BASE;

//Key pointer
volatile int * const key_ptr = (int*)KEY_BASE;

//LED 1 to 4 fromt the right
volatile int* const led = (int *) HEX3_HEX0_BASE;
 
//LED 5 to 6 from the right
volatile int* const led2 = (int *) HEX5_HEX4_BASE;


//Hex code
char hex_code[16] = {0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x07,0x7F,0x67};

//Temperature variable
int outsideTemp = 0;
int insideTemp = 0;
int desiredTemp = 0;
int optimalTemp = 0;
int reachedFlag = 0;
int listen = 0;


//Flags
int insideFlag = 0;
int outsideFlag = 0;
int autoFlag = 0;

//Timer structure
typedef struct _a9_timer
{
    int load;
    int count;
    int control;
    int status; 
} a9_timer;
 
volatile a9_timer* const timer_1 = (a9_timer*)TIMER_A9_BASE;

//Timer structure 2
typedef struct _a9_timer2
{
    int load;
    int count;
    int control;
    int status; 
} a9_timer2;
 
volatile a9_timer2* const timer_2 = (a9_timer2*)TIMER_A9_BASE;


typedef struct _ADC
{
unsigned int ch0 ;
unsigned int ch1 ;
} ADC ;

volatile ADC* const ADC_ptr = (ADC*)ADC_BASE;


typedef struct _GPIO{
unsigned int data ;
unsigned int control ;
} GPIO ;

volatile GPIO* const port_A = (GPIO*) JP1_BASE;


// initializes timer to 'interval'
void set_timer( int interval ){
 
    timer_1->load = interval;
}
 
 
void start_timer() // starts the timer counting, clears previous timeout flag
{
    //Set the status flag in Interrupt Status to 1 
    timer_1->status = 1; 
	
    //3 bc 0 1 1 --> (interrupt, auto, enable)
    //1<<8 --> shift 8 bits from 1
    timer_1->control = 3; 
}
 
 
void stop_timer() // stops timer
{
    //Set the control bit to 2
    timer_1->control = 2;
} 


void set_timer2( int interval ){
 
    timer_2->load = interval;
}
 
 
void start_timer2() // starts the timer counting, clears previous timeout flag
{
    //Set the status flag in Interrupt Status to 1 
    timer_2->status = 1; 
	
    //3 bc 0 1 1 --> (interrupt, auto, enable)
    //1<<8 --> shift 8 bits from 1
    timer_2->control = 3; 
}
 
 
void stop_timer2() // stops timer
{
    //Set the control bit to 2
    timer_2->control = 2;
 
 
} 
 
 
 



//Read switches
int ReadSwitches(void)
{
    //Return the binary value of the switch currently flipped
    return *(SW_ptr);
 
}


int ReadKeys(void){
    //Return binary value of key pressed
    return *(key_ptr);
}

void DisplayTempHex(int one)
{

    int firstval = one % 10;
    int secondval = (one/10)%10;
 
    //Set the value of the LED pointer to the current hexcode based on current switch
    *(led) = hex_code[firstval] + (hex_code[secondval] << 8);
	//*(led) = (hex_code[s] << 16) + (hex_code[ts] << 24);
	//*(led2) = hex_code[m] + (hex_code[tm] << 8);
    
}

void DisplayOutsideTempHex(int one)
{

    int firstval = one % 10;
    //int secondval = (one/10)%10;
 
    //Set the value of the LED pointer to the current hexcode based on current switch
    *(led) = hex_code[firstval] + (0x40 << 8);
	//*(led) = (hex_code[s] << 16) + (hex_code[ts] << 24);
	//*(led2) = hex_code[m] + (hex_code[tm] << 8);
    
}


void DisplayTimeHex(int hr,int mn)
{

    int hour1=0;
    int hour2 = 0;
    int min1 = 0;
    int min2 = 0;

    if(hr >= 10){
        hour1 = hr % 10;
        hour2 = (hr/10)%10;

    }
    else {
        hour1 = hr % 10;
        hour2 = (hr/10)%10;
    } 

    if(mn >= 10){

        min1 = mn % 10;
        min2 = (mn/10)%10;
    }
    else {
        min1 = mn % 10;
        min2 = (mn/10)%10;
    } 

  

    //Set the value of the LED pointer to the current hexcode based on current switch
    *(led) = hex_code[min1] + (hex_code[min2] << 8) + (hex_code[hour1] << 16) + (hex_code[hour2] << 24);
    
	//*(led) = (hex_code[s] << 16) + (hex_code[ts] << 24);
    
}


int numConverter(int num)
{
	int numbers[10] = { 0x3F, 0x6, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x7, 0x7F, 0x6F };
	return numbers[num];
}

void timeDisplay(int hours, int minutes)
{
	int currentTime = 0;
	
	currentTime += numConverter(minutes % 10 );
	currentTime += numConverter(minutes / 10 ) << 8;

	currentTime += numConverter(hours % 10 ) << 16;
	currentTime += numConverter(hours / 10 ) << 24;

	*(led) = currentTime;
}


void GetADC(int value){
    int potdata =0;

    if(value>>15){
                //Bit mask for the first 15bits
                potdata = value & 0xFFF;
                if(potdata == 0){
                    port_A->data = 0b0;
                    if(insideFlag == 1)
                    {
                        desiredTemp=16;
                    }
                    else if(outsideFlag == 1){
                        outsideTemp=-5;
                    }
					    
					
                }
                if(potdata < 410){
                    port_A->data = 0b1;
					
                    if(insideFlag == 1)
                    {
                        desiredTemp=17;
                    }
                    else if(outsideFlag == 1){
                        outsideTemp=-4;
                    }
                }
                if(potdata > 410*1+1 && potdata <410*2){
                    port_A->data = 0b11;
					
                    if(insideFlag == 1)
                    {
                        desiredTemp=18;
                    }
                    else if(outsideFlag == 1){
                        outsideTemp=-3;
                    }
                }
                if(potdata > 410*2+1 && potdata < 410*3){
                    port_A->data = 0b111;
					
                    if(insideFlag == 1)
                    {
                        desiredTemp=19;
                    }
                    else if(outsideFlag == 1){
                        outsideTemp=-2;
                    }
                }
                if(potdata > (410*3+1) && potdata < 410*4){
                    port_A->data = 0b1111;
					
                     if(insideFlag == 1)
                    {
                        desiredTemp=20;
                    }
                    else if(outsideFlag == 1){
                        outsideTemp=-1;
                    }
                }
                if(potdata > 410*4+1 && potdata < 410*5){
                    port_A->data = 0b11111;
					
                     if(insideFlag == 1)
                    {
                        desiredTemp=21;
                    }
                    else if(outsideFlag == 1){
                        outsideTemp=0;
                    }
                }
                if(potdata > 410*5+1 && potdata < 410*6){
                    port_A->data = 0b111111;
					
                     if(insideFlag == 1)
                    {
                        desiredTemp=22;
                    }
                    else if(outsideFlag == 1){
                        outsideTemp=28;
                    }
                }
                if(potdata > 410*6+1 && potdata < 410*7){
                    port_A->data = 0b1111111;
					
                    if(insideFlag == 1)
                    {
                        desiredTemp=23;
                    }
                    else if(outsideFlag == 1){
                        outsideTemp=29;
                    }
                }
                if(potdata > 410*7+1 && potdata < 410*8){
                    port_A->data = 0b11111111;
					
                    if(insideFlag == 1)
                    {
                        desiredTemp=24;
                    }
                    else if(outsideFlag == 1){
                        outsideTemp=30;
                    }
                }
                if(potdata > 410*8+1 && potdata < 410*9){
                    port_A->data = 0b111111111;
					
                    if(insideFlag == 1)
                    {
                        desiredTemp=25;
                    }
                    else if(outsideFlag == 1){
                        outsideTemp=31;
                    }
                }
                if(potdata > 410*9+1 && potdata < 410*10){
                    port_A->data = 0b1111111111;
					

                    if(insideFlag == 1)
                    {
                        desiredTemp=26;
                    }
                    else if(outsideFlag == 1){
                        outsideTemp=32;
                    }
                }
            }
            
            //Display either outside or inside depending
            if(insideFlag == 1)
            {
                DisplayTempHex(desiredTemp);
                insideFlag = 0;
            }
            else if(outsideFlag == 1){
                //DisplayOutsideTempHex(outsideTemp);
                if(outsideTemp >= 0){
                    DisplayTempHex(outsideTemp);
                }
                else{
                    DisplayOutsideTempHex(abs(outsideTemp));
                }
                outsideFlag = 0;
            }

            
}

int noneFlag = 0;

void main(){
    //ADC stuff
    int ADCdata = 0;
    
    port_A->control = 0xCFF;
    ADC_ptr->ch1 = 1;

    //Interval for 3 seconds
    int interval = 200000000;
    unsigned long long int intervalTimer2=12000000000;

    int started = 0;
    //int time = 0;
 
    //Set initial load of timer (1 milli sec --> 200)
    set_timer(interval);
    set_timer2(interval);
	//start_timer2();// starting 2nd timer, as it runs continuously 

	int runTmFlag = 0;
    int ranTm2Flag = 0;
    int randomFlag = 0;
    int current_button = 0;
    int previous_button = 0;

    int current_button2 = 0;
    int previous_button2 = 0;

    int current_button4 = 0;
    int previous_button4 = 0;

    int current_button5 = 0;
    int previous_button5 = 0;

    int current_button3 = 0;
    int previous_button3 = 0;
    int hours=0, minutes=0, storedHours=0, storedMinutes=0;

    while(1){
		
		current_button = ReadKeys();
		current_button2 = ReadKeys();
		current_button3 = ReadKeys();
        current_button4 = ReadKeys();
        current_button5 = ReadKeys();
        //Starting mode
		if(ReadSwitches() == 1) //Channel 1
		{
            /*
            if((current_button != previous_button) && current_button == 1){
                noneFlag = 1;
                desiredTemp += 1;
                DisplayTempHex(desiredTemp);
            }
            else if((current_button2 != previous_button2) && current_button2 == 2){
                noneFlag = 1;
                desiredTemp -= 1;
                DisplayTempHex(desiredTemp);
            }*/

            if(noneFlag!= 1){
                DisplayTimeHex(0,0);
            }

            if((current_button4 != previous_button4) && current_button4 == 4){
                noneFlag = 1;
                storedMinutes += 5;
                
                if (storedMinutes== 60) 
                {
                    storedMinutes= 0;
                    storedHours+= 1;
                }

                if (hours== 24)
                {
                    storedHours= 0;
                }
                DisplayTimeHex(storedHours, storedMinutes);
            }
            else if((current_button5 != previous_button5) && current_button5 == 8){
                noneFlag = 1;
                 storedMinutes -= 5;

                if (storedMinutes== 0) 
                {
                    storedMinutes= 59;
                    storedHours-= 1;
                }

                if (storedHours== -1)
                {
                    storedHours= 23;
                }
                DisplayTimeHex(storedHours, storedMinutes);
            }

            //Set inside flag to 1
            insideFlag = 1;
			//Uncomment when using on actual board, disabled adc for sim
            //int prevADC = ADCdata;
            ADCdata = ADC_ptr->ch0;


            if(current_button == 1){
                GetADC(ADCdata);
            }
            
            

        }
        else if(ReadSwitches() == 2){
            //Set outside temperature for auto mode
            outsideFlag = 1;
            
			/*if((current_button != previous_button) && current_button == 1){
                noneFlag = 1;
                outsideTemp += 5;
                if(outsideTemp >= 0){
                    DisplayTempHex(outsideTemp);
                }
                else{
                    DisplayOutsideTempHex(abs(outsideTemp));
                }
            }
            else if((current_button2 != previous_button2) && current_button2 == 2){
                noneFlag = 1;
                outsideTemp -= 1;
                if(outsideTemp >= 0){
                    DisplayTempHex(outsideTemp);
                }
                else{
                    DisplayOutsideTempHex(abs(outsideTemp));
                }
                
            }*/
			
            if(noneFlag!= 1){
                DisplayTimeHex(0,0);
            }

            if((current_button4 != previous_button4) && current_button4 == 4){
                noneFlag = 1;
                storedMinutes += 5;
                
                if (storedMinutes== 60) 
                {
                    storedMinutes= 0;
                    storedHours+= 1;
                }

                if (hours== 24)
                {
                    storedHours= 0;
                }
                DisplayTimeHex(storedHours, storedMinutes);
            }
            else if((current_button5 != previous_button5) && current_button5 == 8){
                noneFlag = 1;
                 storedMinutes -= 5;

                if (storedMinutes== 0) 
                {
                    storedMinutes= 59;
                    storedHours-= 1;
                }

                if (storedHours== -1)
                {
                    storedHours= 23;
                }
                DisplayTimeHex(storedHours, storedMinutes);
            }
			//Uncomment on actual board
            ADCdata = ADC_ptr->ch0;
            if(current_button == 1){
                GetADC(ADCdata);
            }

        }
        else{ //Channel 0
            
            if(ranTm2Flag != 1){
                start_timer2();
            }

            if(minutes!=storedMinutes || hours!=storedHours)
            {
                ranTm2Flag=1;
                DisplayTimeHex(hours,minutes);
                //Start incrementing timer
                if (timer_2->status == 1)
                {
                    minutes += 5;

                    if (minutes== 60) 
                    {
                        minutes= 0;
                        hours+= 1;
                    }

                    if (hours== 24)
                    {
                        hours= 0;
                    }

                    //(*timer_2).status= 1;
                    timer_2->status = 1; 
                }
            }
            
            
			

            if(minutes==storedMinutes && hours==storedHours && randomFlag==0)
            {
                randomFlag=1;
                stop_timer2();
                runTmFlag=0;
            }

            if(randomFlag == 1){
                   
                //If outside temp has been set at start, then use auto mode
                if(outsideTemp != 0){
                    autoFlag = 1;
                    
                    //Set optimal temperature (can add more complex logic if needed)
                    if(outsideTemp > 27){
                        optimalTemp = 19;
                    }
                    else if(outsideTemp <  1){
                        optimalTemp = 23;
                    }
                }

                //Display current temperature
                DisplayTempHex(insideTemp);
                
                //
                if(runTmFlag != 1){
                    //Reset inside temp if restarting
                    runTmFlag = 1;
                    start_timer();
                    started = 1;
                    
                }
                

                //Press the third key to reset
                if((current_button3 != previous_button3) && current_button3 == 1){
                    //runTmFlag = 0;
                    insideTemp = 0;
                }
                
                if(started == 1)
                {
                
                    //Get the current count
                    //current_count = timer_1->count;
                    
                    //If status flag is 1, means timer has reached the end 
                    if(timer_1->status == 1)
                    {
                        //Increment time by 1 every 1ms
                        insideTemp=insideTemp+1;
                        
                        
                        
                        //Clear timeout flag once the timer is done (Set the status flag in Interrupt Status to 1)
                        timer_1->status = 1; 

                    }
                
                }

                
                if(autoFlag == 0 && insideTemp == desiredTemp){
                    stop_timer();
                    reachedFlag = 1;
                }
                else if(autoFlag == 1 && insideTemp == optimalTemp){
                    stop_timer();
                    reachedFlag = 1;
                }
            }

            
		}



    previous_button = current_button;
    previous_button2 = current_button2;
    previous_button3 = current_button3;
    previous_button4 = current_button4;
    previous_button5 = current_button5;

    }

}