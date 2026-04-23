// ************************   SENSOR FUNCTIONS   ************************//

// This file contains the various functions that the arduino uses to
// monitor the state of the car and its surroundings using the various
// sensors on the car, not including the gps and compass



// ************************   CALC_BATT_VOLTAGE   ************************//

// This function should calculate the total battery voltage using a voltage divider on pin A1,
// and cell 1 voltage using a voltage divider on pin A0.
// It assume the voltage dividers has values of R1, R2, R3, and R4
void calc_batt_voltage() {
  calc_batt_time += calc_batt_delay;
  const int R1 = 470;
  const int R2 = 150;
  float tot_batt_scale = 1.01;  // Emperically measured value
  analogRead(batt_volt_pin);    // quick "primer" read to get it moving since we have extremely high resistors (see chatGPT)
  float tmp1 = analogRead(batt_volt_pin) * 3.3 / 1024.0;

  // simple low-pass filter - simply combine current reading with previous reading
  float alpha = 0.95;
  volts_total = alpha * volts_total + (1 - alpha) * tmp1 * (R1 + R2) / R2 * tot_batt_scale;
  volts_min = min(volts_total, volts_min);

  LOW_BATTERY = 0;
  if (volts_total < low_voltage_threshold) LOW_BATTERY = 1;

}  //End of calc_batt_voltage



// ************************   CALC_RPM   ************************//

// Calculates rpm of encoder AND wheel
void calc_rpm() {
  static unsigned long last_micros_rpm = 0;
  unsigned long micros_now = micros();
  // rpm_encoder = hall_count / 6 (to revolutions) / delta_t (in micro-seconds) * 1E6 (to seconds) * 60 (to minutes) -> rotations/minute (RPM)
  rpm_encoder = 1.0E7 * hall_count / (micros_now - last_micros_rpm);
  rpm_wheel = rpm_encoder / 2.6;  // gearbox has 2.6:1 reduction from slipper (encoder) and wheel
  // 6 reads\revolution, measured in micro-seconds...
  hall_count = 0;
  last_micros_rpm = micros_now;
}  //End of calc_rpm



// ************************   GET_LIDAR_DATA   ************************//

// Using front Lidar sensor, guestimate the distance to any object in front
void get_lidar_data() {
  int16_t dist_cm;                            // Leave as int16_t
  luna.getData(dist_cm, lidar_adr);           // Gets distance data from lidar sensor in cm
  dist_lidar = float(dist_cm) / 2.54 / 12.0;  // Returns the distance in ft
}  //End of get_lidar_data


// ************************   CALC_AVOIDCANCE_ANGLE   ************************//

float calc_avoidance_angle() {
  // Note:  avoid_heading should be an integer, but to make filter work MUST be a floating point
  // if something is in front, determine angle to avoid object, and decay at an appropriate rate to avoid it
  static int avoid_dir = 1;
  static bool set_dir_flag = 0;
  if (dist_lidar > 10 || dist_lidar < 3)  // there is nothing in front, so either decay angle, or just do nothing
  {
    // low pass filter - slowly decay it over time.  alpha determined by experiment, but is up for debate
    float alpha = 0.008;
    avoid_heading = (1 - alpha) * avoid_heading;
    if (abs(avoid_heading) < 3)  //
    {
      avoid_heading = 0;
      if (set_dir_flag)  //
      {
        avoid_dir = -avoid_dir;
        set_dir_flag = 0;
      }
    }
  }     //
  else  // Something is in the way - determine angle to drive around
  {
    set_dir_flag = 1;
    avoid_heading = avoid_dir * 90 * exp(-0.145 * dist_lidar);  // curve fit reasonable data to find equation
  }
  return avoid_heading;
}


// ************************   SET_STEERING   ************************//

// Sets the steering trim of the cars
void set_steering(int range) {
  long now = millis();
  // static bool isclear = 0;
  // if (!isclear) lcd.clear(), isclear = 1;
  lcd.clear();
  while (millis() < now + servo_trim_time * 1000) {
    int t_minus = servo_trim_time - (millis() - now) / 1000;
    lcd.setCursor(0, 0);
    lcd.print(F("Returning in        "));
    lcd.setCursor(14, 0);
    lcd.print(t_minus);
    lcd.print(F(" sec"));
    lcd.setCursor(2, 1);
    lcd.print(F("Set Steering Pot"));
    // Serial.println(F("Set Steering Pot"));
    steering_trim = map(analogRead(steering_trim_pin), 0, 1023, -range, range);  // potentially change to pwm values 1000 - 2000
    servo_straight = 90 + steering_trim;                                         // Note:  Can't be servo_straight = servo_straight + steering_trim for this step
    servo_left = 55 + steering_trim;
    servo_right = 125 + steering_trim;
    steering_servo.write(servo_straight);
    lcd.setCursor(0, 3);
    lcd.print(F("Trim:               "));
    lcd.setCursor(5, 3);
    lcd.print(steering_trim);
    lcd.print(" / ");
    lcd.print(servo_straight);
    delay(100);
  }
  lcd.clear();
}  //End of set_steering