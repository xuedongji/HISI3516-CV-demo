#pragma once

#include <iostream> 
#include <sstream>
#include <vector>
#include <time.h>
#include <string>
#include <time.h>
#include <windows.h>
#include <iomanip>
#include <fstream>
#include <cstdlib>
#include <map>
#include <opencv2/core/core.hpp>  
#include <opencv2/highgui/highgui.hpp> 
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/imgproc.hpp>  

#define WIDTH 1280
#define HEIGHT 720
#define PI 3.1415926

using namespace cv;
using namespace std;

bool findRect(Mat& mat);