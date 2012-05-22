#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include <iostream>
#include <stdio.h>

using namespace std;
using namespace cv;

int main(int argc, const char** argv) {
	IplImage* frame;
	CvCapture* capture;
	CvSize frameSize;
	IplImage* hsvFrame;

	capture;
	capture = cvCaptureFromCAM( 1 );
	if (!capture) {
		printf("Could not capture from camera");
		return 1;
	}
	
	frame = cvQueryFrame(capture);
	frameSize = cvGetSize(frame);
	hsvFrame = cvCreateImage(frameSize, IPL_DEPTH_8U, 3);
	
	while (true) {
		frame = cvQueryFrame(capture);
		
		if (!frame) {
			printf("Could not get frame.");
			return 2;
		}
		
		cvtColor(frame, hsvFrame, CV_BGR2HSV);
		
		imshow("Window", Mat(hsvFrame));
		
		int exit = waitKey(10);
		if((char)exit == 'e')
			break;
	}
	return 0;
}
