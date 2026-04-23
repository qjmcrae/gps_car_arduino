// This version is to match the hardware of v10.0 -


///////////////////////////////////////////////////////////////////
//                                                               //
//                       JAMBOREE DEMO                           //
//                                                               //
///////////////////////////////////////////////////////////////////


//================== Version Update ===================//
//
//   RC Control - hold constant angle, avoid stuff in between
//   This is for Jamboree Demonstration
//
//=====================================================//



//=================  Pin definitions  =================//
// try to define each pins' use
//=====================================================//

#define D8 20  // Neo-pixel LED strip, might affect pin A6 - RP2040 doesn't work to just do "8" below
// Digital pins, D-pins ...
// Pins D0, D1 used for hardware serial..
// TX = D0
// RX = D1
// Pins D5,D6,D7 unused
byte Clk = 2;                  // Encoder Clk pin
byte Dt = 3;                   // Encoder DT pin
byte Sw = 4;                   // Encoder SW (button) pin
byte pixel_pin = D8;           // Neo-pixel LED strip
byte esc_servo_pin = 9;        // Signal to ESC
byte steering_servo_pin = 10;  // Signal to Steering Servo
byte green_car_pin = 11;       // To enable board usage on green car, not used for Slick Science
byte hall_pin = 12;            // Hall Effect Sensor
byte buzzer_pin = 13;          // Piezo electric buzzer pin

// Analog pins, A-pins...
// pins A4, A5 used for I2C protocol...
// SDA = A4;
// SCL = A5;
// Pins A0,A3,A6,A7 unused
int batt_volt_pin = A1;      // input pin for voltage divider of total voltage of main battery
int steering_trim_pin = A2;  // analog read of pot to correct steering of cars


//=============== Initialize Variables ================//
// Setup program-level stuff, i.e. timers, etc.
//=====================================================//

// Battery Voltage
float volts_total = 10;  //probably doesn't matter, but initializing to make filter happy
float volts_min = 10;  // initialize to larger than real value - make sure early on we just show true min, not artificial min
float low_voltage_threshold = 7.0;  // This might-should be adjusted - trying to capture reasonable value.
bool LOW_BATTERY = 0;

bool beeped = 0;

volatile byte LCD_screen = 1;
volatile byte LCD_screen_old = 0;
byte num_LCD_screens = 9;

int neo_delay = 100;
unsigned long neo_time = 0;

byte disp_freq = 2;  // in hz
int disp_delay = 1000 / disp_freq;
unsigned long disp_time = 0;

byte calc_batt_freq = 5;  // in hz
int calc_batt_delay = 1000 / calc_batt_freq;
unsigned long calc_batt_time = 0;

// Are these needed?
// float calc_dist_freq = 0.5;  // in hz
// long calc_dist_delay = 1000 / calc_dist_freq;
// unsigned long calc_dist_time = 0;

// LED variables ...
unsigned long timer[] = { 0, 0, 0 };
const byte R = 0;
const byte G = 1;
const byte B = 2;

// GPS and compass variables ...
float dist_lidar;
bool hmc_flag = 0;        // flag to determine if we are using the HMC or the QMC compass
bool armed = 0;           // don't arm until arive at 1st gps location
byte ind_gps = 0;         // This is the index to which of the gps lat/long points we are currently going for ...
float dist_to_target;     // changed from int, may need to re-layout screen
int gps_heading;          // heading from current location to next GPS point
int car_heading;          // heading of car - uses compass if going slow, possibly use GPS if going fast
float avoid_heading;      // heading to avoid stuff - currently f(distance)  = 90*e^-0.15*dist
int desired_heading;      // combination of gps_heading and avoid_heading
int heading_error;        // difference between where we want to be pointing and where we are pointing - want to be 0
int compass_offset = 90;  // This is because the compass is mounted x° off straight

int dist_traveled = 0;
char gps_time[9];
char gps_date[11];
int gps_speed;
bool ampm = 0;

// Servo variables ...
int servo_write_freq = 100;  // in hz
long servo_write_delay = 1000 / servo_write_freq;
unsigned long servo_write_time = 0;
byte servo_command = 90;
int esc_command = 1500;
int servo_trim_range = 15;
int servo_trim_time = 20;


// Values range between ~40 and ~140
byte servo_straight = 90;
byte servo_left = 55;    // 60;
byte servo_right = 125;  // 127;
byte servo_large_circle = 105;
int steering_trim;

// Values range between ~1000 and ~2000
int esc_default = 1500;     // 90;
int esc_slow_grass = 1606;  // 109;
// qj - int esc_slow_pavement = 1578;  // 104;
int esc_slow_pavement = 104;
int esc_fast_forward = 1778;  // 140;
int esc_full_forward = 1944;  // 170;
int esc_slow_reverse = 1444;  // 80;
int esc_fast_reverse = 1222;  // 40;
int esc_full_reverse = 1111;  // 20;
int esc_stop = esc_default;

// PID Controller Stuff...
int pid_freq = 100;  // in hz
long pid_delay = 1000 / pid_freq;
unsigned long pid_time = 0;

bool pid_flag = 1;         // Pick between hard coded and pid
volatile int hall_count = 0;  // count for number of times isr_hall() has been tripped in a cycle
float target_speed, rpm_speed;
// float rpm = 0;
int rpm_encoder, rpm_wheel;
int set_rpm_encoder;
int pid_command = esc_command;
int steer_command = servo_command;
long P, I, D;


//=============== Different cases in void loop ==============//
// Change as needed -EH
//===========================================================//

enum Car_state {
  STATE_NO_GPS,
  STATE_OBSTACLE_STOP,
  STATE_AT_TARGET,
  STATE_DRIVING,
  STATE_LOW_BATTERY,
  STATE_NO_RPM_READING
};
Car_state currentState;

// //============= different LCD Screens ============//
// enum lcd_screen {
//     Title_Screen,
//     Main_Screen,
//     Position_Screen,
//     Radio_Screen,
//     Environment_Screen,
//     Battery_Screen,
//     Object_Avoid_Screen,
//     Compass_Screen,
//     PID_Screen
//     };

// lcd_screen LCD_screen = Title_Screen;


//============== Compass offsets and FS imports =========//
// -C
//=======================================================//
float offsetX = 0.0;
float offsetY = 0.0;
float offsetZ = 0.0;
// Imports for littleFS as well as definitions.
#include <LittleFS_Mbed_RP2040.h> // File System for storing data like Compass calibration.
#define LFS_MBED_RP2040_VERSION_MIN_TARGET      "LittleFS_Mbed_RP2040 v1.1.0"
#define LFS_MBED_RP2040_VERSION_MIN             1001000
#define _LFS_LOGLEVEL_          1 // Logs data to Serial connection for troubleshooting purposes
#define RP2040_FS_SIZE_KB       64 // This is the specified storage size in KB
#define FORCE_REFORMAT          false // Change this to true if you want to reformat the littleFS file system
bool FS_init = false;
LittleFS_MBED *myFS;
char compass_calibration[] = MBED_LITTLEFS_FILE_PREFIX "/compass.txt";
//=============== Initialize Libraries ================//
// Include Libraries, Setup objects, modules, etc.
//=====================================================//

// Enables I2C Communication
#include <Wire.h>

// To use onboard rgb led, and A6, A7 WiFiNINA library is needed
#include <WiFiNINA.h>

#include "mbed.h" // Ensures we have access to the correct Mbed types

// Setup Neo-pixel LED array
#include <Adafruit_NeoPixel.h>
Adafruit_NeoPixel neo_pixel = Adafruit_NeoPixel(8, pixel_pin, NEO_GRB + NEO_KHZ800);

//Setup LCD Screen
#include <LiquidCrystal_I2C.h>
// next 2 lines for old LiquidCrystal_I2C library...
LiquidCrystal_I2C lcd_0x27(0x27, 20, 4);  // set the LCD address for a 20 chars and 4 line display
LiquidCrystal_I2C lcd_0x3F(0x3F, 20, 4);  // set the LCD address for a 20 chars and 4 line display
LiquidCrystal_I2C lcd = lcd_0x27;         // Assume it is the 0x27, then if 0x3F is there, switch...

// Initialize servo stuff ...
#include <Servo.h>
Servo steering_servo, esc_servo;  // create servo objects to control a servo

// Setup GPS ...
#include <TinyGPS++.h>
TinyGPSPlus gps;  // The TinyGPS++ object

// Setup Compass 1 ...
#include <Adafruit_HMC5883_U.h>
Adafruit_HMC5883_Unified compass_HMC = Adafruit_HMC5883_Unified(12345);  // Assign a unique ID to this sensor at the same time

// Setup Compass 2 ...
#include <QMC5883LCompass.h>
QMC5883LCompass compass_QMC;

// Setup Dist sensor, Currently LiDAR
#include <TFLI2C.h>            // TFLuna-I2C Library v.0.1.1
TFLI2C luna;                   // create object for distance sensor, willing to rename
#define lidar_adr TFL_DEF_ADR  // set address for distance sensor
