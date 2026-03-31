// ************************   LITTLEFS FUNCTIONS   ************************//

// This file contains the functions that are used for the LittleFS for storing misc data that we need for each car.
// The data will be saved in text files. For now, just the variable compass_calibration

// Returns data as a chararray to out_str
// int is used to check if the file is opened properly or if the command operated successfully
int FS_readData(const char * path, char* out_str, size_t size) {
  FILE *file = fopen(path, "r");

  // Testing if file exists
  if (file) {
    // Serial.println("Open ok!");
  } else {
    // Serial.println("Open failed");
    // troubleshooting beep for fail open
    // beep();
    return 0;
  }  


  char c;
  char tempString[15];

  // Loops reads characters and then ends when file is stopped.
  while (true) {
    int len = strlen(tempString);
    if (len+1 < sizeof(tempString)) {
      tempString[len] = c;
      tempString[len+1] = '\\0';
    } else {
      break;
    }

  if (strlen(tempString) + 1 <= size) {
    strcpy(out_str, tempString);
  }

  fclose(file);
  // Serial.println(tempString);

  return 1;
}
}


// Writes data into a path. You need to give the path along with the data and sizeof(data), (MBED_LITTLEFS_FILE_PREFIX path, data, sizeof(data))
// 
void FS_writeData(const char * path, const char * n, size_t nSize) { 
  // Serial.print("Writing file: ");
  // Serial.println(path);

  // Code was getting stuck here, turns out I forgot to mount the FS using myFS = new LittleFS_MBED();

  FILE *file = fopen(path, "w");

  if (file) {
    // Serial.println("File opened");
  } else {
    // Serial.println("File failed to open");
    return;
  }

  if (fwrite( (uint8_t *) n, 1, nSize, file )) {
    // Serial.println("Writing OK");
  } else {
    // Serial.println("Writing failed");
    // troubleshooting beep for fail
    // beep();
  }

  fclose(file);
}