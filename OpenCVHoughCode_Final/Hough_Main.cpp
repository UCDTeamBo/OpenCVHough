
//Initiating portions of the code: Make sure Camera_Capture is commented in order to have it run of webcam
//#define Camera_Capture
#define Display_Results
//#define Template_Process;

//#include "opencv2/core/core.hpp"
#ifdef Display_Results
//#include "opencv2/highgui/highgui.hpp"
#endif
//#include "opencv2/imgproc/imgproc.hpp"
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

int Angelas_magic(int x_of_circle, int y_of_circle, int w_of_image, int h_of_image)
{
	// variables
	int ctr_width = x_of_circle; // horizontal position of center of cross (from circle)
	int ctr_height = y_of_circle; // vertical position of center of cross (from circle)
	int w = w_of_image; // width of entire image
	int h = h_of_image; // height of entire image
	// may not want to include w_i and h_i, the width and height of the cross in pixels
	// these are guesses
	int w_i = w/10;
	int h_i = h/10;
	int distance = 10; // determined by LIDAR or size of cross
	int output = 000;
	time_t start_time;
	time_t now;


	// set start point
	/*std::cout << "Please enter horizontal coordinate of center of cross: " << endl;
	std::cin >> ctr_width;
	std::cout << "Please enter vertical coordinate of center of cross: " << endl;
	std::cin >> ctr_height;

	std::cout << "Please enter distance to object in feet: " << endl;
	std::cin >> distance;

	std::cout << "Clocks per second: " << CLOCKS_PER_SEC << endl;*/


		//commented for test purposes		output = 000; // hover until given directions to move
		// may not be necessary with blocking statements

		if (ctr_width < (w/2 - w_i/2))
		{
			// cross is on left side, need to move left
			std::cout << "Cross is on left side." << "\n";
			output = 100; // move left, specify duration?
			// will it do this until conditional not true?
			// may need to set with blocking statement in verilog
			std::cout << "Output signal is: " << output << "\n";

			// keep output for certain amt of time
			// set output for time

			std::cout << "entering diff" << "\n";
			start_time = time(NULL);
			std::cout << "start time is: " << start_time << "\n";
			now = time(NULL);
			std::cout << "time is" << now << " looping" << "\n";

			while ((difftime(now, start_time)) < 1)
			{
				now = time(NULL);

			}
			std::cout << "now is " << now << "\n";
			std::cout << "Entering hover mode!" << "\n";

			output = 000;
			// every time this runs, end our code and go back to OCV code

			ctr_width += 100;
		}

		else if (ctr_width > (w/2 + w_i/2))
		{
			// cross is on right side, need to move right
			std::cout << "Cross is on right side." << "\n";
			output = 101;

			//start of loop, copied
			std::cout << "Output signal is: " << output << "\n";
			// timing
			start_time = time(NULL);
			now = time(NULL);

			while ((difftime(now, start_time)) < 1)
			{
				now = time(NULL);

			}

			output = 000;
			//end of loop, copied


			ctr_width -= 100;
		}

		if (ctr_height > (h / 2 + h_i / 2))
		{
			// cross is on lower half, move down
			std::cout << "Cross is on lower half." << "\n";
			//output = 011;
			output = 22;

			//start of loop, copied
			std::cout << "Output signal is: " << output << "\n";
			// timing
			start_time = time(NULL);
			now = time(NULL);

			while ((difftime(now, start_time)) < 1)
			{
				now = time(NULL);

			}

			output = 000;
			//end of loop, copied

			ctr_height += 100;
		}

		else if (ctr_height < (h / 2 - h_i / 2))
		{
			// cross is on upper half, move up/down
			std::cout << "Cross is on upper half." << "\n";
			//output = 010;
			output = 77;

			//start of loop, copied
			std::cout << "Output signal is: " << output << "\n";
			// timing
			start_time = time(NULL);
			now = time(NULL);

			while ((difftime(now, start_time)) < 1)
			{
				now = time(NULL);

			}

			output = 000;
			//end of loop, copied

			ctr_height -= 100;
		}

		if (distance > 10)
		{
			std::cout << "Cross is still " << distance << " away." << "\n";
			// if using a blocking statement, distance might not update
			// this may not be necessary if nonblocking, just use 4
			// move forward
			output = 001;

			//start of loop, copied
			std::cout << "Output signal is: " << output << "\n";
			// timing
			start_time = time(NULL);
			now = time(NULL);

			while ((difftime(now, start_time)) < 1)
			{
				now = time(NULL);

			}

			output = 000;
			//end of loop, copied

			distance--;
		}

		else if (distance > 4)
		{
			std::cout << "Cross is still " << distance << " away." << "\n";
			// after centering again, move closer
			output = 001;

			//start of loop, copied
			std::cout << "Output signal is: " << output << "\n";
			// timing
			start_time = time(NULL);
			now = time(NULL);

			while ((difftime(now, start_time)) < 1)
			{
				now = time(NULL);

			}

			output = 000;
			//end of loop, copied

			distance--;
		}

		else
		{
			// should be centered
			// distance should be <= 4
			// execute avoidance sequence
			output = 111;

			//start of loop, copied
			std::cout << "Output signal is: " << output << "\n";
			// timing
			start_time = time(NULL);
			now = time(NULL);

			while ((difftime(now, start_time)) < 1)
			{
				now = time(NULL);

			}

			//end of loop, copied WITHOUT output = 000
			std::cout << "avoid" << "\n";
		}

		std::cout << "Output signal is: " << output << "\n";

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
