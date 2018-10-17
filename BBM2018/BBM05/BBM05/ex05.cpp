/**
 * Bildbasierte Modellierung SS 2017
 * Prof. Dr.-Ing. Marcus Magnor
 *
 * Betreuer: JP Tauscher (tauscher@cg.cs.tu-bs.de)
 * URL: https://graphics.tu-bs.de/teaching/ss17/bbm
 */

#include<cstdlib>
#include<ctime>
#include <iostream>
#include <set>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include "opencv2/core/core.hpp"  
#include "opencv2/features2d/features2d.hpp"  
#include "opencv2/highgui/highgui.hpp"  
#include <vector>  
#include <opencv2/legacy/legacy.hpp>  
#include "opencv2/core/core.hpp"  
#include "opencv2/features2d/features2d.hpp"  
#include "opencv2/highgui/highgui.hpp"  
#define random(a,b) (rand()%(b-a+1)+a)

using namespace cv;
using namespace std;
/**
 * Datum: 15.5.2018
 *
 * Übungsblatt: 5
 * Abgabe: 29.5.2018
 */

// Nutze hier deine Funktion aus Übungsbatt 4. Du darfst auch die Musterlösung verwenden.
void createPanorama(const Mat &img1, const Mat &img2, Mat H, Mat &panorama) {
/* TODO */
	std::vector<Point2f> corners(4);
	corners[0] = Point2f(0.f , 0.f);
	corners[1] = Point2f((float) img1.cols, 0.f);
	corners[2] = Point2f(0.f ,  (float) img1.rows);
	corners[3] = Point2f((float) img1.cols , (float) img1.rows);

    perspectiveTransform(corners, corners, H);

    int max_x = static_cast<int>(std::max(corners[1].x, corners[3].x) + .5f);
    int min_x = static_cast<int>(std::min(corners[0].x, corners[2].x));
    int max_y = static_cast<int>(std::max(corners[2].y, corners[3].y) + .5f);
    int min_y = static_cast<int>(std::min(corners[0].y, corners[1].y));

    int offset_x = -std::min(0, min_x);
    int offset_y = -std::min(0, min_y);

    Size pano_size(std::max(max_x, img2.cols) + offset_x, std::max(max_y, img2.rows) + offset_y);

    H.convertTo(H, CV_32FC1);
    Mat offset = Mat::eye(3, 3, CV_32F);
    offset.at<float>(0, 2) = offset_x;
    offset.at<float>(1, 2) = offset_y;

    Mat trans = offset * H;
    Mat left, right;

    warpPerspective(img1, left, trans, pano_size);
    warpPerspective(img2, right, offset, pano_size);

    Mat  mask_l, mask_r;

    threshold(left, mask_l, 0.001, 1.0, CV_THRESH_BINARY);
    threshold(right, mask_r, 0.001, 1.0, CV_THRESH_BINARY);

    add(right, left, panorama);
    add(mask_r, mask_l, mask_l);
    divide(panorama, mask_l, panorama);

}

float DistanceBetweenDescriptors(Mat &m1, Mat &m2) {    //distance betweend two descriptor  using Euclidean distance
    float d = 0.0f;
    for (unsigned int i = 0; i < 32; ++i) {
        d += (m1.at<uchar>(0,i) - m2.at<uchar>(0,i)) * (m1.at<uchar>(0,i) - m2.at<uchar>(0,i));
    }
    return cvSqrt(d);
}

/**
 * Aufgabe: Features (10 Punkte)
 *
 * Featuredeskriptoren finden und beschreiben lokale Merkmale in Bildern. Diese Merkmale
 * sind markant und können idealerweise in anderen Bildern der selben Szene wiedergefunden werden.
 *
 * In der Vorlesung betrachten wir den SIFT-Algorithmus. SIFT ist durch ein US-Patenent geschützt und daher nur
 * in der OpenCV Extension \code{nonfree} enthalten. Diese ist im CIP-Pool nicht installiert. OpenCV bietet jedoch eine
 * Reihe weiterer Featuredeskriptoren. Mit ORB stellt OpenCV eine eigens entwickelte freie Alternative zu SIFT zur
 * Verfügung.
 *
 * - Berechne mit einem Featuredeskriptor deiner Wahl Featurepunkte der Testbilder.
 * - Zeichne die Features in die Testbilder ein.
 * - Implementiere das Finden von Korrespondenzen zwischen Paaren von Testbildern (ohne Verwendungung einer
 *   OpenCV \code{DescriptorMatcher} Klasse).
 *   Aus der Vorlesung wissen wir, dass ein Match nur akzeptiert werden soll, wenn die beste Deskriptordistanz
 *   kleiner ist als das 0.6-fache der zweitbesten Deskriptordistanz. Implementiere diese Prüfung. Was passiert,
 *   wenn der Faktor auf 0.4 bzw. 0.8 gesetzt wird?
 * - Visualisiere die Korrespondenzen in geeigneter Form.
 */

// Matches a descriptor desc against a list of descriptors desclist with an acceptance ratio ratio.
// Returns the id of the match in featlist or -1
int matchDescriptors(const Mat &desc, const Mat &desclist, float ratio) {
/* TODO */
	float first_distance = FLT_MAX;  //firstly the distance is infinite
	float second_distance = FLT_MAX;
	unsigned int first_index = -1;
	unsigned int second_index = -1;
	float distance = 0.0f;
	for(unsigned int i = 0; i < desclist.rows; i++){
		distance = DistanceBetweenDescriptors(desc.rowRange(0,1),desclist.rowRange(i,i+1));  //a line each time, 32 dimension vector, a descriptor
		if(distance < first_distance){        //determine the first distance and the index
			second_distance = first_distance;
			second_index = first_index;
			first_distance = distance;
			first_index = i;
		}
		else if(distance < second_distance){  //determine the second distance and the index
			second_distance = distance;
			second_index = i;
			}
		}
	return (first_distance < ratio * second_distance)? first_index : -1;// if the distance meet the ratio
}

// Finds matches between two lists of descriptors desclist1 and desclist2 with an acceptance ratio ratio.
// Returns a pairs of ids ptpairs.
void findMatches(const Mat &desclist1, Mat &desclist2,
                 vector<std::pair<unsigned int, unsigned int>> &ptpairs, float ratio) {
/* TODO */	
					 unsigned int list2_index = -1;
					 for(unsigned int i = 0; i < desclist1.rows; i++){
						 list2_index = matchDescriptors(desclist1.rowRange(i,i+1),desclist2,ratio);
						 if(list2_index != -1){
						 ptpairs.push_back(make_pair<unsigned int,unsigned int>(i,list2_index));//first is the index in Keypoints1, second Keypoints2
						 }
					 }
}

/**
 * Aufgabe: RANSAC (10 Punkte)
 *
 * Tatsächlich liefern selbst sehr gute Matching-Algorithmen stets einige
 * Korrespondenzen, die nicht denselben 3D Punkt beschreiben. Homographien, die
 * auf diesen Korrespondenzen beruhen, liefern nicht die gewünschten
 * Transformationen. Der RANSAC-Algorithmus versucht, aus der Gesamtmenge der
 * Korrespondenzen diejenigen herauszusuchen, die zueinander konsistent sind.
 *
 * - Implementiere selbständig den RANSAC-Algorithmus. Verwende als Qualitätsmaß den
 *   Abstand zwischen den Featurepunkten des einen Bildes und den
 *   transformierten Featurepunkten des anderen Bildes: Abweichungen von mehr
 *   als vier Pixeln in einer Koordinate kennzeichnen einen Punkt als
 *   Outlier. Bei gleicher Anzahl konsistenter Korrespondenzen entscheidet
 *   die Gesamtabweichung aller gültigen Korrespondenzen.
 */

void findHomographyRANSAC(const unsigned int &ransac_iterations,
                          const vector<KeyPoint> &keypoints1, const vector<KeyPoint> &keypoints2,
                          const vector<std::pair<unsigned int, unsigned int>> &matches, Mat &H) {

/* TODO */

    /**
     * - Berechne mit allen von dir bestimmmten gültigen Korrespondenzen eine Homographie zwischen den Bildern.
     * - Stelle mit dieser Homographie ein Panorama her.
     */
		unsigned int iter = 0;
		vector<unsigned int> best_match;
		vector<unsigned int> inliers;
		unsigned int inlier_index = 0;
		vector<Point2f> PointSet1;
		vector<Point2f> PointSet2;
		RNG rng; 
		//srand((unsigned)time(NULL));
		while(iter < ransac_iterations){   //Number of iterations is const
			vector<unsigned int> random_samples;
			while(random_samples.size() < 4) { ////used to generate  4 random coresspodence
				random_samples.push_back(rng.uniform(0, matches.size()));
				//random_samples.push_back(random(0,matches.size()));
				}
			for(auto it = random_samples.begin(); it != random_samples.end(); ++it) {
				cout<<"random"<<*it<<endl;
			}
			
			
			for(auto it = random_samples.begin(); it != random_samples.end(); ++it) {
				PointSet1.push_back(keypoints1[matches[*it].first].pt);
				PointSet2.push_back(keypoints2[matches[*it].second].pt);
			}
			Mat H_temp = getPerspectiveTransform(PointSet1, PointSet2); //get the homography
			PointSet1.clear();
			PointSet2.clear();
			cout<<"H_temp"<<H_temp<<endl;

			//Mat H_inv = H.inv();
			random_samples.clear(); //  empty set

			
			vector<Point2f> point1,point2, points_trans_1to2; 

			for( auto it = matches.begin(); it != matches.end(); ++it) {
				point1.push_back(keypoints1[it->first].pt);
				point2.push_back(keypoints2[it->second].pt);

				perspectiveTransform(point1, points_trans_1to2, H_temp); //get the tranformed point form picture1 to picture2

				float d1to2_dx = point2[0].x - points_trans_1to2[0].x;  //distance between transformened point and oiginal point in picture 2 in x axis
			    float d1to2_dy = point2[0].y - points_trans_1to2[0].y; // in y axis

				point1.clear();
			    point2.clear();
			    points_trans_1to2.clear();

				if((-4.0f <= d1to2_dx <= 4.0f) && (-4.0f <= d1to2_dy <= 4.0f)) {  //requirement in aufgabe
					inliers.push_back(inlier_index);
				}
				++inlier_index;
			}
			inlier_index = 0;

			if(inliers.size() > best_match.size()){   //if the vector have the most pair meet the requirement, count it as the best match vector
				best_match.clear();
				best_match = inliers;
			}
		    inliers.clear();
			++iter;
}
		cout<<best_match.size()<<endl;
		vector<Point2f> src, dst;
		for(auto it = best_match.begin(); it != best_match.end(); ++it) {
			src.push_back(keypoints1[matches[*it].first].pt);
			dst.push_back(keypoints2[matches[*it].second].pt);
		}
			
		H = findHomography(src,dst);//??????can i use this？
		//H = getPerspectiveTransform(src,dst);
}


int main(int argc, char **argv) {
    Mat img1, img2;

    Mat img1_U8 = imread("left.png", IMREAD_COLOR);
    Mat img2_U8 = imread("right.png", IMREAD_COLOR);

    //float ratio = atof(argv[3]);

    img1_U8.convertTo(img1, CV_32F, 1 / 255.);
    img2_U8.convertTo(img2, CV_32F, 1 / 255.);

   // imshow("Input Image left", img1);
   // imshow("Input Image right", img2);
   // waitKey(0);

/* TODO */

    /**
     * \textit{Hinweis: Die OpenCV High-Level \code{Sticher}-Klasse ist \textbf{nicht} hilfreich bei der Bearbeitung
     * der Aufgaben. Für ein fertiges Panorama ohne Bearbeitung aller Aufgaben gibt es keine Punkte.}
     */
	
	//ORB orb;
	//vector<KeyPoint> keyPoints_1, keyPoints_2;
	//Mat descriptors_1, descriptors_2;

	//orb(img1, Mat(), keyPoints_1, descriptors_1);
	//orb(img2, Mat(), keyPoints_2, descriptors_2);

	img1.convertTo(img1,CV_8U,255,0); //must be in te rang 0 -255, not 0-1!!!!!!!!!!
	img2.convertTo(img2,CV_8U,255,0);

	vector<KeyPoint> keyPoints_1, keyPoints_2; 
	
	//Ptr<ORB> orb = ORB::create(100, 2, 8, 31, 0, 2, ORB::HARRIS_SCORE, 31, 20); 
    //Ptr<ORB> orb = ORB::create();
	ORB orb;
	orb.detect(img1,keyPoints_1); 
	orb.detect(img2,keyPoints_2);
    //orb.detect(img2, keyPoints_2);

	/*for (vector<KeyPoint>::iterator it = keyPoints_1.begin() ; it != keyPoints_1.end(); ++it)
    {
		circle(img1, it->pt, 3, Scalar(0, 0, 255));
    }
	
	for (vector<KeyPoint>::iterator it = keyPoints_2.begin() ; it != keyPoints_2.end(); ++it)
    {
		circle(img2, it->pt, 3, Scalar(0, 0, 255));
    }
	
	imshow("Input Image left", img1);
    imshow("Input Image right", img2);
    waitKey(0);*/


	Mat descriptors_1, descriptors_2; 
    orb.compute(img1, keyPoints_1, descriptors_1); 
    orb.compute(img2, keyPoints_2, descriptors_2);   //using ORB to get the ketpoints and descriptors

//////////////////////////////////////////////nur test
	cout<<descriptors_1.rows<<endl<<descriptors_1.cols<<endl;   //500feature * 32 dimension
	cout<<descriptors_1.rowRange(0,1)<<endl;
	float d1 = DistanceBetweenDescriptors(descriptors_1.rowRange(0,1),descriptors_1.rowRange(1,2));
	cout<<"d1"<<d1<<endl;
////////////////////////////////////////////////////////////////////////////////////////////

	float ratio = 0.4f;
	Mat M_Translation = (Mat_<float>(3, 3) << 1, 0, img1.cols, 0, 1, 0, 0, 0, 1);
    Mat output;
    createPanorama(img2,img1,M_Translation,output);
	
	vector<std::pair<unsigned int, unsigned int>> ptpairs;
	findMatches(descriptors_1,descriptors_2,ptpairs,ratio);

	CvPoint r1,r2;
	for (vector<pair<unsigned int, unsigned int>>::iterator it = ptpairs.begin() ; it != ptpairs.end(); ++it)
    {
		//if(it->second != -1){
		circle(img1, keyPoints_1[it->first].pt, 3, Scalar(0, 0, 255));
		circle(img2, keyPoints_2[it->second].pt, 3, Scalar(0, 0, 255));
		r1.x=keyPoints_1[it->first].pt.x;
		r1.y=keyPoints_1[it->first].pt.y;
		r2.x=keyPoints_2[it->second].pt.x+img1.cols;
		r2.y=keyPoints_2[it->second].pt.y;
		line( output, cvPoint(r1.x, r1.y),cvPoint(r2.x, r2.y ), Scalar(255,0,0),0.5 );  
		//}
    }
	cout<<"size"<<ptpairs.size()<<endl;
	imshow("Input Image left", img1);
    imshow("Input Image right", img2);
	imshow("111",output);
	waitKey(0);

	///////////////////////////////////RANSCA//////////////////////////

	Mat H(3,3,CV_32FC1);
	findHomographyRANSAC(50,keyPoints_1,keyPoints_2,ptpairs,H);
	Mat output_2;
    createPanorama(img1,img2,H,output_2);
	imshow("panoram",output_2);
	waitKey(0);


}
