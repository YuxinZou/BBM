/**
 * Bildbasierte Modellierung SS 2017
 * Prof. Dr.-Ing. Marcus Magnor
 *
 * Betreuer: JP Tauscher (tauscher@cg.cs.tu-bs.de)
 * URL: https://graphics.tu-bs.de/teaching/ss17/bbm
 */

#include <iostream>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace cv;
using namespace std;

/**
 * Datum: 8.5.2018
 *
 * Übungsblatt: 4
 * Abgabe: 15.5.2018
 */



int main(int argc, char **argv) {

    /*if (argc < 3) {
        std::cerr << "usage: " << argv[0] << " <image_left> " << "<image_right>" << std::endl;
        exit(1);
    }*/

    Mat img1, img2;

    Mat img1_U8 = imread("left.png");
    /*if (img1_U8.empty()) {
        std::cerr << "Could not load image file: " << argv[1] << std::endl;
        exit(1);
    }*/
    img1_U8.convertTo(img1, CV_32F, 1 / 255.);


    Mat img2_U8 = imread("right.png");
    /*if (img2_U8.empty()) {
        std::cerr << "Could not load image file: " << argv[2] << std::endl;
        exit(1);
    }*/
    img2_U8.convertTo(img2, CV_32F, 1 / 255.);

    imshow("Input Image left", img1);
    imshow("Input Image right", img2);

    waitKey(0);

    /**
    * Aufgabe: Homographien (5 Punkte)
    *
    * Unter der Annahme, dass Bilder mit einer verzerrungsfreien Lochbildkamera
    * aufgenommen werden, kann man Aufnahmen mit verschiedenen Bildebenen und
    * gleichem Projektionszentren durch projektive Abbildungen, sogenannte
    * Homographien, beschreiben.
    *
    * - Schreibe eine Translation als Homographie auf (auf Papier!).
    * - Verschiebe die Bildebene eines Testbildes um 20 Pixel nach rechts, ohne
    *   das Projektionszentrum zu ändern. Benutze dafür \code{warpPerspective}.
    * - Wieviele Punktkorrespondenzen benötigt man mindestens, um eine projektive
    *   Abbildung zwischen zwei Bildern bis auf eine Skalierung eindeutig zu
    *   bestimmen? Warum? (Schriftlich beantworten!)

    */
	
    Mat image_translation;
	Mat M_Translation = (Mat_<float>(3, 3) << 1, 0, 20, 0, 1, 0, 0, 0, 1);
	cout<<M_Translation<<endl;

    warpPerspective(img1, image_translation, M_Translation, img1.size());

    imshow("Image Translation", image_translation);
	waitKey(0);

/* TODO */

    /**
    * Aufgabe: Panorama (15 Punkte)
    *
    * Ziel dieser Aufgabe ist es, aus zwei gegebenen Bildern ein Panorama zu konstruieren.
    * Dafür muss zunächst aus den gegeben Punktkorrespondenzen:
    *
    * \begin{center}
    * \begin{tabular}{|c|c|}
    * \hline
    * linkes Bild & rechtes Bild \\
    * $(x, y)$ & $(x, y)$ \\ \hline \hline
    * (463, 164) & (225, 179)\\ \hline
    * (530, 357) & (294, 370)\\ \hline
    * (618, 357) &(379, 367)\\ \hline
    * (610, 153) & (369, 168)\\ \hline
    * \end{tabular}
    * \end{center}
    *
    * eine perspektivische Transformation bestimmt werden, mit der die Bilder auf eine
    * gemeinsame Bildebene transformiert werden können.
    *
    * - Berechne die Transformation aus den gegebenen Punktkorrespondenzen.
    *   Benutze die Funktion \code{getPerspectiveTransform}. Welche Dimension
    *   hat der Rückgabewert der Funktion? Warum?
    */
	    vector<Point2f> links(4); 
		links[0] = Point2f(463, 164);
        links[1] = Point2f(530, 357);
        links[2] = Point2f(618, 357);
        links[3] = Point2f(610, 153);  

        vector<Point2f> recht(4);		
        recht[0] = Point2f(225, 179);
        recht[1] = Point2f(294, 370);
        recht[2] = Point2f(379, 367);
        recht[3] = Point2f(369, 168);

        Mat H_Transformation = getPerspectiveTransform(links,recht );
        cout <<"H_Transformation"<<H_Transformation << endl;
		cout<<H_Transformation.channels()<<endl;

/* TODO */

    /**
    * - Bestimme die notwendige Bildgröße für das Panoramabild.
    */
		// width = cols height = rows
		vector<Point2f> Ecke(4); 
		vector<Point2f> Ecks_trans(4);
		Ecke[0] = Point2f(0.f, 0.f); // left top
		Ecke[1] = Point2f(0.f, (float)(img1.rows -1)); //right top
		Ecke[2] = Point2f((float)(img1.cols - 1), 0.f); //left bottom
        Ecke[3] = Point2f((float)(img1.cols - 1), (float)(img1.rows -1)); //right bottom
		perspectiveTransform(Ecke, Ecks_trans, H_Transformation);
		cout<<Ecke[0]<<" "<<Ecke[1]<<" "<<Ecke[2]<<" "<<Ecke[3]<<" "<<endl;
		cout<<Ecks_trans<<endl;

		float min_x = min(Ecks_trans[0].x , Ecks_trans[1].x); //left
		float max_x = max(Ecks_trans[2].x , Ecks_trans[3].x); //right
		float min_y = min(Ecks_trans[0].y , Ecks_trans[2].y); //top
		float max_y = max(Ecks_trans[1].y , Ecks_trans[3].y); //bottom

		cout<<min_x<<" "<<max_x<<" "<<min_y<<" "<<max_y<<" "<<endl;

		float offset_x_left = (min_x < 0.f)? abs(min_x) : 0.f;
		float offset_x_right = (min_x > (float)(img2.cols - 1))? min_x - (float)(img2.cols - 1) : 0.f;
		float offset_y_top = (min_y < 0.f)? abs(min_y) : 0.f;
		float offset_y_bottom = (max_y > (float)(img2.rows - 1))? max_y - (float)(img2.rows - 1) : 0.f;
		
		cout<<offset_x_left<<" "<<offset_x_right<<" "<<offset_y_top<<" "<<offset_y_bottom<<" "<<endl;
		int width = img2.cols + cvCeil(offset_x_left + offset_x_right);
		int height = img2.rows + cvCeil(offset_y_top + offset_y_bottom);

		cout<<"width"<<width<<endl;
		cout<<"height"<<height<<endl;

		Mat Panoramabild(height,width,CV_32FC3);
		waitKey(0);
/* TODO */

    /**
    * - Projiziere das linke Bild in die Bildebene des rechten Bildes. Beachte
    *   dabei, dass auch der linke Bildrand in das Panoramabild projiziert
    *   wird.
    */  H_Transformation.convertTo(H_Transformation, CV_32FC1);
		//Mat offset_Translation1 = (Mat_<float>(3, 3) << 1, 0, - Ecks_trans[0].x, 0, 1, - Ecks_trans[0].y , 0, 0, 1);
		//cout<<offset_Translation1<<endl;
		Mat offset_Translation = (Mat_<float>(3, 3) << 1, 0, offset_x_left, 0, 1, offset_y_top, 0, 0, 1);
		Mat left, right;
    warpPerspective(img1, left,offset_Translation * H_Transformation , Panoramabild.size());
    warpPerspective(img2, right, offset_Translation , Panoramabild.size());

	imshow("left image", left);
	imshow("right image", right);

	waitKey(0);
/* TODO */

    /**
    * - Bilde das Panoramabild so, dass Pixel, für die zwei Werte vorhanden sind,
    *   ihren Mittelwert zugeordnet bekommen.
    */

	    for (int x=0; x < Panoramabild.cols; x++) {
			for (int y=0; y < Panoramabild.rows; y++) {
				if ((left.at<Vec3f>(y,x)[0] != 0) && (left.at<Vec3f>(y,x)[1] != 0) && (left.at<Vec3f>(y,x)[2] != 0)) {
					if ((right.at<Vec3f>(y,x)[0] != 0) && (right.at<Vec3f>(y,x)[1] != 0) && (right.at<Vec3f>(y,x)[2] != 0)) {
						Panoramabild.at<Vec3f>(y,x) = (left.at<Vec3f>(y,x)+ right.at<Vec3f>(y,x))/2.f;
					} 
					else {
						Panoramabild.at<Vec3f>(y,x) = left.at<Vec3f>(y,x);
					}
            
				} 
				else {
					Panoramabild.at<Vec3f>(y,x) = right.at<Vec3f>(y,x);
            }
        }
    }
		imshow("Panoramabild",Panoramabild);
/* TODO */

    /**
    * - Zeige das Panoramabild an.
    */

/* TODO */

    /**
     * \textit{Hinweis: Die OpenCV High-Level \code{Sticher}-Klasse ist \textbf{nicht} hilfreich bei der Bearbeitung
     * der Aufgaben. Für ein fertiges Panorama ohne Bearbeitung aller Aufgaben gibt es keine Punkte.}
     */
		waitKey(0);
}
