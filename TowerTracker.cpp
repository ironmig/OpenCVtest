#include "TowerTracker.h"

TowerTracker::TowerTracker(ThresholdValues t) {
	thresh = t;
	data.reset(new Data{0,0,0});
	frame = Mat(height,width,CV_8UC3); //inital frame
	blur_frame = Mat(height,width,CV_8UC3); //blured frame
	hsv_frame = Mat(height,width,CV_8UC3); //converted to HSV frame
	binary_frame = Mat(height,width,CV_8UC1);
	erode_element = getStructuringElement(MORPH_RECT,Size(erode_kernel_size,erode_kernel_size), Point(-1,1));
	dilate_element = getStructuringElement(MORPH_RECT,Size(dilate_kernel_size,dilate_kernel_size),Point(-1,1));
	contours = std::vector < std::vector <Point> >();
	rectangles = std::vector < RotatedRect >();
	cap = cv::VideoCapture(0);
	frameCenterX = frame.cols/2;
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
	  for (std::vector<Point > cur : this->contours)
	  {
		  rectangles.push_back(minAreaRect (cur));
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
		float ratio = RectangleRatio(rectangles.at(0));
		if (ratio >= minRectRatio && ratio <= maxRectRatio)
		{
			r = rectangles.at(0);
		} else {
			throw std::string("Largest Rectangle not within ratio constraint");
		}
	}
	throw std::string("Empty Rectangles Vector");
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
	rectangle(frame,pts[0],pts[2],Scalar(0,0,0));

	//Draws dot at top center of rectangle
	float centerX = r.center.x;
	Point p = Point(r.center.x,pts[0].y);
	circle(frame,p, 5, Scalar(0,0,255),5);

	//Calculates horizontal off center-ness of rectangle
	float error = centerX - frameCenterX;

	//Calculates the proportion of the frame taken up by rectangle
	float areaProportion= r.size.area() / frameArea;

	//Prints out error and area, error is positive if approaching, - if moving away
	std::cout << " Area : " << areaProportion << " Horizontal Error: " << error << std::endl;
}
void TowerTracker::run()
{
	#if DEBUG_GUI
	namedWindow(mainWindow, WINDOW_AUTOSIZE );
	namedWindow(altWindow,WINDOW_NORMAL);
	#endif

	if (!cap.isOpened())
	{
		std::cout << "Camera opening error" << std::endl;
		return;
	}
	SetCamSettings();

	int key;
	while (true)
	{
		key = waitKey(200) & 255;
		if (key == 27)break;

		contours.clear();
		rectangles.clear();

		if (!cap.read(frame))
		{
			std::cout << "Could not read frame" << std::endl;
			continue;
		}

		BlurFrame();
		ConvertToHSV ();
		ThresholdFrame();
		ErodeFrame();
		DilateFrame();
		GetContours();
		GetRectangles();
		std::sort(rectangles.begin(),rectangles.end(),RectangleSorter);

		#if DEBUG_GUI
		//Debugging GUI Stuff
		circle(frame,Point(frame.cols/2,frame.rows/2),5,Scalar(255,0,0),5);
		drawContours(frame,contours,-1,Scalar(0,255,0),3);
		imshow(mainWindow, binary_frame);
		imshow(altWindow,frame);
		#endif

		//Try to find a good target rectangle
		try {
			GetCorrectRect();
		}
		catch (std::string* s) {
			std::cout << *s << std::endl;
			continue;
		}
		catch (...) {
			std::cout << "Could not get a good rectangle" << std::endl;
			continue;
		}

		ProcessRect();

	}
}
TowerTracker::~TowerTracker()
{

}
