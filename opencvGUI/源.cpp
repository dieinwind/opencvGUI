#include <opencv2\highgui\highgui.hpp>

#include <iostream>

#include <nuitrack/Nuitrack.h>

#include <iomanip>
#include <iostream>


using namespace std;
using namespace cv;
using namespace tdv;

#define WINDOW_NAME "【程序窗口】"        //为窗口标题定义的宏 


void on_MouseHandle(int event, int x, int y, int flags, void* param);
void DrawCircle(Mat& img, Point centerPoint, int radius);

Point centerPoint;
int radius;

Rect g_rectangle;
bool g_bDrawingCircle = false;//是否进行绘制
RNG g_rng(12345);

// Callback for the hand data update event
void onHandUpdate(nuitrack::HandTrackerData::Ptr handData)
{
	if (!handData)
	{
		// No hand data
		std::cout << "No hand data" << std::endl;
		return;
	}

	auto userHands = handData->getUsersHands();
	if (userHands.empty())
	{
		// No user hands
		return;
	}

	auto rightHand = userHands[0].rightHand;
	if (!rightHand)
	{
		// No right hand
		std::cout << "Right hand of the first user is not found" << std::endl;
		return;
	}
	if (rightHand->zReal != 0)
	{
		centerPoint = Point(400 + rightHand->xReal*1.2, 400 - rightHand->yReal*1.2);
		radius = 50000 / rightHand->zReal;
	}

	std::cout << std::fixed << std::setprecision(3);
	std::cout << "Right hand position: "
		"x = " << rightHand->xReal << ", "
		"y = " << rightHand->yReal << ", "
		"z = " << rightHand->zReal << std::endl;
}

int main()
{
	//【1】准备参数
	Mat srcImage(1000, 1500, CV_8UC3), tempImage;
	srcImage.copyTo(tempImage);
	srcImage = Scalar::all(0);

	

	//【2】设置鼠标操作回调函数
	namedWindow(WINDOW_NAME);
	setMouseCallback(WINDOW_NAME, on_MouseHandle, (void*)&srcImage);

	//【3】程序主循环，当进行绘制的标识符为真时，进行绘制
	centerPoint = Point(400, 400);
	radius = 20;


	//
	std::string configPath = "";
	// Initialize Nuitrack
	try
	{
		nuitrack::Nuitrack::init(configPath);
	}
	catch (const nuitrack::Exception& e)
	{
		std::cerr << "Can not initialize Nuitrack (ExceptionType: " << e.type() << ")" << std::endl;
		return EXIT_FAILURE;
	}

	// Create HandTracker module, other required modules will be
	// created automatically
	auto handTracker = nuitrack::HandTracker::create();
	

	// Connect onHandUpdate callback to receive hand tracking data
	handTracker->connectOnUpdate(onHandUpdate);

	// Start Nuitrack
	try
	{
		nuitrack::Nuitrack::run();
	}
	catch (const nuitrack::Exception& e)
	{
		std::cerr << "Can not start Nuitrack (ExceptionType: " << e.type() << ")" << std::endl;
		return EXIT_FAILURE;
	}

	int errorCode = EXIT_SUCCESS;


	while (true)
	{
		try
		{
			// Wait for new hand tracking data
			nuitrack::Nuitrack::waitUpdate(handTracker);
		}
		catch (nuitrack::LicenseNotAcquiredException& e)
		{
			std::cerr << "LicenseNotAcquired exception (ExceptionType: " << e.type() << ")" << std::endl;
			errorCode = EXIT_FAILURE;
			break;
		}
		catch (const nuitrack::Exception& e)
		{
			std::cerr << "Nuitrack update failed (ExceptionType: " << e.type() << ")" << std::endl;
			errorCode = EXIT_FAILURE;
		}


		srcImage.copyTo(tempImage);//拷贝源图到临时变量
		DrawCircle(tempImage, centerPoint, radius);//当进行绘制的标识符为真，则进行绘制
		imshow(WINDOW_NAME, tempImage);
		if (waitKey(10) == 27) break;//按下ESC键，程序退出
	}
	// Release Nuitrack
	try
	{
		nuitrack::Nuitrack::release();
	}
	catch (const nuitrack::Exception& e)
	{
		std::cerr << "Nuitrack release failed (ExceptionType: " << e.type() << ")" << std::endl;
		errorCode = EXIT_FAILURE;
	}
	return 0;

}

//鼠标回调函数，根据不同的鼠标事件进行不同的操作
void on_MouseHandle(int event, int x, int y, int flags, void* param)
{

	Mat& image = *(cv::Mat*) param;
	switch (event)
	{
		case EVENT_MOUSEMOVE:
		{
			g_bDrawingCircle = true;
			//centerPoint = Point(x, y);
		}
		break;

		case EVENT_LBUTTONDOWN:
		{
			centerPoint = Point(x, y);

			g_bDrawingCircle = true;
		}
		break;

		case EVENT_LBUTTONUP:
		{
			g_bDrawingCircle = false;//置标识符为false
		}
		break;
	}
}
//矩形绘制函数
void DrawCircle(Mat& img, Point centerPoint, int radius)
{
	cv::circle(img, centerPoint, radius, cv::Scalar(g_rng.uniform(0, 255), g_rng.uniform(0, 255), g_rng.uniform(0, 255)), 10);//随机颜色
}