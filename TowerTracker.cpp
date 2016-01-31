#include "TowerTracker.h"

TowerTracker::TowerTracker() {
	frame = Mat(height,width,CV_8UC3); //inital frame
	blur_frame = Mat(height,width,CV_8UC3); //blured frame
	hsv_frame = Mat(height,width,CV_8UC3); //converted to HSV frame
	binary_frame = Mat(height,width,CV_8UC1);
	erode_element = getStructuringElement(MORPH_RECT,Size(erode_kernel_size,erode_kernel_size), Point(-1,1));
	dilate_element = getStructuringElement(MORPH_RECT,Size(dilate_kernel_size,dilate_kernel_size),Point(-1,1));
	contours = std::vector < std::vector <Point> >();
	rectangles = std::vector < RotatedRect >();
	cap = cv::VideoCapture(1);
}
void TowerTracker::ThresholdFrame(cv::Mat& src, cv::Mat& dest, TowerTracker::ThresholdValues thresh)
{
	inRange(src,cv::Scalar(thresh.HLOW,thresh.SLOW,thresh.VLOW),Scalar(thresh.HHIGH,thresh.SHIGH,thresh.VHIGH),dest);
}
void TowerTracker::ErodeFrame(cv::Mat& src, cv::Mat& dest, cv::Mat erode_src)
{
	erode(src,dest, erode_src);
}
void TowerTracker::DilateFrame(cv::Mat& src, cv::Mat& dest, cv::Mat dilate_src)
{
	dilate(src,dest,dilate_src);
}
bool TowerTracker::RectangleSorter(RotatedRect x, RotatedRect y)
{
	return x.size.area() > y.size.area();
}
void TowerTracker::GetRectangles(std::vector < std::vector <Point> > contours,std::vector<RotatedRect>* rectangles)
{
	  for (std::vector<Point > cur : contours)
	  {
		  rectangles->push_back(minAreaRect (cur));
	  }
}
void TowerTracker::GetContours (InputOutputArray image, OutputArrayOfArrays contours)
{
	findContours(image, contours, CV_RETR_LIST, CV_CHAIN_APPROX_NONE);
}
void TowerTracker::ConvertToHSV (Mat& src, Mat& dest)
{
	cvtColor(src,dest,CV_BGR2HSV);
}
void TowerTracker::BlurFrame(Mat& src, Mat& dest, int kernel_size)
{
	medianBlur(src,dest,kernel_size);
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
	cap.set(CAP_PROP_FRAME_WIDTH,width);
	cap.set(CAP_PROP_FRAME_HEIGHT, height);
	cap.set(CAP_PROP_FPS, frameRate);
	cap.set(CAP_PROP_BRIGHTNESS,brightness);

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

		BlurFrame(frame,blur_frame,blur_kernel_size);
		ConvertToHSV (blur_frame,hsv_frame);
		ThresholdFrame(hsv_frame,binary_frame,thresh);
		ErodeFrame(binary_frame,binary_frame, erode_element);
		DilateFrame(binary_frame,binary_frame, dilate_element);
		GetContours (binary_frame, contours);
		GetRectangles(contours,&rectangles);
		std::sort(rectangles.begin(),rectangles.end(),RectangleSorter);

		#if DEBUG_GUI
		//Debugging GUI Stuff
		if (rectangles.size() >= 1)
		{
			Point2f pts[4];
			rectangles.at(0).points(pts);
			rectangle(frame,pts[0],pts[2],Scalar(0,0,0));
			std::cout << rectangles.at(0).size.area() << std::endl << std::endl;
		}

		drawContours(frame,contours,-1,Scalar(0,255,0),3);
		imshow(mainWindow, binary_frame);
		imshow(altWindow,frame);
		#endif
	}
}
TowerTracker::~TowerTracker()
{

}
