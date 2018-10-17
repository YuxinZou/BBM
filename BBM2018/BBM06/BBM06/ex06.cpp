/**
 * Bildbasierte Modellierung SS 2017
 * Prof. Dr.-Ing. Marcus Magnor
 *
 * Betreuer: JP Tauscher (tauscher@cg.cs.tu-bs.de)
 * URL: https://graphics.tu-bs.de/teaching/ss17/bbm
 */

#include <iostream>
#include <vector>

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/contrib/contrib.hpp>

using namespace cv;

/**
 * Datum: 29.5.2018
 *
 * Übungsblatt: 6
 * Abgabe: 5.6.2018
 */

/**
 * Die Middleburry Stereo Datasets (\url{http://vision.middlebury.edu/stereo/data/}) dienen als Benchmark
 * für Algorithmen zur Schätzung von Tiefenkarten aus rektifizierten Stereo-Bildpaaren. Die Scores können unter
 * \url{http://vision.middlebury.edu/stereo/eval3/} eingesehen werden.
 */


int main(int argc, char **argv) {

   /* if (argc < 3) {
        std::cerr << "usage: " << argv[0] << " <image_1> " << "<image_2>" << std::endl;
        exit(1);
    }*/

    Mat img1, img2;

    Mat img1_U8 = imread("imgL.png", IMREAD_COLOR);
    Mat img2_U8 = imread("imgR.png", IMREAD_COLOR);

    /*if (img1_U8.empty() || img2_U8.empty()) {
        std::cout <<L"Could not load image file: " << argv[1] << " or " << argv[2] << std::endl;
        exit(0);
    }*/


    img1_U8.convertTo(img1, CV_32F, 1 / 255.);
    img2_U8.convertTo(img2, CV_32F, 1 / 255.);

	//namedWindow("Input Image 1", WINDOW_NORMAL); 
	imshow("Input Image 1", img1);
	//namedWindow("Input Image 2", WINDOW_NORMAL);  
    imshow("Input Image 2", img2);
    waitKey(0);

    /**
     * Aufgabe: Erzeugen einer Tiefenkarte (10 Punkte)
     *
     * Implementiere ein eigenes Verfahren zur Tiefenschätzung auf einem Middleburry Bildpaar Deiner Wahl.
     * Der Suchradius soll dabei vorgegeben werden können. Der Pixel in Bild 1 an der Position $\vec{x}_1$
     * ist beschrieben durch einen Deskriptor $d_1(\vec{x}_1)$. Für jeden Pixel $\vec{x}_2$ innerhalb des
     * Suchradius muss nun der Deskriptor $d_2(\vec{x}_2)$ mit $d_1$ verglichen werden. Verwende als Deskriptor
     * zunächst einfach die Farbe des Pixels. Zeige die erzeugte Tiefenkarte unter Verwendung einer geeigneten
     * Color Map an.
     */

/* TODO */
	//parameter
/*
cam0=[974.085 0 266.209; 0 974.085 244.114; 0 0 1]
cam1=[974.085 0 307.175; 0 974.085 244.114; 0 0 1]
doffs=40.966
baseline=173.557
width=713
height=488
ndisp=63
*/
	Mat depth_pic = Mat::zeros(img1.rows, img1.cols, CV_32FC1);  //for the depth picture
	float focal_length = 974.085;
	float baseline = 173.557;
	float doffs = 40.966;  //maybe useful????

	float min_cost = FLT_MAX;
	unsigned int disparity_range = 50;    //from 0 to 50  or should also to be minus?  -50 to 50????
	int current_dx = -1;
	//float depth_pic_max = FLT_MIN; //upper bound
	//float depth_pic_min = FLT_MAX; //lower bound  // set for normalization

for(unsigned int i = 0; i < img1.rows; i++){
		for(unsigned int j = 0; j < img1.cols - disparity_range; j++){
			current_dx = -1;
			min_cost = FLT_MAX;
			for( int k = 0 ; k < disparity_range; k++){
				Vec3f discriptor = img1.at<Vec3f>(i,j) - img2.at<Vec3f>(i,j+k);
				float discriptor_blue_disparity = discriptor[0];
				float discriptor_green_disparity = discriptor[1];
				float discriptor_red_disparity = discriptor[2];
				float current_cost = cvSqrt(discriptor_blue_disparity * discriptor_blue_disparity + discriptor_green_disparity * discriptor_green_disparity + discriptor_red_disparity * discriptor_red_disparity);
				if(current_cost < min_cost ){
					min_cost = current_cost;
					current_dx = k;
				}

		}
			depth_pic.at<float>(i,j) = float(current_dx);
			//depth_pic.at<float>(i,j) = focal_length * baseline / (float(current_dx)+ doffs);  //the problem is, if dx is 0, then the max is infinte
			//std::cout<<"pixel"<<depth_pic.at<float>(i,j)<<std::endl;
	}
}
	Mat color_map;
    normalize(depth_pic, color_map, 0, 255, NORM_MINMAX, CV_8UC1);
    applyColorMap(color_map, color_map, COLORMAP_JET);
	imshow("depth_pic",color_map);
	waitKey(0);
	


    /**
     * Aufgabe: Robustere Methoden (20 Punkte)
     *
     * In dieser Aufgabe soll die Tiefenschätzung robuster gegenüber Fehlern
     * gemacht werden.  Hier ist Deine Kreativität gefragt.  Überlege Dir wie die
     * Disparität zuverlässiger bestimmt werden kann und implementiere Deinen
     * Ansatz. Möglich wären zum Beispiel:
     * - bessere Deskriptoren, etwa ein Fenster von mehreren Pixeln Breite
     * - Regularisierung, d.h. benachbarte Pixel sollten ähnliche Tiefenwerte
     *   haben, auch wenn dadurch die Deskriptoren etwas weniger gut passen; dazu
     *   könnte man beispielsweise mit der Lösung der ersten Aufgabe beginnen und
     *   in einem zweiten Schritt die Disparitäten der Nachbarpixel in den
     *   Deskriptor mit einbauen. Das Ganze würde man dann solange wiederholen, bis
     *   sich nichts mehr ändert.
     * - Weitere Inspiration kann beim Stöbern durch die Paper in den Middleburry-Scores
     *   gefunden werden.
     */

/* TODO */
///////////////////////////////////////////////////////////////////////////////////////////////////////////
//add a window of more pixels//

	Mat depth_pic_Robust = Mat::zeros(img1.rows, img1.cols, CV_32FC1);  


	int window_size = 1; //window is 3*3 
	current_dx = -1;
	float sum;
	for(int i = window_size; i < img1.rows - window_size; i++){
		for(int j = window_size; j < img1.cols - disparity_range; j++){
			current_dx = -1;
			min_cost = FLT_MAX;
			for( int k = 0 ; k < disparity_range; k++){
				sum = 0.0f;
				for(int x = - window_size; x < window_size; x++){
					for(int y = - window_size; y < window_size; y++){
						Vec3f discriptor = img1.at<Vec3f>(i+x,j+y) - img2.at<Vec3f>(i+x,j+y+k);
						float discriptor_blue_disparity = discriptor[0];
						float discriptor_green_disparity = discriptor[1];
						float discriptor_red_disparity = discriptor[2];
						float current_cost = cvSqrt(discriptor_blue_disparity * discriptor_blue_disparity + discriptor_green_disparity * discriptor_green_disparity + discriptor_red_disparity * discriptor_red_disparity);
						sum += current_cost;
					}
				}
				if(sum < min_cost ){
					min_cost = sum;
					current_dx = k;
				}

		}
			depth_pic.at<float>(i,j) = float(current_dx);
			//std::cout<<"pixel"<<depth_pic.at<float>(i,j)<<std::endl;
	}
}
	Mat color_map_2;
    normalize(depth_pic, color_map_2, 0, 255, NORM_MINMAX, CV_8UC1);
    applyColorMap(color_map_2, color_map_2, COLORMAP_JET); 
	imshow("depth_pic_2",color_map_2);
	waitKey(0);

///////////////////////////////////////////////////////////////////////////////////////////

//add the gewichte neighbor pixel into the middle pixel and do the iteration for 4 times, 
//so that the depth value more similar, but the outcome seems not so gut.
	Mat disparity = Mat::zeros(img1.rows,img1.cols,CV_32FC1); 
	Mat cost = Mat::zeros(img1.rows,img1.cols,CV_32FC1);  //when the disparity of the pixel is D 
	float p1 = 0.1f;   //P1 is the weight on the disparity change by plus or minus 1 between neighbor pixels. 
	float p2 = 0.025f;  //P2 is the weight on the disparity change by more than 1 between neighbor pixels.
	unsigned int iteration_number = 4;
	for(unsigned int i = 0; i < img1.rows; i++){
		for(unsigned int j = 0; j < img1.cols - disparity_range; j++){
			current_dx = -1;
			min_cost = FLT_MAX;
			for( int k = 0 ; k < disparity_range; k++){
				Vec3f discriptor = img1.at<Vec3f>(i,j) - img2.at<Vec3f>(i,j+k);
				float discriptor_blue_disparity = discriptor[0];
				float discriptor_green_disparity = discriptor[1];
				float discriptor_red_disparity = discriptor[2];
				float current_cost = cvSqrt(discriptor_blue_disparity * discriptor_blue_disparity + discriptor_green_disparity * discriptor_green_disparity + discriptor_red_disparity * discriptor_red_disparity);
				//std::cout<<"current_cost:"<<current_cost * 255<<std::endl;
				if(current_cost < min_cost ){
					min_cost = current_cost;
					current_dx = k;
				}

			}
			disparity.at<float>(i,j) = float(current_dx);
		}
	}
	while(iteration_number > 0){  //more easy when the lteration is constant
		for(unsigned int i = window_size; i < img1.rows; i++){
			for(unsigned int j = window_size; j < img1.cols - disparity_range; j++){
				//std::cout<<"pixel"<<disparity.at<float>(i,j)<<std::endl;
				disparity.at<float>(i,j) = 0.5f * disparity.at<float>(i,j);
				//Traverse all the 8 neighbor to update the disparity
				for(int x = - window_size; x < window_size; x++){
					for(int y = - window_size; y < window_size; y++){
						if(abs(disparity.at<float>(i+x,j+y) - disparity.at<float>(i,j)) <= 1.0)
							disparity.at<float>(i,j) += p1 * disparity.at<float>(i+x,j+y);
						else
							disparity.at<float>(i,j) += p2 * disparity.at<float>(i+x,j+y);
					}
				}
			}
		}
							iteration_number--;
	}
	Mat color_map_3;
	/*for(unsigned int i = 70; i < 100; i++){
		for(unsigned int j = 70; j < 100; j++){
			std::cout<<"value before nomalization"<<disparity.at<float>(i,j)<<std::endl;
		}
	}*/
    normalize(disparity, color_map_3, 0, 255, NORM_MINMAX, CV_8UC1);
		/*for(unsigned int i = 70; i < 100; i++){
		for(unsigned int j = 70; j < 100; j++){
			std::cout<<"value after nomalization"<<color_map_3.at<float>(i,j)<<std::endl;
		}
	}*/
    applyColorMap(color_map_3, color_map_3, COLORMAP_JET); 
	imshow("depth_pic_3",color_map_3);
	waitKey(0);

}
