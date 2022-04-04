#pragma once
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <string>

using namespace cv;
using namespace std;

class LaneDetector {
private:
	Mat invertedPerspectiveMatrix;
	vector<Point2f> outPts;
	vector<Point> drawingPts;
	Mat out; // bird eye view drawing

public:
	void fillter_colors(Mat img, Mat &img_fill);
	void preprocess(Mat &img_fill, Mat &gray, Mat &binary);
	Mat Roi(Mat org, Mat binary);
	Mat birdseye(Mat org); // first
	vector<Point2f> SlidingWindows(Mat image, Rect window);
	Mat HalfDownsizing(Mat roi);
	vector<int> SumCol(Mat binary);
	int getLeftX_base(vector<int> sumArray);
	int getRightX_base(vector<int> sumArray);

	void LeftDrawingLine(vector<Point2f> pts, Mat &org, Mat processed);
	void RightDrawingLine(vector<Point2f> pts, Mat &org);
	Mat FinalDrawing(Mat &org);
};