#include <opencv2\highgui\highgui.hpp>

#include <iostream>

#include <nuitrack/Nuitrack.h>

#include <iomanip>
#include <iostream>


using namespace std;
using namespace cv;
using namespace tdv;

#define WINDOW_NAME "�����򴰿ڡ�"        //Ϊ���ڱ��ⶨ��ĺ� 


void on_MouseHandle(int event, int x, int y, int flags, void* param);
void DrawCircle(Mat& img, Point centerPoint, int radius);

Point centerPoint;
int radius;

Rect g_rectangle;
bool g_bDrawingCircle = false;//�Ƿ���л���
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
	//��1��׼������
	Mat srcImage(1000, 1500, CV_8UC3), tempImage;
	srcImage.copyTo(tempImage);
	srcImage = Scalar::all(0);

	

	//��2�������������ص�����
	namedWindow(WINDOW_NAME);
	setMouseCallback(WINDOW_NAME, on_MouseHandle, (void*)&srcImage);

	//��3��������ѭ���������л��Ƶı�ʶ��Ϊ��ʱ�����л���
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


		srcImage.copyTo(tempImage);//����Դͼ����ʱ����
		DrawCircle(tempImage, centerPoint, radius);//�����л��Ƶı�ʶ��Ϊ�棬����л���
		imshow(WINDOW_NAME, tempImage);
		if (waitKey(10) == 27) break;//����ESC���������˳�
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

//���ص����������ݲ�ͬ������¼����в�ͬ�Ĳ���
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
			g_bDrawingCircle = false;//�ñ�ʶ��Ϊfalse
		}
		break;
	}
}
//���λ��ƺ���
void DrawCircle(Mat& img, Point centerPoint, int radius)
{
	cv::circle(img, centerPoint, radius, cv::Scalar(g_rng.uniform(0, 255), g_rng.uniform(0, 255), g_rng.uniform(0, 255)), 10);//�����ɫ
}