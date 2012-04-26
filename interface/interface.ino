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
  
  Serial.flush(); // Clear out the buffer
}
