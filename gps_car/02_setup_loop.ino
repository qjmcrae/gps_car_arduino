///////////////////////////////////////////////////////////////////////////
//
//                                void setup
//
///////////////////////////////////////////////////////////////////////////
void setup() {
  Serial1.begin(9600);
  WiFi.lowPowerMode();

  // Serial.begin(9600);
  // Serial.println(F("Starting Setup"));
  // delay(1000);

  pinMode(LEDR, OUTPUT);  // LED(27)
  pinMode(LEDG, OUTPUT);  // LED(25)
  pinMode(LEDB, OUTPUT);  // LED(26)

  pinMode(Clk, INPUT);                   // D2
  pinMode(Dt, INPUT);                    // D3
  pinMode(Sw, INPUT_PULLUP);             // D4
  pinMode(pixel_pin, OUTPUT);            // D8
  pinMode(esc_servo_pin, OUTPUT);        // D9
  pinMode(steering_servo_pin, OUTPUT);   // D10
  pinMode(green_car_pin, INPUT_PULLUP);  // D11
  pinMode(hall_pin, INPUT);              // D12
  pinMode(buzzer_pin, OUTPUT);           // D13

  pinMode(batt_volt_pin, INPUT);      // A1
  pinMode(steering_trim_pin, INPUT);  // A2

  attachInterrupt(digitalPinToInterrupt(Clk), isr_encoder, FALLING);    //D2
  attachInterrupt(digitalPinToInterrupt(hall_pin), isr_hall, FALLING);  //D12


  // check for student entered craziness and fix
  min_dist_to_tgt = constrain(min_dist_to_tgt, 1, 10);     // in case students do anything crazy!
  delay_at_target = constrain(delay_at_target, 1, 10);     // same
  servo_wag_speed = constrain(servo_wag_speed, 100, 500);  // same
  neo_red = constrain(neo_red, 0, 255);                    // same
  neo_green = constrain(neo_green, 0, 255);                // same
  neo_blue = constrain(neo_blue, 0, 255);                  // same
  // delay(250);

  // I2C addresses can be in decimal or hexadecimal
  Wire.begin();
  delay(250);

  // Test to see which LCD screen is attached - only ones I am aware of are 0x27 and 0x3F
  lcd = lcd_0x27;     // Assume it is the 0x27, then if 0x3F is there, switch...
  byte address = 63;  // This is 0x3F
  Wire.beginTransmission(address);
  byte error = Wire.endTransmission();
  if (error == 0) {  // this means there is a 0x3F - if it is there, assume it is the only one there
    lcd = lcd_0x3F;
  }

  // Initialize LCD
  // Serial.println(F("lcd.begin(20, 4)"));
  lcd.init();
  lcd.begin(20, 4);
  lcd.setBacklight(HIGH);
  lcd.clear();
  disp_lcd_info();
  // delay(100);

  // Servo Setup
  steering_servo.attach(steering_servo_pin);
  esc_servo.attach(esc_servo_pin);
  delay(250);


  // Fix Steering offsets

  ///////////////////////////////////////////////////  WARNING  //////////////////////////////////////
  //
  // For this to make sense, you MUST run it on each car individually to set the pot, then leave the
  // pot in that same position from then on.  If it gets moved, or if it is not run, it will make
  // the wheels point in weird directions!
  //
  ////////////////////////////////////////////////////////////////////////////////////////////////////

  steering_trim = map(analogRead(steering_trim_pin), 0, 1023, -servo_trim_range, servo_trim_range);  // potentially change to pwm values 1000 - 2000
  servo_straight = servo_straight + steering_trim;
  servo_left = servo_left + steering_trim;
  servo_right = servo_right + steering_trim;


  //  /* Initialize the compass */
  //  Test for compass, and which one...
  //  Test for HMC5883 Compass (Honeywell version, requires adafruit library
  //  or QMC5883, chinese follow-on to HMC Chip (which is no longer made)
  address = 30;  // This is 0x1E, for the HMC5883 Chips (NOT the QMC5883 chips)
  Wire.beginTransmission(address);
  error = Wire.endTransmission();
  if (error == 0)  // this means there is IS 0x1E - if it is there, compass (HMC) is plugged in
  {
    compass_HMC.begin();  // initialize HMC compass
    hmc_flag = 1;
  }     //
  else  // Either there is no HMC compass, or it is the QMC...
  {
    address = 13;  // This is 0x0D, for the QMC5883 Chips
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
    if (error == 0)  // this means there is IS 0x0D - if it is there, compass (QMC) is plugged in
    {
      compass_QMC.init();
      hmc_flag = 0;
    }     //
    else  //  No compass has been found - stop the program...
    {
      stop_no_compass();
    }
  }
  delay(100);


  //neo-pixel initialization
  neo_pixel.begin();
  neo_pixel.show();  // Initialize all pixels to 'off'

  neo_design(1);  // (do some interesting stuff)
  delay(1000);    // make sure to write above for at least some time ...

  for (int j = 0; j < 8; j++) {
    neo_pixel.setPixelColor(j, neo_pixel.Color(neo_red, neo_green, neo_blue));
  }
  neo_pixel.show();

  delay(1000);

  disp_time = millis() + 1500;  // Don't change this display for extra 1.5 seconds
  LCD_screen = 2;               // go to the next LCD Screen

  neo_design(0);  // turn neo_pixel off



  // calibrate_compass();
  // Serial.println(F("end of setup"));
}  // end of setup
// ************************   END SETUP   ************************//




///////////////////////////////////////////////////////////////////////////
//                                                                       //
//                                void loop                              //
//                                                                       //
///////////////////////////////////////////////////////////////////////////
// ************************   BEGIN LOOP   ************************//
void loop() {

  // long press encoder button (> 2 sec) to enter steering adjust mode
  unsigned long now = millis();  // so I don't keep calling millis for the next few logical steps...
  static long last_Sw = now;
  if (!digitalRead(Sw))  // pressing switch
  {
    if ((now - last_Sw) > 2000) set_steering(servo_trim_range);
  }  // switch is not on
  else
    last_Sw = now;
  static unsigned long brake_time;
  float target_lat = target_lats[ind_gps];
  float target_lon = target_longs[ind_gps];

  // if gps not there after 15 seconds, stop the program ...
  if (millis() > 15000 && gps.charsProcessed() < 10)
    stop_no_gps();  // Stop the program, display "Check Wiring" error

  while (Serial1.available() > 0) {
    gps.encode(Serial1.read());
  }


  // Do the following functions need to be in a timer?

  // get gps data ...
  get_gps_data(target_lat, target_lon);

  // get compass data ...
  get_compass_data(target_lat, target_lon);

  // Check battery status..
  if (millis() > calc_batt_time) calc_batt_voltage();

  // get distance from LIDAR sensor
  get_lidar_data();

  if (dist_lidar > 20 || dist_lidar < 3) avoid_heading = 0;
  else avoid_heading = 90 * exp(-0.15 * dist_lidar);

  desired_heading = gps_heading + avoid_heading;

  // calculate heading error, or difference between where we are pointed and where we want to point
  heading_error = car_heading - desired_heading;
  // wrap heading_error so it is between -180 and 180
  if (heading_error < -180) {
    heading_error += 360;
  }
  if (heading_error > 180) {
    heading_error += -360;
  }


  //  ****************   Determine car state...    ********************  //

  // Low Battery
  if (LOW_BATTERY) currentState = STATE_LOW_BATTERY;

  // No GPS
  else if (!gps.location.isValid() || gps.location.age() > 1250) currentState = STATE_NO_GPS;

  // There is an obstacle too close - stop!
  else if (dist_lidar < 3 && dist_lidar > 0) currentState = STATE_OBSTACLE_STOP;

  // // There is an obstacle not too close - go around
  // else if (dist_lidar < 3 && dist_lidar > 0) currentState = STATE_OBSTACLE_AVOID;

  // At target
  else if (fabs(dist_to_target) < min_dist_to_tgt) currentState = STATE_AT_TARGET;

  // Driving
  else currentState = STATE_DRIVING;

  if (millis() > servo_write_time) {
    switch (currentState) {

      case STATE_LOW_BATTERY:
        esc_servo.write(esc_stop);
        steering_servo.write(servo_straight); 
        digitalWrite(buzzer_pin,1);   // just beep!
        break;


      case STATE_NO_GPS:
        flash(LEDR, 250, 250, 255, 0, R);  // flash red lights until GPS acquired
        // set wheels straight, servo stopped
        servo_command = servo_straight;
        esc_command = esc_stop;
        LCD_screen = 6;  // this is the battery screen - force it to display what is happening
        break;


      case STATE_OBSTACLE_STOP:
        digitalWrite(buzzer_pin, 0);  // in case it is mid-beep...
        int flash_on;
        servo_command = servo_straight;
        if (brake_time + 2000 > millis()) {
          esc_command = esc_full_reverse;
          flash_on = 30;  // bright flashing to tell just sittin' there
        } else {
          esc_command = esc_stop;
          flash_on = 255;  // dim flashing to tell under brakes
        }
        // maybe change to neo_pixel
        flash(LEDR, 250, 150, 0, flash_on, R);  // flash lights on/off in Purple (red and blue)
        // flash(LEDG, 250, 150, 0, flash_on, G);
        digitalWrite(LEDG, LOW);
        flash(LEDB, 250, 150, 0, flash_on, B);

        // prev_esc_Time = micros();
        // prev_servo_Time = millis();
        break;


      case STATE_AT_TARGET:
        ind_gps++;

        if (!armed)  // 1st target - arm after "getting in the box"
        {
          armed = 1;
          wag_servo(150);  // arming warning ...
          countdown();     // blink, beep, display countdown ...
        }                  //
        else               // Already armed, or in other words, going for it and found target (targets #2 - end) ...
        {
          // esc_command = esc_full_reverse;  // Brake at target
          esc_servo.write(esc_full_reverse);  // Brake at target

          wag_servo(servo_wag_speed, delay_at_target);  // Signal at target

          if (ind_gps == num_gps_tgts)  // This means it got to the last one, so STOP!!!!
          {
            lcd.begin(20, 4);
            lcd.clear();
            lcd.setCursor(0, 1);
            lcd.print(F("COURSE COMPLETE!!!"));

            esc_servo.write(esc_stop);
            wag_servo(100);

            while (1)  // make sure you never go back!!!
            {
              flash(LEDR, random(50, 250), random(50, 250), 0, 255, R);
              flash(LEDG, random(50, 250), random(50, 250), 0, 255, G);
              flash(LEDB, random(50, 250), random(50, 250), 0, 255, B);
              if (millis() > neo_time) {
                neo_design(1);
              }
              // beep();
              delay(5000);
            }
          }
          // if not at last one ...
          digitalWrite(LEDR, LOW);
          digitalWrite(LEDG, LOW);
          for (int i = 0; i < 10; i++)  // flash blue when at target
          {
            digitalWrite(LEDB, HIGH);
            delay(100);
            digitalWrite(LEDB, LOW);
            delay(100);
          }  // end flash blue lights

          servo_command = servo_straight;
          esc_command = esc_stop;

        }  // end Armed / not armed on finding target
        break;


      case STATE_DRIVING:
        if (beeped != 1) beep();  // beep first time it acquires GPS
        brake_time = millis();    // update brake time

        int kp = 1;  // default to gain of 1 for GPS Car, primarily to set direction if using old test platform...
        steer_command = constrain(servo_straight - kp * heading_error, servo_left, servo_right);
        servo_command = steer_command;

        if (pid_trigger == 0)  // Pick between hard coded and pid - this is open loop / hard-coded
        {
          int esc_forward = esc_slow_pavement;
          esc_command = esc_forward;  // go fairly slow

        }     //
        else  // Use PID to determine throttle...
        {
          if (dist_to_target > 30) target_speed = 6;       // more than 30 m away, go faster
          else if (dist_to_target > 10) target_speed = 5;  // closer than 30m, but further than 10m
          else target_speed = 3.5;                         // closer than 10m

          pid_command = esc_pid(target_speed);
          pid_command = constrain(pid_command, esc_default, esc_full_forward);  // limit output of pid, to reasonable values
          esc_command = pid_command;
        }
        if (!armed) esc_command = esc_stop;

        digitalWrite(LEDR, HIGH);
        digitalWrite(LEDG, HIGH);
        digitalWrite(LEDB, HIGH);

        //  beep progressively quicker as we get closer - this may not work given the delay of how often this is called ...
        long beep_delay = constrain(map(dist_to_target, 0, 40, 50, 5000), 0, 5000);  // beep faster as it gets closer to the target
        unsigned long now = millis();
        static unsigned long beep_on = 0;
        static unsigned long beep_off;
        if (now > beep_on) {
          beep_on = now + beep_delay;
          digitalWrite(buzzer_pin, 1);
          beep_off = now + 40;  // the 40 is how long it beeps every time, the "beep_delay" is how long it waits between beeps
        }
        if (now > beep_off) digitalWrite(buzzer_pin, 0);
        break;
    }
    steering_servo.write(servo_command);
    esc_servo.write(esc_command);
    servo_write_time = servo_write_time + servo_write_delay;
  }
  now = millis();
  // if (now > neo_time) neo_design(1);
  neo_design(2000);
  // neo_design(911);
  if (now > disp_time) disp_lcd_info();  // display info to LCD screen
}  // End of loop














// // check if have GPS lock yet (or if it has been too long since update) - if not, let user know
// //  if (!gps.location.isValid())
// if (!gps.location.isValid() || gps.location.age() > 1250) {
//   flash(LEDR, 250, 250, 255, 0, R);  // flash red lights until GPS acquired
//   // set wheels straight, servo stopped
//   steering_servo.write(servo_straight);
//   esc_servo.write(esc_stop);
// }  //
// else  // have (current) gps
// {
//   if (beeped != 1) beep();  // beep first time it acquires GPS

//   //  update servo command every so often ...
//   if (millis() > servo_write_time) {

//     if (fabs(dist_to_target) < min_dist_to_tgt)  // check if we're there! Are we close enough?
//     {
//       ind_gps++;

//       if (!armed)  // 1st target - arm after "getting in the box"
//       {
//         armed = 1;
//         wag_servo(150);  // arming warning ...
//         countdown();     // blink, beep, display countdown ...
//       }                  //
//       else               // Already armed, or in other words, going for it and found target (targets #2 - end) ...
//       {
//         esc_servo.write(esc_full_reverse);  // Brake at target

//         wag_servo(servo_wag_speed, delay_at_target);  // Signal at target

//         if (ind_gps == num_gps_tgts)  // This means it got to the last one, so STOP!!!!
//         {
//           lcd.begin(20, 4);
//           lcd.clear();
//           lcd.setCursor(0, 1);
//           lcd.print(F("COURSE COMPLETE!!!"));

//           esc_servo.write(esc_stop);
//           wag_servo(100);

//           while (1)  // make sure you never go back!!!
//           {
//             flash(LEDR, random(50, 250), random(50, 250), 0, 255, R);
//             flash(LEDG, random(50, 250), random(50, 250), 0, 255, G);
//             flash(LEDB, random(50, 250), random(50, 250), 0, 255, B);
//             if (millis() > neo_time) {
//               neo_design(1);
//             }
//             // beep();
//             delay(5000);
//           }
//         }
//         // if not at last one ...
//         digitalWrite(LEDR, LOW);
//         digitalWrite(LEDG, LOW);
//         for (int i = 0; i < 10; i++)  // flash blue when at target
//         {
//           digitalWrite(LEDB, HIGH);
//           delay(100);
//           digitalWrite(LEDB, LOW);
//           delay(100);
//         }  // end flash blue lights

//         servo_command = servo_straight;
//         esc_command = esc_stop;

//       }  // end Armed / not armed on finding target

//     }                                           // end within 2m of target ...
//     else if (dist_lidar < 3 && dist_lidar > 0)  // Something is in the way...
//     {                                           // if we came close to something, 1st hit the brakes for 2 seconds, then just set to 0 ...
//       int flash_on;
//       servo_command = servo_straight;
//       if (brake_time + 2000 > millis()) {
//         esc_command = esc_full_reverse;
//         flash_on = 30;  // bright flashing to tell just sittin' there
//       } else {
//         esc_command = esc_stop;
//         flash_on = 255;  // dim flashing to tell under brakes
//       }
//       // maybe change to neo_pixel
//       flash(LEDR, 250, 150, 0, flash_on, R);  // flash lights on/off in Purple (red and blue)
//       // flash(LEDG, 250, 150, 0, flash_on, G);
//       digitalWrite(LEDG, LOW);
//       flash(LEDB, 250, 150, 0, flash_on, B);

//       // prev_esc_Time = micros();
//       // prev_servo_Time = millis();

//     }     //
//     else  // Not close to target, nothing in the way, so steer to gps target, lights on white
//     {
//       brake_time = millis();  // update brake time

//       int kp = 1;  // default to gain of 1 for GPS Car, primarily to set direction if using old test platform...
//       steer_command = constrain(servo_straight - kp * heading_error, servo_left, servo_right);
//       servo_command = steer_command;

//       if (!armed) rpm = calc_mag_rpm();  // calc rpm while not armed - otherwise called in PID function
//       else                               // determine throttle command, either using PID or Open Loop
//       {
//         if (pid_trigger == 0)  // Pick between hard coded and pid - this is open loop / hard-coded
//         {
//           int esc_forward = esc_slow_pavement;
//           //  set esc command based on how far away from target...
//           esc_command = esc_forward;  // go fairly slow
//           // if (dist_to_target > 30) {
//           //   esc_command = esc_forward + .1 * esc_default;  // more than 30m away, go faster than normal
//           // } else if (dist_to_target > 10) {
//           //   esc_command = esc_forward + .05 * esc_default;  // more than 10m away, go a bit faster than normal
//           // }

//           // if we are going too fast, slow back down...
//           // if (gps.speed.mph() > 7.5) {
//           //   esc_command = esc_forward;
//           // }
//         }     //
//         else  // Use PID to determine throttle...
//         {
//           if (dist_to_target > 30) target_speed = 6.5;     // more than 30 m away, go faster
//           else if (dist_to_target > 10) target_speed = 5;  // closer than 30m, but further than 10m
//           else target_speed = 4;                           // closer than 10m

//           pid_command = esc_pid(target_speed);

//           pid_command = constrain(pid_command, esc_default, esc_full_forward);  // limit output of pid, to reasonable values
//           esc_command = pid_command;
//         }
//       }

//       digitalWrite(LEDR, HIGH);
//       digitalWrite(LEDG, HIGH);
//       digitalWrite(LEDB, HIGH);

//       //  beep progressively quicker as we get closer - this may not work given the delay of how often this is called ...
//       long beep_delay = constrain(map(dist_to_target, 0, 40, 50, 5000), 0, 5000);  // beep faster as it gets closer to the target
//       unsigned long now = millis();
//       static unsigned long beep_on = 0;
//       static unsigned long beep_off;
//       if (now > beep_on) {
//         beep_on = now + beep_delay;
//         digitalWrite(buzzer_pin, 1);
//         beep_off = now + 40;  // the 40 is how long it beeps every time, the "beep_delay" is how long it waits between beeps
//       }
//       if (now > beep_off) digitalWrite(buzzer_pin, 0);
//     }

//     steering_servo.write(servo_command);
//     esc_servo.write(esc_command);
//     servo_write_time = servo_write_time + servo_write_delay;
//   }  // end servo_write_time
// }  // end have gps


// now = millis();
// // if (now > neo_time) neo_design(1);
// neo_design(2000);
// // neo_design(911);
// if (now > disp_time) disp_lcd_info();  // display info to LCD screen

// }  // end of loop
//    // ************************   END LOOP   ***************