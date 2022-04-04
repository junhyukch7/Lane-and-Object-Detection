#pragma once
#include <opencv2/opencv.hpp>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cmath>

using namespace std;
using namespace cv;
using namespace cv::dnn;

class ObjectDetector {
private:
	const String model = "MobileNetSSD_deploy.caffemodel";
	const String config = "MobileNetSSD_deploy.prototxt";

public:
	Mat calib(Mat& org);
	Mat simple_calib(Mat& org);
	Net Net_setup(void);
	Mat Detecting(Mat img, Net net);
};