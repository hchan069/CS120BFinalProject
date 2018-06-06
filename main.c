#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <stdio.h>
#include "io.c"
#include "scheduler.h"
#include "timer.h"
#include "pwm.h"
#include "bit.h"


#define F_CPU 1000000
#include <util/delay.h>

int HORIZONTAL= 0;//neutral value on x-axis
int VERTICAl = 0;// neutral value on y-axis
int HORIZONTALMOV =0;
int VERTICAlMOV =0;
unsigned char col = 0x10;
unsigned char row = 0x10;


int inputTick(int state);
void transmit_column(unsigned char data);
void transmit_row(unsigned char data);

void transmit_column(unsigned char data) {
    int i;
    for (i = 0; i < 8 ; ++i) {
        PORTC = 0x08;
        // set SER = next bit of data to be sent.
        PORTC |= ((data >> i) & 0x01);
        // set SRCLK = 1. Rising edge shifts next bit of data into the shift register
        PORTC |= 0x02;
    }
    // set RCLK = 1. Rising edge copies data from ?Shift? register to ?Storage? register
    PORTC |= 0x04;
    // clears all lines in preparation of a new transmission
    PORTC = 0x00;
}

void transmit_row(unsigned char data) {
    int i;
    for (i = 0; i < 8 ; ++i) {
        // Sets SRCLR to 1 allowing data to be set
        // Also clears SRCLK in preparation of sending data
        PORTB = 0x08;
        // set SER = next bit of data to be sent.
        PORTB |= ((data >> i) & 0x01);
        // set SRCLK = 1. Rising edge shifts next bit of data into the shift register
        PORTB |= 0x02;
    }
    // set RCLK = 1. Rising edge copies data from “Shift” register to “Storage” register
    PORTB |= 0x04;
    // clears all lines in preparation of a new transmission
    PORTB = 0x00;
}

void ADC_init(void){
    ADMUX|=(1<<REFS0);
    //ADCSRA |=(1<<ADEN) |(1<ADPS2)|(1<ADPS1) |(1<<ADPS0);
    ADCSRA |= (1 << ADEN) | (1 << ADSC) | (1 << ADATE); //ENABLE ADC, PRESCALER 128
}

//Enumeration of states.
// Monitors joystick connected to PA.
enum SM2_STATES {SM2_Init, SM2_Display};
unsigned char isLeft = 0;
unsigned char isRight = 0;

int SM2_Tick(int state) {
    // === Local Variables ===
    //unsigned long input;
    ADC_init();
    //input = ADC;
    unsigned static check_back_home = 0;
    // === Transitions ===
    switch (state) {
        case SM2_Init: state = SM2_Display; break;
        case SM2_Display: break;
        default: state = SM2_Init;
        break;
    }
    switch (state){ //actions
        case SM2_Display:
        switch (ADMUX)//changing between channels by switch statement
        {
            case 0x40://When ADMUX==0x40
            {
                ADCSRA |=(1<<ADSC);//start ADC conversion
                while ( !(ADCSRA & (1<<ADIF)));//wait till ADC conversion
                HORIZONTALMOV = ADC;//moving value
                ADC=0;//reset ADC register
                ADMUX=0x41;//changing channel
                break;
            }
            case 0x41:
            {
                ADCSRA |=(1<<ADSC);// start ADC conversion
                while ( !(ADCSRA & (1<<ADIF)));// wait till ADC conversion
                VERTICAlMOV = ADC;// moving value
                ADC=0;// reset ADC register
                ADMUX=0x40;// changing channel
                break;
            }
        }
        if(HORIZONTALMOV<(HORIZONTAL+400) && check_back_home == 0){ //left
            //if(Xval <0x80) Xval = Xval << 1;
            isLeft = 1;
            check_back_home = 3;
            
        }
        if(HORIZONTALMOV>(HORIZONTAL+800) && check_back_home == 0) { //right
            //if(Xval>=0x02) Xval = Xval >> 1;
            isRight = 1;
            
            check_back_home = 3;
        }
        /*if(VERTICAlMOV<VERTICAl + 300 && check_back_home ==0){ //down
            if(Yval>=0x02)
            Yval = Yval >> 1;
            
            check_back_home = 3; 
        }
        
        if(VERTICAlMOV>VERTICAl+800 && check_back_home ==0){ //up
            if(Yval<0x80)
            Yval = Yval << 1;
            
            check_back_home = 3;
        }*/
        // we check each time user should back the home and to do next order
        if( (HORIZONTALMOV>HORIZONTAL+300) && (HORIZONTALMOV<HORIZONTAL+800)  && check_back_home!=0){
            check_back_home--;
            //isLeft = 0;
            //isRight = 0;
        }

        //transmit_row(Yval);
        //transmit_column(~Xval);
        break;
        default: break;
        
    }
    return state;
};

static unsigned char Xval = 0x08; // sets the pattern displayed on columns //PortB
static unsigned char Yval = 0x08;
unsigned char iter1 = 0;
unsigned char column_valueC[8] = {0xFF, 0x81, 0x81, 0x81, 0x00, 0xF0, 0x10, 0xFF};
unsigned char column_valueD[8] = {0xFF, 0x81, 0x42, 0x3C, 0x00, 0xF0, 0x10, 0xFF};
unsigned char column_valueE[8] = {0xFF, 0x99, 0x81, 0x81, 0x00, 0xF0, 0x10, 0xFF};
unsigned char column_valueF[8] = {0xFF, 0xD8, 0xC0, 0xC0, 0x00, 0xF0, 0x10, 0xFF};
unsigned char column_valueG[8] = {0xFF, 0x81, 0x89, 0x8F, 0x00, 0xF0, 0x10, 0xFF};
unsigned char column_valueA[8] = {0xFF, 0x90, 0x90, 0xFF, 0x00, 0xF0, 0x10, 0xFF};
unsigned char column_valueB[8] = {0xFF, 0x09, 0x09, 0x0F, 0x00, 0xF0, 0x10, 0xFF};
unsigned char column_valueC5[8] = {0xFF, 0x81, 0x81, 0x81, 0x00, 0xF1, 0x91, 0x9F};

enum SM3_STATES {SM3_Start};
int SM3_Tick (int state) {
    
    static unsigned char iter2 = 0;
    static unsigned char column_select = 0x7F;
    
    switch (state) {
        case SM3_Start: state = SM3_Start; break;
        default: state = SM3_Start; break;
    }
    switch (state) {
        case SM3_Start:
            transmit_column(0xFF);
            if (iter1 == 0) {                 
                transmit_row(column_valueC[iter2]);               
            }
            else if (iter1 == 1) {
                transmit_row(column_valueD[iter2]);              
            }
            else if (iter1 == 2) {
                transmit_row(column_valueE[iter2]); 
            }
            else if (iter1 == 3) {
                transmit_row(column_valueF[iter2]); 
            }
            else if (iter1 == 4) {
                transmit_row(column_valueG[iter2]); 
            }
            else if (iter1 == 5) {
                transmit_row(column_valueA[iter2]);
            }
            else if (iter1 == 6) {
                transmit_row(column_valueB[iter2]);
            }
            else if (iter1 == 7) {
                transmit_row(column_valueC5[iter2]);
            }
            
            transmit_column(column_select);
            //transmit_row(0x00);
            //transmit_column(0xFF);
            iter2++;
            if (column_select != 0xFE)
                column_select = (column_select >> 1) | 0x80;
            else {
                column_select = 0x7F;
                iter2 = 0;
            }                
            
            break;
    }
    return state;
    
}

#define note_C4 261.63
//#define note_Cs4 277.18
#define note_D4 293.66
//#define note_Ds4 311.13
#define note_E4 329.63
#define note_F4 349.23
//#define note_Fs4 369.99
#define note_G4 392
//#define note_Gs4 415.30
#define note_A4 440
//#define note_As4 466.16
#define note_B4 493.88
#define note_C5 523.25

#define POWER_ON (~PINA & 0x10)
#define SAVE (~PINA & 0x20)

enum SM1states {SM1_Init, SM1_Off, SM1_PressOn, SM1_On, SM1_IncreaseNote, SM1_IncreaseNoteWait, SM1_DecreaseNote, SM1_DecreaseNoteWait, SM1_PressOff} SM1state;
//double notes[] = {note_C4, note_Cs4, note_D4, note_Ds4, note_E4, note_F4, note_Fs4, note_G4, note_Gs4, note_A4, note_As4, note_B4, note_C5};
double notes[] = {note_C4, note_D4, note_E4, note_F4, note_G4, note_A4, note_B4, note_C5};


int SM1_Tick(int state) {
    
    switch(state) { // transitions
        case SM1_Init: state = SM1_Off; break;
        case SM1_Off: state = (POWER_ON) ? SM1_PressOn : SM1_Off; break;
        case SM1_PressOn: state = (POWER_ON) ? SM1_PressOn : SM1_On; break;
        case SM1_On:
            if(isLeft) { // left (MY DOWN)
                state = SM1_DecreaseNote;
                break;
            }
            if(isRight){ //right (MY UP)
                state = SM1_IncreaseNote;
                break;
            }         
            if (POWER_ON) {
                state = SM1_PressOff;
                break;
            }
            if (SAVE) {
                eeprom_write_byte((uint8_t *)1, 1);
                eeprom_write_byte((uint8_t*)46, state);
                eeprom_write_byte((uint8_t*)4, iter1);
                break;
            }
            //if (!isLeft) state = SM1_On; break;
        case SM1_DecreaseNote: state = SM1_DecreaseNoteWait; break;
        case SM1_DecreaseNoteWait:
        state = (isLeft) ? SM1_DecreaseNoteWait : SM1_On; break;
        case SM1_IncreaseNote: state = SM1_IncreaseNoteWait; break;
        case SM1_IncreaseNoteWait:
        state = (isRight) ? SM1_IncreaseNoteWait : SM1_On; break;
        //state = (TUNE_UP) ? SM1_PressUpWait : SM1_On; break;
        case SM1_PressOff: state = (POWER_ON) ? SM1_PressOff : SM1_Off; break;
    }
    switch(state) { // actions
        case SM1_Init: break;
        case SM1_Off: set_PWM(0); break;
        case SM1_PressOn: break;
        case SM1_On: 
            set_PWM(notes[iter1]); 
            break;
        case SM1_DecreaseNote:
            if (iter1 > 0)
                iter1--;
            if (iter1 == 0)
                iter1 = ( sizeof(notes)/sizeof(notes[0]) - 1 );
            break;
        case SM1_DecreaseNoteWait: isLeft = 0; break;
        case SM1_IncreaseNote:
            if (iter1 < ( sizeof(notes)/sizeof(notes[0]) ) )
               iter1++;
            if (iter1 == ( sizeof(notes)/sizeof(notes[0]) ) )
              iter1 = 0;
            break;
        case SM1_IncreaseNoteWait: isRight = 0; break;
        case SM1_PressOff: break;
        
    }
    return state;
}

int main(void)
{
    //set data direction registers
    DDRA = 0x0C; PORTA = 0xF3;
    DDRB = 0xFF; PORTB = 0x00;
    DDRC = 0xFF; PORTC = 0x00;
    DDRD = 0xFF; PORTD = 0x00;

    unsigned long int tick1_calc = 50;
    unsigned long int tick2_calc = 75;
    unsigned long int tick3_calc = 1;
    unsigned long int GCD = 1;
    unsigned long int SMTick1_period = tick1_calc/GCD;
    unsigned long int SMTick2_period = tick2_calc/GCD;
    unsigned long int SMTick3_period = tick3_calc/GCD;

    static task task1, task2, task3;
    task *tasks[] = { &task1, &task2, &task3};
    const unsigned short numTasks = sizeof(tasks)/sizeof(task*);
 
    if(eeprom_read_byte((uint8_t *) 1) != 1) {
        task1.state = SM1_Init; //task init state
    } else {
        task1.state = eeprom_read_byte((uint8_t*)46); //task init state
        iter1 = eeprom_read_byte((uint8_t*)4);
    }
    
    task1.period = SMTick1_period;
    task1.elapsedTime = task1.period;
    task1.TickFct = &SM1_Tick;
    
    task2.state = SM2_Init; //task init state
    task2.period = SMTick2_period;
    task2.elapsedTime = task2.period;
    task2.TickFct = &SM2_Tick;
    
    task3.state = SM3_Start; //task init state
    task3.period = SMTick3_period;
    task3.elapsedTime = task3.period;
    task3.TickFct = &SM3_Tick;

    ADC_init();
    PWM_on();
    LCD_init();
   // LCD_ClearScreen();q
    TimerSet(GCD);
    TimerOn();
    
    unsigned char i;
    unsigned char musicNodes[] = {0x0E, 0x1F, 0x0E, 0x10, 0x14, 0x12, 0x14, 0x18};
        
    LCDCharBuilder(1, musicNodes);
    LCD_WriteData(0x01);
    LCD_DisplayString(2, " MUSIC NOTE");
    LCD_WriteData(0x02);
    LCD_WriteData(0x01);
    LCD_Cursor(33);
    
  	eeprom_read_byte((uint8_t*)46);

     
    while(1)
    {
        // Scheduler code
        for ( i = 0; i < numTasks; i++ ) {
            // Task is ready to tick
            if ( tasks[i]->elapsedTime == tasks[i]->period ) {
                // Setting next state for task
                tasks[i]->state = tasks[i]->TickFct(tasks[i]->state);
                // Reset the elapsed time for next tick.
                tasks[i]->elapsedTime = 0;
            }
            tasks[i]->elapsedTime += 1;
        }
        while(!TimerFlag);
        TimerFlag = 0;
    }
}
