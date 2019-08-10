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

	//��˹�˲�
	GaussianBlur(grayImg, grayImg, Size(5, 5), 2, 2);

	normalize(grayImg, grayImg, 0, 255, NORM_MINMAX, CV_8UC1);
	//�̶���ֵ
	threshold(grayImg, grayImg, MY_BINARY_TH, 255, CV_THRESH_BINARY); //��ֵ��

																	  //�������״ �����̫������ʱ����ͨosd�����Ŀ��
	Mat element = getStructuringElement(MORPH_ELLIPSE, Size(7, 7), Point(-1, -1));
	dilate(grayImg, grayImg, element);
	erode(grayImg, grayImg, element);

	//imshow("�������", grayImg);

	//Ѱ����������
	vector<vector<Point> > contours;
	//ֻ��ȡ�����������������ʾ��ʽΪ�㼯
	findContours(grayImg, contours, CV_RETR_EXTERNAL, CHAIN_APPROX_NONE);
	//�趨��С���
	double minArea = grayImg.rows * grayImg.cols / (SCALE_TH*SCALE_TH);
	//��ѡ����
	vector<vector<Point> > final_cont;
	//���ɸѡ
	for (int i = 0; i < (int)contours.size(); ++i)
	{
		double area = contourArea(contours[i]);
		if (area > minArea) {
			final_cont.push_back(contours[i]);
		}
	}
	//�������������
	if (final_cont.size() == 0)
	{
		//LOG(enInfo) << frameID << "  No target find , circle too small!\n";
		return false;
	}
	//��Ӿ���
	vector<Rect> boundRect(final_cont.size());
	//͹������
	vector<Point> hullpoints;
	float aspectRatio = 0, conRecRatio = 0, conHullRatio = 0, dAspectRatio = 0, dConRecRatio = 0, dConHullRatio = 0;

	int target = 0;
	static unsigned int voteCount = 0;
	//��ÿ��������м���
	for (int i = 0; i < (int)final_cont.size(); ++i)
	{
		boundRect[i] = boundingRect(Mat(final_cont[i]));
		convexHull(Mat(final_cont[i]), hullpoints, true);
		//��Ӿ��γ����  �����ӽ�1
		aspectRatio = (float)boundRect[i].width / (float)boundRect[i].height;
		//����Ӿ��ε������,�ж�����  �����ӽ� PI/4 
		conRecRatio = (float)contourArea(final_cont[i]) / (float)boundRect[i].area();
		//��͹��������ȣ��ж�Բ�Ĳ�ȱ�� �����ӽ�1
		conHullRatio = (float)contourArea(final_cont[i]) / (float)contourArea(hullpoints);

		dAspectRatio = abs(1 - aspectRatio);
		dConRecRatio = abs(PI / 4 - conRecRatio) / (PI / 4);
		dConHullRatio = abs(1 - conHullRatio);

		if (dAspectRatio < WH_ERR && dConRecRatio < CR_ERR && dConHullRatio < CH_ERR)
		{
			//ɸѡ��ӽ�Բ������
			//�ҵ�Ŀ��
			isfind = true;
			target = i;
			if (voteCount >= 3)
			{
				//��ͼ�ͱ�����ĵ�

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



