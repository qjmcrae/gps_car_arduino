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


// ************************   ISR_ENCODER   ************************//

// isr of encoder to change the lcd screens
void isr_pid()  // display encoder
{
  static int last_encoder_debounce = 0;
  static int encoder_delay = 250;
  if ((millis() - last_encoder_debounce) > encoder_delay) {
    if (digitalRead(Dt) == LOW)  //~HK new value for gain
      gain -= 0.01;
    else
      gain += 0.01;
    if (gain < 0) gain = 0;

    switch (Sw_count) {  //~HK sets appropriate gain to new value
      case 0:
        Kp = gain;
        break;

      case 1:
        Ki = gain;
        break;

      case 2:
        Kd = gain;
        break;
    }
    last_encoder_debounce = millis();
  }
}
