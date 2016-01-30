#include <opencv2/opencv.hpp>
using namespace cv;
int main ( int argc, char **argv )
{
	  Mat frame;
	  VideoCapture cap(1);
	  namedWindow("Display Image", WINDOW_NORMAL );
	  cap >> frame;
	  imshow("Display Image", frame);
	  waitKey(0);
	  return 0;
}
