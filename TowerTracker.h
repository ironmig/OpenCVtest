#ifndef SRC_TOWERTRACKER_H_
#define SRC_TOWERTRACKER_H_

//OpenCV test
#include "opencv2/opencv.hpp"
#include <algorithm>
#include <memory>
#include <string>
#include <mutex>
#include <thread>

#define DEBUG_GUI true

using namespace cv;
class TowerTracker
{
public:
	void run();
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
	Data GetData();
	void Start();
	void Stop();
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
	bool KeepRunning();


	const int height = 480;
	const int width = 640;
	const int frameRate = 30;
	const double brightness = 0; //115.0/255;
	const int blur_kernel_size = 3;
	const int erode_kernel_size = 3;
	const int dilate_kernel_size = 3;
	const float minRectRatio = 0.6 - .15; //minimum ratio (length divided by width) allowed to filter false positives
	const float maxRectRatio = 0.6 + .15; //maximum ratio (length divided by width) allowed to filter false positives
	const int frameCenterX = width/2;
	const float frameArea = height*width;
	const float minArea= .02;
	const float maxArea = .05;

	cv::VideoCapture cap;
	bool isRunning;
	std::mutex running_mut;
	Mat frame,blur_frame,hsv_frame,binary_frame,erode_element,dilate_element;
	std::vector < std::vector <Point> > contours;
	std::vector < RotatedRect > rectangles;
	struct ThresholdValues thresh;
	Data data;
	RotatedRect r;
	std::mutex data_mut;
	std::thread runThread;

	#if DEBUG_GUI
	const std::string mainWindow = "main";
	const std::string altWindow = "alt";
	#endif
};

#endif /* SRC_TRACKTOWER_H_ */
