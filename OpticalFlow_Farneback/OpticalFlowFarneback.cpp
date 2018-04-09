#include <cstdio>
#include <cmath>
#include <vector>
#include <fstream>
#include <string>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;



///////////////////////////////////////////////////////////////////////////////

Mat start  (2, 1, CV_32F, Scalar (0));
Mat finish (2, 1, CV_32F, Scalar (0));
bool isStartSet = 0;

//static void onMouse (int event, int x, int y, int, void* ptr)
//{
//	if (event != CV_EVENT_LBUTTONDOWN)
//		return;
//
//	start.at <float> (0, 0) = (float) y;
//	start.at <float> (1, 0) = (float) x;
//
//	isStartSet = 1;
//	printf ("START\n");
//}

Rect box;//矩形对象
bool drawing_box;//记录是否在画矩形对象

void onMouse(int event, int x, int y, int flag, void *img)//鼠标事件回调函数，鼠标点击后执行的内容应在此
{
	switch (event)
	{
	case CV_EVENT_LBUTTONDOWN://鼠标左键按下事件
		drawing_box = true;//标志在画框
		box = Rect(x, y, 0, 0);//记录矩形的开始的点
		break;
	case CV_EVENT_MOUSEMOVE://鼠标移动事件
		if (drawing_box){//如果左键一直按着，则表明在画矩形
			box.width = x - box.x;
			box.height = y - box.y;//更新长宽
		}
		break;
	case CV_EVENT_LBUTTONUP://鼠标左键松开事件
		drawing_box = false;//不在画矩形
		//这里好像没作用
		if (box.width<0){//排除宽为负的情况，在这里判断是为了优化计算，不用再移动时每次更新都要计算长宽的绝对值
			box.x = box.x + box.width;//更新原点位置，使之始终符合左上角为原点
			box.width = -1 * box.width;//宽度取正
		}
		if (box.height<0){//同上
			box.y = box.y + box.height;
			box.height = -1 * box.width;
		} 
		isStartSet = 1;
		printf ("START\n");
		break;
	default:
		break;
	}
}

int float2int(float value)
{
	if (value > 0)
		return(ceil(value));
	else
		return(floor(value));
}
 
int main (int argc, char **argv)
{
	bool stop (false);
	Mat frame, gray, grayPrev, flow, prevFrame;
	std::ifstream fin("E:\\DataSet\\trafficLight-yuanboyuan\\picture\\3\\pics.txt");  
	std::string s; 
	fin>>s;
	prevFrame = imread(s, -1);
	if (prevFrame.empty())
	{
		std::cout<<"No picture"<<std::endl;
		return 0;
	}

	namedWindow( "Extracted Frame" , CV_WINDOW_NORMAL);

	setMouseCallback ("Extracted Frame", onMouse);

	cvtColor (prevFrame, gray, CV_RGB2GRAY);
	while (!isStartSet)
	{
		imshow ("Extracted Frame", prevFrame);
		waitKey (30);
	}

	while ( !stop )
	{
		fin>>s;
		frame = imread(s, -1);
		if (frame.empty())
		{
			std::cout<<"No picture"<<std::endl;
			break;
		}
		cvtColor (frame, gray, CV_RGB2GRAY);
		if (grayPrev.data)
		{
			calcOpticalFlowFarneback(grayPrev, gray, flow, 0.5, 3, 15, 3, 5, 1.2, 0);
			Point2f MaxPointValue = (0.0f, 0.0f);
			float diff_XY=.0f;
			Point2f OpticalPoint=(0.0f,0.0f);
			// 用来计算平均 flow 速度
			int numOfSelectedPoint = 0 ;
			float sumX = .0f ;
			float sumY = .0f ;
			float vx = .0f;
			float vy = .0f;
			for (int r=box.y; r < (box.height + box.y); r+=5)
			{
				for (int c=box.x; c < (box.width + box.x); c+=5)
				{
					Point2f fxy = flow.at<Point2f>(r, c);
					if (fabs(fxy.x) > MaxPointValue.x)
					{
						MaxPointValue.x = fabs(fxy.x);
						OpticalPoint.x = fxy.x;
					}
					if (fabs(fxy.y) > MaxPointValue.y)
					{
						MaxPointValue.y = fabs(fxy.y);
						OpticalPoint.y = fxy.y;
					}
					
					//if (diff_XY >1)
					//{

					//	numOfSelectedPoint++ ;
					//	sumX += fxy.x  ;
					//	sumY += fxy.y  ;
					//}

					////line(frame, Point(c, r), Point(cvRound(c+fxy.x), cvRound(r+fxy.y)), CV_RGB(0,255,0));

				}

			}
			//vx = sumX / numOfSelectedPoint;
			//vy = sumY / numOfSelectedPoint;

			//cout<<"sumX:"<<sumX<<"   sumY:"<<sumY<<endl;
			//cout<<"VX:"<<vx<<"   VY:"<<vy<<endl;
			//rectangle(frame, Point2f(box.x + vx, box.y + vy), Point2f( box.x + vx + box.width, box.y + vy + box.height ),Scalar(0,255,0), 2, 8, 0);
			
			box.x += int(OpticalPoint.x+0.5);
			box.y += int(OpticalPoint.y+0.5);
			cout<<"VX:"<<OpticalPoint.x<<"   VY:"<<OpticalPoint.y<<endl;
			rectangle(frame,box,Scalar(0,255,0), 1, 8, 0);

			imshow ("Extracted Frame", frame);
			if (waitKey(3) >=0 )
				stop = true;
		}
		gray.copyTo (grayPrev);
				
	}

	waitKey (1);
	return 0;
}
