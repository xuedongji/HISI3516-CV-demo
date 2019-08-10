#include "findCircle.h"

#include <iostream>
#include <sstream>
#include <vector>
#include <time.h>
#include <string>
#include <time.h>
#include <iomanip>
#include <fstream>
#include <cstdlib>
#include <map>


#include "opencv2/core/core.hpp"
#include "opencv2/opencv.hpp"
#include "opencv2/imgproc/imgproc.hpp"

using namespace cv;
using namespace std;

static unsigned long frameID = 0;

bool findcircle(Mat& grayImg,float WH_ERR,float CR_ERR,float CH_ERR,int MY_BINARY_TH,int SCALE_TH)
{
	++frameID;
	bool isfind = false;

	//高斯滤波
	GaussianBlur(grayImg, grayImg, Size(5, 5), 2, 2);

	normalize(grayImg, grayImg, 0, 255, NORM_MINMAX, CV_8UC1);
	//固定阈值
	threshold(grayImg, grayImg, MY_BINARY_TH, 255, CV_THRESH_BINARY); //二值化


	//卷积核形状 卷积核太大膨胀时会连通osd光标与目标
	Mat element = getStructuringElement(MORPH_ELLIPSE, Size(7, 7), Point(-1, -1));
	dilate(grayImg, grayImg, element);
	erode(grayImg, grayImg, element);

	
	//寻找所有轮廓
	vector<vector<Point> > contours;
	//只提取最外层轮廓、轮廓表示方式为点集
	findContours(grayImg, contours, CV_RETR_EXTERNAL, CHAIN_APPROX_NONE);
	//设定最小面积
	double minArea = grayImg.rows * grayImg.cols / (SCALE_TH*SCALE_TH);
	//候选轮廓
	vector<vector<Point> > final_cont;
	//面积筛选
	for (int i = 0; i < (int)contours.size(); ++i)
	{
		double area = contourArea(contours[i]);
		if (area > minArea) {
			final_cont.push_back(contours[i]);
		}
	}
	//无面积达标的区域
	if (final_cont.size() == 0)
	{
		//LOG(enInfo) << frameID << "  No target find , circle too small!\n";
		return false;
	}
	//外接矩形
	vector<Rect> boundRect(final_cont.size());
	//凸包计算
	vector<Point> hullpoints;
	float aspectRatio = 0, conRecRatio = 0, conHullRatio = 0, dAspectRatio = 0, dConRecRatio = 0, dConHullRatio = 0;

	int target = 0;
	static unsigned int voteCount = 0;
	//对每个区域进行计算
	for (int i = 0; i < (int)final_cont.size(); ++i)
	{
		boundRect[i] = boundingRect(Mat(final_cont[i]));
		convexHull(Mat(final_cont[i]), hullpoints, true);
		//外接矩形长宽比  尽量接近1
		aspectRatio = (float)boundRect[i].width / (float)boundRect[i].height;
		//与外接矩形的面积比,判断填充度  尽量接近 PI/4 
		conRecRatio = (float)contourArea(final_cont[i]) / (float)boundRect[i].area();
		//与凸包的面积比，判断圆的残缺度 尽量接近1
		conHullRatio = (float)contourArea(final_cont[i]) / (float)contourArea(hullpoints);

		dAspectRatio = abs(1 - aspectRatio);
		dConRecRatio = abs(PI / 4 - conRecRatio) / (PI / 4);
		dConHullRatio = abs(1 - conHullRatio);

		if (dAspectRatio < WH_ERR && dConRecRatio < CR_ERR && dConHullRatio < CH_ERR)
		{
			//筛选最接近圆的区域
			//鍤找到目标
			isfind = true;
			target = i;
			if (voteCount >= 3)
			{
				//画图和标记中心点

				rectangle(grayImg,
					Point(boundRect[target].x, boundRect[target].y),
					Point(boundRect[target].x + boundRect[target].width, boundRect[target].y + boundRect[target].height),
					Scalar(255, 255, 255),2,8,0);

				/*
				LOG(enInfo) << "\n--------------- result ---------------" << "\n\n"	\
					    << "  dAspectRatio: " << dAspectRatio << "\n"	\
					    << "  dConRecRatio: " << dConRecRatio << "\n"	\
					    << "  dConHullRatio:" << dConHullRatio << "\n\n"	\
					    << "  x: " << boundRect[target].x << "   y: " << boundRect[target].y << "\n"	\
					    << "--------------------------------------" << "\n\n";
				*/
				printf("%ld  find target , count: %d\n",frameID,voteCount);
				printf("\n--------------- result ---------------\n");
				printf("  dAspectRatio: %lf\n",dAspectRatio);
				printf("  dConRecRatio: %lf\n",dConRecRatio);
				printf("  dConHullRatio: %lf\n\n",dConHullRatio);
				printf("  x: %d   y: %d\n",boundRect[target].x+int(boundRect[target].width/2),boundRect[target].y+ int(boundRect[target].height/2));
				printf("--------------------------------------\n\n");
			}
		}
	}
	if (isfind) ++voteCount;
	else voteCount = 0;

	return isfind;
}

void XDJ_FindCircleProc(unsigned char* data,int width,int height,float WH_ERR,float CR_ERR,float CH_ERR,int MY_BINARY_TH,int SCALE_TH)
{

	Mat frame(height,width,CV_8UC1,data);
	if(!findcircle(frame,WH_ERR,CR_ERR,CH_ERR,MY_BINARY_TH,SCALE_TH)) printf("%ld  no target\n",frameID);

	return;
}



