#ifndef SRC_TOWERTRACKER_H_
#define SRC_TOWERTRACKER_H_

//OpenCV test
#include "opencv2/opencv.hpp"
#include <algorithm>
#include <memory>
#include <string>
#include <mutex>

#define DEBUG_GUI true

using namespace cv;
class TowerTracker
{
public:
	struct ThresholdValues {
		double HLOW;
		double HHIGH;
		double SLOW;
		double SHIGH;
		double VLOW;
		double VHIGH;
	};
	struct Data {
		float Area;
		float CenterX;
		float CenterY;
	};
	TowerTracker(ThresholdValues t);
	std::shared_ptr<Data> GetData();
	void run();
	void Lock();
	void Unlock();
	virtual ~TowerTracker();
private:
	static bool RectangleSorter(RotatedRect x, RotatedRect y);
	static float RectangleRatio(RotatedRect x);

	void SetCamSettings();
	void ThresholdFrame();
	void ErodeFrame();
	void DilateFrame();
	void ConvertToHSV();
	void BlurFrame();
	void GetContours ();
	void GetRectangles();
	void ProcessRect();
	void GetCorrectRect();
	const int height = 640;
	const int width = 480;
	const int frameRate = 30;
	const double brightness = 0; //115.0/255;
	const int blur_kernel_size = 3;
	const int erode_kernel_size = 3;
	const int dilate_kernel_size = 3;
	struct ThresholdValues thresh;

	cv::VideoCapture cap;
	Mat frame,blur_frame,hsv_frame,binary_frame,erode_element,dilate_element;
	std::vector < std::vector <Point> > contours;
	std::vector < RotatedRect > rectangles;

	float frameCenterX;
	const float frameArea = height*width;

	const float minArea= .02;
	const float maxArea = .05;

	//Defines threshold of rectangles' length/width ratio to filter false positives
	const float minRectRatio = 0.6 - .05;
	const float maxRectRatio = 0.6 + .05;

	std::shared_ptr<Data> data;
	RotatedRect r;

	#if DEBUG_GUI
	const std::string mainWindow = "main";
	const std::string altWindow = "alt";
	#endif
};

#endif /* SRC_TRACKTOWER_H_ */
