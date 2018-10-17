
#include <iostream>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include "cv.h"                            
#include "highgui.h"
#include "cvaux.h"
#include "cxcore.h"
#include "opencv2/opencv.hpp"
#include <string>
#include <math.h>

using namespace cv;
using namespace std;

int main(int argc,char* argv[])
{
	
	
	Mat image = imread("distorted.png");  
	if (image.empty())
        return -1;
	//Mat  image1(image.rows,image.cols,CV_8UC3,Scalar(0,0,0));  
	Mat image1;
	image.copyTo(image1);
	
	cout << "height：" << image.rows << endl;
	cout << "wide：" << image.cols << endl;
	cout << "channel：" << image.channels() << endl;
	namedWindow("Exercise1", WINDOW_AUTOSIZE); // create a new window
	imshow("Exercise1", image);    // show the picture in the window 
	waitKey(0);
////////////////////////////////////////////////////////////////////////////////////////


	vector<Mat> channels;
	Mat imageRedChannel;
	Mat imageGreenChannel;
    Mat imageBlueChannel;
    split(image, channels); 
	imageRedChannel = channels[2]; 
	imageGreenChannel = channels[1];
	imageBlueChannel = channels[0];
      
    Mat tmp(image.size(),CV_8U,Scalar(0));        //  CV_8U data depth  
												 //other two channel set 0
    vector<Mat> r,g,b;   //used to represent r g b picture   
  
    for(int i=0;i<3;i++)  
    {  
        if(i==0)  
            b.push_back(imageBlueChannel);  
        else  
            b.push_back(tmp);  
  
        if(i==1)  
            g.push_back(imageGreenChannel);  
        else  
            g.push_back(tmp);  
  
        if(i==2)  
            r.push_back(imageRedChannel);  
        else  
            r.push_back(tmp);  
    }  

    Mat image_b,image_g,image_r;  
	Mat combine1,combine;
      
    merge(b,image_b);  
    merge(g,image_g);  
    merge(r,image_r); 

	hconcat(image_r,image_g,combine1);
	hconcat(combine1,image_b,combine);
	namedWindow("channel", WINDOW_AUTOSIZE);
	imshow("channel",combine);
	waitKey(0);

	/////////////////////////////////////////////////////////////////
	Rect R_r;
	R_r.x=image.cols/2-5;
	R_r.y=image.rows/2-5;
	R_r.width=10;
	R_r.height=10;

	rectangle(image1,R_r,Scalar(0,0,255),CV_FILLED,1,0);

	namedWindow("middle", WINDOW_AUTOSIZE); 
	imshow("middle",image1);
	waitKey(0);
	 
////////////////////////////////////////////
	Mat image2(image.rows,image.cols,CV_8UC3,Scalar(0,0,0));
    int rows=image.rows;//height
    int cols=image.cols; //width
	int x_c=image.rows/2;
	int y_c=image.cols/2;
	int x_d=0;
	int y_d=0;
	double k_1=0.001;
	double k_2=0.000005;
	double r_r=0;
	double Lr=0;
    for(int i=0; i<rows; i++)
    {
        for(int j=0; j<cols; j++)
        {
			r_r=sqrt(double((i-x_c)*(i-x_c)+(j-y_c)*(j-y_c)));
			Lr = 1+k_1*r_r+k_2*r_r*r_r;
			x_d= (int)((i - x_c)/Lr) + x_c;
			y_d= (int)((j - y_c)/Lr) + y_c;
				for(int k=0; k<3; k++)
					{
                    image2.at<Vec3b>(i,j)[k]=image.at<Vec3b>(x_d,y_d)[k];
				}
         }
     }
	namedWindow("entzerren", WINDOW_AUTOSIZE); 
	imshow("entzerren",image2);
	waitKey(0);
    return 0;
	

	/*Mat image = imread("pp.jpg",1);  
	cout << "Hoehe：" << image.rows << endl;
	cout << "Breite：" << image.cols << endl;
	namedWindow("Exercise1", WINDOW_AUTOSIZE); // create a new window
	imshow("Exercise1",image);    // show the picture in the window 
	waitKey(0);
	return 0;*/

	
    
}




