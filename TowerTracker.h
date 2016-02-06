#ifndef SRC_TOWERTRACKER_H_
#define SRC_TOWERTRACKER_H_

//OpenCV test
#include "opencv2/opencv.hpp"
#include  <algorithm>

#define DEBUG_GUI true

using namespace cv;
class TowerTracker
{
private:
	struct ThresholdValues {
		double HLOW;
		double HHIGH;
		double SLOW;
		double SHIGH;
		double VLOW;
		double VHIGH;
	};
	static void ThresholdFrame(cv::Mat& src, cv::Mat& dest, TowerTracker::ThresholdValues thresh);
	static void ErodeFrame(cv::Mat& src, cv::Mat& dest, cv::Mat erode_src);
	static void DilateFrame(cv::Mat& src, cv::Mat& dest, cv::Mat dilate_src);
	static bool RectangleSorter(RotatedRect x, RotatedRect y);
	static void GetContours (InputOutputArray image, OutputArrayOfArrays contours);
	static void ConvertToHSV(Mat& src, Mat& dest);
	static void BlurFrame(Mat& src, Mat& dest, int kernel_size);
	static void GetRectangles(std::vector < std::vector <Point> > contours, std::vector<RotatedRect>* rectangles);

	const int height = 640;
	const int width = 480;
	const int frameRate = 30;
	const double brightness = 0; //115.0/255;
	const int blur_kernel_size = 3;
	const int erode_kernel_size = 3;
	const int dilate_kernel_size = 3;
	const struct ThresholdValues thresh = {77.5,87.5,204,242.25,242.25,255};

	cv::VideoCapture cap;
	Mat frame;
	Mat blur_frame;
	Mat hsv_frame;
	Mat binary_frame;
	Mat erode_element;
	Mat dilate_element;
	std::vector < std::vector <Point> > contours;
	std::vector < RotatedRect > rectangles;
	float frameCenterX;
	#if DEBUG_GUI
	const std::string mainWindow = "main";
	const std::string altWindow = "alt";

	//
	const float minRatio = .02;
	const float maxRatio = .05;

	const float minRectRatio = 0.83;
	const float maxRectRatio = 0.97;

	#endif
public:
	TowerTracker();
	void run();
	virtual ~TowerTracker();
};

#endif /* SRC_TRACKTOWER_H_ */
