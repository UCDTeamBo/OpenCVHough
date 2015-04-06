#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <stdio.h>
#include <iostream>
using namespace cv;




/*int main()
{

	Mat image_input,grey_image,color_image;
	 

	image_input = imread("C:/Users/Lindsey/Documents/Visual Studio 2013/Projects/OpenCVHoughCode_Final/download.jpg", CV_LOAD_IMAGE_COLOR);

	cvtColor(image_input, grey_image, CV_BGR2GRAY);
	Canny(grey_image, grey_image, 90, 90);
	cvtColor(grey_image, color_image, CV_GRAY2BGR);


	vector<Vec2f> Hough_Output;

	
	namedWindow("HELLO", WINDOW_AUTOSIZE);
	imshow("HELLO", grey_image);

	waitKey(0);

	HoughLines(grey_image, Hough_Output, 1, CV_PI / 180, 150, 0, 0);

	std::cout << Hough_Output.size() << '\n';

	//int i = 1;

	for (size_t i = 0; i < Hough_Output.size(); i++)
	{
		//std::cout << "hi";
		float rho = Hough_Output[i][0];
		float theta = Hough_Output[i][1];

		Point pt1, pt2;
		double a = cos(theta);
		double b = sin(theta);

		double x0 = a*rho;
		double y0 = b*rho;

		pt1.x = cvRound(x0 + 1000 * (-b));
		pt1.y = cvRound(y0 + 1000 * (a));
		pt2.x = cvRound(x0 - 1000 * (-b));
		pt2.y = cvRound(y0 - 1000 * (a));
		line(color_image, pt1, pt2, Scalar(0, 0, 255), 3, CV_AA);
	}

	namedWindow("HELLO", WINDOW_AUTOSIZE);
	imshow("HELLO", color_image);

	waitKey(0);

	return(0);
}*/


static double angle(cv::Point pt1, cv::Point pt2, cv::Point pt0)
{
	double dx1 = pt1.x - pt0.x;
	double dy1 = pt1.y - pt0.y;
	double dx2 = pt2.x - pt0.x;
	double dy2 = pt2.y - pt0.y;
	return (dx1*dx2 + dy1*dy2) / sqrt((dx1*dx1 + dy1*dy1)*(dx2*dx2 + dy2*dy2) + 1e-10);
}


void setLabel(cv::Mat& im, const std::string label, std::vector<cv::Point>& contour)
{
	int fontface = cv::FONT_HERSHEY_SIMPLEX;
	double scale = 0.4;
	int thickness = 1;
	int baseline = 0;

	cv::Size text = cv::getTextSize(label, fontface, scale, thickness, &baseline);
	cv::Rect r = cv::boundingRect(contour);

	cv::Point pt(r.x + ((r.width - text.width) / 2), r.y + ((r.height + text.height) / 2));
	cv::rectangle(im, pt + cv::Point(0, baseline), pt + cv::Point(text.width, -text.height), CV_RGB(255, 255, 255), CV_FILLED);
	cv::putText(im, label, pt, fontface, scale, CV_RGB(0, 0, 0), thickness, 8);
}

int main(){
	using namespace cv;

	//DO I NEED an int detect_plus() { and a } at the end?

	//Load source image
	Mat src = imread("C:/Users/Lindsey/Documents/Visual Studio 2013/Projects/OpenCVHoughCode_Final/Blue_Cross_New.png");
	if (src.empty())
		return -1;

	//next part in original code finds contours by converting to grayscale and using Canny
	// Convert to grayscale
	Mat gray;
	cvtColor(src, gray, CV_BGR2GRAY);

	/*namedWindow("HELLO", WINDOW_AUTOSIZE);
	imshow("HELLO", gray);

	waitKey(0);*/

	//Convert to binary image using Canny
	Mat bw;
	Canny(gray, bw, 90, 90);

	namedWindow("HELLO", WINDOW_AUTOSIZE);
	imshow("HELLO", bw);

	waitKey(0);

	//"The array bw is now a binary image with edges from the shapes. Obtain the contours,"
	// Find contours
	std::vector<std::vector<Point> > contours;
	findContours(bw.clone(), contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);

	//I don't completely understand this next section, but I think it sorts out actual contours from noise
	// The array for storing the approximation curve
	std::vector<Point> approx;

	// We'll put the labels in this destination image
	Mat dst = src.clone();

	std::cout << contours.size() << " SIZE OF CONTOURS";
	for (int i = 0; i < contours.size(); i++)
	{
		// Approximate contour with accuracy proportional
		// to the contour perimeter
		approxPolyDP(
			Mat(contours[i]),
			approx,
			arcLength(Mat(contours[i]), true) * 0.02,
			true
			);

		
		// Skip small or non-convex objects 
		/*if (std::fabs(contourArea(contours[i])) < 100 || !isContourConvex(approx))
			continue;*/


		//"The vector approx now contains the vertices of the polygonal approximation for the contour.
		//We will use its size to determine" the shape of the contour.

		// num vertices of the current contour
		int vtc = approx.size();

		// Get the degree (in cosines) of all vertices
		//MAY BE A PROBLEM IF THE INNER VERTICES READ AS 270 DEG AND NOT 90 DEG
		std::vector<double> cos;
		for (int j = 2; j < vtc + 1; j++)
			cos.push_back(angle(approx[j%vtc], approx[j - 2], approx[j - 1]));

		// Sort ascending the corner degree values
		std::sort(cos.begin(), cos.end());

		// Get the lowest and the highest degree
		double mincos = cos.front();
		double maxcos = cos.back();

		//detect a plus and labels it
		//NOT SURE IF THIS vtc 8 OR 12, depends on if inner vertices count
		// -0.1 is just below cos(90), 0.3 is a ways above cos(90), I think can be lowered to 0.1 if inaccurate
		//setLabel(dst, "X", contours[i]);
		//std::cout << "HI";
		if (vtc >= 8 /*&& mincos >= -0.1 && maxcos <= 0.3*/)
		{
			setLabel(dst, "X", contours[i]);
		}

		else
		{
			setLabel(dst, "?", contours[i]);
		}
	}

	//Display both src and dst images
	imshow("src", src);
	imshow("dst", dst);
	waitKey(0);
	return 0;

}
