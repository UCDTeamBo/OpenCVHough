#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <opencv2/opencv.hpp>
#include <stdio.h>
#include <iostream>
using namespace cv;


//Initiating portions of the code: Make sure Camera_Capture is commented in order to have it run of webcam
#define Camera_Capture
#define Display_Results
//#define Template_Process;


//Definitions
vector<vector<Point> > contours;
vector<Vec4i> hierarchy;
vector<Vec3f> vecCircles;
vector<Vec3f>::iterator itrCircles;
vector<vector<Point> > contours_2;
vector<Vec4i> hierarchy_2;
vector<Vec3f> vecCircles_2;
vector<Vec3f>::iterator itrCircles_2;

Mat drawingFinal;
Mat temp;
int top_x, top_y, bottom_x, bottom_y;
Mat drawing;
Scalar color;
Mat Copy;
int count;
int top_x_2, top_y_2, bottom_x_2, bottom_y_2, middle_x, middle_y;
int num_templates=10;


Mat MatchingMethod(Mat Source)
{
	/// Source image to display
	Mat img_display,result;
	Source.copyTo(img_display);
	int i;
	/// Create the result matrix
	char buffer[400];
	double minVal_Final = 0; double maxVal_Final = 0; Point minLoc_Final; Point maxLoc_Final; Point matchLoc; int rows; int columns;
	cvtColor(Source, Source, CV_BGR2GRAY);
	for (i = 1; i < num_templates + 1; i++){
		Mat Template;
		sprintf(buffer, "C:/Users/Lindsey/Documents/Visual Studio 2013/Projects/OpenCVHoughCode_Final/Templates/t%d.png", i);
		Template = imread(buffer);
		cvtColor(Template, Template, CV_BGR2GRAY);
		int result_cols = Source.cols - Template.cols + 1;
		int result_rows = Source.rows - Template.rows + 1;

		result.create(result_rows, result_cols, CV_32FC1);

		/// Do the Matching and Normalize
		matchTemplate(Source, Template, result, CV_TM_CCOEFF);
		normalize(result, result, 0, 1, NORM_MINMAX, -1, Mat());

		//namedWindow("Template", WINDOW_AUTOSIZE);
		/// Localizing the best match with minMaxLoc
		double minVal; double maxVal; Point minLoc; Point maxLoc;
		Point matchLoc;
		minMaxLoc(result, &minVal, &maxVal, &minLoc, &maxLoc, Mat());
		//imshow("Template", result);
		//waitKey(0);
		if (maxVal > maxVal_Final)
		{
			maxVal_Final = maxVal;
			minVal_Final = minVal;
			minLoc_Final = minLoc;
			maxLoc_Final = maxLoc;
			rows = Template.rows;
			columns = Template.cols;

		}

	}

	int match_method = 4;
		/// For SQDIFF and SQDIFF_NORMED, the best matches are lower values. For all the other methods, the higher the better
	if (match_method == CV_TM_SQDIFF || match_method == CV_TM_SQDIFF_NORMED)
	{
		matchLoc = minLoc_Final;
	}
	else
	{
		matchLoc = maxLoc_Final;
	}
		
	rectangle(img_display, matchLoc, Point(matchLoc.x + columns, matchLoc.y + rows), Scalar::all(0), 2, 8, 0);
	rectangle(result, matchLoc, Point(matchLoc.x + columns, matchLoc.y + rows), Scalar::all(0), 2, 8, 0);
	
	/// Show me what you got
	
	
	return result;
}

int main()
{

	Mat image_input,grey_image,color_image, hsv_image, color_threshed;
	bool Success;

#ifdef Camera_Capture 
	VideoCapture cap(0);
#else
	VideoCapture cap("C:/Users/Lindsey/Documents/Visual Studio 2013/Projects/OpenCVHoughCode_Final/IMG_0450.mov");
#endif

#ifdef Display_Results
	namedWindow("Transformed", WINDOW_AUTOSIZE);
	namedWindow("Original", WINDOW_AUTOSIZE);
#else 
#endif

	//Infinite loop video/camera input
	for (;;){

		//Error Check Successfull camera READ 
		Success = cap.read(image_input);
		
		if (!Success){
			std::cout << "Cannot read a frame from video stream" << "\n";
		}

		//Convert to grayscale
		cvtColor(image_input, grey_image, CV_BGR2GRAY);
		
		//Threshold image BINARY
		threshold(grey_image, grey_image, 220, 255, THRESH_BINARY);

		//Threshold image ORANGE_Color (run on original image)
		inRange(image_input, Scalar(10,90,200), Scalar(140,230,250), color_threshed);

		cvtColor(grey_image, color_image, CV_GRAY2BGR);

		//Isolate orange colors byy finding contours then encapsulating in Boxes (Boxes are filled with white to generate region with orange)

		findContours(color_threshed, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));

		vector<vector<Point> > contours_poly(contours.size());
		vector<Rect> boundRect(contours.size());
		vector<Point2f>center(contours.size());
		vector<float> radius(contours.size());

		for (int i = 0; i < contours.size(); i++)
		{
			approxPolyDP(Mat(contours[i]), contours_poly[i], 3, true);
			boundRect[i] = boundingRect(Mat(contours_poly[i]));
		}


		
		grey_image.copyTo(drawing);
		threshold(drawing, drawing, 256, 256, THRESH_BINARY);

		for (int i = 0; i< contours.size(); i++)
		{
			Scalar color = Scalar(255, 255, 255);
			drawContours(drawing, contours_poly, i, color, 1, 8, vector<Vec4i>(), 0, Point());
			top_x = boundRect[i].tl().x;
			top_y = boundRect[i].tl().y;
			bottom_x = boundRect[i].br().x;
			bottom_y = boundRect[i].br().y;
			rectangle(drawing, Point(top_x - 5, top_y - 5), Point(bottom_x + 5, bottom_y + 5) , color, CV_FILLED, 8, 0);
			
		}
		

		//Combine the black and white and orange region image to find region with cross
		bitwise_and(drawing, grey_image, drawingFinal);

		
		//Eliminate noise within the image NEED TO ADD CENTER PROTOCOL 
		erode(drawingFinal, drawingFinal, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));
		//dilate(drawingFinal, drawingFinal, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));


		//Isolate white regions in the resulting image and find the largest white region to find cross
		
		drawingFinal.copyTo(Copy);
		cvtColor(drawingFinal,drawingFinal, CV_GRAY2BGR);
		//Canny(Copy, Copy, 60, 60);
		findContours(Copy, contours_2, hierarchy_2, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));

		vector<vector<Point> > contours_poly_2(contours_2.size());
		vector<Rect> boundRect_2(contours_2.size());
		vector<Point2f>center_2(contours_2.size());
		vector<float>radius_2(contours_2.size());

		for (int i = 0; i < contours_2.size(); i++)
		{
			approxPolyDP(Mat(contours_2[i]), contours_poly_2[i], 3, true);
			boundRect_2[i] = boundingRect(Mat(contours_poly_2[i]));
		}

		int max_size = 0;
		int box_size = 0;
		int center_x=0, center_y=0;
		//std::cout << "RRRG" << contours_2.size() << "\n";

		if (contours_2.size() == 0){
			continue; //COULDNT FIND CROSS!!!
		}

		count = 0;
		for (int i = 0; i< contours_2.size(); i++)
		{
			color = Scalar(80, 0, 80);
			//drawContours(drawingFinal, contours_poly_2, i, color, 1, 8, vector<Vec4i>(), 0, Point());
			top_x_2 = boundRect_2[i].tl().x;
			top_y_2 = boundRect_2[i].tl().y;
			bottom_x_2 = boundRect_2[i].br().x;
			bottom_y_2 = boundRect_2[i].br().y;
			box_size = abs(bottom_x_2 - top_x_2)*abs(top_y_2 - bottom_y_2);
			middle_x = (top_x_2 + bottom_x_2) / 2;
			middle_y = (top_y_2 + bottom_y_2) / 2;
			if (box_size > max_size)
			{
				max_size = box_size;
				center_x = middle_x;
				center_y = middle_y;
				
			}
			else
			{
				//Needs to be done -- Take average of blobs
			}
			
		}

		circle(drawingFinal, Point(center_x, center_y), 32, Scalar(0, 255, 0), 1, 8);
		
#ifdef Display_Results
		imshow("Transformed", drawingFinal);
		imshow("Original", image_input);
#endif

#ifdef Camera_Capture 
		waitKey(1);
#else
		waitKey(0);
#endif
		
		//imwrite("C:/Users/Lindsey/Documents/Visual Studio 2013/Projects/OpenCVHoughCode_Final/HSV_Out.png", image_input);

#ifdef Template_Process
		Mat results;
		results = MatchingMethod(image_input);
#ifdef Display_Results
		namedWindow("Template", WINDOW_AUTOSIZE);
		imshow("Template", results);
#endif
#endif

#ifdef Camera_Capture
#else
		char k;
		k = cvWaitKey(0);
		if (k == 'q')
		{ 
			break; 
		}
#endif

		//garbage Keeping 
		contours_poly.clear();
		boundRect.clear();
		center.clear();
		radius.clear();

		contours_poly_2.clear();
		boundRect_2.clear();
		center.clear();
		radius.clear();


	}
	return(0);
}




/*HoughLines(grey_image, Hough_Output, 1, CV_PI / 180, 50, 0, 0);

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
}*/

//namedWindow("HELLO", WINDOW_AUTOSIZE);
//imwrite("C:/Users/Lindsey/Documents/Visual Studio 2013/Projects/OpenCVHoughCode_Final/light_output.png", color_image);
//imshow("HELLO", color_image);
