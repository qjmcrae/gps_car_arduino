// ************************   COMPASS FUNCTIONS   ************************//

// This file contains various functions for the compass



// ************************   GET_COMPASS_DATA   ************************//

void get_compass_data(float target_lat, float target_lon) {
  int compass_heading;
  if (hmc_flag) {
    sensors_event_t event;
    compass_HMC.getEvent(&event);
    compass_heading = atan2(event.magnetic.y, event.magnetic.x) * 180.0 / M_PI;  // - compass_offset;
  } else {
    compass_QMC.read();
    compass_heading = compass_QMC.getAzimuth();
  }
  compass_heading = compass_heading - compass_offset;
  gps_heading = atan2(target_lon - gps.location.lng(), target_lat - gps.location.lat()) * 180.0 / M_PI;

  // Once you have your compass_heading, you must then add your 'Declination Angle', which
  // is the 'Error' of the magnetic field in your location in radians.
  // Find yours here: http://www.magnetic-declination.com/
  // Salt Lake is 11°0', or 11.0°

  float declinationAngle = 11.0;
  compass_heading += declinationAngle;

  car_heading = compass_heading;
  // Once going above a certain speed, switch to GPS angle - hopefully overcomes crappy compass reading
  // qj - not sure yet...  Seems like a good idea, but first pass had weird effects, and haven't applied yet
  // if (gps.speed.mph() > 4 && dist_to_target > 5) car_heading = gps.course.deg();

  // Check for wrap due to addition of declination or subtraction of offset.
  // define car heading between ±180°.  Could also be 0 -> 360, but doing other.
  if (abs(car_heading) > 180) {
    if (car_heading > 180) {
      car_heading -= 360;
    }
    if (car_heading < -180) {
      car_heading += 360;
    }
  }

}  //End of get_compass_data



// ************************   STOP_NO_COMPASS   ************************//

// This code is meant to be called if the compass is not connected
// It gives the "Compass NOT detected" error on LCD
void stop_no_compass() {
  lcd.clear();
  lcd.setCursor(0, 1);
  lcd.print(F("Compass NOT detected"));
  lcd.setCursor(0, 2);
  lcd.print(F("Program will not"));
  lcd.setCursor(0, 3);
  lcd.print(F("continue ..."));
  while (1) {
    Serial.println(F("Ooops, no Compass detected ... Check your wiring!"));
  }
}  //End of stop_no_compass


// ********************  CALIBRATE COMPASS  ****************************//

// This is currently a 1-time deal - must comment out call to this for normal use
// Want the min/max x and y readings from the compass for calibration

void calibrate_compass() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("Rotate car 360°"));
  while (1)  //
  {
    static float max_x;
    static float min_x;
    static float max_y;
    static float min_y;
    static float max_z;
    static float min_z;

    if (hmc_flag)  //
    {
      sensors_event_t event;
      compass_HMC.getEvent(&event);
      max_x = max(max_x, event.magnetic.x);
      min_x = min(min_x, event.magnetic.x);
      max_y = max(max_y, event.magnetic.y);
      min_y = min(min_y, event.magnetic.y);
      max_z = max(max_z, event.magnetic.z);
      min_z = min(min_z, event.magnetic.z);
      lcd.setCursor(0, 1);
      lcd.print("x:");
      lcd.print(min_x,1);
      lcd.print(",");
      lcd.print(max_x,1);
      lcd.print(",");
      lcd.print( (min_x + max_x) / 2,2 );

      lcd.setCursor(0, 2);
      lcd.print("y:");
      lcd.print(min_y,1);
      lcd.print(",");
      lcd.print(max_y,1);
      lcd.print(",");
      lcd.print( (min_y + max_y) / 2, 2);

      lcd.setCursor(0, 3);
      lcd.print("z:");
      lcd.print(min_z,1);
      lcd.print(",");
      lcd.print(max_z,1);
      lcd.print(",");
      lcd.print( (min_z + max_z) / 2, 2);

      // delay(250);
    }     //
    else  //
    {
      compass_QMC.calibrate();
    }
  }
}