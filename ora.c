#include <avr/io.h>
#include <avr/interrupt.h>
#define F_CPU 8000000UL
#include <util/delay.h>

volatile uint8_t a=0, b=0, c=0, tmp=0, sor=1, keyb=0;
volatile uint8_t btn=1;

uint16_t num=0;

typedef struct time {
	uint8_t sec;
	uint8_t min;
	uint8_t hour;
	uint8_t date;
	uint8_t month;
	uint16_t year;
	}time;	

time t;

void port_init(void);
void timer_init(void);
void led_out(uint8_t);
uint8_t leap_year(void);
void seg_out(uint16_t, uint16_t, uint8_t);
void keyboard_in(void);
void input_num(uint16_t);
void set_time(void);


int main(void) {
	port_init();
	timer_init();
	
	t.sec=5;
	t.min=29;
	t.hour=5;
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
		case 5: {keyboard_in(); set_time(); led_out(16); break;}
	}
	if (t.sec>=0 && t.sec<=3 && t.min==30 && t.hour==5) {
		btn=1;
		//PORTE=0x08;
	}
	else {
		PORTE=0x00;
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
	DDRE=0x08;
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

void seg_out(uint16_t jobb, uint16_t bal, uint8_t numOfByte) {
	if (numOfByte) {
		jobb%=100;
		bal/=100;
	}
	else if (!numOfByte) {
		jobb&=0xFF;
		bal&=0xFF;
	}
	PORTA=((t.sec%2) ? 0xC0 : 0x00);
	switch (a) {
		case 0: {PORTA=0x80|0x00|(jobb%10); a++; break;}
		case 1: {PORTA=0x80|0x10|(jobb/10); a++; break;}
		case 2: {PORTA=0x80|0x20|(bal%10); a++; break;}
		case 3: {PORTA=0x80|0x30|(bal/10); a=0; break;}
	}	
}

void keyboard_in(void) {
	uint8_t oszlop, x=1, sor, in;
	for (sor=1; sor<=4; sor++) {
		PORTC=x<<3;
		_delay_us(20);
		in=(PINC&0x07);
		if (in==0x07) {
			c=0;
		}
		else if (in==0x06 && c==0) {
			oszlop=1;
			c=1;
		}
		else if (in==0x05 && c==0) {
			oszlop=2;
			c=1;
		}
		else if (in==0x03 && c==0) {
			oszlop=3;
			c=1;
		}
		x=x<<1;
		if (c && sor==4) {
			if (oszlop==1) {
				keyb='*';
			}
			else if (oszlop==2) {
				keyb=0;
			}
			else if (oszlop==3) {
				keyb='#';
			}
		}
		else if (c && sor!=4 && sor!=0) {
			keyb=(oszlop+3*(sor-1));
		}
		
	}
<<<<<<< HEAD
}

void input_num(uint16_t max) {
	if (keyb=='*') {
		num/10;
	}
	else if ((keyb!='0') && (num<=max)) {
		//num*=10;
		num+=keyb;
	}
	else if (keyb=='#') {
		tmp++;
	}
	led_out(keyb);
	seg_out(num, num, 1);
}

void set_time(void) {
	led_out(tmp);
	switch (tmp) {
		case 0: { 		//year
			input_num(9999);
			if (tmp==1) {
				t.year=num;
				num=0;
			}
			break;
		}
		case 1: { 		//month
			input_num(12);
			if (tmp==2) {
				t.month=num;
			}
			break;
		}
		case 2: { 		//date
			if (t.month==2 && leap_year()) {
				input_num(29);
			}
			else if (t.month==2 && !leap_year()) {
				input_num(28);
			}
			else if ((t.month==4 || t.month==6 || t.month==9 || t.month==11)) {
				input_num(30);
			}
			else {
				input_num(31);
			}
			if (tmp==3) {
				t.date=num;
			}
			break;
		}
		case 3: { 		//hour
			input_num(23);
			if (tmp==4) {
				t.hour=num;
			}
			break;
		}
		case 4: { 		//min
			input_num(59);
			if (tmp==5) {
				t.min=num;
			}
			break;
		}
		case 5: { 		//sec
			input_num(59);
			if (tmp==6) {
				t.sec=num;
				tmp==0;
				btn=1;
			}
			break;
		}
=======
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
>>>>>>> 18de3b95006ee6858430a0a534a7b6e1f3bee6de
	}
}








