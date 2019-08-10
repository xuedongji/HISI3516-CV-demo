
#include "findRect.h"



/**
两直线的交点

@param a 线段1
@param b 线段2
@return 交点
*/
cv::Point2f computeIntersect(cv::Vec4i a, cv::Vec4i b)
{
	int x1 = a[0], y1 = a[1], x2 = a[2], y2 = a[3], x3 = b[0], y3 = b[1], x4 = b[2], y4 = b[3];

	if (float d = ((float)(x1 - x2) * (y3 - y4)) - ((y1 - y2) * (x3 - x4)))
	{
		cv::Point2f pt;
		pt.x = ((x1 * y2 - y1 * x2) * (x3 - x4) - (x1 - x2) * (x3 * y4 - y3 * x4)) / d;
		pt.y = ((x1 * y2 - y1 * x2) * (y3 - y4) - (y1 - y2) * (x3 * y4 - y3 * x4)) / d;
		return pt;
	}
	else
		return cv::Point2f(-1, -1);
}

/**
对多个点按顺时针排序
@param corners 点的集合
*/
void sortCorners(std::vector<cv::Point2f>& corners)
{
	if (corners.size() == 0) return;
	//先延 X轴排列
	cv::Point pl = corners[0];
	int index = 0;
	for (int i = 1; i < corners.size(); i++)
	{
		cv::Point point = corners[i];
		if (pl.x > point.x)
		{
			pl = point;
			index = i;
		}
	}
	corners[index] = corners[0];
	corners[0] = pl;

	cv::Point lp = corners[0];
	for (int i = 1; i < corners.size(); i++)
	{
		for (int j = i + 1; j<corners.size(); j++)
		{
			cv::Point point1 = corners[i];
			cv::Point point2 = corners[j];
			if ((point1.y - lp.y*1.0) / (point1.x - lp.x)>(point2.y - lp.y*1.0) / (point2.x - lp.x))
			{
				cv::Point temp = point1;
				corners[i] = corners[j];
				corners[j] = temp;
			}
		}
	}
}
/**
点到点的距离
@param p1 点1
@param p2 点2
@return 距离
*/
double getSpacePointToPoint(cv::Point p1, cv::Point p2)
{
	int a = p1.x - p2.x;
	int b = p1.y - p2.y;
	return sqrt(a * a + b * b);
}
int findLargestSquare(const vector<vector<cv::Point> >& squares, vector<cv::Point>& biggest_square)
{
	if (!squares.size()) return -1;

	int max_width = 0;
	int max_height = 0;
	int max_square_idx = 0;
	for (int i = 0; i < squares.size(); i++)
	{
		cv::Rect rectangle = boundingRect(Mat(squares[i]));
		if ((rectangle.width >= max_width) && (rectangle.height >= max_height))
		{
			max_width = rectangle.width;
			max_height = rectangle.height;
			max_square_idx = i;
		}
	}
	biggest_square = squares[max_square_idx];
	return max_square_idx;
}
/**
根据三个点计算中间那个点的夹角   pt1 pt0 pt2
*/
double getAngle(cv::Point pt1, cv::Point pt2, cv::Point pt0)
{
	double dx1 = pt1.x - pt0.x;
	double dy1 = pt1.y - pt0.y;
	double dx2 = pt2.x - pt0.x;
	double dy2 = pt2.y - pt0.y;
	return (dx1*dx2 + dy1*dy2) / sqrt((dx1*dx1 + dy1*dy1)*(dx2*dx2 + dy2*dy2) + 1e-10);
}
double getDistance(cv::Point pt1, cv::Point pt2)
{
	double dx = pt1.x - pt2.x;
	double dy = pt1.y - pt2.y;
	return sqrt(dx*dx + dy*dy);
}



bool findRect(Mat& mat)
{

	static unsigned long long frameID = 0;
	++frameID;
	bool isfind = false;
	static unsigned int voteCount = 0;

	Mat grayImg;
	//获取灰度图像
	cvtColor(mat, grayImg, CV_BGR2GRAY);

	GaussianBlur(grayImg, grayImg, Size(3, 3), 2, 2);

	normalize(grayImg, grayImg, 0, 255, NORM_MINMAX, CV_8UC1);

	threshold(grayImg, grayImg, 200, 255, CV_THRESH_BINARY); //二值化
	//卷积核形状 卷积核太大膨胀时会连通osd光标与目标
	Mat element = getStructuringElement(MORPH_ELLIPSE, Size(9, 9), Point(-1, -1));
	//腐蚀除杂
	dilate(grayImg, grayImg, element);
	erode(grayImg, grayImg, element);

	int thresh = 50;
	//边缘检测  像素点梯度大于第一个阈值的必然是边缘，梯度值在第一个阈值与第二个阈值之间则为疑似边缘点，再根据联通性判断
	//Canny(grayImg, grayImg, thresh, thresh * 3, 3);

	imshow("闭运算", grayImg);

	vector<vector<cv::Point> > contours;
	//寻找边框
	findContours(grayImg, contours, RETR_LIST, CHAIN_APPROX_SIMPLE);

	//cout << contours.size() << endl;

	vector<cv::Point> hull, tmpPolygon, polygon;

	//设定最小面积
	double minArea = grayImg.rows * grayImg.cols / 2500;
	//cout << grayImg.rows << endl<< grayImg.cols << endl << minArea << endl << endl;

	//候选轮廓
	vector<vector<Point>> final_cont;
	//面积筛选
	for (int i = 0; i < contours.size(); ++i)
	{
		double area = contourArea(contours[i]);
		//cout << area << endl;
		if (area > minArea) {
			final_cont.push_back(contours[i]);
			Scalar color = Scalar(0,0,255);
			drawContours(mat, contours, i, color,1, 8);
		}
	}

	//无面积达标的区域
	if (final_cont.size() == 0)
	{
		cout << "  No target found !" << endl;
		return isfind;
	}

	int idx = 0;
	for (idx = 0; idx < final_cont.size(); idx++)
	{
		//边框的凸包
		convexHull(final_cont[idx], hull);
		//多边形拟合凸包边框(此时的拟合的精度较低)
		//主要功能是把一个连续光滑曲线折线化，对图像轮廓点进行多边形拟合
		approxPolyDP(final_cont[idx], polygon, 2, true);


		for (int i = 0; i < polygon.size(); ++i)
		{
			circle(mat, polygon[i], 4, Scalar(0, 255, 0));
		}
		//筛选出各个角度都接近直角的凸四边形
		bool isConvex = isContourConvex(Mat(polygon));
		if (polygon.size() == 4 && isConvex)
		{
			cout << frameID << "  find candidate   " << endl;
			//边长判断
			double maxL = arcLength(polygon, true)*0.05;//该函数计算曲线长度或闭合轮廓周长

														//找到剩余顶点连线中，边长大于 2 * maxL的四条边作为四边形物体的四条边
			vector<Vec4i> lines;
			for (int i = 0; i < polygon.size(); i++)
			{
				cv::Point p1 = polygon[i];
				cv::Point p2 = polygon[(i + 1) % polygon.size()];
				if (getSpacePointToPoint(p1, p2) > maxL)
				{
					lines.push_back(Vec4i(p1.x, p1.y, p2.x, p2.y));
				}
			}

			//角度判断
			double maxCosine = 0;

			for (int j = 2; j < 5; j++)
			{
				double cosine = fabs(getAngle(polygon[j % 4], polygon[j - 2], polygon[j - 1]));
				maxCosine = MAX(maxCosine, cosine);
			}

			cout << "vote:  " << voteCount << endl;
			//要求最小角大于45度,用于检测正俯视
			if (maxCosine < 0.7 && lines.size() == 4)
			{
				cout << frameID << "  Find target   " << endl;
				//筛选最接近圆的区域
				//找到目标
				isfind = true;

				if (voteCount > 2)
				{
					//计算出这四条边中 相邻两条边的交点，即物体的四个顶点
					vector<cv::Point> vCorners;
					for (int i = 0; i < lines.size(); i++)
					{
						cv::Point cornor = computeIntersect(lines[i], lines[(i + 1) % lines.size()]);
						vCorners.push_back(cornor);
					}
					int targetX = (vCorners[0].x + vCorners[2].x) / 2;
					int targetY = (vCorners[0].y + vCorners[2].y) / 2;

					cout << idx << "   x = " << targetX << "  y = " << targetY << endl;

					char str[64] = { 0 };
					sprintf(str, "%d x:%d y:%d", idx, targetX, targetY);
					cv::Point mark = cv::Point(targetX+20, targetY);
					putText(mat, str, mark, cv::FONT_HERSHEY_DUPLEX, 0.5, cv::Scalar(0, 0, 255), 1);
					
					//绘制出四条边
					for (int i = 0; i < vCorners.size(); i++)
					{
						line(mat, vCorners[i], vCorners[(i + 1) % vCorners.size()], Scalar(0, 0, 255), 2);
					}
				}
			}
		}
		else
		{
			cout << frameID << "  No target   " << endl;
		}
	}

	if (isfind) ++voteCount;
	else voteCount = 0;

	/*
	//找到这个最大的四边形对应的凸边框，再次进行多边形拟合，此次精度较高，拟合的结果可能是大于4条边的多边形
	//接下来的操作，主要是为了解决圆角顶点的连线会有切边的问题
	hull = hulls[idex];
	approxPolyDP(hull, polygon, 3, true);
	vector<cv::Point> newApprox;



	//找到高精度拟合时得到的顶点中 距离小于 低精度拟合得到的四个顶点 maxL的顶点，排除部分顶点的干扰
	//c++11新标准
	for (cv::Point p : polygon)
	{
	if (!(getSpacePointToPoint(p, largest_square[0]) > maxL &&
	getSpacePointToPoint(p, largest_square[1]) > maxL &&
	getSpacePointToPoint(p, largest_square[2]) > maxL &&
	getSpacePointToPoint(p, largest_square[3]) > maxL))
	{
	newApprox.push_back(p);
	}
	}

	*/


	return isfind;
}
