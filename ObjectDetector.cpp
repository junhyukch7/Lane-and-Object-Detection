#include "ObjectDetector.h"


string CLASSES[] = { "background", "aeroplane", "bicycle", "bird", "boat",
	"bottle", "bus", "car", "cat", "chair", "cow", "diningtable",
	"dog", "horse", "motorbike", "person", "pottedplant", "sheep",
	"sofa", "train", "tvmonitor" };

int CHECKERBOARD[2]{ 6,9 };

Mat ObjectDetector::calib(Mat& org) {
	// 처음부터 끝까지 cailbration과정 수행
	// Creating vector to store vectors of 3D points for each checkerboard image
	std::vector<std::vector<cv::Point3f> > objpoints;

	// Creating vector to store vectors of 2D points for each checkerboard image
	std::vector<std::vector<cv::Point2f> > imgpoints;

	// Defining the world coordinates for 3D points
	std::vector<cv::Point3f> objp;
	for (int i{ 0 }; i < CHECKERBOARD[1]; i++)
	{
		for (int j{ 0 }; j < CHECKERBOARD[0]; j++)
			objp.push_back(cv::Point3f(j, i, 0));
	}


	// Extracting path of individual image stored in a given directory
	std::vector<cv::String> images;
	// Path of the folder containing checkerboard images
	std::string path = "camera_cal/calibration*.jpg";

	cv::glob(path, images);

	cv::Mat frame, gray;
	// vector to store the pixel coordinates of detected checker board corners 
	std::vector<cv::Point2f> corner_pts;
	bool success;

	// Looping over all the images in the directory
	for (int i{ 0 }; i < images.size(); i++)
	{
		frame = cv::imread(images[i]);
		cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);

		// Finding checker board corners
		// If desired number of corners are found in the image then success = true  
		success = cv::findChessboardCorners(gray, cv::Size(CHECKERBOARD[0], CHECKERBOARD[1]), corner_pts, CALIB_CB_ADAPTIVE_THRESH | CALIB_CB_FAST_CHECK | CALIB_CB_NORMALIZE_IMAGE);

		/*
		 * If desired number of corner are detected,
		 * we refine the pixel coordinates and display
		 * them on the images of checker board
		*/
		if (success)
		{
			cv::TermCriteria criteria(TermCriteria::EPS | TermCriteria::MAX_ITER, 30, 0.001);

			// refining pixel coordinates for given 2d points.
			cv::cornerSubPix(gray, corner_pts, cv::Size(11, 11), cv::Size(-1, -1), criteria);

			// Displaying the detected corner points on the checker board
			cv::drawChessboardCorners(frame, cv::Size(CHECKERBOARD[0], CHECKERBOARD[1]), corner_pts, success);

			objpoints.push_back(objp);
			imgpoints.push_back(corner_pts);
		}

		cv::imshow("Image", frame);
		cv::waitKey(0);
	}

	cv::destroyAllWindows();

	cv::Mat cameraMatrix, distCoeffs, R, T;

	/*
	 * Performing camera calibration by
	 * passing the value of known 3D points (objpoints)
	 * and corresponding pixel coordinates of the
	 * detected corners (imgpoints)
	*/
	cv::calibrateCamera(objpoints, imgpoints, cv::Size(gray.rows, gray.cols), cameraMatrix, distCoeffs, R, T);

	std::cout << "cameraMatrix : " << cameraMatrix << std::endl;
	std::cout << "distCoeffs : " << distCoeffs << std::endl;
	std::cout << "Rotation vector : " << R << std::endl;
	std::cout << "Translation vector : " << T << std::endl;

	Mat dst = Mat::zeros(org.size().height, org.size().width, CV_8UC3);

	undistort(org, dst, cameraMatrix, distCoeffs);

	return dst;

}

Mat ObjectDetector::simple_calib(Mat& org) {
	//calib()함수에서 얻은 데이터를 기반으로 간단하게 왜곡보정
	Mat cameraMatrix = (Mat1d(3, 3) << 1156.915062142131, 0, 665.9387119551399, 0, 1152.114644805994, 388.7955240062732, 0, 0, 1);
	Mat distCoeffs = (Mat1d(1, 5) << -0.2371543906423672, -0.08871932875028064, -0.0007898871023714299, -0.0001171743090811421, 0.1119047389586262);

	Mat dst = Mat::zeros(org.size().height, org.size().width, CV_8UC3);

	undistort(org, dst, cameraMatrix, distCoeffs);

	return dst;
}

Net ObjectDetector::Net_setup() {
	
	Net net = readNet(model, config);

	if (net.empty()) {
		cerr << "can't load network by using following files: " << endl;
		cerr << "prototxt: " << config << endl;
		cerr << "caffemodel: " << model << endl;
	}

	return net;
}

Mat ObjectDetector::Detecting(Mat img, Net net) {
	Mat blob = blobFromImage(img, 0.0078, Size(300, 300), Scalar(127.5, 127.5, 127.5));
	net.setInput(blob);
	Mat res = net.forward();

	Mat detect(res.size[2], res.size[3], CV_32FC1, res.ptr<float>());

	ostringstream ss;

	for (int i = 0; i < detect.rows; i++) {
		float confidence = detect.at<float>(i, 2);
		if (confidence < 0.2)
			break;

		int idx = cvRound(detect.at<float>(i, 1));
		int x1 = cvRound(detect.at<float>(i, 3)* img.cols);
		int y1 = cvRound(detect.at<float>(i, 4)* img.rows);
		int x2 = cvRound(detect.at<float>(i, 5)* img.cols);
		int y2 = cvRound(detect.at<float>(i, 6)* img.rows);

		rectangle(img, Rect(Point(x1, y1), Point(x2, y2)),
			Scalar(0, 255, 0),2);

		ss.str("");
		String conf(ss.str());
		String label = CLASSES[idx];
		int baseLine = 0;
		Size labelSize = getTextSize(label, FONT_HERSHEY_SIMPLEX,
			0.5, 1, &baseLine);
		putText(img, label, Point(x1, y1 - 1), FONT_HERSHEY_SIMPLEX,
			0.8, Scalar(0, 255, 0),2);
	}

	return img;
}