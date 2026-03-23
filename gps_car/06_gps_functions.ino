// ************************   GPS FUNCTIONS   ************************//

// This file contains various functions for the gps



// ************************   GET_GPS_DATA   ************************//

// This code is meant to be called to get GPS data
void get_gps_data(float target_lat, float target_lon) {

  static float old_lat = 0;
  static float old_lon = 0;
  float lat = gps.location.lat();
  float lon = gps.location.lng();

  if (old_lat == 0 || old_lon == 0) {  // take care of first time
    old_lat = lat;
    old_lon = lon;
  }

  // estimate how far traveled total...
  if (gps.location.isValid()) {
    dist_to_target = gps.distanceBetween(lat, lon, target_lat, target_lon);
    dist_traveled = dist_traveled + gps.distanceBetween(lat, lon, old_lat, old_lon);
    old_lat = lat;
    old_lon = lon;
  }

  // Figure out GPS Date / Time
  byte gps_month  = gps.date.month();
  byte gps_day    = gps.date.day();
  byte gps_year   = gps.date.year() - 2000;   // put it in a shorter format
  int gps_hour    = gps.time.hour();          // this needs to be int so late at night it doesn't get all jacked up
  byte gps_minute = gps.time.minute();
  byte gps_second = gps.time.second();

  // Knowing that Salt Lake is 6-7 hours behind the standard GPS time (depending on DST) ...
  // Also, assuming that DST is between between Feb 28 and Nov 1 (not accurate, but close enough)
  byte UT_hour_offset = 7;
  if (gps_month > 2 && gps_month < 11 && gps_day > 7) {
    UT_hour_offset = 6;  // when on daylight savings time...
  }
  if (gps_hour < UT_hour_offset) {
    gps_day = gps_day - 1;  // the whole late-night thing (hasn't crossed 12am yet)
  }
  gps_hour = gps_hour - UT_hour_offset;
  if (gps_hour < 0) {
    gps_hour = gps_hour + 24;
  }
  if (gps_hour > 12) {
    gps_hour = gps_hour - 12;
    ampm = 1;
  }

  sprintf(gps_time, "%2d:%02d:%02d", gps_hour, gps_minute, gps_second);
  sprintf(gps_date, "%0d/%0d/%0d", gps_month, gps_day, gps_year);
  gps_speed = round(gps.speed.mph());
}  //End of get_gps_data



// ************************   STOP_NO_GPS   ************************//

// This code is meant to be called if the GPS signal isn't found
// It gives the "Check Wriring" error on LCD
void stop_no_gps() {
  static bool write_flag = 1;
  lcd.clear();
  while (1) {
    if (write_flag) {
      lcd.setCursor(0, 0);
      lcd.print(F("No GPS"));
      lcd.setCursor(0, 1);
      lcd.print(F("Check Wiring"));
      write_flag = 0;
      delay(500);
    } else {
      lcd.clear();
      write_flag = 1;
      delay(250);
    }
  }
}  //End of stop_no_gps