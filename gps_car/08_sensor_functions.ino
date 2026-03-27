// ************************   SENSOR FUNCTIONS   ************************//

// This file contains the various functions that the arduino uses to
// monitor the state of the car and its surroundings using the various
// sensors on the car, not including the gps and compass



// ************************   CALC_BATT_VOLTAGE   ************************//

// This function should calculate the total battery voltage using a voltage divider on pin A1,
// and cell 1 voltage using a voltage divider on pin A0.
// It assume the voltage dividers has values of R1, R2, R3, and R4
void calc_batt_voltage() {
  const int R1 = 470;
  const int R2 = 150;
  float tot_batt_scale = 1.01;   // Emperically measured value
  analogRead(batt_volt_pin);  // quick "primer" read to get it moving since we have extremely high resistors (see chatGPT)
  float tmp1 = analogRead(batt_volt_pin) * 3.3 / 1024.0;
  volts_total = tmp1 * (R1 + R2) / R2 * tot_batt_scale;

}  //End of calc_batt_voltage



// ************************   CALC_MAG_RPM   ************************//

// Calculates rpm of magnet wheel
float calc_mag_rpm() {
  static unsigned long last_micros_rpm = 0;
  unsigned long micros_now = micros();
  rpm = 6.0E7 * hall_count / (micros_now - last_micros_rpm) / 6;
  // 6 reads\revolution, measured in micro-seconds...
  hall_count = 0;
  last_micros_rpm = micros_now;
  return rpm;
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
  while (millis() < now + servo_trim_time*1000) {
    int t_minus = servo_trim_time - (millis() - now) / 1000;
    static bool isclear = 0;
    if (!isclear) lcd.clear(), isclear = 1;
    lcd.setCursor(0, 0);
    lcd.print(F("Returning in        "));
    lcd.setCursor(14,0);
    lcd.print(t_minus);
    lcd.print(F(" sec"));
    lcd.setCursor(2, 1);
    lcd.print(F("Set Steering Pot"));
    // Serial.println(F("Set Steering Pot"));
    steering_trim = map(analogRead(steering_trim_pin), 0, 1023, -range, range);  // potentially change to pwm values 1000 - 2000
    servo_straight = 90 + steering_trim;  // Note:  Can't be servo_straight = servo_straight + steering_trim for this step
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