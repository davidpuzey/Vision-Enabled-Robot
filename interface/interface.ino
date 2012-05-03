/* Set the error codes */
const int ERROR_NO_CMD = 1; // Command doesn't exist
const int ERROR_LT_CHARS = 2; // Not enough parameters for function

/* Set pin assignments */
const int IRPD_SENSOR_LEFT = 0; // The pin for polling the left side of the IRPD
const int IRPD_SENSOR_RIGHT = 1; // The pin for polling the right side of the IRPD
const int IRPD_RESPONSE = 2; // The pin for reading the response from the IRPD
const int SERVO_WHEEL_FRONT_LEFT = 3; // The pin for the servo controlling the front left wheel
const int SERVO_WHEEL_FRONT_RIGHT = 5; // The pin for the servo controlling the front right wheel
const int SERVO_WHEEL_BACK_LEFT = 6; // The pin for the servo controlling the back left wheel
const int SERVO_WHEEL_BACK_RIGHT = 9; // The pin for the servo controlling the back right wheel
const int ULTRASONIC_PULSE = 7; // The pin for sending the pulse to trigger the ultrasonic sensor
const int ULTRASONIC_ECHO = 8; // The pin for receiving the echo from the ultrasonic sensor
const int SERVO_PLATFORM_X = 10; // The pin for the servo controlling the platforms X rotation
const int SERVO_PLATFORM_Y = 11; // The pin for the servo controlling the platforms Y rotation


void setup() {
    Serial.begin(9600);
}

void loop()
{
    chkCommand();
    delay(10);
    
}

void movePlatform(int *params) {
  Serial.println("movePlatform");
}

void readUltrasonic(int* params) {
  Serial.println("readUltrasonic");
}

void readIRPD(int* params) { 
  Serial.println("readIRPD");
}

void move(int* params) {
  Serial.println("move");
}

void turn(int* params) {
  Serial.println("turn");
}

void wheelSpeed(int* params) {
  Serial.println("wheelSpeed");
}

/**
 * chkCommand - Checks for a new command on the serial interface
 */
void chkCommand() {
  if (Serial.available() <= 0)
    return;
  char cmdChar = Serial.read();
  int minChars = 0, maxChars = 0;
  char* moreChars;
  void (*func) (int*);
  
  switch (cmdChar) {
    case 'p': // Move the platform
      minChars = 2;
      func = movePlatform;
      break;
    case 'u': // Read ultrasonic sensor
      func = readUltrasonic;
      break;
    case 'i': // Read infrared proximity detector
      func = readIRPD;
      break;
    case 'm': // Move the robot
      minChars = 2;
      func = move;
      break;
    case 't': // Turn the robot
      minChars = 2;
      func = turn;
      break;
    case 'w': // Set the speed of an individual wheel
      minChars = 2;
      func = wheelSpeed;
      break;
    default:
      error(ERROR_NO_CMD); // Command doesn't exist
  }
  
  if (func) {
    if (maxChars < minChars)
      maxChars = minChars;
    
    int *paramChars = getMoreChars(minChars, maxChars);
    if (minChars > 0 && paramChars == '\0')
      error(ERROR_LT_CHARS); // Not enough characters have been provided for the function
    else
      func(paramChars);
  }
  
  int inChar = Serial.read();
  while (inChar != -1 && (char)inChar != ';') {
    inChar = Serial.read();
  }
  //Serial.flush(); // Clear out the buffer
}

/**
 * getMoreChars - Gets at least a certain number of characters and tries to get more
 * Params:
 *   minNum (int) The minimum number of characters to return
 *   maxNum (int) The maximum number of characters to try and find
 * @return (char*) Returns an array of characters between minNum and maxNum long
 */
int *getMoreChars(int minNum, int maxNum) {
  if (Serial.available() < minNum || minNum == 0)
    return '\0';
  
  int inChar[maxNum + 1];
  int index = 0;
  while (Serial.available() > 0 && index < maxNum) {
    inChar[index] = Serial.read();
    index++;
  }
  inChar[index] = '\0';
  return inChar;
}

void error(int err_no) {
  switch (err_no) {
    case ERROR_NO_CMD:
      Serial.println("Error: Command doesn't exist.");
      break;
    case ERROR_LT_CHARS:
      Serial.println("Error: Not enough characters have been provided for the function.");
      break;
    default:
      Serial.println("Generic error");
  }
}
