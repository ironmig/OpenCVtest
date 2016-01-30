#include <opencv2/opencv.hpp>
using namespace cv;
int main ( int argc, char **argv )
{
  Mat frame = imread("/home/mike/Downloads/half-life-2-episode-one-2d.jpg", 1 );
  //VideoCapture cap(0);
  namedWindow("Display Image", WINDOW_NORMAL );
  imshow("Display Image", frame);
  waitKey(0);
  return 0;
}
