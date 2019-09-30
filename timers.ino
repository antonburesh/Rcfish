#define F_CPU 8000000UL
#define LED PB2

void setupPWM() {
  //for 16mhz
//  ICR1 = 49;
//  OCR1A = 7;
// for 8mhz
  ICR1 = 46 / 2;
  OCR1A = 6 / 2;
  TCCR1A = (1 << WGM11) | (0 << WGM10) | (1 << COM1A1) | (1 << COM1A0);
  TCCR1B = (0 << CS12) | (1 << CS11) | (0 << CS10) | (1 << WGM12) | (1 << WGM13);
}

void setupTimer0() {
//  TCNT0 = 255 - 74;//181 for 16mhz
  TCNT0 = 255 - 36;//for 8mhz
//  TCCR0A = (0 << WGM01) | (0 << WGM00);
//  TCCR0B = (0 << CS02) | (1 << CS01) | (1 << CS00) | (0 << WGM02);
  TCCR0 = (0 << CS02) | (1 << CS01) | (1 << CS00);
  //Set interrupt on compare match
//  TIMSK0 = 1 << TOIE0;
  TIMSK = 1 << TOIE0;
}

//Max 63 transa
volatile byte start = 0;
volatile byte cur = 0;
volatile char binary[11];
//volatile char binary[] = "01000110001";
volatile bool pause = false;
unsigned int id = 0;

//1 3 6 10 15 21 28 36 45 55 66 78 91 105 120 136 153 171 190 210 231 253 276
//300 325 351 378 406 435 465 496 528 561 595 630 666 703 741 780 820 861 903
//946 990 1035 1081 1128 1176 1225 1275 1326 1378 1431 1485 1540 1596 1653

ISR (TIMER0_OVF_vect) {
  if (binary[cur] == 0) {
    TCCR1A ^= ((1 << COM1A1) | (1 << COM1A0));
//    TCNT0 = 181;//for 16mhz
    TCNT0 = 255-36;//for 8 mhz
  } else {
    TCCR1A ^= ((1 << COM1A1) | (1 << COM1A0));
//    TCNT0 = 107;//181 - 74 for 16mhz
    TCNT0 = 255-36*2;//for 8mhz
  }
  cur++;
  if (cur > 11) {
    pause = true;
  }
}

void convert(unsigned int val) {
  byte i;
  for (i = 0; i < 11; i++) {
    binary[10 - i] = val & 1;
    val >>= 1;
  }
}

void delay_pause(int ms){
   for(;ms;ms--)
      _delay_ms(1);
}

int main(void) {
  //output
  DDRB = (1 << PB1) | (1 << PB4) | (1 << LED);
  //input
  DDRB &= ~(1 << PB3);
  //pull up pb3
  PORTB = (1 << PB3);
  //set 0 to pb1 and pb4
  PORTB &= ~(1 << PB4) & ~(1 << PB1);
  _delay_ms(10);
  //check for random
  if (~PINB & (1 << PB3)) {
    while (~PINB & (1 << PB3)) {
      PORTB ^= (1 << LED);
      _delay_ms(45);
      start++;
      if (start > 63) {
        start = 1;
        id = 0;
      }
      id = id + start;
    }
    //write eeprom
    eeprom_write_word((uint16_t*)1, id);
  } else {
    id = eeprom_read_word((uint16_t*)1);
  }

  //power up led
  PORTB |= (1 << LED);
  convert(id);
  cli();
  setupPWM();
  setupTimer0();
  sei();
  while (1) {
    if (pause) {
      cli();
      TCCR1A &= ~((1 << COM1A1) | (1 << COM1A0));
      delay_pause(12 + rand() % 10);
      pause = false;
      cur = 0;
      TCNT0 = 255-36;
      sei();
    }
  }
}
