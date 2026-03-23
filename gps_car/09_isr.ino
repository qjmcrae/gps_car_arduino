// ************************   ISR   ************************//

// This file contains the various ISR's that the arduino uses
// isr == Interrupt Service Routine



// ************************   ISR_ENCODER   ************************//

// isr of encoder to change the lcd screens
void isr_encoder() {
  if (LCD_screen_old == LCD_screen)  //this inherently slows it down a bit so it doesn't jump too many screens at once
  {
    if (digitalRead(Dt) == LOW) {
      LCD_screen--;
    } else {
      LCD_screen++;
    }
  }
  // constrain lcd_screen in disp function
}



// ************************   ISR_HALL   ************************//

// isr for the Hall Effect Sensor
void isr_hall() {
  hall_count++;  // Counts the number of times a magnet passes the hall sensor
}