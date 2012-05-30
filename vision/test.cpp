#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include <iostream>
#include <stdio.h>

using namespace std;
using namespace cv;

int main(int argc, const char** argv) {
	Mat frame, hsvFrame, thresholdFrame; // Frames
	VideoCapture capture(0); // Open camera 0

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
		
		cvtColor(frame, hsvFrame, CV_BGR2HSV);
		inRange(hsvFrame, Scalar(15, 20, 50), Scalar(50, 100, 150), thresholdFrame);
		
		imshow("HSV Image", thresholdFrame);
		
		int exit = waitKey(10);
		if((char)exit == 'q')
			break;
	}
	return 0;
}
