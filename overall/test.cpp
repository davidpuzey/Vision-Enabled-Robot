#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "rs232.h"

#include <iostream>
#include <stdio.h>
#include <cmath>

using namespace std;
using namespace cv;


int main(int argc, const char** argv) {

	Point pointA, pointB, pointC, pointD;
	Size sizeA;
	
	sizeA = Size(2,4);
	pointA = Point(9,6);
	pointB = Point(2,4);
	pointC = pointA - pointB;
	pointD = Point(pointA.x - sizeA.width, pointA.y - sizeA.height);
	printf("pointC (%d,%d) --- pointD (%d,%d)\n", pointC.x, pointC.y, pointD.x, pointD.y);
	return 0;
}
