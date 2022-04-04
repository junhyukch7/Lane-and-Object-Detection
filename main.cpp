#include "opencv2/opencv.hpp"
#include "ObjectDetector.h"
#include "LaneDetector.h"
#include <cmath>

int main(void) {
	// Object Detection.
	ObjectDetector obj;
	// Lane Detection.
	LaneDetector lane;

	Mat img = imread("test6.jpg");
	img = obj.simple_calib(img); // calibration
	Mat org = img.clone();

	if (img.empty()) {
		cerr << "image not load" << endl;
		return -1;
	}

	// preprocessing 
	Mat img_filled, gray, binary, roi_img, imgWarp;
	imgWarp = lane.birdseye(org);
	lane.fillter_colors(imgWarp, img_filled);
	lane.preprocess(img_filled, gray, binary);
	Mat roi = lane.Roi(imgWarp, binary);

	//Sliding windows & drawing
	Mat halfimage = lane.HalfDownsizing(roi);
	vector<int> sumarr = lane.SumCol(halfimage);
	int leftxbase = lane.getLeftX_base(sumarr);
	int rightxbase = lane.getRightX_base(sumarr);
	vector<Point2f> pts = lane.SlidingWindows(roi, Rect(leftxbase, 660, 120, 60));
	lane.LeftDrawingLine(pts, org, roi);
	pts = lane.SlidingWindows(roi, Rect(rightxbase, 660, 120, 60));
	lane.RightDrawingLine(pts, org);
	Mat overlay = lane.FinalDrawing(org);

	//Object Detector start
	Net net = obj.Net_setup();
	Mat result = obj.Detecting(img, net);
	img = img + overlay;
	imshow("result", img);
	waitKey(0);
	return 0;

}

