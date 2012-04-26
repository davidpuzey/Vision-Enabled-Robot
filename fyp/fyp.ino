
#include <Servo.h> 
 
Servo front_left, front_right, back_left, back_right;
int spd, fwd, bck, inc;
String input;

void setup() {
  spd = 20;
  inc = 1;
  Serial.begin(9600);
  /**
   * Set up wheel servos.
   * Pins 3, 5, 6 and 9 are being used for the wheels
   */
  front_left.attach(3);
  front_right.attach(5);
  back_left.attach(6);
  back_right.attach(9);
}
 
 
void loop() {
  //if (Serial.available() > 0) {
  //  input = "";
  //  while (Serial.available() > 0)
  //    input += Serial.read();
  //}
  //spd = atoi(input);
  spd = getIntegerFromSerial(spd);
    
  //spd = 90;
  fwd = spd + 90;
  bck = (spd * -1) + 90;
  front_left.write(bck);
  front_right.write(fwd);
  back_left.write(bck);
  back_right.write(fwd);
/*  if (spd <= 1 || spd >= 20) 
    inc *= -1;
  spd += inc;
  delay(100);
*/
}

int getIntegerFromSerial(int defInt) {
  int arrLen = 6;
  int i = 0;
  char arrIn[arrLen];
  if (Serial.available() <= 0)
    return defInt;
  while (Serial.available() > 0 && i < arrLen) {
    arrIn[i] = Serial.read();
    i++;
  }
  Serial.println(atoi(arrIn));
  delay(1000); 
  return atoi(arrIn);
}


