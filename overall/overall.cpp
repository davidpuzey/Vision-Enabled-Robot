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
// CvFont font;
//cvinitFont(&font, CV_FONT_HERSHEY_SIMPLEX, 1.0,1.0,0,1,CV_AA);
//cvPutText(im, "text here", cvPoint(10,130), &font, cvScalar(255,255,255,0));
//putText(frame, area, cvPoint(0,0), FONT_HERSHEY_SIMPLEX, 10, 

	int err, bsize=0, platformX=90, platformY=90,
		cport_nr=0,        /* /dev/ttyS0 (COM1 on windows) */
		bdrate=9600;       /* 9600 baud */
	unsigned char buf[4096];
	Size frameSize, midpoint, coordOffset, coordChange, offset, servoChange;
	
	
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
		midpoint = Size(frameSize.width / 2, frameSize.height / 2); // The midpoint of the image
		
		//GaussianBlur(frame, frame, Size(5,5), 1.2, 1.2);
		//erode(frame, frame, Mat());
		cvtColor(frame, hsvFrame, CV_BGR2HSV); // convert to the hsv colour space for easier detection
		inRange(hsvFrame, Scalar(50, 30, 30), Scalar(90, 255, 255), thresholdFrame); // find the object by colour
		
		Moments moment = moments(thresholdFrame, true);
		double moment10 = moment.m10;
		double moment01 = moment.m01;
		double area = moment.m00; // determine the area (using centre of gravity)
		
		int posx = moment10 / area; // X position of the object
		int posy = moment01 / area; // Y position of the object
		// Radius - A=pi*r^2 --- r=sqrt(A/pi)
		int radius = sqrt(area/PI);

		if (area > 50)
			circle(frame, Point(posx,posy), radius, Scalar(100,50,0), 4, 8, 0); // add a circle around the ball for displaying
		
		offset = Size(midpoint.width - posx, midpoint.height - posy); // How much the object is offset from the centre of the image
		
		// TODO improve by not moving if the coordinates are within a certain radius of the centre of the image
		servoChange = Size((offset.width/frameSize.width)*180, (offset.height/frameSize.height)*180); // How much to modify the servos by. Determined by scaling the objects offset from the centre onto the 180 degress of the servos
		platformX += servoChange.width; // Set the new servo x position
		platformY += servoChange.height; // Set the new servo y position
		if (platformX < 0) platformX = 0;
		if (platformX > 180) platformX = 180;
		if (platformY < 0) platformY = 0;
		if (platformY > 180) platformY = 180;
		buf[0] = 'p'; // platform move command
		buf[1] = platformX; // x position
		buf[2] = platformY; // y position
		SendBuf(cport_nr, buf, 3); // Send the command
		
		imshow("Detected Ball", frame); // display the image
		//imshow("tmp frame", tmpFrame);
		
		int exit = waitKey(10);
		if((char)exit == 'q')
			break;
	}
	return 0;
}
