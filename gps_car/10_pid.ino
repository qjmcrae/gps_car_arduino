// ************************   PID FUNCTIONS   ************************//

// perhaps rename variables so they are all the same or rename them
// to specify which PID they are a part off


// ************************   ESC_PID   ************************//

int esc_pid(int speed) {
  // pid_time = millis() + pid_delay;
  pid_time += pid_delay;

  // long P, D;
  // static long I, prev_P;
  static long prev_P;
  int throttle_command, pid_throttle;  // make variables "static" if you uncomment Integral Check

  // The following gains were found from testing on the dyno
  // const float Kp = 0.27;
  // const float Ki = 0.07;
  // const float Kd = 0.25;

  const float mph_to_rpm = 175.36766 / 2.6;  // 5280 ft\mile * 12 in\ft * 2.6 rev_enc\rev_wheel / 15.65625 in\rev_wheel / 60 minutes\hr

  // const long max_I = (esc_full_forward - esc_default) / Ki;  // Determine the max I value for desired max influence
  long max_I = (esc_full_forward - esc_default) / Ki;  // Determine the max I value for desired max influence

  static long last_time_call = millis();
  long time_call_rpm = millis();

  rpm = calc_mag_rpm();
  rpm_speed = rpm / mph_to_rpm;  // for display purposes
  set_rpm = speed * mph_to_rpm;

  delta_t_rpm_call = time_call_rpm - last_time_call;
  last_time_call = time_call_rpm;


  P = set_rpm - rpm;                    // current error of rpm
  I = constrain(I + P, -max_I, max_I);  // summation of the error of rpm
  if (!armed) I = 0;
  D = (P - prev_P);  // difference between current and previous error scaled by deltaT

  pid_throttle = P * Kp + I * Ki + D * Kd;

  throttle_command = constrain(esc_default + pid_throttle, esc_full_reverse, esc_full_forward);  // Keep it in range of the ESC

  prev_P = P;  // grab error for next time through


  return throttle_command;
}
