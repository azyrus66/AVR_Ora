#include <avr/io.h>
#include <avr/interrupt.h>
#define F_CPU 8000000UL
#include <util/delay.h>

volatile uint8_t a=0, b=0, sor=1;
volatile uint8_t btn=1;

void port_init(void);
void timer_init(void);
void led_out(uint8_t);
uint8_t leap_year(void);
void seg_out(uint16_t, uint16_t, uint8_t);
uint8_t keyboard_in(void);

typedef struct{
	uint16_t sec;
	uint16_t min;
	uint16_t hour;
	uint16_t date;
	uint16_t month;
	uint16_t year;
	}time;
	
	time t;


int main(void) {
	port_init();
	timer_init();
	
	t.sec=5;
	t.min=06;
	t.hour=18;
	t.date=24;
	t.month=11;
	t.year=2018;
	
	while (1) {
		switch (PING) {
			case 1: {btn=1; break;}
			case 2: {btn=2; break;}
			case 4: {btn=3; break;}
			case 8: {btn=4; break;}
			case 16: {btn=5; break;}
		}
	}
	
	
	while(1) {};
	return 0;
}

ISR(TIMER0_OVF_vect) {
	switch (btn) {
		case 1: {seg_out(t.sec, t.min, 0); led_out(1); break;}
		case 2: {seg_out(t.min, t.hour, 0); led_out(2); break;}
		case 3: {seg_out(t.date, t.month, 0); led_out(4); break;}
		case 4: {seg_out(t.year, t.year, 1); led_out(8); break;}
		case 5: {seg_out(keyboard_in(), keyboard_in(), 1); led_out(keyboard_in()); break;}
	}
}

ISR(TIMER1_COMPA_vect) {
	if (++t.sec==60) {
		t.sec=0;
		if (++t.min==60) {
			t.min=0;
			if (++t.hour==24) {
				t.hour=0;
				if (++t.date==32) {
					t.month++;
					t.date=1;
				}
				else if (t.date==31) {
					if ((t.month==4) || (t.month==6) || (t.month==9) || (t.month==11)) {
						t.month++;
						t.date=1;
					}
				}
				else if (t.date==30) {
					if(t.month==2) {
						t.month++;
						t.date=1;
					}
				}
				else if (t.date==29) {
					if((t.month==2) && (!leap_year())) {
						t.month++;
						t.date=1;
					}
				}
				if (t.month==13) {
					t.month=1;
					t.year++;
				}
			}
		}
	}
}

void port_init(void) {
	DDRB=DDRD=0xF0;
	DDRA=0xFF;
	DDRC=0xF8;
	DDRG=0;
	PORTC=0;
	
}

void timer_init(void) {
	//----------TIMER1_COMPAT----------
	TCCR1B=(1<<WGM12)|(1<<CS12);
	OCR1A=32150-1;				// 1 sec
	TIMSK=(1<<OCIE1A);
	
	//----------TIMER0_OVF-------------
	TCCR0=(1<<CS01);			// clk/8
	TIMSK|=(1<<TOIE0);
	sei();
}

void led_out(uint8_t led) {
	PORTD=led;
	PORTB=(led<<4);
}

uint8_t leap_year(void) {
	if (t.year%4) {
		if ((t.year%100) && (t.year%400)) {
			return 1;
		}
	}
	return 0;
}

void seg_out(uint16_t jobb, uint16_t bal, uint8_t one) {
	if (one) {
		bal=bal/100;
		jobb=jobb%100;
	}
	else {
		bal%100;
		jobb=jobb%100;
	}
	PORTA=((t.sec%2) ? 0xC0 : 0x00);
		
	switch (a) {
		case 0: {PORTA=0x80|0x00|(jobb%10); a++; break;}
		case 1: {PORTA=0x80|0x10|(jobb/10); a++; break;}
		case 2: {PORTA=0x80|0x20|(bal%10); a++; break;}
		case 3: {PORTA=0x80|0x30|(bal/10); a=0; break;}
	}	
}

uint8_t keyboard_in(void) {
	uint8_t oszlop, c=0, x=1, sor=1, in;
	for (int i=1; i<=4; i++) {
		PORTC=x<<3;
		_delay_us(20);
		in=(PINC&0x07);
		if (in==0x06) {oszlop=1; c=1; break;}
		if (in==0x05) {oszlop=2; c=1; break;}
		if (in==0x03) {oszlop=3; c=1; break;}
		x=x<<1;
		sor++;
	}
	if (c && sor!=4) {
		return (oszlop+3*(sor-1));
	}
	else if (c && sor==4) {
		if (oszlop==1) {
			return '*';
		}
		else if (oszlop==2) {
			return 0;
		}
		else if (oszlop==3) {
			return '#';
		}
	}
	else {
		return 0;
	}
}








