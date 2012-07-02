#include <Servo.h>

/* Set the error codes */
const int ERROR_NO_CMD = 1; // Command doesn't exist
const int ERROR_LT_CHARS = 2; // Not enough parameters for function
const int ERROR_WHEEL_OUT_OF_RANGE = 3; // The wheel selected doesn't exist

/* Set pin assignments */
const int IRPD_SENSOR_LEFT = 12; // The pin for polling the left side of the IRPD
const int IRPD_SENSOR_RIGHT = 13; // The pin for polling the right side of the IRPD
const int IRPD_RESPONSE = 4; // The pin for reading the response from the IRPD
const int SERVO_WHEEL_FRONT_LEFT = 3; // The pin for the servo controlling the front left wheel
const int SERVO_WHEEL_FRONT_RIGHT = 5; // The pin for the servo controlling the front right wheel
const int SERVO_WHEEL_BACK_LEFT = 6; // The pin for the servo controlling the back left wheel
const int SERVO_WHEEL_BACK_RIGHT = 9; // The pin for the servo controlling the back right wheel
const int ULTRASONIC_PULSE = 7; // The pin for sending the pulse to trigger the ultrasonic sensor
const int ULTRASONIC_ECHO = 8; // The pin for receiving the echo from the ultrasonic sensor
const int SERVO_PLATFORM_X = 10; // The pin for the servo controlling the platforms X rotation
const int SERVO_PLATFORM_Y = 11; // The pin for the servo controlling the platforms Y rotation

//const int MAX_SPEED = 20; // The maximum speed that the servos can go

Servo wheels[4], platform[2];
int wheelOffset[4] = {-30,0,15,-15}; //fl,fr,bl,br

void setup() {
    Serial.begin(9600);
    pinMode(ULTRASONIC_PULSE, OUTPUT);
    pinMode(ULTRASONIC_ECHO, INPUT);
    pinMode(IRPD_SENSOR_LEFT, OUTPUT);
    pinMode(IRPD_SENSOR_RIGHT, OUTPUT);
    pinMode(IRPD_RESPONSE, INPUT);
    // We limit the minimum and maximum pulse as the servos will not go any faster for larger or smaller values
    // We also apply an offset to each of the wheels to easier to calibrate them
    wheels[0].attach(SERVO_WHEEL_FRONT_LEFT, (1300 + wheelOffset[0]), (1700 + wheelOffset[0]));
    wheels[1].attach(SERVO_WHEEL_FRONT_RIGHT, (1300 + wheelOffset[1]), (1700 + wheelOffset[1]));
    wheels[2].attach(SERVO_WHEEL_BACK_LEFT, (1300 + wheelOffset[2]), (1700 + wheelOffset[2]));
    wheels[3].attach(SERVO_WHEEL_BACK_RIGHT, (1300 + wheelOffset[3]), (1700 + wheelOffset[3]));
    platform[0].attach(SERVO_PLATFORM_X);
    platform[1].attach(SERVO_PLATFORM_Y);
    changeAllMotors(0,0,0,0);
}

void loop()
{
    chkCommand();
    delay(100);
    
}

/**
 * movePlatform - Move the platform to the provided x and y coordinates
 * Params:
 *   params (*int) A list of integers:
 *     x - The X coordinate to move the platform too (if it greater than 180 then nothing will happen)
 *     y - The Y coordinate to move the platform too (if it greater than 180 then nothing will happen)
 */
void movePlatform(int *params) {
  int x = params[0];
  int y = params[1];
  if (x >= 0 && x <= 180)
    platform[0].write(x);
  if (y >= 0 && y <= 180)
    platform[1].write(y);
  Serial.print("p;");
}

/**
 * readUltrasonic - Sends the ultrasonic reading to the computer
 * Params:
 *   params (*int) Anything, it doens't matter, it's only here so the serial reading function works properly
 * NOTE: How to find distance from ultrasonic:
 *       The value returned is in microseconds and we want to get the distance in centimetres
 *       The speed of sound is about 343 metres per second
 *       To get centimeters per microsecond we need to times by 100 & divide by 1000000 (can just divide by 10000)
 *       This gives us 0.0343. As the value returned is for the distance there and back we need to divide by 2
 *       Now we have 0.01715 that we can times our value by, as this isn't a nice number to work with we can get
 *       the recipricol of this number and then divide our value instead this gives us 58.209... we can round this to 58
 */
void readUltrasonic(int* params) {
  unsigned long int echo;
  int distance;
  digitalWrite(ULTRASONIC_PULSE, HIGH); // Send high pulse to trigger ultrasonic detector
  delayMicroseconds(20); // Delay so that there is a long enough pulse to trigger the ultrasonic detector. Technically 10us is fine, but 20 to be safe.
  digitalWrite(ULTRASONIC_PULSE, LOW); // Stop pulse to ultrasonic
  echo = pulseIn(ULTRASONIC_ECHO, HIGH, 40000); // Read in the response, shouldn't exceed 36000, so 40000 max wait will be perfectly fine
  distance = echo / 58; // Get the distance in cm
  delay(10); // Ensure there will be at least 10ms before the next pulse
  Serial.print("u");
  Serial.print(distance);
  Serial.print(";");
}

/**
 * readIRPD - Sends the IRPD reading to the computer
 * Params:
 *   params (*int) Anything, it doens't matter, it's only here so the serial reading function works properly
 */
void readIRPD(int* params) { 
  Serial.print("i0;");
}

/**
 * move - Sets the speed of the robot and how long to maintain that speed before returning to 0
 * Params:
 *   params (*int) A list of integers:
 *     spd - The speed to set the servos to
 *     time - How long it should maintain the speed before stopping, in seconds. 0 will will set the speed indefinately until it's told otherwise
 * TODO: maybe use milliseconds, or hundereds of milliseconds for the time
 */
void move(int* params) {
  int spd = params[0];
  int time = params[1];
  spd -= 90; // This is pointless since it's just changed back later, but I'm leaving it in for now
  changeAllMotors(spd,spd,spd,spd);
  Serial.print("m;");
}

/**
 * turn - Sets how sharply the robot should turn and for how long to turn
 * Params:
 *   params (*int) A list of integers:
 *     angle - The sharpness of the turn
 *     time - How long it should turn for, in seconds. 0 will cause the robot to turn indefinately until it's told otherwise
 * TODO: need to determine the a range of values for the angle
 * TODO: maybe use milliseconds, or hundereds of milliseconds for the time
 */
void turn(int* params) {
  int angle = params[0];
  int time = params[1];
  angle -= 90; // This is pointless since it's just changed back later, but I'm leaving it in for now
  changeAllMotors(-angle,angle,-angle,angle);
  Serial.print("t;");
}

/**
 * wheelSpeed - Sets the speed of a wheel for a specific amount of time
 * Params:
 *   params (*int) A list of integers:
 *     wheel - The wheel to set the speed of
 *     spd - The speed to set the wheels to
 *     time - How long it should turn for, in seconds. 0 will cause the robot to turn indefinately until it's told otherwise
 * TODO: maybe use milliseconds, or hundereds of milliseconds for the time
 */
void wheelSpeed(int* params) {
  int wheel = params[0];
  int spd = params[1];
  int time = params[2];
  //wheel -= 49; // Temporarily correct so that ascii numbers can be used to select a motor
  spd -= 90; // This is pointless since it's just changed back later, but I'm leaving it in for now
  changeMotorSpeed(wheel, spd);
  Serial.print("w;");
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
      minChars = 3;
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

/**
 * error - An error handler to send errors back to the computer
 * err_no (int) The error number (it's advisable to use the error constants
 */
void error(int err_no) {
  Serial.print("e");
  Serial.print(err_no);
  Serial.print(";");
/*  switch (err_no) {
    case ERROR_NO_CMD:
      Serial.print("Command doesn't exist.");
      break;
    case ERROR_LT_CHARS:
      Serial.print("Not enough characters have been provided for the function.");
      break;
    case ERROR_WHEEL_OUT_OF_RANGE:
      Serial.print("Selected wheel does not exist.");
      break;
    default:
      Serial.print("Generic error");
  }*/
}

/**
 * changeAllMotors - A helper function to change all motor speeds at the same time
 * Params:
 *   fl_motor (int) The speed for the front left motor
 *   fr_motor (int) The speed for the front right motor
 *   bl_motor (int) The speed for the back left motor
 *   br_motor (int) The speed for the back right motor
 */
void changeAllMotors(int fl_motor, int fr_motor, int bl_motor, int br_motor) {
  changeMotorSpeed(0, fl_motor);
  changeMotorSpeed(1, fr_motor);
  changeMotorSpeed(2, bl_motor);
  changeMotorSpeed(3, br_motor);
}

/**
 * changeMotorSpeed - change the speed of a selected motor
 * Params:
 *   motor (int) Which motor to change the speed of, range 0 to 3
 *   spd (int) The speed to change the motor too
 */
void changeMotorSpeed(int motor, int spd) {
  if (motor < 0 || motor > 3) { // Ensure the motor exists
    error(ERROR_WHEEL_OUT_OF_RANGE);
    return;
  }
  /*if (spd < -MAX_SPEED) // Ensure the speed is within range
    spd = -MAX_SPEED;
  else if (spd > MAX_SPEED)
    spd = MAX_SPEED;*/
  if (motor == 0 || motor == 2) // Correct direction for left side as the servos are mounted the opposite way round
    spd *= -1;
  spd = spd + 90; // Turn the speed into a servo setting
  wheels[motor].write(spd);
}
