// ************************   DISP_LCD_INFO   ************************//

// General funcionality of LCD
void disp_lcd_info() {

  disp_time += disp_delay;

  // Constrain LCD_screen to correct values - This used to be done in ISR, but figure this is better...

  // Option 1 - cap at 0 and top
  LCD_screen = constrain(LCD_screen, 0, num_LCD_screens);  // Cap screens at limits

  // Option 2 - wrap lcd screen around...
  // if (LCD_screen > num_LCD_screens) {
  //   LCD_screen = 0;
  // }
  // if (LCD_screen < 0) {
  //   LCD_screen = num_LCD_screens;
  // }

  if (LCD_screen_old != LCD_screen)  // if screen changes, clear old screen
  {
    lcd.clear();
    LCD_screen_old = LCD_screen;
  }

  switch (LCD_screen) {
    case 1:  // Title screen
      Title_Screen();
      break;
    case 2:  // Main Screen
      Main_Screen();
      break;
    case 3:  // Position Information
      Position_Screen();
      break;
    case 4:  // Radio Information
      Radio_Screen();
      break;
    case 5:  // Environmental Information
      Environment_Screen();
      break;
    case 6:  // Battery Information
      Battery_Screen();
      break;
    case 7:  // Battery Information
      Object_Avoid_Screen();
      break;
    case 8:
      Compass_Screen();
      break;
    case 9:
      PID_Screen();
      break;
    default:
      lcd.setCursor(0, 1);
      lcd.print(F("**Undefined Screen**"));
      lcd.setCursor(5, 2);
      lcd.print(LCD_screen);
      break;
  }
}



// ************************   "******"_SCREEN   ************************//

// Specific Screen layouts

void Title_Screen() {  // Title screen
  //   01234567890123456789
  //0     GPS Guided Car
  //1      team_name[]
  //2       sub_name[]
  //3     course_name[]
  lcd.setCursor(0, 0);
  lcd.print(F("   GPS Guided Car "));
  lcd.setCursor(0, 1);
  lcd.print(team_name);
  lcd.setCursor(0, 2);
  lcd.print(sub_name);
  lcd.setCursor(0, 3);
  lcd.print(course_name);
}

void Main_Screen() {  // Main Screen
  //   01234567890123456789
  //0
  //1  LIDAR:####
  //2  Hdg ### GPS### Sat##
  //3  Dist ##m  ##:##:##ampm
  if (gps.location.isValid())  // valid GPS - just display location
  {
    //   01234567890123456789
    //0  ###mph Alt ### D ###
    //1  LIDAR:####
    lcd.setCursor(0, 0);
    lcd.print(gps_speed);
    lcd.print(F("mph "));
    lcd.print(F("Alt "));
    lcd.print(round(gps.altitude.feet()));
    lcd.print(F(" D "));
    lcd.print(dist_traveled);

    lcd.setCursor(10, 1);
    if (ind_gps == 0) {
      //   01234567890123456789
      //0  ###mph Alt ### D ###
      //1  LIDAR:#### Start Box  // seems to be too long for lcd
      lcd.print(F(" Start Box"));
    } else {
      //   01234567890123456789
      //0  ###mph Alt ### D ###
      //1  LIDAR:####  Tgt #/#
      lcd.print(F(" Tgt "));
      lcd.print(ind_gps + 1);
      lcd.print(F("/"));
      lcd.print(num_gps_tgts);
    }
  } else  // invalid gps - blink "Acquiring GPS..."
  {
    //   01234567890123456789
    //0  Acquiring GPS...
    //1  LIDAR:####
    blink_acquiring();
  }

  lcd.setCursor(0, 1);
  lcd.print(F("LIDAR:     "));
  lcd.setCursor(6, 1);
  lcd.print(constrain(dist_lidar, 0, 99), 1);

  // clear heading and gps data, so i don't have to clear the whole line every time
  lcd.setCursor(0, 2);
  lcd.print(F("Hdg "));
  byte i = 4;
  if (car_heading < 0) i = i - 1;
  lcd.setCursor(4, 2);
  lcd.print(F("    "));
  lcd.setCursor(i, 2);
  lcd.print(car_heading);
  lcd.setCursor(8, 2);
  lcd.print(F("GPS "));
  i = 12;
  if (gps_heading < 0) i = i - 1;
  lcd.setCursor(12, 2);
  lcd.print(F("    "));
  lcd.setCursor(i, 2);
  lcd.print(gps_heading);
  lcd.setCursor(15, 2);
  lcd.print(F("Sat"));
  lcd.print(gps.satellites.value());

  lcd.setCursor(0, 3);
  lcd.print(F("Dist"));
  lcd.setCursor(5, 3);
  lcd.print(F("    "));
  lcd.setCursor(5, 3);
  lcd.print(constrain(dist_to_target, -5, 99));
  lcd.print(F("m"));
  lcd.setCursor(10, 3);
  lcd.print(gps_time);
  lcd.print(ampm ? F("pm") : F("am"));
}

void Position_Screen() {  // Position Information
  //   01234567890123456789
  //0  Tgt Lat: ##.######
  //1  GPS Lat: ##.######
  //2  Tgt Lng:-###.######
  //3  GPS Lng:-###.######
  lcd.setCursor(0, 0);
  lcd.print(F("Tgt "));
  lcd.print(ind_gps + 1);
  lcd.print(F("/"));
  lcd.print(num_gps_tgts);
  lcd.print(F(": "));
  lcd.print(target_lats[ind_gps], 6);
  lcd.setCursor(0, 1);
  lcd.print(F("GPS Lat: "));
  lcd.print(gps.location.lat(), 6);
  lcd.setCursor(0, 2);
  lcd.print(F("Tgt Lng:"));
  lcd.print(target_longs[ind_gps], 6);
  lcd.setCursor(0, 3);
  lcd.print(F("GPS Lng:"));
  lcd.print(gps.location.lng(), 6);
}

void Radio_Screen() {  // Radio Information
  //   01234567890123456789
  //0     Servo/ESC Info
  //1  ESC Cmd: ###
  //2  Steer Cmd: ###
  //3  Heading Error:  ###
  lcd.setCursor(0, 0);
  lcd.print(F("   Servo/ESC Info   "));
  lcd.setCursor(0, 1);
  lcd.print(F("ESC Cmd:            "));
  lcd.setCursor(10, 1);
  lcd.print(esc_command);
  lcd.setCursor(0, 2);
  lcd.print(F("Steer Cmd:          "));
  lcd.setCursor(10, 2);
  lcd.print(servo_command);
  lcd.setCursor(0, 3);
  lcd.print(F("Heading Error:      "));
  byte i = 14;
  if (heading_error < 0) i = i - 1;
  lcd.setCursor(i, 3);
  lcd.print(heading_error);
}

void Environment_Screen() {  // Environmental Information
  //   01234567890123456789
  //0  ##/##/## ##:##:##ampm
  //1  gps_age:  ###
  //2  GPS Speed:  ##mph
  //3  Heading: ###
  lcd.setCursor(0, 0);
  lcd.print(gps_date);
  lcd.print(F(" "));
  lcd.print(gps_time);
  lcd.print(ampm ? F("pm") : F("am"));
  lcd.setCursor(0, 1);
  lcd.print(F("gps_age:            "));
  lcd.setCursor(8, 1);
  if (gps.location.isValid()) lcd.print(gps.location.age());
  else lcd.print("  N/A  ");
  lcd.setCursor(0, 2);
  lcd.print(F("GPS Speed: "));
  lcd.print(gps_speed);
  lcd.print(F(" mph "));
  lcd.setCursor(0, 3);
  lcd.print(F("Heading:            "));
  lcd.setCursor(8, 3);
  lcd.print(car_heading);
}

void Battery_Screen() {  // Battery Information
  //   01234567890123456789
  //0     Battery Status
  //1  Cell 1: ##.## V
  //2  Cell 2: ##.## V
  //3  Total : ##.## V
  // calc_batt_voltage();  // measure battery voltage
  lcd.setCursor(0, 0);
  lcd.print(F("   Battery Status   "));
  if (LOW_BATTERY) {
    lcd.setCursor(0, 1);
    lcd.print("WARNING:Low Battery!");
  }
  lcd.setCursor(0, 2);
  lcd.print(F("Min Batt: "));
  lcd.print(volts_min);
  lcd.print(F(" V"));

  lcd.setCursor(0, 3);
  lcd.print(F("Battery : "));
  lcd.print(volts_total);
  lcd.print(F(" V"));
}


void Object_Avoid_Screen() {  // Radio Information
  //   01234567890123456789
  //0   Object Avoid Info
  //1  GPS/OA Hdg:-###,-###
  //2  Desired Hdg: ###
  //3  Heading Error:  ###
  lcd.setCursor(0, 0);
  lcd.print(F("Avoid - Dist:       "));
  lcd.setCursor(13, 0);
  lcd.print(dist_lidar);
  lcd.setCursor(0, 1);
  lcd.print(F("GPS/OA Hdg:         "));
  lcd.setCursor(11, 1);
  lcd.print(gps_heading);
  lcd.print("/");
  lcd.print(int(avoid_heading));
  lcd.setCursor(0, 2);
  lcd.print(F("Act/Des Hg:         "));
  lcd.setCursor(11, 2);
  lcd.print(car_heading);
  lcd.print("/");
  lcd.print(desired_heading);
  lcd.setCursor(0, 3);
  lcd.print(F("Heading Error:      "));
  byte i = 15;
  if (heading_error < 0) i = i - 1;
  lcd.setCursor(i, 3);
  lcd.print(heading_error);
}

void Compass_Screen() {
  // 01234567890123456789
  //0      Compass Info
  //1
  if (digitalRead(Sw) == LOW) {
    calibrate_compass();
  }
  lcd.setCursor(0, 0);
  lcd.print(F("compass:   Hdg:     "));
  lcd.setCursor(15, 0);
  lcd.print(car_heading);
  lcd.setCursor(0, 1);
  if ((offsetX == 0 && offsetY == 0)) {
    lcd.print(F("Press encode"));
    lcd.setCursor(0, 2);
    lcd.print(F("To calibrate"));
  } else {
    lcd.print(F("offsetX: "));
    lcd.print(offsetX);
    lcd.setCursor(0, 2);
    lcd.print(F("offsetY: "));
    lcd.print(offsetY);
    lcd.setCursor(0, 3);
    lcd.print(F("offsetZ: "));
    lcd.print(offsetZ);
  }
}

void PID_Screen() {  // PID Information
  //   01234567890123456789
  //0  RPM:  #### / ####
  //1  SPEED: #.## / #.##
  //2  ESC: ####  P: #####
  //3  I: #####   D: #####
  lcd.setCursor(0, 0);
  lcd.print(F("RPM:                "));
  lcd.setCursor(6, 0);
  lcd.print(set_rpm_encoder);
  lcd.print(F(" / "));
  lcd.print(rpm_encoder);

  lcd.setCursor(0, 1);
  lcd.print(F("SPEED:              "));
  lcd.setCursor(7, 1);
  lcd.print(target_speed);
  lcd.print(F(" / "));
  lcd.print(rpm_speed);

  lcd.setCursor(0, 2);
  lcd.print(F("ESC:       P:       "));
  lcd.setCursor(5, 2);
  lcd.print(esc_command);
  lcd.setCursor(14, 2);
  lcd.print(P);

  lcd.setCursor(0, 3);
  lcd.print(F("I:         D:       "));
  lcd.setCursor(3, 3);
  lcd.print(I);
  lcd.setCursor(14, 3);
  lcd.print(D);
}


void blink_acquiring() {
  static bool acq_disp_flag = 0;
  lcd.setCursor(0, 0);
  if (acq_disp_flag) {
    lcd.print(F("                "));
    acq_disp_flag = 0;
  } else {
    lcd.print(F("Acquiring GPS..."));
    acq_disp_flag = 1;
  }
}
