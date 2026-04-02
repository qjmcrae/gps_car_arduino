// ************************   COMPASS FUNCTIONS   ************************//

// This file contains various functions for the compass



// ************************   GET_COMPASS_DATA   ************************//

void get_compass_data(float target_lat, float target_lon) {
  int compass_heading;
  if (hmc_flag) {
    sensors_event_t event;
    compass_HMC.getEvent(&event);
    compass_heading = atan2((event.magnetic.y - offsetY), (event.magnetic.x - offsetX)) * 180.0 / M_PI;  // - compass_offset;
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
  lcd.print(F("Rotate car all directions"));
      float xMin = 9999;
      float zMin = 9999;
      float yMin = 9999;
      float yMax = -9999;
      float xMax = -9999;
      float zMax = -9999;

    if (hmc_flag)  //
    {
      unsigned long start_time = millis();


      while (millis() - start_time < 20000) { // Spin car for 20 seconds so we can get the highest and lowest values
        sensors_event_t event;
        compass_HMC.getEvent(&event);

        if (event.magnetic.x < xMin) xMin = event.magnetic.x;
        if (event.magnetic.x > xMax) xMax = event.magnetic.x;
        if (event.magnetic.y < yMin) yMin = event.magnetic.y;
        if (event.magnetic.y > yMax) yMax = event.magnetic.y;
        if (event.magnetic.z < zMin) zMin = event.magnetic.z;
        if (event.magnetic.z > zMax) zMax = event.magnetic.z;


        delay(10);
      }

      offsetX = (xMax + xMin) / 2;
      offsetY = (yMax + yMin) / 2;
      offsetZ = (zMax + zMin) / 2;


      char finalBuffer[32];

    snprintf(finalBuffer, sizeof(finalBuffer), "%.2f:%.2f", offsetX, offsetY);
      // writing the data that we just got
    FS_writeData(compass_calibration, finalBuffer, strlen(finalBuffer));
      }     //
    else  //
    {
      compass_QMC.calibrate();
    }
}

// ************************   RETRIEVE_COMPASS_DATA   ************************//
// This function is used to retrieve the compass data from LittleFS on startup. The values are stored as
// "offsetX:offsetY", this parses both parts and assigns the offsets to the global variables.
// TODO: add zValue
//
void retrieve_Compass_Data() {
  char temp[25];

  int correct = FS_readData(compass_calibration, temp, sizeof(temp));
  if (!correct) return;

    char* xValue = strtok(temp, ":");
    Serial.println(xValue);

    char* yValue = strtok(NULL, ":");
    Serial.println(yValue);

    offsetX = atof(xValue);
    // Serial.println(values[0]);
    offsetY = atof(yValue);
    // Serial.println(values[1]);
}
