#include "LaneDetector.h"
#include <algorithm>

void LaneDetector::fillter_colors(Mat image, Mat &img_filtered) {
	Mat img_bgr = image.clone();
	Mat img_hsv, img_combine;
	Mat white_mask, white_image;
	Mat yellow_mask, yellow_image;

	Scalar lower_white = Scalar(200, 200, 200);
	Scalar upper_white = Scalar(255, 255, 255);
	Scalar lower_yellow = Scalar(10, 100, 100);
	Scalar upper_yellow = Scalar(40, 255, 255);

	//Filter yellow pixels( Hue 30 )
	cvtColor(img_bgr, img_hsv, COLOR_BGR2HSV);

	//Filter white pixels
	inRange(img_bgr, lower_white, upper_white, white_mask);
	bitwise_and(img_bgr, img_bgr, white_image, white_mask);


	inRange(img_hsv, lower_yellow, upper_yellow, yellow_mask);
	bitwise_and(img_bgr, img_bgr, yellow_image, yellow_mask);

	//Combine the two above images
	addWeighted(white_image, 1.0, yellow_image, 1.0, 0.0, img_combine);

	img_filtered = img_combine.clone();
	//imshow("filtered image", img_filtered);
	//waitKey(0);
}
void LaneDetector::preprocess(Mat &img_fill, Mat &gray, Mat &binary) {
	cvtColor(img_fill, gray, COLOR_BGR2GRAY);
	threshold(gray, binary, 0, 255, THRESH_BINARY | THRESH_OTSU);
	//morphologyEx(binary, binary, MORPH_ERODE, Mat());	
	//imshow("gray", gray);
	//waitKey(0);
	//imshow("binary", binary);
	//waitKey(0);
}
Mat LaneDetector::Roi(Mat image, Mat binary) {
	// making zeros matrix for roi
	Mat mask = Mat::zeros(image.rows, image.cols, CV_8UC1); // CV_8UC3 to make it a 3 channel

	// trapezoid points of roi
	Point mask_points[1][4];

	mask_points[0][0] = Point(370, 0); // Bot left
	mask_points[0][1] = Point(image.size().width- 300, 0); // Bot right
	mask_points[0][2] = Point(image.size().width - 300, image.size().height); // top right
	mask_points[0][3] = Point(370, image.size().height); // Top left

	const Point* ppt[1] = { mask_points[0] };
	int npt[] = { 4 }; // points of trapezoid
	fillPoly(mask, ppt, npt, 1, Scalar(255, 255, 255), 8);

	//imshow("Mask", mask);
	//waitKey(0);

	Mat roi = binary.clone();
	bitwise_and(binary,mask, roi); // masking the original image with roi

	//imshow("ROI", roi);
	//waitKey(0);
	return roi;
}
Mat LaneDetector::birdseye(Mat org) {
	Mat Matx;
	Mat imgWarp;
	int width, height;
	vector<Point2f> srcRectCoord;
	vector<Point2f> dstRectCoord;

	int upX_diff = 100;
	int upY_diff = 260;
	int downX_diff = 230;
	int downY_diff = 80;

	int dstX = 350;

	width = org.cols;
	height = org.rows;
	// width : 1280, height : 720

	Point p1s = Point2f(width / 2 - upX_diff, height - upY_diff);
	Point p2s = Point2f(width / 2 + upX_diff, height - upY_diff);
	Point p3s = Point2f(downX_diff, height - downY_diff);
	Point p4s = Point2f(width - downX_diff, height - downY_diff);

	Point p1d = Point2f(dstX, 0);
	Point p2d = Point2f(width - dstX, 0);
	Point p3d = Point2f(dstX, height);
	Point p4d = Point2f(width - dstX, height);

	srcRectCoord = { p1s, p2s, p3s, p4s };
	dstRectCoord = { p1d, p2d, p3d, p4d };

	/* 1. calculating matrix between a img and bird view img. */
	Matx = getPerspectiveTransform(srcRectCoord, dstRectCoord);

	/* 2. calculating inverse matrix between a img and bird view img. */
	invertedPerspectiveMatrix = getPerspectiveTransform(dstRectCoord, srcRectCoord);

	/* 3. get perspective warping img */
	warpPerspective(org, imgWarp, Matx, Size(width, height), INTER_LINEAR);

	return imgWarp;
}

vector<Point2f> LaneDetector::SlidingWindows(Mat image, Rect window) {
	vector<Point2f> points;
	const Size imgSize = image.size(); //CV8UC1
	bool shouldBreak = false;
	Mat timg = Mat::zeros(image.size().height, image.size().width, CV_8UC1);
	Mat result = Mat::zeros(image.size().height, image.size().width, CV_8UC1);

	while (true)
	{
		float currentX = window.x + window.width * 0.5f;

		Mat roi = image(window); //Extract region of interest
		vector<Point2f> locations;

		findNonZero(roi, locations); //Get all non-black pixels. All are white in our case

		float avgX = 0.0;

		for (int i = 0; i < locations.size(); ++i) //Calculate average X position
		{
			float x = locations[i].x;
			avgX += window.x + x;
		}

		avgX = locations.empty() ? currentX : avgX / locations.size();

		Point point(avgX, window.y + window.height * 0.5f);
		points.push_back(point);

		// if pixel of loaction point is zero, delete that point
		if (locations.empty()) {
			points.pop_back();
		}
		
		// drawing windows
		/*
		Mat DrawResultHere = image.clone();
		Mat DrawResultGrid = image.clone();
		// Draw only rectangle
		rectangle(DrawResultHere, window, Scalar(255), 1, 8, 0);
		// Draw grid
		rectangle(DrawResultGrid, window, Scalar(255), 1, 8, 0);

		// Show  rectangle
		imshow("Step 2 draw Rectangle", DrawResultHere);
		result = timg + DrawResultHere;
		timg = result.clone();
		waitKey(100);

		// Select windows roi
		Mat Roi = image(window);
		//Show ROI
		imshow("Step 4 Draw selected Roi", Roi);
		waitKey(0);
		*/

		//Move the window up
		window.y -= window.height;

		//For the uppermost position
		if (window.y < 0)
		{
			window.y = 0;
			shouldBreak = true;
		}

		//Move x position
		window.x += (point.x - currentX);

		//Make sure the window doesn't overflow, we get an error if we try to get data outside the matrix
		if (window.x < 0) {
			window.x = 0;
		}
		if (window.x + window.width >= imgSize.width) {
			window.x = imgSize.width - window.width - 1;
		}

		if (shouldBreak) {
			//imshow("result", result);
			//waitKey(0);
			break;
		}
	}
	return points;
}

vector<int> LaneDetector::SumCol(Mat binary) {
	vector<int> Sumarr;
	for (int i = 0; i < binary.cols; i++) {
		Mat oneCol = binary.col(i);

		int sumResult = sum(oneCol)[0];
		Sumarr.push_back(sumResult);
	}
	return Sumarr;
}
Mat LaneDetector::HalfDownsizing(Mat roi) {
	int y1 = roi.size().height / 2;
	int y2 = roi.size().height;
	int x1 = 0;
	int x2 = roi.size().width;
	return roi(Range(y1, y2),Range(x1, x2));
}
int LaneDetector::getLeftX_base(vector<int> sumArray)
{
	int leftX_base;

	int midPoint = sumArray.size() / 2;
	int qtrPoint = midPoint / 2;

	// This is for get subset of vector range.
	vector<int>::const_iterator begin = sumArray.begin();
	vector<int>::const_iterator last = sumArray.begin() + sumArray.size();

	// full width : 1280px
	// Get pixel data from 1/4 width ~ 2/4 width(or 2/4 ~ 3/4 width for right side.) for sumArray matrix.
	// position should be referenced with 'begin' iterator.
	vector<int> left_qtr(begin+qtrPoint, begin + midPoint);

	
	// max index and value from a certain array.
	int leftMaxIndex = max_element(left_qtr.begin(), left_qtr.end()) - left_qtr.begin();
	// int leftMaxValue = *max_element(left_qtr.begin(), left_qtr.end());

	// adjust pixel index for global x width.
	leftX_base = leftMaxIndex + qtrPoint;
	
	return leftX_base;
}

int LaneDetector::getRightX_base(vector<int> sumArray)
{
	int rightX_base;

	int midPoint = sumArray.size() / 2;
	int qtrPoint = midPoint / 2;

	// This is for get subset of vector range.
	vector<int>::const_iterator begin = sumArray.begin();
	vector<int>::const_iterator last = sumArray.begin() + sumArray.size();

	vector<int> right_qtr(begin + midPoint, begin + midPoint + qtrPoint);

	// max index and value from a certain array.
	int rightMaxIndex = max_element(right_qtr.begin(), right_qtr.end()) - right_qtr.begin();
	// int rightMaxValue = *max_element(right_qtr.begin(), right_qtr.end());

	// adjust pixel index for global x width.
	rightX_base = rightMaxIndex + midPoint;

	return rightX_base;
}

void LaneDetector::LeftDrawingLine(vector<Point2f> pts, Mat &org, Mat processed) {
	perspectiveTransform(pts, outPts, invertedPerspectiveMatrix);
	// calclulate slope and equation for finding lane coordinate
	int osize = outPts.size();
	float x1 = outPts[0].x;
	float y1 = outPts[0].y;

	float x2 = outPts[osize - 1].x;
	float y2 = outPts[osize - 1].y;

	float slope = (y2 - y1) / (x2 - x1);

	int x = outPts[osize - 1].x;
	int y = outPts[osize - 1].y;

	float editx = (y - slope * x - org.size().height) / -slope;
	
	drawingPts.push_back(Point(editx, org.size().height)); // need to edit Left Bottom 
	drawingPts.push_back(Point(outPts[osize-1].x, outPts[osize - 1].y)); // Left Top
	/*
	//Draw the points onto the out image
	for (int i = 0; i < outPts.size() - 1; ++i)
	{
		//line(org, outPts[i], outPts[i + 1], Scalar(255, 100, 0), 3);
	}
	
	cvtColor(processed, out, COLOR_GRAY2BGR);
	
	//Draw a line on the processed image
	for (int i = 0; i < pts.size() - 1; ++i) {
		line(out, pts[i], pts[i + 1], Scalar(255, 100, 0));
	}
	*/
}

void LaneDetector::RightDrawingLine(vector<Point2f> pts, Mat &org) {
	perspectiveTransform(pts, outPts, invertedPerspectiveMatrix);

	std::reverse(outPts.begin(), outPts.end()); // clock wise for fillpoly
	// calclulate slope and equation for finding lane coordinate
	int osize = outPts.size();
	float x1 = outPts[0].x;
	float y1 = outPts[0].y;

	float x2 = outPts[osize - 1].x;
	float y2 = outPts[osize - 1].y;

	float slope = (y2 - y1) / (x2 - x1);

	int x = outPts[osize - 1].x;
	int y = outPts[osize - 1].y;

	float editx = (y - slope * x - org.size().height) / -slope;

	drawingPts.push_back(Point(outPts[0].x, outPts[0].y)); // Top Right
	drawingPts.push_back(Point(editx, org.size().height)); // Bottom Right need to edit.
	/*
	//Draw the points onto the out image
	for (int i = 0; i < outPts.size() - 1; ++i)
	{
		//line(org, outPts[i], outPts[i + 1], Scalar(255, 100, 0), 3);
	}
	
	//Draw a line on the processed image
	for (int i = 0; i < pts.size() - 1; ++i) {
		line(out, pts[i], pts[i + 1], Scalar(255, 100, 0));
	}
	*/
}

Mat LaneDetector::FinalDrawing(Mat &org){
	Mat overlay = Mat::zeros(org.size(), org.type());
	Point overlay_point[1][4];
	overlay_point[0][0] = Point(drawingPts[0].x, drawingPts[0].y);
	overlay_point[0][1] = Point(drawingPts[1].x, drawingPts[1].y);
	overlay_point[0][2] = Point(drawingPts[2].x, drawingPts[2].y);
	overlay_point[0][3] = Point(drawingPts[3].x, drawingPts[3].y);
	
	const Point* ppt[1] = { overlay_point[0] };
	int npt[] = { 4 };
	fillPoly(overlay, ppt, npt, 1, Scalar(255, 100, 0));
	drawingPts.clear();

	//Show results
	//imshow("Preprocess", out);
	//waitKey(0);
	//imshow("src", org);
	//waitKey(0);
	return overlay;
}
