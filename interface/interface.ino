String inString;

void setup() {
    Serial.begin(9600);
}

void loop()
{
    while (Serial.available() > 0) {
      delay(5);
      
      inString += (char) Serial.read();
    }
    if (inString.length() > 0) {
      Serial.println(inString);
      inString = "";
    }
}

/**
 * chkCommand - Checks for a new command on the serial interface
 */
void chkCommand() {
  if (Serial.available() <= 0)
    return;
  char cmdChar = Serial.read();
  int charNo = 0;
  
  switch (cmdChar) {
    case 'p': // Move the platform
      charNo = 0;
      // movePlatform(getMoreChars(2));
      break;
    case 'u': // Read ultrasonic sensor
      charNo = 0;
      break;
    case 'i': // Read infrared proximity detector
      charNo = 0;
      break;
    case 'm': // Move the robot
      charNo = 0;
      break;
    case 't': // Turn the robot
      charNo = 0;
      break;
    case 'w': // Set the speed of an individual wheel
      charNo = 0;
      break;
  }
  
  Serial.flush(); // Clear out the buffer
}
