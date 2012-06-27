#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "rs232.h"

#include <iostream>
#include <stdio.h>
#include <cmath>

using namespace std;
using namespace cv;

const double PI = 3.141592;

int main(int argc, const char** argv) {
	int err, bsize=0, platformX=90, platformY=90,
		cport_nr=0,        /* 0 is Arduino com 0 and 1 is Arduino com 1 */
		bdrate=9600;       /* 9600 baud */
	unsigned char buf[4096];
	Size frameSize;
	Point objectPos, midpoint, offset, servoChange;
	stringstream textCoords;
	
	
	if(OpenComport(cport_nr, bdrate)) {
		printf("Can not open comport\n");
		return(0);
	}

	Mat frame, hsvFrame, thresholdFrame; // Frames
	VideoCapture capture(1); // Open camera 0

	if (!capture.isOpened()) {
		printf("Could not capture from camera.");
		return 1;
	}
	
	// Centre the platform
	buf[0] = 'p';
	buf[1] = 90;
	buf[2] = 90;
	SendBuf(cport_nr, buf, 3);
	
	while (true) {
		capture >> frame;
		
		if (frame.empty()) {
			printf("Could not get frame.");
			return 2;
		}
		
		frameSize = Size(frame.cols, frame.rows); // The frame size
		midpoint = Point(frameSize.width / 2, frameSize.height / 2); // The midpoint of the image
		
		GaussianBlur(frame, frame, Size(1,1), 0.5, 0.5);
		//GaussianBlur(frame, frame, Size(3,3), 1.2, 1.2);
		//erode(frame, frame, Mat());
		cvtColor(frame, hsvFrame, CV_BGR2HSV); // convert to the hsv colour space for easier detection
		inRange(hsvFrame, Scalar(70, 160, 50), Scalar(100, 255, 255), thresholdFrame); // find the object by colour
		GaussianBlur(thresholdFrame, thresholdFrame, Size(9,9), 1.2, 1.2);
		
		/* moments:
		 * 	m00 - area
		 * 	m10 - for working out the X position
		 * 	m01 - for working out the Y position
		 */
		Moments moment = moments(thresholdFrame, true);
		double area = moment.m00; // determine the area (using centre of gravity)
		objectPos = Point(moment.m10 / area, moment.m01 / area); // The X and Y coordinates
		// Radius - A=pi*r^2 --- r=sqrt(A/pi)
		int radius = sqrt(area/PI);
		
		if (area > 50)
			circle(frame, objectPos, radius, Scalar(100,50,0), 4, 8, 0); // add a circle around the ball for displaying
		
		offset = midpoint - objectPos; // How much the object is offset from the centre of the image
		
		textCoords.str("");
		textCoords << "Coords (" << offset.x << "," << offset.y << ")";
		putText(frame, textCoords.str(), Point(0,20), FONT_HERSHEY_SIMPLEX, 0.6, Scalar::all(255));
		
		// TODO Improve by not moving if the coordinates are within a certain radius of the centre of the image
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
		servoChange = Point((offset.x/frameSize.width)*180, (offset.y/frameSize.height)*180); // How much to modify the servos by. Determined by scaling the objects offset from the centre onto the 180 degress of the servos
		if (servoChange.x <  -10 && servoChange.x > 10) {
			platformX += servoChange.x; // Set the new servo x position
			if (platformX < 0) platformX = 0;
			if (platformX > 180) platformX = 180;
		}
		if (servoChange.y <  -10 && servoChange.y > 10) {
			platformY += servoChange.y; // Set the new servo y position
			if (platformY < 0) platformY = 0;
			if (platformY > 180) platformY = 180;
		}
		buf[0] = 'p'; // platform move command
		buf[1] = platformX; // x position
		buf[2] = platformY; // y position
		SendBuf(cport_nr, buf, 3); // Send the command
		
		imshow("hsv frame", hsvFrame);
		imshow("Detected Ball", thresholdFrame); // display the image
		
		int exit = waitKey(10);
		if((char)exit == 'q')
			break;
	}
	return 0;
}
