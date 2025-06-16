#include <avr/io.h> 
#include <util/delay.h> 

#define STEP_PIN PB2 // Pin connected to STEP on A4988 
#define DIR_PIN PB1 // Pin connected to DIR on A4988 
#define BUTTON_180_PIN PD2 // Button 1 (180° rotation) 
#define BUTTON_360_PIN PD3 // Button 2 (360° rotation) 
#define BUTTON_30_PIN PD4 // Button 3 (30° rotation) 

int f180 = 0; int f360 = 0; int f30 = 0; 

#define STEPS_PER_REV 200 // Number of steps per full revolution for your motor (1.8° per step) 

// Function to initialize the buttons
void buttons_init(void) {
  DDRD &= ~((1 << BUTTON_180_PIN) | (1 << BUTTON_360_PIN) | (1 << BUTTON_30_PIN)); // Set buttons as inputs 
  PORTD |= (1 << BUTTON_180_PIN) | (1 << BUTTON_360_PIN) | (1 << BUTTON_30_PIN); // Enable pull-up resistors 
} 

// Function to read the state of button 1 (180°) 
uint8_t button_180_pressed(void) {
  return !(PIND & (1 << BUTTON_180_PIN)); // Button press is active-low 
}

// Function to read the state of button 2 (360°) 
uint8_t button_360_pressed(void) {
  return !(PIND & (1 << BUTTON_360_PIN)); // Button press is active-low 
}

// Function to read the state of button 3 (30°) 
uint8_t button_30_pressed(void) {
  return !(PIND & (1 << BUTTON_30_PIN)); // Button press is active-low 
}

// Function to initialize the ADC (for speed control) 
void adc_init(void) {
  ADMUX = (1 << REFS0); // Set reference voltage to AVcc, and select ADC0 (PC0 pin) 
  ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1); // Enable ADC, set prescaler to 64 
}

// Function to read the ADC value from ADC0 (PC0) 
uint16_t adc_read(void) {
  ADCSRA |= (1 << ADSC); // Start conversion 
  while (ADCSRA & (1 << ADSC)); // Wait for conversion to complete 
    return ADC; // Return the ADC value (10-bit) 
} 

// Function to provide a variable microsecond delay (loop-based) 
void variable_delay_us(uint16_t delay) {
    while (delay--) {
      _delay_us(1); 
    } 
}

// Function to pulse the step pin
void stepper_step(uint16_t delay) {
  PORTB |= (1 << STEP_PIN); // Set STEP_PIN high 
  _delay_us(1); // Short delay to ensure the pulse is registered 
  PORTB &= ~(1 << STEP_PIN); // Set STEP_PIN low 
  variable_delay_us(delay); // Delay between steps 
}

// Rotate stepper motor forward a number of steps 
void rotate_forward(uint16_t steps, uint16_t delay) {
  PORTB |= (1 << DIR_PIN); // Set direction forward 
  for (uint16_t i = 0; i < steps; i++) {
    stepper_step(delay); 
  } 
} 

int main(void) { 
  // Configure STEP_PIN and DIR_PIN as outputs 
  DDRB |= (1 << STEP_PIN) | (1 << DIR_PIN); 

// Initialize buttons and ADC 
buttons_init(); 
adc_init();

uint16_t adc_value; 
uint16_t step_delay; 
uint16_t steps; 

while (1) { 
  // Read potentiometer value for speed control 
  adc_value = adc_read();
  step_delay = 1023 - adc_value; // Map ADC value to delay (inverted for higher speed) 

  // Check if any button is pressed and set the mode accordingly 
  if (button_180_pressed()) {
    f180 = 1;
    f360 = f30 = 0; // Reset other modes
    steps = 100; // 180° rotation 
    _delay_ms(200); // Debounce delay 
  }
  else if (button_360_pressed()) {
    f360 = 1;
    f180 = f30 = 0; // Reset other modes
    steps = 200; // 360° rotation 
    _delay_ms(200); // Debounce delay 
  } 
  else if (button_30_pressed()) {
    f30 = 1;
    f180 = f360 = 0; // Reset other modes 
    steps = 17; // 30° rotation 
    _delay_ms(200); // Debounce delay 
  }

  // Rotate stepper motor based on the selected mode 
  if (f180) {
    rotate_forward(steps, step_delay); // Continuous rotation for 180° 
  } else if (f360) {
    rotate_forward(steps, step_delay); // Continuous rotation for 360° 
  } else if (f30) {
    rotate_forward(steps, step_delay); // Continuous rotation for 30° 
    } 
  } 
}
