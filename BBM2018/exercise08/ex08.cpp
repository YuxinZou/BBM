/**
 * Bildbasierte Modellierung SS 2017
 * Prof. Dr.-Ing. Marcus Magnor
 *
 * Betreuer: JP Tauscher (tauscher@cg.cs.tu-bs.de)
 * URL: https://graphics.tu-bs.de/teaching/ss17/bbm
 */

/**
 * Datum: 13.6.2018
 *
 * Übungsblatt: 8
 * Abgabe: 19.6.2018
 */

/**
 * In dieser Aufgabe soll ein Algorithmus implementiert werden, um aus
 * Videodaten eines einfachen Laser\-scanners 3D-Punktwolken zu erzeugen. Der
 * Scanner besteht aus einem Linienlaser, wie sie beispielsweise an
 * Scannerkassen Verwendung finden, einer Kamera und zwei Hintergrundebenen.
 * Der Laser überstreicht das Objekt, das vor den in einem Winkel zueinander
 * stehenden Hintergrundebenen platziert ist. Objekt und Ebenen werden dabei
 * von der Kamera aufgenommen. Die Geometrie der Ebenen wird als bekannt
 * vorausgesetzt; sie kann beispielsweise durch ein optisches
 * Kalibrierverfahren bestimmt werden. Das Licht des Lasers bildet zu jedem
 * Zeitpunkt eine weitere Ebene, die die beiden Hintergrundebenen in jeweils
 * einer Linie schneidet. In Verbindung mit der bekannten Geometrie der Ebenen
 * kann aus diesen Linien die durch den Laser aufgespannte Ebene im
 * Koordinatensystem der Kamera bestimmt werden. Die Koordinaten der Punkte, an
 * denen der Laser das Objekt trifft, können auf diese Ebene projiziert werden,
 * um 3D-Koordinaten zu erhalten. Wird nun der Laser über das Objekt bewegt,
 * kann aus den Einzelbildern jeweils ein Linienprofil des Objekts
 * rekonstruiert werden; zusammen ergeben diese Profile eine 3D-Punktwolke des
 * Objekts.
 *
 * Ein Laserscanner, der nach diesem Prinzip funktioniert, wurde am Institut
 * für Robotik und Prozessinformatik der TU Braunschweig entwickelt. Aufbau und
 * Funktionsweise sind im dem Paper
 * \emph{\href{https://link.springer.com/chapter/10.1007/11861898_72}{Low-Cost
 * Laser Range Scanner and Fast Surface Registration Approach}} erklärt.
 *
 * Unser Programm zur 3D-Rekonstruktion erhält als ersten Parameter den Namen
 * des Eingabevideos, als zweiten den der Ausgabedatei (jede Zeile ein
 * 3D-Punkt, Koordinaten durch Komma getrennt, Endung \code{.asc}). Als dritter
 * Parameter kann optional eine Ganzzahl angegeben werden, die bestimmt, jeder
 * wievielte Frame des Videos rekonstruiert wird; dadurch lässt sich die
 * Laufzeit beliebig auf Kosten der Auflösung reduzieren. Die entstehenden
 * Punktwolken können mit Meshlab gerendert werden.
 */



#include <iostream>
#include <fstream>
#include <cmath>

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#define GLM_SWIZZLE

#include <glm/glm.hpp>
#include <glm/ext.hpp>


using namespace cv;

/**
 * Aufgabe: Vorüberlegungen (10 Punkte)
 *
 * Um für jeden Punkt eine Tiefe berechnen zu können, muss aus den beiden
 * Linien, an denen die Laserebene 10011die Hintergrundebenen schneidet, die
 * Laserebene im Kamerakoordinatensystem rekonstruiert werden. Vereinfachend
 * können wir annehmen, dass die Kamera sehr weit vom Objekt entfernt ist, die
 * Projektion ist also näherungsweise orthographisch. Anstelle einer
 * automatischen Kalibrierung der Hintergrundebenen soll angenommen werden,
 * dass diese senkrecht sowie symmetrisch um die horizontale Bildmitte
 * angeordnet sind und sich dort orthogonal treffen.
 *
 * - Skizziere eine Draufsicht der beiden Hintergrundebenen.
 *
 * - Beschreibe $z$ als Funktion von $x$ jeweils für die linke und rechte
 *   Bildhälfte. Diese Funktion wird später verwendet, um 3D-Koordinaten aus
 *   den 2D-Bildkoordinaten zu berechnen.
 */
int depth(int x){
    int z;
    z = std::abs(x);
    return z;
}
glm::vec3 trans_point3f_to_glmvec3(Point3f &point){
    return glm::vec3(point.x,point.y,point.z);
}

Point3f trans_glmvec3_to_point3f(glm::vec3 &point){
    return Point3f(point.x,point.y,point.z);
}

std::pair<Point2f, Point2f> find_line(Mat &bimg) {
    /**
     * Aufgabe: Kalibrierlinien (5 Punkte)
     *
     * Zunächst müssen im Eingabebild die beiden Kalibrierlinien gefunden
     * werden. Dafür wird das Bild binarisiert und in den linken und rechten
     * Teil separiert. Finde in einem solchen Binärbild die dominanteste Linie
     * und gib zwei Punkte zurück, die auf dieser Linie liegen.
     */
    //4 integers
    vector<Vec4i> lines;
    HoughLinesP( bimg, lines, 1, CV_PI/180, 80, 30, 10 );
    Point2f pt1(0,0);
    Point2f pt2(0,0);
    std::pair<Point2f, Point2f> point;
    if(lines.size() > 0){

        pt1.x = lines[0][0];
        pt1.y = lines[0][1];
        pt2.x = lines[0][2];
        pt2.y = lines[0][3];
        point.first = pt1;
        point.second = pt2;

        //Mat color(bimg.rows,bimg.cols,CV_8UC3);
        //line( color, pt1, pt2, Scalar(0,0,255), 3, 8 );
        //imshow("image",color);
        //waitKey(0);


    }
    else{
       // std::cout<<"can't find any line"<<std::endl;
        point.first = Point2f(0,0);
        point.second = Point2f(0,0);
    }
    //std::cout<<"pt1.x  "<<point.first.x<<"pt1.y  "<<point.first.y<<"pt2.x  "<<point.second.x<<"pt2.y  "<<point.second.y<<std::endl;
    return point;

/* TODO */
}



int main(int argc, char** argv) {
    if (argc < 3 or argc > 4) {
        std::cerr << "Usage: " << argv[0] << " infile.avi outfile.asc [frame_index_skip=1]" << std::endl;
        return 1;
    }

    VideoCapture cap(argv[1]);
    if (!cap.isOpened())
    {
        std::cerr << "Failed to open file: " << argv[1] << std::endl;
        return -1;
    }

    std::ofstream outfile(argv[2]);
    int frame_index_skip = argc > 3 ? atoi(argv[3]) : 1; //define index skip if given...


    /**
     * Aufgabe: Laserscanner (15 Punkte)
     * Führe folgende Schritte für jeden Frame des Videos durch:
     */

    std::ofstream outFile;
    outFile.open (argv[2]);

    int frame_index = 0;
    while(1) {
        Mat img;
        cap >> img;


        if (frame_index % frame_index_skip != 0){
            frame_index++;
            continue;
        }
        if(img.empty()) break; //terminate when no more video frames

        //std::cout << "processing frame " << frame_index << std::endl;

        // convert RGB image to gray
        Mat gimg;
        cvtColor(img, gimg, CV_BGR2GRAY);

        /**
         * - Binarisiere das Bild, so dass nur die Laserlinie erhalten bleibt.
         */
        Mat BiImg;
        threshold(gimg,BiImg,50,255,THRESH_BINARY);
        //imshow("image",BiImg);
        //waitKey(0);
        //std::cout<<"height "<<BiImg.rows<<"width "<<BiImg.cols<<std::endl;
        Rect left_rect(0,0,BiImg.cols/2,BiImg.rows);
        Rect right_rect(BiImg.cols/2,0,BiImg.cols/2,BiImg.rows);
        Mat left_image = BiImg(left_rect);
        Mat right_image = BiImg(right_rect);
        //imshow("image_left",left_image);
        //imshow("right_image",right_image);

       // waitKey(0);


/* TODO */

        /**
         * - Finde die Kalibrierlinie im linken Bild. Berechne die
         *   $z$-Koordinaten gemäß deiner theoretischen Überlegungen.
         */
        std::pair<Point2f, Point2f> left_line = find_line(left_image);
        std::pair<Point2f, Point2f> right_line = find_line(right_image);
        //std::cout<<"pt1.x  "<<left_line.first.x<<"pt1.y  "<<left_line.first.y<<"pt2.x  "<<left_line.second.x<<"pt2.y  "<<left_line.second.y<<std::endl;
        if((left_line.first.x == 0 && left_line.first.y == 0 && left_line.second.x == 0 && left_line.second.y == 0)
            || (right_line.first.x == 0 && right_line.first.y == 0 && right_line.second.x == 0 && right_line.second.y == 0)
           ){
            std::cout<<"cant find any lines"<<std::endl;
            continue;
        }
        /*Point2f aaa(right_line.first.x + BiImg.cols/2 ,right_line.first.y);
        Point2f bbb(right_line.second.x + BiImg.cols/2 ,right_line.second.y);
        Mat color(BiImg.rows,BiImg.cols,CV_8UC3);
        line( color, left_line.first, left_line.second, Scalar(0,0,255), 3, 8 );
        line( color, aaa, bbb, Scalar(0,0,255), 3, 8 );
        imshow("image",color);
        waitKey(0);*/


        Point2f left_p1,left_p2;
        left_p1.x = left_line.first.x - BiImg.cols/2;
        left_p1.y = BiImg.rows/2 - left_line.first.y;
        left_p2.x = left_line.second.x - BiImg.cols/2;
        left_p2.y = BiImg.rows/2 - left_line.second.y;

        // 3d point in left image in camera space
        Point3f left_3d_p1(left_p1.x,left_p1.y,depth(left_p1.x));
       //std::cout<<"left_3d_p1.x "<<left_3d_p1.x<<"left_3d_p1.y "<<left_3d_p1.y<<"left_3d_p1.z "<<left_3d_p1.z<<std::endl;
        Point3f left_3d_p2(left_p2.x,left_p2.y,depth(left_p2.x));
       // std::cout<<"left_3d_p2.x "<<left_3d_p2.x<<"left_3d_p2.y "<<left_3d_p2.y<<"left_3d_p2.z "<<left_3d_p2.z<<std::endl;



/* TODO */

        /**
         * - Finde die Kalibrierlinie im rechten Bild. Berechne die
         *   $z$-Koordinaten gemäß deiner theoretischen Überlegungen. Beachte
         *   dabei, dass die von \code{find\_line} zurückgegebenen Koordinaten
         *   relativ zum Ursprung der rechten Bildhälfte sind.
         */

        Point2f right_p1,right_p2;
        right_p1.x = right_line.first.x;
        right_p1.y = BiImg.rows/2 - right_line.first.y;
        right_p2.x = right_line.second.x;
        right_p2.y = BiImg.rows/2 - right_line.second.y;
        //3d point in the right image in camera space
        Point3f right_3d_p1(right_p1.x,right_p1.y,depth(right_p1.x));
        //std::cout<<"right_3d_p1.x "<<right_3d_p1.x<<"right_3d_p1.y "<<right_3d_p1.y<<"right_3d_p1.z "<<right_3d_p1.z<<std::endl;
        Point3f right_3d_p2(right_p2.x,right_p2.y,depth(right_p2.x));
        //std::cout<<"right_3d_p2.x "<<right_3d_p2.x<<"right_3d_p2.y "<<right_3d_p2.y<<"right_3d_p2.z "<<right_3d_p2.z<<std::endl;
/* TODO */


        /**
         * - Finde eine Ebene, die durch beide Kalibrierlinien geht. Für jeden
         *   Punkt $\vec{x}$ auf der Ebene soll gelten: $\vec{n} \cdot \vec{x}
         *   = d$.
         */
        //the four point are all in the plane, so we just need to comoute the normal of the plane with the 4 point
        Point3f n = (left_3d_p2 - left_3d_p1).cross(left_3d_p1 - right_3d_p1);


        //glm::vec3 normal = trans_point3f_to_glmvec3(n);
        //normal = glm::normalize(normal);

        n = 1.0 / (sqrt(n.x * n.x + n.y * n.y + n.z * n.z)) * n;
        //std::cout<<"n.x "<<n.x<<"n.y "<<n.y<<"n.z "<<n.z<<std::endl;

/* TODO */

        /**
         * - Projiziere jeden hellen Punkt des Binärbildes auf die Ebene und
         *   schreibe die 3D-Koordinaten in der Form $x, y, z$ nach
         *   \code{outfile}. Da im OpenCV-Koordinatensystem $y$ nach unten hin
         *   wächst, solltest du die $y$-Koordinate invertieren, um die
         *   korrekte Darstellung im Renderer zu erreichen. Abhängig von der
         *   Wahl deines Koordinatensystems musst du möglicherweise auch $z$
         *   invertieren.
         */
        for(int x = 0; x < BiImg.cols  ; x++){
            for(int y = 0; y < BiImg.rows; y++){
                if(BiImg.at<uchar>(x,y) == 255){
                    Point3f p0(x-BiImg.cols/2 , BiImg.rows/2 - y , 0);
                    Point3f dire(0,0,1); // to the z axis positive
                    float Z = ((left_3d_p1 - p0).dot(n)) / (dire.dot(n));
                    //std::cout<< "Z "<<Z<<std::endl;
                    Point3f p(x-BiImg.cols/2 , BiImg.rows/2 - y , Z);
                    outFile<<p.x<<", "<<p.y<<", "<<p.z<<std::endl;
                }
            }
        }

/* TODO */

        /**
         * - Stelle dein Ergebnis mit Meshlab dar. Nutze nicht die Grid
         *   Triangulation und verwende einen geeigneten Shader.
         *   Entspricht das Ergebnis deinen Erwartungen? Warum?
         */

        frame_index++;
    }

    outFile.close();
    std::cout << "File Saved :) \n" << std::endl;

    return 0;
}
