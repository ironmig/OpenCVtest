#include "TowerTracker.h"

TowerTracker::TowerTracker(ThresholdValues t) {
	thresh = t;
	data = Data{};
	frame = Mat(height,width,CV_8UC3); //inital frame
	blur_frame = Mat(height,width,CV_8UC3); //blured frame
	hsv_frame = Mat(height,width,CV_8UC3); //converted to HSV frame
	binary_frame = Mat(height,width,CV_8UC1);
	erode_element = getStructuringElement(MORPH_RECT,Size(erode_kernel_size,erode_kernel_size), Point(-1,1));
	dilate_element = getStructuringElement(MORPH_RECT,Size(dilate_kernel_size,dilate_kernel_size),Point(-1,1));
	contours = std::vector < std::vector <Point> >();
	rectangles = std::vector < RotatedRect >();
	cap = cv::VideoCapture(1);
	isRunning = false;

	if (!cap.isOpened())
	{
		std::cout << "Camera opening error" << std::endl;
		return;
	}
	SetCamSettings();
}
void TowerTracker::ThresholdFrame()
{
	inRange(hsv_frame,cv::Scalar(thresh.HLOW,thresh.SLOW,thresh.VLOW),Scalar(thresh.HHIGH,thresh.SHIGH,thresh.VHIGH),binary_frame);
}
void TowerTracker::ErodeFrame()
{
	erode(binary_frame,binary_frame,erode_element);
}
void TowerTracker::DilateFrame()
{
	dilate(binary_frame,binary_frame,dilate_element);
}
bool TowerTracker::RectangleSorter(RotatedRect x, RotatedRect y)
{
	return x.size.area() > y.size.area();
}
void TowerTracker::GetRectangles()
{
	std::cout << "Getting rects. Contours: " << contours.size() << std::endl;
	  for (uint i = 0; i < contours.size(); i ++)
	  {
		  rectangles.push_back(minAreaRect (contours[i]));
	  }
}
void TowerTracker::GetContours ()
{
	findContours(binary_frame, contours, CV_RETR_LIST, CV_CHAIN_APPROX_NONE);
}
float TowerTracker::RectangleRatio(RotatedRect x)
{
	return x.size.height/x.size.width;
}
void TowerTracker::GetCorrectRect()
{
	if (rectangles.size() >= 1)
	{
		float maxArea = 0;
		int highest = 0;
		for (uint i = 0; i < rectangles.size(); i ++)
		{
			if (rectangles.at(i).size.area() > maxArea) {
				highest = i;
				maxArea = rectangles.at(i).size.area();
			}
		}


//		float ratio = RectangleRatio(rectangles.at(highest));
//		std::cout << "Ratio: " << ratio << std::endl;
//		if (ratio >= minRectRatio && ratio <= maxRectRatio)
//		{
			r = rectangles.at(highest);
//		} else {
//			throw std::string("Largest Rectangle not within ratio constraint");
//		}
	}
	else throw std::string("Empty Rectangles Vector");
}
void TowerTracker::ConvertToHSV ()
{
	cvtColor(blur_frame,hsv_frame,CV_BGR2HSV);
}
void TowerTracker::BlurFrame()
{
	medianBlur(frame,blur_frame,blur_kernel_size);
}
void TowerTracker::SetCamSettings()
{
	cap.set(CAP_PROP_FRAME_WIDTH,width);
	cap.set(CAP_PROP_FRAME_HEIGHT, height);
	cap.set(CAP_PROP_FPS, frameRate);
	cap.set(CAP_PROP_BRIGHTNESS,brightness);
}
void TowerTracker::ProcessRect()
{
	//Draws outline of rect on screen
	Point2f pts[4];
	r.points(pts);
	rectangle(frame,pts[0],pts[2],Scalar(255,255,255));

	//Draws dot at top center of rectangle
	Point center (r.center.x,r.center.y-r.size.height/2);
	circle(frame,center,5, Scalar(255,0,0),5);

	//Calculates horizontal off center-ness of rectangle
	//negative if to left, positive if to right
	float error = center.x - frameCenterX;

	//Calculates the proportion of the frame taken up by rectangle
	float areaProportion= r.size.area() / frameArea;

	//Prints out error and area, error is positive if approaching, - if moving away
	std::cout << " Area : " << areaProportion << " Horizontal Error: " << error << std::endl;

	{
		std::lock_guard<std::mutex> lock(data_mut);
		data.Area = areaProportion;
		data.CenterX = r.center.x;
		data.CenterY = pts[0].y;
	}
}
TowerTracker::Data TowerTracker::GetData()
{
	std::lock_guard<std::mutex> l(data_mut);
	return data;
}
bool TowerTracker::KeepRunning()
{
	std::lock_guard<std::mutex> l(running_mut);
	return isRunning;
}
void TowerTracker::Stop()
{
	{
		std::lock_guard<std::mutex> l(running_mut);
		isRunning = false;
	}
	if (runThread.joinable())
	{
		runThread.join();
	} else {
		throw "Could not join thread";
	}
}
void TowerTracker::Start()
{
	std::cout << "Entered start" << std::endl;
	if (!isRunning)	{
		std::cout << "Not running" << std::endl;
		{
			std::lock_guard<std::mutex> l(running_mut);
			isRunning = true;
		}
		isRunning = true;
		runThread = std::thread(&TowerTracker::run,this);
	}
	std::cout << "Exit start" << std::endl;
}
void TowerTracker::run()
{
	#if DEBUG_GUI
	namedWindow(mainWindow, WINDOW_AUTOSIZE );
	namedWindow(altWindow,WINDOW_NORMAL);
	#endif

	std::cout << "running" << std::endl;
	while (true)
	{
		int key = waitKey(200) & 255;
		if (key == 27) {
			break;
		}
		std::cout << "Loop start " << std::endl;
		contours.clear();
		rectangles.clear();

		if (!cap.read(frame))
		{
			std::cout << "Could not read frame" << std::endl;
			continue;
		}
		std::cout << "Read frame" << std::endl;
		BlurFrame();
		ConvertToHSV ();
		ThresholdFrame();
		ErodeFrame();
		DilateFrame();
		GetContours();
		GetRectangles();
		std::cout << "Rects: " << rectangles.size() << std::endl;
		//std::sort(rectangles.begin(),rectangles.end(),RectangleSorter);
		std::cout << "Rects sorted: " << rectangles.size() << std::endl;
		std::cout << "Sorted" << std::endl;

		//Try to find a good target rectangle
		try {
			GetCorrectRect();
			ProcessRect();
		}
		catch (std::string s) {
			std::cout << s << std::endl;
		}
		catch (...) {
			std::cout << "Could not get a good rectangle" << std::endl;
		}

		//Debugging GUI Stuff
		#if DEBUG_GUI
		circle(frame,Point(frame.cols/2,frame.rows/2),5,Scalar(255,0,0),5);
		drawContours(frame,contours,-1,Scalar(0,255,0),3);
		imshow(mainWindow, binary_frame);
		imshow(altWindow,frame);
		#endif
	}
}
TowerTracker::~TowerTracker()
{

}
