/**
 * Bildbasierte Modellierung SS 2017
 * Prof. Dr.-Ing. Marcus Magnor
 *
 * Betreuer: JP Tauscher (tauscher@cg.cs.tu-bs.de)
 * URL: https://graphics.tu-bs.de/teaching/ss17/bbm
 */

#include <iostream>
#include <queue>

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/contrib/contrib.hpp>

#include "cv.h"                            
#include "highgui.h"
#include "cvaux.h"
#include "cxcore.h"
#include "opencv2/opencv.hpp"
#include <string>
#include <math.h>
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <cstdlib>
#include <stdio.h>  

using namespace cv;
using namespace std;

/**
 * Datum: 24.4.2018
 *
 * Übungsblatt: 3
 * Abgabe: 8.5.2018
 */

/**
 * Aufgabe: Median-Filter (10 Punkte)
 * Der Median-Filter ist ein einfacher nichtlinearer Filter, der sich
 * gut eignet, um bestimmte Arten von Bildrauschen zuentfernen.
 * - Implementiere einen Median-Filter, ohne \code{medianBlur} zu verwenden.
 *
 */

/* TODO */
void medianFilter(const Mat& image, Mat& image_out);
//unsigned char GetMedianNum(unsigned char * bArray, int iFilterLen); 
void salt(Mat& image, int n);
void pepper(Mat& image, int n);

int main(int argc, char **argv) {

	Mat image_gray = imread("fence.jpg",IMREAD_GRAYSCALE);
	//Mat image_gray = imread("skyscraper.jpg",IMREAD_GRAYSCALE);
	//Mat image_gray = imread("quad.png",IMREAD_GRAYSCALE);
	if (image_gray.empty())
    return -1;

	Mat image_gray_median(image_gray.rows,image_gray.cols,CV_8UC1);

	namedWindow("grau", WINDOW_NORMAL);
	imshow("grau", image_gray); 
	waitKey(0);

	medianFilter(image_gray,image_gray_median);

	namedWindow("median_grau", WINDOW_NORMAL); 
	imshow("median_grau", image_gray_median);
	waitKey(0);
    /**
     * - Wende den Median-Filter auf ein Graustufenbild an.
     */

/* TODO */

	//Mat image_color =imread("skyscraper.jpg",IMREAD_COLOR);
	Mat image_color = imread("fence.jpg",IMREAD_COLOR);
	//Mat image_color = imread("quad.png",IMREAD_COLOR);
	namedWindow("image_color", WINDOW_NORMAL); 
	imshow("image_color", image_color);
	waitKey(0);

    Mat median_image_color;
	vector<Mat> channels(3);
	vector<Mat> median_channels(3);
	Mat imageRedChannel(image_gray.rows,image_gray.cols,CV_8UC1);
	Mat imageGreenChannel(image_gray.rows,image_gray.cols,CV_8UC1);
    Mat imageBlueChannel(image_gray.rows,image_gray.cols,CV_8UC1);
	split(image_color, channels); 
	medianFilter(channels[2],imageRedChannel);
	medianFilter(channels[1],imageGreenChannel);
	medianFilter(channels[0],imageBlueChannel);
	median_channels[2]= imageRedChannel; 
	median_channels[1] = imageGreenChannel;
	median_channels[0] = imageBlueChannel;
	merge(median_channels,median_image_color);
    namedWindow("median_image_color", WINDOW_NORMAL); 
	imshow("median_image_color", median_image_color);
	waitKey(0);

    /**
     * - Wende den Median-Filter auf die einzelnen Kanäle eines Farbbilds an
     */


/* TODO */


    /**
     * - Wie kann man ungewollte Farbverschiebungen vermeiden?
     * - Für welche Arten von Rauschen eignet sich der Median-Filter gut, für welche nicht?
     */
	Mat image_gray1 = imread("fence.jpg",IMREAD_GRAYSCALE);
	//Mat image_gray1 = imread("skyscraper.jpg",IMREAD_GRAYSCALE);
	//Mat image_gray1 = imread("quad.png",IMREAD_GRAYSCALE);
	salt(image_gray1,5000);
	pepper(image_gray1,5000);
	namedWindow("noise_grau", WINDOW_NORMAL);
	imshow("noise_grau", image_gray1); 
	waitKey(0);

	Mat image_gray1_median(image_gray.rows,image_gray.cols,CV_8UC1);
	medianFilter(image_gray1,image_gray1_median);
	namedWindow("nosie_filter", WINDOW_NORMAL);
	imshow("nosie_filter", image_gray1_median); 
	waitKey(0);
	
    /**
     * Aufgabe: Hough-Transformation (10 Punkte)
     *
     * Die Hough-Transformation kann für das Finden von Linien in Bildern verwendet werden.
     *
     * In dieser Aufgabe sollst du die Hough-Transformation implementieren ohne die Funktionen \code{HoughLines}
     * oder \code{HoughLinesP} zu verwenden.
     */


    /**
    * - Erzeuge ein Kantenbild. Verwende dazu einen Filter deiner Wahl. Begründe die Wahl des Kantendetektors.
    */
	Mat canny;
	//unsigned int pixel;
	Canny(image_gray,canny,50,150,3); // value is 0 or 255
	imshow("canny", canny); 
	/*for(int i = 600; i < 700; i++){
		for(int j = 600; j < 700; j++){
		pixel = canny.at<uchar>(i,j);
	    cout<<pixel<<endl;
	 }
	}*/
	namedWindow("canny", WINDOW_NORMAL); 
	imshow("canny", canny); 
	waitKey(0);

/* TODO */

    /**
    * - Transformiere das Kantenbild in den Hough-Raum und zeige diesen in einer geeigneten Color Map an.
    */
	float angle;
	float distance;
	int distance_int;
	double distance_max = sqrt((double)(canny.cols * canny.cols + canny.rows * canny.rows)); //rows= height= 768 cols = height =1024 
	cout<<sin(-1.04719667)<<endl;
	int hough_rows = cvRound(distance_max);
	int hough_cols = 180; // change -90 -> 90 to 0 -> 180
	Mat hough = Mat::zeros(hough_rows * 2, hough_cols, CV_16UC1); // parameter(ρ,θ) -p -> p  into 0 ->2p
	cout<<hough.rows<<endl;
	cout<<hough.cols<<endl;
	for (int i = 0; i < canny.rows; i++) {
        for (int j = 0; j < canny.cols; j++) {
			if(canny.at<uchar>(i, j) == 255) {
				for (int k = 0 ; k < 180; k++) {
					angle = ((float) k - 90 )/ 180 * CV_PI; //real angle in radian
					//angle = (float) k / 180 * CV_PI;
					distance = j * cos(angle) + i * sin(angle);
					//if(distance > 0) {distance_int = cvRound(distance) + hough_rows;}
					//else {distance_int = cvRound(distance);
					//	  distance_int = abs(distance_int);}
					distance_int = cvRound(distance) + hough_rows;
						if(k >= 0 && k < hough.cols && distance_int >= 0 && distance_int < hough.rows){
							hough.at<ushort>(distance_int,k) += 1; // accumulate
						}
					}
				}
			}
		}
    Mat hough_color;
    normalize(hough, hough_color, 0, 255, NORM_MINMAX, CV_8UC1);
    applyColorMap(hough_color, hough_color, COLORMAP_JET);
	namedWindow("hough space", WINDOW_NORMAL);
    imshow("hough space", hough_color);
    waitKey(0);

/* TODO */

    /**
    * - Finde die markantesten Linien und zeichne diese in das Originalbild ein.
    */

/* TODO */
	 vector<Point> lines;
	 int maxvalue = 0;
	 	 	for (int i = 0; i < hough.rows; i++) {
				for (int j = 0; j < hough.cols; j++) {
				 if (hough.at<ushort>(i, j) > maxvalue)
					{
						 maxvalue = hough.at<ushort>(i, j);

					}
				}
			}
	cout<<"max"<<maxvalue<<endl;
    int threshold = cvRound(0.8 * maxvalue);
	 Point pt;
	 	for (int i = 1; i < hough.rows -1; i++) {
			for (int j = 1; j < hough.cols -1; j++) {
				int value = hough.at<ushort>(i, j);
				if( value > threshold) 
				{
					if((value > hough.at<ushort>(i-1, j-1)) && (value > hough.at<ushort>(i, j-1)) && (value > hough.at<ushort>(i+1, j-1)) && (value > hough.at<ushort>(i-1, j)) && (value > hough.at<ushort>(i+1, j)) && (value > hough.at<ushort>(i-1, j+1)) && (value > hough.at<ushort>(i, j+1)) && (value > hough.at<ushort>(i+1, j+1)))
                   {
						pt.x = i;
					    pt.y = j;
					    lines.push_back(pt);
				   }
				 }
			}
}
		cout<<"size"<<lines.size()<<endl;
		for (unsigned int i = 0; i<lines.size(); i++) 
			{
				//float r = lines[i].x > hough_rows ? lines[i].x - hough_rows : lines[i].x ; 
				float r = lines[i].x - hough_rows;
				float theta = (float) (lines[i].y -90)/ 180 * CV_PI;
				Point pt1,pt2;
				double a=cos(theta);
				double b=sin(theta);
				double x = r * a;
				double y = r * b;
				pt1.x = 0;
				pt1.y = cvRound(r / b);
				pt2.x = canny.cols;   
				pt2.y = cvRound((- a * canny.cols + r) / b);

				//pt2.x = cvRound(r / a);
				//pt2.y = 0;

				//pt1.x = cvRound(x - 10000 * b);
				//pt1.y = cvRound(y + 10000 * a);
				//pt2.x = cvRound(x + 10000 * b);
				//pt2.y = cvRound(y - 10000 * a);
				line(image_color, pt1, pt2, Scalar(0,0,225), 2, CV_AA);
			 } 
   // namedWindow("Hough", WINDOW_NORMAL);
	imshow("Hough", image_color);
	waitKey(0);
	
	}

/*unsigned char GetMedianNum(unsigned char * Array, int kernelsize)  
{   
    unsigned char Temp; 
    for (int j = 0; j < kernelsize - 1; j ++)  
    {  
        for (int i = 0; i < kernelsize - j - 1; i ++)  
        {  
            if (Array[i] > Array[i + 1])  
            {  
                Temp = Array[i];  
                Array[i] = Array[i + 1];  
                Array[i + 1] = Temp;  
            }  
        }  
    }  
    if ((kernelsize % 2) == 1){    
        Temp = Array[(kernelsize - 1) / 2];  
    }  
    else{    
        Temp = (Array[kernelsize / 2] + Array[kernelsize / 2 + 1]) / 2;  
    }  
    return Temp;  
}  */

void medianFilter(const Mat& image, Mat& image_out) {
	if (image.channels() != 1) {
        cout << "the input should be a gray picture " << endl; 
		exit(1);
    }
    int rows=image.rows;
	int cols=image.cols; 
	for (int j=1;j< rows -1;j++)  
    {  
		for (int i=1;i< cols-1;i++)  
        {  
				unsigned char window[9];      
				 window[0] = image.at<uchar>(j-1,i-1);
				 window[1] = image.at<uchar>(j-1,i);
				 window[2] = image.at<uchar>(j-1,i+1);
				 window[3] = image.at<uchar>(j,i-1);
				 window[4] = image.at<uchar>(j,i);
				 window[5] = image.at<uchar>(j,j+1);
				 window[6] = image.at<uchar>(j+1,i-1);
				 window[7] = image.at<uchar>(j+1,i);
				 window[8] = image.at<uchar>(j+1,i+1);
                 //image_out.at<uchar>(j,i) =GetMedianNum(window, 9);
				 unsigned char Temp;   
				 for (int m = 0; m < 9 - 1; m ++){  
					for (int n = 0; n < 9 - m - 1; n ++){  
						 if (window[n] > window[n + 1]){  
								Temp = window[n];  
								window[n] = window[n + 1];  
								window[n + 1] = Temp;  
							}  
						}  
					}  
				image_out.at<uchar>(j,i) = window[4]; 
        }  
    } 
}

void salt(Mat& image, int n){
    for(int k=0; k<n; k++){
        int i = rand()%image.cols;
        int j = rand()%image.rows;
        
        if(image.channels() == 1){
            image.at<uchar>(j,i) = 255;
        }else{
            image.at<Vec3b>(j,i)[0] = 255;
            image.at<Vec3b>(j,i)[1] = 255;
            image.at<Vec3b>(j,i)[2] = 255;
        }
    }
}

void pepper(Mat& image, int n)  
{  
    for(int k=0; k<n; k++)  
    {  
        int i = rand()%image.cols;  
        int j = rand()%image.rows;  
  
        if(image.channels() == 1)  
        {  
            image.at<uchar>(j,i) = 0;  
        }  
        else if(image.channels() == 3)  
        {  
            image.at<Vec3b>(j,i)[0] = 0;  
            image.at<Vec3b>(j,i)[1] = 0;  
            image.at<Vec3b>(j,i)[2] = 0;  
        }  
    }  
}  