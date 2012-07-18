Vision-Enabled-Robot
====================

Folders:
 - fyp - an old Arduino test folder
 - interface - current arduino code
 - vision - opencv vision code
            *ball_detect.cpp - The vision code
            *example.cpp - An example created by opencv I used to start getting a handle on it
            *build_all.sh - A script I took from the opencv examples to quickly compile my code
				The xml files are used with the example
 - control - control interface, using serial communication to send commands to the robot
             *control.cpp - The actual control code
             *rs232.c - The library used to set up serial communication
 - overall - combination of vision and control code, also has added code for moving the
             platform based to keep the ball centred, this part is untested but in theory
             it should work.
				 *overall.cpp - The combination of control.cpp and ball_detect.cpp
             *rs232.c - The library used to set up serial communication
             *build_all.sh - A script I took from the opencv examples to quickly compile my code, this needs modifying to include rs232.c in compilation
 - xbeeConfiguration - A program to configure the xbee modules
                       xbeeConfiguration.cpp - The program to configure the xbee modules, it sends the characters to put it into configuration mode, once it returns 'OK' you can start sending configuration commands
                       *rs232.c - The library used to set up serial communication