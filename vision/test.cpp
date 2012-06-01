#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include <iostream>
#include <stdio.h>

using namespace std;
using namespace cv;

int main(int argc, const char** argv) {
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
		
		GaussianBlur(frame, frame, Size(5,5), 1.2, 1.2);
		//erode(frame, frame, Mat());
		cvtColor(frame, hsvFrame, CV_BGR2HSV);
		inRange(hsvFrame, Scalar(34, 100, 100), Scalar(70, 255, 255), thresholdFrame);
		
		Moments moment = moments(thresholdFrame, true);
		double moment10 = moment.m10;
		double moment01 = moment.m01;
		double area = moment.m00;
		
		int posx = moment10 / area;
		int posy = moment01 / area;
		
		imshow("HSV Image", thresholdFrame);
		//imshow("tmp frame", tmpFrame);
		
		int exit = waitKey(10);
		if((char)exit == 'q')
			break;
	}
	return 0;
}
