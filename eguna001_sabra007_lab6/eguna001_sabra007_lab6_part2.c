/*
 *
 * Sargis Abrahamyan sabra007@ucr.edu
 * Enrico Gunawan eguna001@ucr.edu
 * Lab section: B21
 * Assignment: Lab 6 Exercise 2
 *
 * I acknowledge all content contained herein, excluding template or example
 * code, is my own original work.
 *
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include "io.c"
	
volatile unsigned char TimerFlag = 0; // TimerISR() sets this to 1. C programmer should clear to 0.

// Internal variables for mapping AVR's ISR to our cleaner TimerISR model.
unsigned long _avr_timer_M = 1; // Start count from here, down to 0. Default 1 ms.
unsigned long _avr_timer_cntcurr = 0; // Current internal count of 1ms ticks

void TimerOn() {
	// AVR timer/counter controller register TCCR1
	TCCR1B = 0x0B;// bit3 = 0: CTC mode (clear timer on compare)
	// bit2bit1bit0=011: pre-scaler /64
	// 00001011: 0x0B
	// SO, 8 MHz clock or 8,000,000 /64 = 125,000 ticks/s
	// Thus, TCNT1 register will count at 125,000 ticks/s

	// AVR output compare register OCR1A.
	OCR1A = 125;	// Timer interrupt will be generated when TCNT1==OCR1A
	// We want a 1 ms tick. 0.001 s * 125,000 ticks/s = 125
	// So when TCNT1 register equals 125,
	// 1 ms has passed. Thus, we compare to 125.
	// AVR timer interrupt mask register
	TIMSK1 = 0x02; // bit1: OCIE1A -- enables compare match interrupt

	//Initialize avr counter
	TCNT1=0;

	_avr_timer_cntcurr = _avr_timer_M;
	// TimerISR will be called every _avr_timer_cntcurr milliseconds

	//Enable global interrupts
	SREG |= 0x80; // 0x80: 1000000
}

void TimerOff() {
	TCCR1B = 0x00; // bit3bit1bit0=000: timer off
}

void TimerISR() {
	TimerFlag = 1;
}

// In our approach, the C programmer does not touch this ISR, but rather TimerISR()
ISR(TIMER1_COMPA_vect) {
	// CPU automatically calls when TCNT1 == OCR1 (every 1 ms per TimerOn settings)
	_avr_timer_cntcurr--; // Count down to 0 rather than up to TOP
	if (_avr_timer_cntcurr == 0) { // results in a more efficient compare
		TimerISR(); // Call the ISR that the user uses
		_avr_timer_cntcurr = _avr_timer_M;
	}
}

// Set TimerISR() to tick every M ms
void TimerSet(unsigned long M) {
	_avr_timer_M = M;
	_avr_timer_cntcurr = _avr_timer_M;
}

enum States { START, Init, Left, Right, Pause } state;
unsigned char tmpB;
unsigned char tmpA;
unsigned char score;
void Tick() {
	// State Transitions
	switch (state) {
		case START:
			state = Init;
		break;
		case Init:
			state = Left;
		break;
		case Left:
			if(tmpB == 0x04 && !tmpA)
			{	
				state = Right;
			}
			else if(tmpA)
			{
				state = Pause;
			}
			else
			{
				state = Left;
			}
		break;
		case Right:
			if(tmpB == 0x01 && !tmpA)
			{
				state = Left;
			}
			else if(tmpA)
			{
				state = Pause;
			}
			else
			{
				state = Right;
			}
		break;

		case Pause:
			
				state = Init;
			
			break;
		default:
			state = START;
		break;
	}

	// State Actions
	switch (state) {
		case Init:
		tmpB = 0x01; break;
		case Left:
			 tmpB = tmpB<<1;
			break;
		case Right:
			 tmpB = tmpB>>1;
			break;
		case Pause:
			{
				if(tmpB == 0x02)
				{	
				
					score++;
					
					
				}
				else
				{
					if (score)
					{
						score--;
					}
					
				}
			}	
			break;
		default:
		break;
	}

}

int main(void)
{
	DDRC = 0xFF; PORTC = 0x00; // LCD data lines
	DDRD = 0xFF; PORTD = 0x00; // LCD control lines
	DDRB = 0xFF; PORTB = 0x00;

	DDRA = 0x00; PORTA = 0xFF;


	TimerSet(300);
	TimerOn();

	
	LCD_init();

	state = START;
	score = 5;

	while(1)
	{
	
	tmpA = ~PINA & 0x01;
		Tick();
		PORTB = tmpB;
		LCD_Cursor(1);
		if(score == 6)
		{
			LCD_ClearScreen();
			LCD_Cursor(0);
			LCD_DisplayString(1, "You Win");

		}
		else
		{
		LCD_WriteData(score + '0');
		LCD_Cursor(2);
		}
		while (!TimerFlag);
		TimerFlag = 0;
	
	}
}


