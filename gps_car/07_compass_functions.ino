// ************************   COMPASS FUNCTIONS   ************************//

// This file contains various functions for the compass



// ************************   GET_COMPASS_DATA   ************************//

void get_compass_data(float target_lat, float target_lon) {
  int compass_heading;
  if (hmc_flag) {
    sensors_event_t event;
    compass_HMC.getEvent(&event);
    compass_heading = atan2((event.magnetic.y - offsetY) * scaleY, (event.magnetic.x - offsetX) * scaleX) * 180.0 / M_PI;  // - compass_offset;
  } else {
    compass_QMC.read();
    // compass_heading = compass_QMC.getAzimuth();
    compass_heading = atan2((compass_QMC.getY() - offsetY) * scaleY, (compass_QMC.getX() - offsetX) * scaleX) * 180.0 / M_PI;
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
  lcd.print(F("Rotate car all dir."));
      float xMin = 9999;
      float zMin = 9999;
      float yMin = 9999;
      float yMax = -9999;
      float xMax = -9999;
      float zMax = -9999;
      unsigned long start_time = millis();

    if (hmc_flag)  //
    {
      lcd.setCursor(0, 1);
      lcd.print(F("HMC"));


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
      // Offset calculation
      offsetX = (xMax + xMin) / 2;
      offsetY = (yMax + yMin) / 2;
      offsetZ = (zMax + zMin) / 2;

      // Scales calculation
      float DeltaX = (xMax - xMin) / 2;
      float DeltaY = (yMax - yMin) / 2;
      float DeltaZ = (zMax - zMin) / 2;

      float AverageDelta = (DeltaX + DeltaY + DeltaZ) / 3;
      scaleX = AverageDelta / DeltaX;
      scaleY = AverageDelta / DeltaY;
      scaleZ = AverageDelta / DeltaZ;

      char finalBuffer[32];
      char scalesBuffer[32];

    snprintf(finalBuffer, sizeof(finalBuffer), "%.2f:%.2f:%.2f", offsetX, offsetY, offsetZ);
    snprintf(scalesBuffer, sizeof(scalesBuffer), "%.2f:%.2f:%.2f", scaleX, scaleY, scaleZ);
      // writing the data that we just got
    FS_writeData(compass_calibration, finalBuffer, strlen(finalBuffer));
    FS_writeData(compass_scales, scalesBuffer, strlen(scalesBuffer));
      }     //
    else  //
    {
      lcd.setCursor(0, 1);
      lcd.print(F("QMC"));

      while (millis() - start_time < 20000) {
        compass_QMC.read();
        
        if (compass_QMC.getX() < xMin) xMin = compass_QMC.getX();
        if (compass_QMC.getX() > xMax) xMax = compass_QMC.getX();
        if (compass_QMC.getY() < yMin) yMin = compass_QMC.getY();
        if (compass_QMC.getY() > yMax) yMax = compass_QMC.getY();
        if (compass_QMC.getZ() < zMin) zMin = compass_QMC.getZ();
        if (compass_QMC.getZ() > zMax) zMax = compass_QMC.getZ();

        delay(10);
      }

      
      // offset calculation
      offsetX = (xMax + xMin) / 2;
      offsetY = (yMax + yMin) / 2;
      offsetZ = (zMax + zMin) / 2;

      // Scales calculation
      float DeltaX = (xMax - xMin) / 2;
      float DeltaY = (yMax - yMin) / 2;
      float DeltaZ = (zMax - zMin) / 2;

      float AverageDelta = (DeltaX + DeltaY + DeltaZ) / 3;
      scaleX = AverageDelta / DeltaX;
      scaleY = AverageDelta / DeltaY;
      scaleZ = AverageDelta / DeltaZ;

      // This function does the same thing as the above statement
      // compass_QMC.calibrate();

      // offsetX = compass_QMC.getCalibrationOffset(0);
      // offsetY = compass_QMC.getCalibrationOffset(1);
      // offsetZ = compass_QMC.getCalibrationOffset(2);
      // scaleX = compass_QMC.getCalibrationScale(0);
      // scaleY = compass_QMC.getCalibrationScale(1);
      // scaleZ = compass_QMC.getCalibrationScale(2);



      char finalBuffer[32];
      char scalesBuffer[32];

      snprintf(finalBuffer, sizeof(finalBuffer), "%.2f:%.2f:%.2f", offsetX, offsetY, offsetZ);
      snprintf(scalesBuffer, sizeof(scalesBuffer), "%.2f:%.2f:%.2f", scaleX, scaleY, scaleZ);
        // writing the data that we just got
      FS_writeData(compass_calibration, finalBuffer, strlen(finalBuffer));
      FS_writeData(compass_scales, scalesBuffer, strlen(scalesBuffer));
    }
}

// ************************   RETRIEVE_COMPASS_DATA   ************************//
// This function is used to retrieve the compass data from LittleFS on startup. The values are stored as
// "offsetX:offsetY:offsetZ", this parses all parts and assigns the offsets to the global variables.
// This also retrieves the Scales data, stored in the same way as the offsets
//
void retrieve_Compass_Data() {
  char offset_temp[25];
  char scales_temp[25];

  // Recieving offsets
  int correct = FS_readData(compass_calibration, offset_temp, sizeof(offset_temp));
  if (correct) {
    char* xValue = strtok(offset_temp, ":");
    char* yValue = strtok(NULL, ":");
    char* zValue = strtok(NULL, ":");

    if (xValue != NULL && yValue != NULL && zValue != NULL) {
      offsetX = atof(xValue);
      offsetY = atof(yValue);
      offsetZ = atof(zValue);
    } else {
      Serial.println("data incorrect format");
    }

  }

  // Recieving scales
  int scales_correct = FS_readData(compass_scales, scales_temp, sizeof(scales_temp));
  if (scales_correct) {
    char* xScale = strtok(scales_temp, ":");
    char* yScale = strtok(NULL, ":");
    char* zScale = strtok(NULL, ":");

    if(xScale != NULL && yScale != NULL && zScale != NULL) {
      scaleX = atof(xScale);
      scaleY = atof(yScale);
      scaleZ = atof(zScale);
    } else {
      Serial.println("data incorrect format");
    }
  }

  // // QMC compass is newer and uses a class to hide this data. Not a big deal to just set it when we call it.
  // if (!hmc_flag) {
  //   compass_QMC.setCalibrationScales(scaleX, scaleY, scaleZ);
  //   compass_QMC.setCalibrationOffsets(offsetX, offsetY, offsetZ);
  // }

}
