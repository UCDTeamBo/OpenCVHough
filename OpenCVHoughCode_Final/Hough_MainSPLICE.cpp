
//Initiating portions of the code: Make sure Camera_Capture is commented in order to have it run of webcam
//#define Camera_Capture
#define Display_Results
//#define Template_Process;

//#include "opencv2/core/core.hpp"
#ifdef Display_Results
//#include "opencv2/highgui/highgui.hpp"
#endif
//#include "opencv2/imgproc/imgproc.hpp"

//Includes for both Center-Finder and Flight Controller
#include <opencv2/opencv.hpp>
#include <stdio.h>
#include <iostream>
#include <ctime>
#include <cstdlib>

using namespace cv;


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
int height, width;
Size s;

//Volatile Variables for Flight Controller
//***NOTE*** THESE WILL BE REPLACED WITH PROPER OFFSET CALCULATIONS
volatile int* led = 		(int*)0xFF200040;		//Reded LED address
volatile int* hex3_0 = 		(int*)0xFF200020;		//HEX3_HEX0 address
volatile int* hex5_4 = 		(int*)0xFF200010;		//HEX5_HEX4 address
volatile int* switchptr = 	(int*)0xFF200030;		//SW Slider switch address
volatile int* pushptr = 	(int*)0xFF200000;		//KEY push button address

//Non-volatile Variables for Flight Controller
int ctr_width; 		// horizontal position of center of cross (from circle)
int ctr_height; 	// vertical position of center of cross (from circle)
int w = 1920; 		// width of entire image
int h = 1080; 		// height of entire image
int w_i = 540;
int h_i = 360;
int output = 000;
int i = 0;
int switchValue;
time_t start_time, now;
*(led) = 0;

int Angelas_magic(int x_of_circle, int y_of_circle, int w_of_image, int h_of_image);

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
	//VideoWriter out;
	//out.open("C:/Users/Lindsey/Documents/Visual Studio 2013/Projects/OpenCVHoughCode_Final/IMG_0450_out.mov", CV_FOURCC('D', 'I', 'V', 'X'), 120, cv::Size(1200, 1600), true);
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
			break;
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

		imwrite("C:/Users/Lindsey/Documents/Visual Studio 2013/Projects/OpenCVHoughCode_Final/Final_Image.png", drawingFinal);
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
		std::cout << "Image X coordinate: " << center_x << " " << "Image Y coordinate: " << center_y << "\n";
		
		//s = drawingFinal.size();
		//height = s.height();
		//width = s.width();
		
		height = drawingFinal.size.p[0];
		width = drawingFinal.size.p[1];
		std::cout << "Image Heigth: " << height << "Image Width: " << width <<"\n";
		Angelas_magic(center_x, center_y, width, height);
		
		
#ifdef Display_Results
		imshow("Transformed", drawingFinal);
		//waitKey(1);
		imshow("Original", image_input);
		//out.write(drawingFinal);
#endif

#ifdef Camera_Capture 
		waitKey(1);
#else
		waitKey(0);
#endif
		if (contours_2.size() == 0){
			std::cout << "Error: Couldn't find cross!!" << "\n";
			//COULDNT FIND CROSS!!!
		}
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
			//out.release();
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

/*********************************************
			SPLICE AND GPIO NOTES
Three actions per function call.
	One left/right, one up/down, one forward
Switch positions are monitored each loop cycle, gives user time to change them
	Each switch loop does nothing different, but can be easily modified
On DE1-SOC board, LED[2:0] are output simulation
	To Flight Controller
On DE1-SOC board, SW[1:0] are input simulation
	From LIDAR

When updated by Bo Sunday May 24th, 2015
All *(led) calls should be changed to the corresponding GPIO pins for output
All *(switchptr) calls should be changed to the corresponding GPIO pins for input

When updated by Group the week of Monday May 25th, 2015
Timing loops need to be calibrated with speed of UAV to properly maneuver
"Error" distance needs to be calibrated with movement of UAV
Each of the four forward loops should be differentiated based on input signal

Final Notes: 
No delay loop added to humanly differentiate between signals on DE1
If you have questions about any of the following code: 
Contact Chris Bird 
(707) 732-3034 (Phone)
ckbird@ucdavis.edu (Email)
ChrisBird1313 (Skype)

*********************************************/

int Angelas_magic(int x_of_circle, int y_of_circle, int w_of_image, int h_of_image)
{
	//Top left of image is 0,0 by OpenCV Convention
	/* --- BEGIN ACTION LEFT/RIGHT --- */

	if (ctr_width < (w/2 - w_i/2)) //Cross: Left - Move: Left
	{
		output = 100;
		*(led) = 4;
		start_time = time(NULL);
		now = time(NULL);
		while ((difftime(now, start_time)) < 1)
		{
			now = time(NULL);
		}
		output = 000;
		*(led) = 0;
		ctr_width += 100;
	}
	
	else if (ctr_width > (w/2 + w_i/2)) //Cross: Right - Move: Right
	{
		output = 101;
		*(led) = 5;
		start_time = time(NULL);
		now = time(NULL);
		while ((difftime(now, start_time)) < 1)
		{
			now = time(NULL);
		}
		output = 000;
		*(led) = 0;
		ctr_width -= 100;
	}

	/* --- END OF ACTION LEFT/RIGHT --- */
	/* --- BEGIN ACTION UP/DOWN --- */

	if (ctr_height > (h / 2 + h_i / 2)) //Cross: Down - Move: Down
	{
		output = 011;
		*(led) = 3;
		start_time = time(NULL);
		now = time(NULL);
		while ((difftime(now, start_time)) < 1)
		{
			now = time(NULL);
		}
		output = 000;
		*(led) = 0;
		ctr_height += 100;
	}
	
	else if (ctr_height < (h / 2 - h_i / 2)) // Cross: Up - Move: UP
	{
		output = 010;
		*(led) = 2;
		start_time = time(NULL);
		now = time(NULL);
		while ((difftime(now, start_time)) < 1)
		{
			now = time(NULL);
		}
		output = 000;
		*(led) = 0;
		ctr_height -= 100;
	}
	
	/* --- END OF ACTION UP/DOWN --- */
	/* --- BEGIN ACTION FORWARD --- */

	switchValue = *(switchptr);
	if (switchValue = 3) //Cross: Ahead - Move: Ahead
	{
		output = 001;
		*(led) = 1;
		start_time = time(NULL);
		now = time(NULL);
		while ((difftime(now, start_time)) < 1)
		{
			now = time(NULL);
		}
		output = 000;
		*(led) = 0;
		distance--;
	}
	
	else if (switchValue = 2) //Cross: Ahead - Move: Ahead
	{
		output = 001;
		*(led) = 1;
		start_time = time(NULL);
		now = time(NULL);
		while ((difftime(now, start_time)) < 1)
		{
			now = time(NULL);
		}
		output = 000;
		*(led) = 0;
		distance--;
	}

	else if (switchValue = 1) //Cross: Ahead - Move: Ahead
	{
		output = 001;
		*(led) = 1;
		start_time = time(NULL);
		now = time(NULL);
		while ((difftime(now, start_time)) < 1)
		{
			now = time(NULL);
		}
		output = 000;
		*(led) = 0;
		distance--;
	}
	
	/* --- END ACTION FORWARD --- */

	else //AVOID, switch value = 0
	{
		output = 111;
		*(led) = 7;
		start_time = time(NULL);
		now = time(NULL);
		while ((difftime(now, start_time)) < 1)
		{
			now = time(NULL);
		}
	}
	return (0);
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
