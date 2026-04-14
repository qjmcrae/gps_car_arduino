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
  float alpha = 0.9;
  volts_total = alpha * volts_total + (1 - alpha) * tmp1 * (R1 + R2) / R2 * tot_batt_scale;
  volts_min = min(volts_total, volts_min);

  LOW_BATTERY = 0;
  if (volts_total < low_voltage_threshold) LOW_BATTERY = 1;

}  //End of calc_batt_voltage



// ************************   CALC_MAG_RPM   ************************//

// Calculates rpm of magnet wheel
// int calc_mag_rpm() {
//   static unsigned long last_micros_rpm = 0;
//   unsigned long micros_now = micros();
//   long deltat = micros_now - last_micros_rpm;
//   // rpm = 6.0E7 * hall_count / (micros_now - last_micros_rpm) / 6.0;
//   rpm = 1.0E7 * hall_count / deltat / 2.6;  // The 2.6 is to take it from teh encoder to the wheel rpm
//   // 6 reads\revolution, measured in micro-seconds...
//   hall_count = 0;
//   last_micros_rpm = micros_now;
//   return rpm;
// }  //End of calc_mag_rpm


// Calculates rpm of magnet wheel
int calc_mag_rpm() {
  static unsigned long last_micros_rpm = 0;
  unsigned long micros_now = micros();
  long deltaT = micros_now - last_micros_rpm;
// qj -   int rpm_magnet = 6.0E7 * hall_count / deltaT / 6.0;
  int rpm_magnet = 1.0E7 * hall_count / deltaT;
  // 6 reads\revolution, measured in micro-seconds...
  float gear_ratio = 2.6;
  int rpm_wheel = rpm_magnet / gear_ratio;
  // hall_count_max = max(hall_count_max, hall_count);
  hall_count = 0;
  last_micros_rpm = micros_now;
  return rpm_wheel;
}  //End of calc_mag_rpm




// ************************   GET_LIDAR_DATA   ************************//

// Using front Lidar sensor, guestimate the distance to any object in front
void get_lidar_data() {
  int16_t dist_cm;                            // Leave as int16_t
  luna.getData(dist_cm, lidar_adr);           // Gets distance data from lidar sensor in cm
  dist_lidar = float(dist_cm) / 2.54 / 12.0;  // Returns the distance in ft
}  //End of get_lidar_data



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