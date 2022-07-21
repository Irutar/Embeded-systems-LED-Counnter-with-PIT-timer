#define PIO_PERB (volatile unsigned int*) 0xFFFFF400
#define PIO_OERB (volatile unsigned int*) 0xFFFFF410
#define PIO_CODRB (volatile unsigned int*) 0xFFFFF434
#define PIO_SODRB (volatile unsigned int*) 0xFFFFF430

#define LED_A 1UL<<25
#define LED_B 1UL<<24
#define LED_C 1UL<<22
#define LED_D 1UL<<21
#define LED_E 1UL<<20
#define LED_F 1UL<<27
#define LED_G 1UL<<26
#define LED_DOT 1UL<<23
#define ALL_LEDS LED_A | LED_B | LED_C | LED_D | LED_E | LED_F | LED_G | LED_DOT
#define LCD_BRIGHTNESS 1UL<<29
#define LEFT_DIGIT 1UL<<28
#define RIGHT_DIGIT 1UL<<30
#define ALL_TRANSISTORS LCD_BRIGHTNESS | LEFT_DIGIT | RIGHT_DIGIT
#define ALL_LEDS_AND_TRANSISTORS ALL_LEDS | ALL_TRANSISTORS

#define DIGIT_DISPLAY_TIME_IN_MS 10
#define MAXIMUM_NUMBER_OF_DIGITS 10 //This number should never be less than it's current value.
#define MAXIMUM_NUMBER_TO_DISPLAY 99 //The LCD display can only handle 2 digits, therefore the maximum displayable number is 99.
#define DELAY_BETWEEN_INCREMENT_IN_MS 1000

#ifndef _ATMEL_H
#define _ATMEL_H
#include <AT91SAM9263.h>
#endif

#define PIT_REFERENCE_1_MS 6250UL

typedef struct digit
{
  unsigned long bit_pattern;
} Digit;

typedef struct digits
{
  Digit digit[MAXIMUM_NUMBER_OF_DIGITS];
} Digits;

void dbgu_print_ascii(const char* a){}

void init_PIT()
{
  AT91C_BASE_PITC->PITC_PIMR = PIT_REFERENCE_1_MS; //This disables the interrupts and the timer as a side-effect
  AT91C_BASE_PITC->PITC_PIVR; //This clears the status register and counter as a side-effect
}

void delay_using_PIT()
{
  AT91C_BASE_PITC->PITC_PIMR |= AT91C_PITC_PITEN;
  while(AT91C_BASE_PITC->PITC_PISR == (AT91C_BASE_PITC->PITC_PISR & (~AT91C_PITC_PITS)))
   // while (AT91C_BASE_PITC->PITC_PISR & AT91C_PITC_PITS == 0)
    ;
  AT91C_BASE_PITC->PITC_PIMR &= (~AT91C_PITC_PITEN);
  AT91C_BASE_PITC->PITC_PIVR;
}

void delay_ms(unsigned long delay)
{
  volatile unsigned long i = 0;
  for (; i< delay; ++i)
    delay_using_PIT();
}

void clear_LCD_display()
{
  *PIO_CODRB = ALL_LEDS;
}

void init_LCD_display()
{
  *PIO_PERB = ALL_LEDS_AND_TRANSISTORS;
  *PIO_OERB = ALL_LEDS_AND_TRANSISTORS;
  *PIO_SODRB = LEFT_DIGIT | RIGHT_DIGIT;
  *PIO_CODRB = LCD_BRIGHTNESS;
  clear_LCD_display();
}

void disable_LCD_display()
{
  *PIO_SODRB = LEFT_DIGIT | RIGHT_DIGIT;
}

void enable_left_digit_display_only()
{
  *PIO_SODRB = RIGHT_DIGIT;
  *PIO_CODRB = LEFT_DIGIT;
}

void enable_right_digit_display_only()
{
  *PIO_SODRB = LEFT_DIGIT;
  *PIO_CODRB = RIGHT_DIGIT;
}

void init_digits(Digits* digits)
{
  digits->digit[0].bit_pattern = LED_A | LED_B | LED_C | LED_D | LED_E | LED_F;
  digits->digit[1].bit_pattern = LED_B | LED_C;
  digits->digit[2].bit_pattern = LED_A | LED_B | LED_G | LED_E | LED_D;
  digits->digit[3].bit_pattern = LED_A | LED_B | LED_C | LED_D | LED_G;
  digits->digit[4].bit_pattern = LED_F | LED_G | LED_B | LED_C;
  digits->digit[5].bit_pattern = LED_A | LED_F | LED_G | LED_C | LED_D;
  digits->digit[6].bit_pattern = LED_A | LED_F | LED_E | LED_C | LED_D | LED_G;
  digits->digit[7].bit_pattern = LED_A | LED_B | LED_C;
  digits->digit[8].bit_pattern = LED_A | LED_B | LED_C | LED_D | LED_E | LED_F | LED_G;
  digits->digit[9].bit_pattern = LED_A | LED_B | LED_C | LED_D | LED_F | LED_G;
}

void display_digit(Digit digit)
{
  clear_LCD_display();
  *PIO_SODRB = digit.bit_pattern;
}



int main()
{
  Digits digits;
  init_digits(&digits);
  
  init_LCD_display();
  init_PIT();

  unsigned char two_digit_number_to_display = 0;
  unsigned long delay = 0;
  while (1) {
    for (delay = 0; delay < DELAY_BETWEEN_INCREMENT_IN_MS; delay += 2 * DIGIT_DISPLAY_TIME_IN_MS) {
      disable_LCD_display();
      display_digit(digits.digit[two_digit_number_to_display / 10]);
      enable_left_digit_display_only();
      delay_ms(DIGIT_DISPLAY_TIME_IN_MS);

      disable_LCD_display();
      display_digit(digits.digit[two_digit_number_to_display % 10]);
      enable_right_digit_display_only();
      delay_ms(DIGIT_DISPLAY_TIME_IN_MS);
    }
    if (++two_digit_number_to_display >= MAXIMUM_NUMBER_TO_DISPLAY) {
      two_digit_number_to_display = 0;
    }
  }
}