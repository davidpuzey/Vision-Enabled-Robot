#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

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
//putText(frame, area, cvPoint(0,0), FONT_HERSHEY_SIMPLEX, 10, Scalar(255,255,255));

	Mat frame, hsvFrame, thresholdFrame; // Frames
	VideoCapture capture(1); // Open camera 0

	if (!capture.isOpened()) {
		printf("Could not capture from camera.");
		return 1;
	}
	
	while (true) {
		capture >> frame;
		
		if (frame.empty()) {
			printf("Could not get frame.");
			return 2;
		}
		
		//GaussianBlur(frame, frame, Size(5,5), 1.2, 1.2);
		//erode(frame, frame, Mat());
		cvtColor(frame, hsvFrame, CV_BGR2HSV);
		inRange(hsvFrame, Scalar(50, 30, 30), Scalar(90, 255, 255), thresholdFrame);
		
		Moments moment = moments(thresholdFrame, true);
		double moment10 = moment.m10;
		double moment01 = moment.m01;
		double area = moment.m00;
		
		int posx = moment10 / area;
		int posy = moment01 / area;
		// Radius - A=pi*r^2 --- r=sqrt(A/pi)
		int radius = sqrt(area/PI);

		if (area > 50)
			circle(frame, Point(posx,posy), radius, Scalar(100,50,0), 4, 8, 0);
		
		//putText(frame, area, cvPoint(10,10), FONT_HERSHEY_SIMPLEX, 10, Scalar(255,255,255));
		imshow("Detected Ball", frame);
		//imshow("tmp frame", tmpFrame);
		
		int exit = waitKey(10);
		if((char)exit == 'q')
			break;
	}
	return 0;
}
