#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "rs232.h"

#include <iostream>
#include <stdio.h>
#include <cmath>
#include <pthread.h>
#include <stdarg.h>

using namespace std;
using namespace cv;

void onMouse(int, int, int, int, void*);
void semGive(bool &);
bool semTake(bool &);
void *t_objectPosition(void*);
void *t_serialReceive(void*);
void serialSend(char, ...);
void *t_platformPosition(void*);
void *t_wheelMovement(void*);

const double PI = 3.141592;
int cport_nr = 0; // 0 is Arduino com 0 and 1 is Arduino com 1
int spdMove = 90; // Keeping track of the velocity of the robot, start with stationary - TODO Look into sending signed integers ... may already be doing this, ie rather than between 0 & 180 with 90 as the centre, having 0 as the centre and going between -90 and 90
int spdTurn = 90; // Keeping track of the speed of the robot, start with not at all - TODO See above todo (it applies here too)
int radius; // The radius of the ball
bool isRunning = true; // Flag used to ensure that the program is stil running, if set to false then the while loops in the threads will end
// Flags to determine when the robot has completed the command
bool isP = true, isM = true, isT = true;
Point offset(0,0), servoChange(0,0), platform(90,90), mouseCoords(0,0);
stringstream textCoords, serialRet;


// TODO Add error handling stuffs
int main(int argc, const char** argv) {
	int bdrate=9600; // Baud rate 9600
	pthread_t cameraThread, serialThread, platformThread, wheelThread;
	
	
	if(OpenComport(cport_nr, bdrate)) {
		printf("Can not open comport\n");
		return(0);
	}

	
	// Centre the platform
	//buf[0] = 'p';
	//buf[1] = platform.x;
	//buf[2] = platform.y;
	//SendBuf(cport_nr, buf, 3);
	//char buf[2] = {platform.x,platform.y};
	serialSend('p', platform.x, platform.y);

	if (pthread_create(&cameraThread, NULL, t_objectPosition, NULL) != 0) {
		printf("Camera thread could not be created.\n");
		exit(1);
	}
	if (pthread_create(&serialThread, NULL, t_serialReceive, NULL) != 0) {
		printf("Serial thread could not be createa.d\n");
		exit(1);
	}
	if (pthread_create(&platformThread, NULL, t_platformPosition, NULL) != 0) {
		printf("Platform thread could not be created.\n");
		exit(1);
	}
	if (pthread_create(&wheelThread, NULL, t_wheelMovement, NULL) != 0) {
		printf("Wheel thread could not be created.\n");
		exit(1);
	}
	pthread_join(cameraThread, NULL);
	pthread_join(serialThread, NULL);
	pthread_join(platformThread, NULL);
	pthread_join(wheelThread, NULL);
	
	CloseComport(cport_nr);
	
	exit(0);
}

/**
 * onMouse - Used to set the current x & y coordinates of the mouse over the hsv image
 */
void onMouse(int event, int x, int y, int, void*) {
	if (event != CV_EVENT_MOUSEMOVE)
		return;
	mouseCoords = Point(x,y);
}

/**
 * semGive - Poorly simulate semaphores, sets a given boolean value to true;
 * Params:
 *    sem - The boolean variable to set
 */
void semGive(bool &sem) {
	sem = true;
}

/**
 * semTake - Poorly simulate semaphores, returns the value of the semaphore and sets it to false
 * Params:
 *    sem - The boolean variable to get
 */
bool semTake(bool &sem) {
	bool oldSem = sem;
	sem = false;
	return true; // As there is a bit of a problem with them at the moment, i'm just gonna pretend like this function doesn't exist TODO fix it
	return oldSem;
}

/**
 * t_objectPosition - Thread that uses OpenCV to detect objects from a camera
 *                  The coordinates of the object are then stored in a global
 *                  variable which can then be accessed by other threads.
 */
void *t_objectPosition(void *param) {
	Mat frame, hsvFrame, thresholdFrame; // Frames
	Size frameSize;
	Point objectPos, midpoint;
	Moments moment;
	double area;
	stringstream strArea, strHSVCoords;
	Vec3b hsvPixel;
	
	VideoCapture capture(1); // Open camera 1
	
	if (!capture.isOpened()) {
		printf("Could not capture from camera.");
		exit(1);
	}
	
	capture >> frame;
	
	if (frame.empty()) {
		printf("Could not get frame.");
		exit(2);
	}
	
	frameSize = Size(frame.cols, frame.rows); // The frame size
	midpoint = Point(frameSize.width / 2, frameSize.height / 2); // The midpoint of the image
	
	while (isRunning) {
		capture >> frame;
		
		if (frame.empty()) {
			printf("Could not get frame.");
			exit(2);
		}
		
		//GaussianBlur(frame, frame, Size(1,1), 0.5, 0.5);
		GaussianBlur(frame, frame, Size(3,3), 1.2, 1.2);
		erode(frame, frame, Mat());
		cvtColor(frame, hsvFrame, CV_BGR2HSV); // convert to the hsv colour space for easier detection
		inRange(hsvFrame, Scalar(70, 160, 50), Scalar(100, 255, 255), thresholdFrame); // find the object by colour
		GaussianBlur(thresholdFrame, thresholdFrame, Size(9,9), 1.2, 1.2);
		/*vector<Vec3f> circles;
		HoughCircles(thresholdFrame, circles, CV_HOUGH_GRADIENT, 2, 10, 200, 100); // TODO find the final parameter, the minimum distance between circles
		//            (input array   , output , detection method , dp,minDist)
		for (size_t i = 0; i < circles.size(); i++) {
			Point centre(cvRound(circles[i][0]), cvRound(circles[i][1]));
			int tmpRadius = cvRound(circles[i][2]);
			circle(frame, centre, 3, Scalar(0,255,0), -1, 8, 0);
			circle(frame, centre, tmpRadius, Scalar(0,0,255), 3, 8, 0);
		}*/
		
		
		/* moments:
		 * 	m00 - area
		 * 	m10 - for working out the X position
		 * 	m01 - for working out the Y position
		 */
		moment = moments(thresholdFrame, true);
		area = moment.m00; // determine the area (using centre of gravity)
		if (area > 200) { // Stop unwanted anomalies, we probably can't do anything if the ball is this far away anyway
			objectPos = Point(moment.m10 / area, moment.m01 / area); // The X and Y coordinates
			// Radius - A=pi*r^2 --- r=sqrt(A/pi)
			radius = sqrt(area/PI);
			
			circle(frame, objectPos, radius, Scalar(100,50,0), 4, 8, 0); // add a circle around the ball for displaying
			
			offset = midpoint - objectPos; // How much the object is offset from the centre of the image
		} else {
			offset = Point(0,0);
		}
		
		strArea.str("");
		strArea << area;
		setMouseCallback("hsv frame", onMouse);
		hsvPixel = hsvFrame.at<Vec3b>(mouseCoords.x,mouseCoords.y);
		strHSVCoords.str("");
		strHSVCoords << (int)hsvPixel[0] << "," << (int)hsvPixel[1] << "," << (int)hsvPixel[2];
		putText(frame, textCoords.str(), Point(0,20), FONT_HERSHEY_SIMPLEX, 0.6, Scalar::all(255));
		putText(frame, serialRet.str(), Point(0,40), FONT_HERSHEY_SIMPLEX, 0.6, Scalar::all(255));
		putText(frame, strArea.str(), Point(0,60), FONT_HERSHEY_SIMPLEX, 0.6, Scalar::all(255));
		putText(hsvFrame, strHSVCoords.str(), Point(0,20), FONT_HERSHEY_SIMPLEX, 0.6, Scalar::all(255));
		imshow("hsv frame", hsvFrame);
		imshow("Detected Ball", thresholdFrame);
		imshow("The Frame", frame);
		
		int exit = waitKey(10);
		if((char)exit == 'q')
			isRunning = false;
	}
	pthread_exit(NULL);
}

/**
 * t_serialReceive - Thread that deals with the serial communication
 *                   between the computer and the Arduino. It's main task
 *                   is to repeatedly poll the serial port so check for
 *                   new serial data from the Arduino TODO>>, it can also
 *                   send data to the Arduino from other threads ...
 *                   although I may just leave this up to the other
 *                   threads, we'll see when it's written <<TODO
 */
void *t_serialReceive(void *param) {
	int retChars;
	unsigned char retBuf[4096];
	
	while (isRunning) {
		retChars = PollComport(cport_nr, retBuf, 4096);
		if (retChars > 0) {
			serialRet << retBuf;
			printf("Received: %s\n", retBuf);
			switch (retBuf[0]) { // Hacky way of doing it, but it'll do for now TODO Make this better, properly seperate out returned commands and process them
				case 'p':
					semGive(isP);
					break;
				case 'm':
					semGive(isM);
					break;
				case 't':
					semGive(isT);
					break;
			}
		}
		usleep(100000);
	}
	pthread_exit(NULL);
}

/**
 * serialSend - Send serial data to the arduino
 * Params:
 *    command (char) The command to send
 *    ... (char) The rest of the parameters are the parameters for the command
 */
void serialSend(char command, ...) {
	va_list params;
	va_start(params, command);
	int len = 0;
	switch (command) {
		case 'p':
			len = 2;
			break;
		case 'o':
			len = 2;
			break;
		case 'm':
			len = 2;
			break;
		case 't':
			len = 2;
			break;
		case 'w':
			len = 3;
			break;
		case 'u':
			break;
		case 'i':
			break;
	}
	
	unsigned char buf[len+1]; // So we can concatonate the parameters to the command ready for sending
	buf[0] = command;
	printf("Sending: %c ", buf[0]);
	for (int i=0; i<len; i++) { // Add each of the parameters to the buffer
		buf[i+1] = va_arg(params, int); // char doesn't work here, but given that it is actually integers that will be passed in I suppose it isn't such an issue
		printf("%i ", buf[i+1]);
	}
	va_end(params);
	
	printf("\n");
	SendBuf(cport_nr, buf, len+1); // Send the command
}

/**
 * t_platformPosition - Thread to set the platforms position. The current object
 *                      offset is checked at regular intervals (somewhere
 *                      between 100ms and 1000ms) and the platform position is
 *                      then updated to this new position.
 */
void *t_platformPosition(void *param) {
	while (isRunning) {
		// ~TODO Improve by not moving if the coordinates are within a certain radius of the centre of the image
		// TODO In case the amount required to move is outside the bounds of possibility then turning the whole robot would be a good idea
		// TODO Think about using pythag to keep within a circle rather than a square:
		//      offset = offset^2;
		//      distance = sqrt(offset.x + offset.y);
		//      if (distance < circleRadius) then coord is within circle
		//      Need to choose the relevant radius ... perhaps start with 10
		// TODO Work out a the actual relevant coordinates to move too, as currently this isn't even remotely correct, it will overshoot massively
		//      The simplest way to do this is probably to move the platform 1 degree at a time ... however this isn't too efficient so need to work out a better method.
		//      TODO Measure the number of degress required to get the ball from one side of the screen to the other TODO
		//      Then use this to adjust the offset so that the offset is in degrees rather than pixels.
		servoChange = Point(cvRound(offset.x/20), cvRound(offset.y/20)); // How much to modify the servos by. Determined by scaling the objects offset from the centre onto the 180 degress of the servos
		
		textCoords.str("");
		textCoords << "Coords (" << servoChange.x << "," << servoChange.y << ") (" << platform.x << "," << platform.y << ") " << radius;
		
		//if (servoChange.x <  -20 || servoChange.x > 20) {
		//	platform.x = platform.x - servoChange.x; // Set the new servo x position
			if (servoChange.x > 0)
				platform.x -= 1;
			else if (servoChange.x < 0)
				platform.x += 1;
			if (platform.x < 0) platform.x = 0;
			if (platform.x > 180) platform.x = 180;
		//}
		//if (servoChange.y <  -10 || servoChange.y > 10) {
		//	platform.y = platform.y + servoChange.y; // Set the new servo y position
			if (servoChange.y < 0)
				platform.y -= 1;
			else if (servoChange.y > 0)
				platform.y += 1;
			if (platform.y < 0) platform.y = 0;
			if (platform.y > 180) platform.y = 180;
		//}
		/*buf[0] = 'p'; // platform move command
		buf[1] = platform.x; // x position
		buf[2] = platform.y; // y position
		buf[0] = 'o'; // platform move command
		buf[1] = offset.x; // x position
		buf[2] = offset.y; // y position*/
		//char buf[2] = {platform.x,platform.y};
		if ((servoChange.x != 0 || servoChange.y != 0) && semTake(isP))
			serialSend('p', platform.x, platform.y);
		
		usleep(100000);
	}
	pthread_exit(NULL);
}

/**
 * t_movement - Deals with the robots wheel movement. Turning & velocity etc
 */
void *t_wheelMovement(void *param) {
	int newSpdMove = 90;
	int newSpdTurn = 90;
	while (isRunning) {
		// Setting the speed of the robot, forward/backward etc
		// TODO base this on actual distance from ball not just the radius
		if (radius > 45)
			// This translates the radius into a negative velocity, so the robot can pull away from the ball when it is too close.
			// It ranges from 45 to 100 (thats 55 values, we have 20 speeds so 55/20=2.75)
			// Since we're moving backwards we want negative velocity so we take away from 90 (0 velocity)
			// So work out how much we need to take away we bring the radius range down by 45 (since we're only dealing with values over 45, this allows us to work with values from 0 rather than 45)
			// We then divide this by the 2.75 to translate the 0-55 scale to a 0-20 scale (speed doesn't change beyond +-20 :/)
			// It is of course important to convert this to an integer as the maths will give us a float
			newSpdMove = (int)(90-((radius-45)/2.75));
		else if (radius < 35 && radius > 5) // If the radius gets too small we want to stop moving as it's likely that the ball isn't there
			// This translates the radius into a positive velocity, so the robot can move towards the ball when it is too far away.
			// It ranges from 35 to 0 (thats 35 values, we have 20 speeds so 35/20=1.75)
			// Here we're moving forwards so we want a positive velocity this means we need add to 90 (0 velocity)
			// Since the range is between 0 and 35 we don't need to bring the scale down
			// However we need to reverse the sacle, 0-35 should translate to 20-0 (0 radius should be us 20 speed and 35 radius should give us 0 speed)
			// If it were 0-35 translated to 0-20 we could just do radius/1.75, however for the reverse scale we need to divide by -1.75 and then add 20 (the maximum value in the scale)
			// Once again we need to convert to an integer for sending to the robot
			newSpdMove = (int)(90+((radius/-1.75)+20));
		else
			newSpdMove = 90;
		if (newSpdMove != spdMove && semTake(isM)) { // Set the new speed only if it isn't already that speed ... no point in transmitting useless data
			spdMove = newSpdMove;
			serialSend('m', newSpdMove); // TODO Decide whether using spdMove or newSpdMove would be better here, can't see too much difference atm, but worth thinking about
		}
		
		// Turning the robot if the ball moves too far to one side ... maybe even turn to face after a period of the ball being to one side
		// TODO Work out a better solution for varying the turning, atm it will probably act seem weird when comparing the robot travelling at a high and low velocity
		// For the maths see the above explaination for spdMove, it's very similar, except both ranges are scaling 45 to 20 so we are using 2.25 (45/20)
		// TODO 0-20 may not be the best range here, something more complicated would give better results, however atm this is fine
		if (platform.x < 45) // Turning right
			newSpdTurn = (int)(90-((platform.x/-2.25)+20));
		else if (platform.x > 135) // Turning left
			newSpdTurn = (int)(90+((platform.x-135)/2.25));
		else
			newSpdTurn = 90;
		if (newSpdTurn != spdTurn && semTake(isT)) { // Set the new turn speed only if it isn't already turning at this speed ... no point in sending useless data
			spdTurn = newSpdTurn;
			serialSend('t', newSpdTurn); // TODO Decide whether using spdTurn or newSpdTurn would be better here, can't see too much difference atm, but worth thinking about
		}
		usleep(100000);
	}
	pthread_exit(NULL);
}

/*void nullFunction() {
	
	while (true) {
		capture >> frame;
		
		if (frame.empty()) {
			printf("Could not get frame.");
			return 2;
		}
		
		frameSize = Size(frame.cols, frame.rows); // The frame size
		midpoint = Point(frameSize.width / 2, frameSize.height / 2); // The midpoint of the image
		
		//GaussianBlur(frame, frame, Size(1,1), 0.5, 0.5);
		GaussianBlur(frame, frame, Size(3,3), 1.2, 1.2);
		erode(frame, frame, Mat());
		cvtColor(frame, hsvFrame, CV_BGR2HSV); // convert to the hsv colour space for easier detection
		inRange(hsvFrame, Scalar(70, 160, 50), Scalar(100, 255, 255), thresholdFrame); // find the object by colour
		GaussianBlur(thresholdFrame, thresholdFrame, Size(9,9), 1.2, 1.2);*/
		/*vector<Vec3f> circles;
		HoughCircles(thresholdFrame, circles, CV_HOUGH_GRADIENT, 2, 10, 200, 100); // TODO find the final parameter, the minimum distance between circles
		//            (input array   , output , detection method , dp,minDist)
		for (size_t i = 0; i < circles.size(); i++) {
			Point centre(cvRound(circles[i][0]), cvRound(circles[i][1]));
			int tmpRadius = cvRound(circles[i][2]);
			circle(frame, centre, 3, Scalar(0,255,0), -1, 8, 0);
			circle(frame, centre, tmpRadius, Scalar(0,0,255), 3, 8, 0);
		}*/
		
		
		/* moments:
		 * 	m00 - area
		 * 	m10 - for working out the X position
		 * 	m01 - for working out the Y position
		 */
/*		Moments moment = moments(thresholdFrame, true);
		double area = moment.m00; // determine the area (using centre of gravity)
		if (area > 0) {
			objectPos = Point(moment.m10 / area, moment.m01 / area); // The X and Y coordinates
			// Radius - A=pi*r^2 --- r=sqrt(A/pi)
			int radius = sqrt(area/PI);
			
			if (area > 50)
				circle(frame, objectPos, radius, Scalar(100,50,0), 4, 8, 0); // add a circle around the ball for displaying
			
			offset = midpoint - objectPos; // How much the object is offset from the centre of the image
*/			
			// ~TODO Improve by not moving if the coordinates are within a certain radius of the centre of the image
			// TODO In case the amount required to move is outside the bounds of possibility then turning the whole robot would be a good idea
			// TODO Think about using pythag to keep within a circle rather than a square:
			//      offset = offset^2;
			//      distance = sqrt(offset.x + offset.y);
			//      if (distance < circleRadius) then coord is within circle
			//      Need to choose the relevant radius ... perhaps start with 10
			// TODO Work out a the actual relevant coordinates to move too, as currently this isn't even remotely correct, it will overshoot massively
			//      The simplest way to do this is probably to move the platform 1 degree at a time ... however this isn't too efficient so need to work out a better method.
			//      TODO Measure the number of degress required to get the ball from one side of the screen to the other TODO
			//      Then use this to adjust the offset so that the offset is in degrees rather than pixels.
/*			servoChange = Point(cvRound(offset.x/20), cvRound(offset.y/20)); // How much to modify the servos by. Determined by scaling the objects offset from the centre onto the 180 degress of the servos
			
			textCoords.str("");
			textCoords << "Coords (" << servoChange.x << "," << servoChange.y << ") (" << platform.x << "," << platform.y << ")";
			putText(frame, textCoords.str(), Point(0,20), FONT_HERSHEY_SIMPLEX, 0.6, Scalar::all(255));
			
			//if (servoChange.x <  -20 || servoChange.x > 20) {
			//	platform.x = platform.x - servoChange.x; // Set the new servo x position
				if (offset.x > 0)
					platform.x -= 1;
				else if (offset.x < 0)
					platform.x += 1;
				if (platform.x < 0) platform.x = 0;
				if (platform.x > 180) platform.x = 180;
			//}
			//if (servoChange.y <  -10 || servoChange.y > 10) {
			//	platform.y = platform.y + servoChange.y; // Set the new servo y position
				if (offset.y < 0)
					platform.y -= 1;
				else if (offset.y > 0)
					platform.y += 1;
				if (platform.y < 0) platform.y = 0;
				if (platform.y > 180) platform.y = 180;
			//}
			buf[0] = 'p'; // platform move command
			buf[1] = platform.x; // x position
			buf[2] = platform.y; // y position
			//buf[0] = 'o'; // platform move command
			//buf[1] = offset.x; // x position
			//buf[2] = offset.y; // y position
			SendBuf(cport_nr, buf, 3); // Send the command
		}

		retChars = PollComport(cport_nr, retBuf, 4096);
		if (retChars > 0)
			serialRet << retBuf;
		putText(frame, serialRet.str(), Point(0,40), FONT_HERSHEY_SIMPLEX, 0.6, Scalar::all(255));
		
		imshow("hsv frame", hsvFrame);
		imshow("Detected Ball", thresholdFrame);
		imshow("The Frame", frame);
		
		int exit = waitKey(10);
		if((char)exit == 'q')
			break;
		//usleep(250000);
	}
	return 0;
}*/
