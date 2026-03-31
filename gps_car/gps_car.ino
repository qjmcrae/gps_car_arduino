//============= Arduino Code for GPS Car ==============//
// Slick Science SSAV (Small Scale Autonomous Vehicle)
//=====================================================//

/////////////////////////////////////////////////////////////////////////////////////////
//                                                                                     //
//                          BEGIN STUDENT-ADJUSTABLE SECTION                           //
//                                                                                     //
/////////////////////////////////////////////////////////////////////////////////////////

// Student-avaliable variables...
const char team_name[] = "   Slick Science    ";
const char sub_name[] = "    SHAI_HULUD ";

byte min_dist_to_tgt = 2;   // minimum distance (in meters) to target before moving on to next target - More accurate closer, but harder
byte delay_at_target = 2;   // delay for x seconds (and wag servo) once at gps target before going to next target - makes it more obvious when you get there
int servo_wag_speed = 250;  // milliseconds waiting between left/right - make smaller for faster wag, larger for slower

// Neo-pixel colors  (0 -> 255)
byte neo_red = 100;
byte neo_green = 0;
byte neo_blue = 50;

// const char course_name[] = "    Day 1 Demo      ";
// const byte num_gps_tgts = 7;
// const float target_lats[num_gps_tgts]  = {  40.673279,   40.673511,   40.673320,   40.673442,   40.673226,   40.673531,   40.673279};
// const float target_longs[num_gps_tgts] = {-111.939704, -111.939532, -111.939333, -111.940018, -111.939857, -111.939755, -111.939704};

// const char course_name[] = "    SI Test      ";
// const byte num_gps_tgts = 4;   // make sure to update this!
// const float target_lats[num_gps_tgts]  = {40.671524, 40.671457,  40.671618, 40.671524 };
// const float target_longs[num_gps_tgts] = {-111.946140,  -111.946014, -111.946221, -111.946140 };

const char course_name[] = "     Home Test      ";
const byte num_gps_tgts = 4;   // make sure to update this!
const float target_lats[num_gps_tgts]  = {40.645076, 40.645029, 40.645109, 40.645076};
const float target_longs[num_gps_tgts] = {-111.878806, -111.878326, -111.879036, -111.878806};








/////////////////////////////////////////////////////////////////////////////////////////
//                                                                                     //
//                           END STUDENT-ADJUSTABLE SECTION                            //
//                                                                                     //
/////////////////////////////////////////////////////////////////////////////////////////
