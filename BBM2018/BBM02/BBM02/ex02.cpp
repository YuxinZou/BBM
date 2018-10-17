/**
 * Bildbasierte Modellierung SS 2018
 * Prof. Dr.-Ing. Marcus Magnor
 *
 * Betreuer: JP Tauscher (tauscher@cg.cs.tu-bs.de)
 * URL: https://graphics.tu-bs.de/teaching/ss18/bbm
 */

#include <iostream>
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

using namespace cv;
using namespace std;

/**
 * Datum: 17.4.2018
 *
 * Übungsblatt: 2
 * Abgabe: 24.4.2018
 */

int main(int argc, char **argv) {

    //if (argc < 2) {
    //    std::cerr << "usage: " << argv[0] << " <image>" << std::endl;
    //    exit(1);
    //}

    /**
     * Aufgabe: 2D-Operationen auf Bildern (5 Punkte)
     *
     * In der Vorlesung wurde gezeigt, das große Bereiche natürlicher Bilder
     * homogen sind, die meisten Informationen über ein Bild jedoch in den
     * Kanten und Diskontinuitäten zu finden sind. In disem Übungsblatt soll
     * es darum gehen, Kanten und herausragende Punkte in Bildern zu finden.
     *
     * Häufige Grundlage der Verarbeitung von Bildern ist das Anwenden von
     * Filtern.  Es entspricht der Faltung (engl. \emph{convolution}) des
     * Bildes mit einem Filterkern. Filterkerne werden zu sehr verschiedenen
     * Zwecken eingesetzt.
     *
     * - Skizziere (auf Papier) eine eindimensionale Gaußfunktion mit
     *   Mittelwert $\mu$ und Varianz $\sigma^2$.  Was ist die
     *   Fourier-Transformation einer Gaußfunktion?
     * - Lade ein Testbild und wandle es anschließend in ein Grauwertbild mit
     *   float-Werten in $[0, 1]$ um.
     */
	Mat image = imread("lena.png", IMREAD_COLOR);
	Mat image_gray = imread("lena.png", IMREAD_GRAYSCALE ); //读取出来为8位，类型为CV_8UC1

    image_gray.convertTo(image_gray, CV_32FC1, 1/255.);

	imshow("color", image);
    imshow("gray", image_gray);
	waitKey();
	
/* TODO */

    /**
     * - Falte ein verrauschtes Testbild mit Gaußfunktionen verschiedener
     *   Varianzen. Was passiert? Welchen Einfluss hat die Kernelgröße?
     */
	Mat image_gaussian = imread("lena_gaussian.png");
	image_gaussian.convertTo(image_gaussian, CV_32FC1, 1/255.);
	Mat gauss_small, gauss_large;
    int Kernel_size_small = 3;
	int Kernel_size_large = 5;
    float sigma_small = 3;
    float sigma_large = 5;

    GaussianBlur(image_gaussian, gauss_small, Size(Kernel_size_small, Kernel_size_small),sigma_small);
    GaussianBlur(image_gaussian, gauss_large, Size(Kernel_size_large, Kernel_size_large),sigma_large);

	imshow("gaussian", image_gaussian);
    imshow("gaussian_large", gauss_small);
    imshow("gaussian_small", gauss_large);
    waitKey(0);

/* TODO */

    /**
     * - Betrachte die Differenzen zweier gaußgefilterter Bilder (evt.
     *   skalieren). Wie sind die Nulldurchgänge zu interpretieren?
     */
	Mat difference;
    absdiff(gauss_small, gauss_large, difference);

    imshow("gauss difference", difference*5);
    waitKey(0);

/* TODO */

    /**
     * Aufgabe: Diskrete Ableitungen (5 Punkte)
     *
     * Mathematisch sind Ableitungen nur für stetige Funktionen definiert.  Da
     * ein Bild nur pixelweise, d.h. an diskreten Stellen, Informationen
     * liefert, kann man Ableitungen von Bildern nicht direkt bestimmen.  Eine
     * naheliegene Approximation der Ableitung ist der Differenzenquotient.
     * Sei $f:\Omega \subset \mathbb{R} \to \mathbb{R}$ eine Funktion.  Dann
     * ist der Differenzenquotient $D_h(x) = \frac{f(x+h) - f(x)}{h}$ eine
     * Approximation an $f'(x)$ für hinreichend kleines h. Für
     * differenzierbare Funktionen liefert allerdings die zentrale Differenz
     * \begin{equation}
     * D(x) = \frac{f(x+h) - f(x-h)}{2h}
     * \end{equation}
     * eine deutlich bessere Approximation (siehe auch \emph{Numerical Recipes
     * in C} von Press et al., Chapter 5).
     *
     * - Bestimme je einen diskreten Faltungskern, der die zentrale Differenz
     *   pro Bildachse approximiert.
     */
	Mat kernel_dx = (Mat_<double>(1, 3) << -1, 0, 1);
	cout<<kernel_dx<<endl;
	Mat kernel_dy = (Mat_<double>(3, 1) << -1, 0, 1);
	cout<<kernel_dy<<endl;
	//waitKey(0);
/* TODO */

    /**
     * - Implementiere diskretes Differenzieren als Faltung und
     *   wende es auf ein glattes Testbild an. Was passiert, wenn du ein
     *   verrauschtes Testbild verwendest?
     */
	Mat dx, dy; 
	filter2D(image_gray, dx, -1, kernel_dx);  //图像卷积运算函数filter2D()
    filter2D(image_gray, dy, -1, kernel_dy);

    imshow("dx", dx);
    imshow("dy", dy);
    waitKey(0);

/* TODO */

    /**
     * - Verwende in der Implementierung nun Faltung mit dem Sobel-Operator
     *   (\code{Sobel}) und beobachte die Ergebnisse auf dem verrauschten
     *   Testbild.
     */
    //Mat image_saltpepper = imread("lena_saltpepper.png");
	//image_saltpepper.convertTo(image_saltpepper, CV_32FC1, 1/255.);
	Mat sobel_dx, sobel_dy;
    Sobel(image_gray, sobel_dx, -1, 1, 0, 3);
    Sobel(image_gray, sobel_dy, -1, 0, 1, 3);
	//dx=1，dy=0，表示计算X方向的导数，检测出的是垂直方向上的边缘；dx=0，dy=1，表示计算Y方向的导数，检测出的是水平方向上的边缘。 
    imshow("sobel_dx", sobel_dx);
    imshow("sobel_dy", sobel_dy);
    waitKey(0);
/* TODO */

    /**
     * Aufgabe: Features (10 Punkte)
     *
     * Kanten in Bildern werden häufig als Intensitätssprünge beschrieben.
     *
     * - Berechne den Betrag des Gradienten eines Testbildes und bestimme
     *   Schwellwerte des Gradienten, um möglichst alle Kanten zu entdecken
     *   oder möglichst nur stark ausgeprägte Kanten zu entdecken.
     */
	Mat gradient;
	Mat dx_2,dy_2;
    multiply(dx, dx, dx_2,1,-1);
    multiply(dy, dy, dy_2,1,-1);
    gradient = dx_2 + dy_2;
    sqrt(gradient, gradient);

    imshow("gradient", gradient);
    waitKey(0);

	Mat gradient_2;
	Mat kernel_gradient = (Mat_<double>(3, 3) << 0, -1, 0, -1, 0, 1, 0,1,0);
	cout<<kernel_gradient<<endl;
	filter2D(image_gray, gradient_2, -1, kernel_gradient); 
	imshow("gradient_2", gradient_2);
    waitKey(0);

    Mat Kanten_alle, kanten_stark;
    float threshold1 = 0.1;
    float threshold2 = 0.3;

    threshold(gradient, Kanten_alle, threshold1, 1, THRESH_BINARY);
    threshold(gradient, kanten_stark, threshold2, 1, THRESH_BINARY);

    imshow("Kanten_alle", Kanten_alle);
    imshow("kanten_stark", kanten_stark);
    waitKey(0);

	/*double cv::threshold(  
    cv::InputArray src, // 输入图像  
    cv::OutputArray dst, // 输出图像  
    double thresh, // 阈值  
    double maxValue, // 向上最大值  
    int thresholdType // 阈值化操作的类型   
);  */


/* TODO */

    /*
     * - Vergleiche mit dem Ergebnis des Canny-Kantendetektors
     *   (\code{Canny}), wenn er mit diesen Parametern aufgerufen wird.
     */
	Mat grayImage;
	cvtColor(image, grayImage, COLOR_BGR2GRAY);
	Mat canny;
	int threshold3 = 50;
	int threshold4 =150;
    Canny(grayImage, canny, threshold3, threshold4);

    imshow("canny", canny);
    waitKey(0);


	/*Canny( detected_edges, detected_edges, lowThreshold, lowThreshold*ratio, kernel_size );
输入参数:

detected_edges: 原灰度图像
detected_edges: 输出图像 (支持原地计算，可为输入图像)
lowThreshold: 用户通过 trackbar设定的值。
highThreshold: 设定为低阈值的3倍 (根据Canny算法的推荐)
kernel_size: 设定为 3 (Sobel内核大小，内部使用)*/

/* TODO */

    /**
     * Einzelne herausragende Punkte werden auch als Featurepunkte oder Ecken
     * bezeichnet, selbst wenn sie nicht auf einer Kante liegen.
     *
     * - Implementiere die Harris-Corner Detektion. Verwende dabei nicht die
     *   OpenCV Methode \code{cornerHarris}, sondern implementiere selbst eine
     *   Funktion, die ein Grauwertbild, einen Parameter $k$ für die Berechnung
     *   des Featureindikators und einen Schwellwert $t$ für ausreichend großen
     *   Ausschlag des Indikators entgegennimmt und die Featurepunkte
     *   zurückgibt.
     */
	/*vector<Point2i> harris_point;
	Mat dxy, dxx, dyy;
    

    multiply(dx, dy, dxy);
    multiply(dx, dx, dxx);
    multiply(dy, dy, dyy);

	int w = 3;
	Mat harris_kernel;
    harris_kernel = Mat::ones(w, w, CV_32F);
    filter2D(dxx, dxx, CV_32F, harris_kernel);
    filter2D(dyy, dyy, CV_32F, harris_kernel);
    filter2D(dxy, dxy, CV_32F, harris_kernel);

	//R = det M − k(traceM)2
    float k = 0.04;
    Mat temp1,temp2,temp3;
	Mat R;

    temp1 = dxx + dyy;
    multiply(temp1, temp1, temp1);

    multiply(dxx, dyy, temp2);
    multiply(dxy, dxy, temp3);
    R = temp2 - temp3 - k * temp1;

	float threshold_harris = 1111; 
    for (int i = 1; i < R.rows - 1; i++) {
        for (int j = 1; j < R.cols - 1; j++) {
            float r_value = R.at<float>(i, j);
            if (r_value > threshold_harris)
            {
                if ((r_value >= R.at<float>(i - 1, j - 1)) && (r_value >= R.at<float>(i - 1, j))
                     && (r_value >= R.at<float>(i - 1, j + 1)) && (r_value >= R.at<float>(i, j - 1))
                     && (r_value >= R.at<float>(i, j + 1) && (r_value >= R.at<float>(i + 1, j - 1))
                     && (r_value >= R.at<float>(i + 1, j)) && (r_value >= R.at<float>(i + 1, j + 1))
                {
                    harris_point.push_back(Point2i(i, j));
                }
            }
        }
    }*/


/* TODO */

    /**
    * - Zeige die Werte des Detektors vor der Segmentierung mit dem Schwellwert $t$ unter Verwendung
    *   einer geigneten Color Map (\code{applyColorMap}) mit geeigneter Skalierung.
    */

/* TODO */

    /**
     * - Zeichne einen Kreis um jede gefundene Harris-Corner.
     */

/* TODO */
return 0;
}


/*//高斯滤波
        //src:输入图像
        //dst:输出图像
        //Size(5,5)模板大小，为奇数
        //x方向方差
        //Y方向方差
        GaussianBlur(src,dst,Size(5,5),0,0);
        imwrite("gauss.jpg",dst);
        
        //中值滤波
        //src:输入图像
        //dst::输出图像
        //模板宽度，为奇数
        medianBlur(src,dst,3);
        imwrite("med.jpg",dst);
        
        //均值滤波
        //src:输入图像
        //dst:输出图像
        //模板大小
        //Point(-1,-1):被平滑点位置，为负值取核中心
        blur(src,dst,Size(3,3),Point(-1,-1));
        imwrite("mean.jpg",dst);

        //双边滤波
        //src:输入图像
        //dst:输入图像
        //滤波模板半径
        //颜色空间标准差
        //坐标空间标准差
        bilateralFilter(src,dst,5,10.0,2.0);//这里滤波没什么效果，不明白
        imwrite("bil.jpg",dst);*/