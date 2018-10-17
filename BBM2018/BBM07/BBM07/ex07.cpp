/**
 * Bildbasierte Modellierung SS 2017
 * Prof. Dr.-Ing. Marcus Magnor
 *
 * Betreuer: JP Tauscher (tauscher@cg.cs.tu-bs.de)
 * URL: https://graphics.tu-bs.de/teaching/ss17/bbm
 */

/**
 * Datum: 5.6.2018
 *
 * Übungsblatt: 7
 * Abgabe: 12.6.2018
 */

#include <iostream>
#include <iomanip>
#include <opencv2/highgui/highgui.hpp>

#include <cmath>
#include <set>
#include <queue>
#include <cassert>
#include <fstream>
#include <sstream>
#include <vector>
#include <cfloat>

#define GLM_SWIZZLE

#include <glm/glm.hpp>
#include <glm/ext.hpp>


using namespace cv;

/**
 * Das Ziel dieses Übungsblatts ist die Implementierung des
 * \emph{Space-Carving}-Algorithmus, um aus Fotografien eines Objekts eine
 * Volumendarstellung zu erhalten. Der Algorithmus wurde ursprünglich von
 * Kutulakos und Seitz in \emph{A Theory of Shape by Space Carving}
 * veröffentlicht.
 *
 * Zwei Beispielszenen, \emph{scene1} und \emph{scene2}, sind als Archive angefügt
 * und müssen zunächst entpackt werden. Das Programm kann
 * mit dem Aufruf \code{./ex07 <scenename> <outfile> <color\_threshold>
 * <grid\_resolution>} gestartet werden, wobei \code{<scenename>} der Name der
 * Szene, also \emph{scene1} oder \emph{scene2}, \code{<outfile>} eine .obj-Datei
 * für den Output, \code{<color\_threshold>} ein Schwellwert zwischen $0$ und $255$
 * und \code{<grid\_resolution>} die Auflösung des Voxelgitters ist.
 * Der Befehl \code{./ex07 scene1 out.obj 100 50}
 * lädt also alle benötigten Daten aus dem Ordner \emph{scene1}, benutzt einen
 * Farb-Schwellwert von $100$, arbeitet auf einem $50\times50\times50$
 * Voxelgitter und speichert das Ergebnis in out.obj. Die .obj-Datei kann mit allen
 * gängigen Mesh-Viewern geöffnet werden, die farbige Vertices unterstützen.
 * Im CIP-Pool ist das Program \emph{MeshLab} installiert.
 * Zu Anfang sollte ein schwarzer Würfel und kleine blaue Würfel an den Kamerapositionen
 * angezeigt werden. Die Datei ist zu Anfang recht groß und läd unter Umständen einen Moment.
 *
 * \begin{center}
 * \includegraphics[width=0.3\linewidth]{imgConsistent.png}
 * \includegraphics[width=0.3\linewidth]{silConsistent.png}
 * \includegraphics[width=0.3\linewidth]{SpaceCarving.png}
 * \end{center}
 *
 * Von allen möglicherweise sichtbaren Voxeln im Gitter (linkes Bild) gehören
 * nur Voxel zum Objekt, die nicht auf weißen Bildhintergrund projiziert werden
 * (mittleres Bild).  Vergleicht man Farbwerte verschiedener Kameras, lässt
 * sich 3D-Struktur rekonstruieren (rechtes Bild).
 *
 * Die Voxelmittelpunkte werden immer in einen Einheitswürfel von $(-0.5, -0.5,
 * -0.5)$ bis zu $(0.5, 0.5, 0.5)$ gesetzt, d.h. der Mittelpunkt von Voxel
 *  $(0,0,0)$ wird an die Stelle $(-0.5, -0.5, -0.5)$, der Mittelpunkt von
 *  Voxel $(\code{grid\_resolution}-1, \code{grid\_resolution}-1,
 *  \code{grid\_resolution}-1)$ an die Stelle $(0.5, 0.5, 0.5)$ gesetzt.
 *
 * Die eingebundene Bibliothek \code{glm} enthält eine Reihe von hilfreichen Klassen
 * (\code{vec3}, \code{vec4}, \code{mat3}, \code{mat4}), die sehr intuitiv in
 * der Benutzung sind. So kann ein \code{vec4 p} mit einer $4\times4$ Matrix
 * \code{mat4 m} multiplziert werden, indem \code{vec4 mp = m p} aufgerufen
 * wird.
 */


struct voxel {
    glm::vec3 pos, color;
//overloading operator < , only every dimension is bigger, then return true 
    bool operator<(const voxel &v) const {
        for (int i = 0; i < 3; ++i) {
            if (pos[i] < v.pos[i]) return true;
            else if (pos[i] > v.pos[i]) return false;
        }
        return false;
    }
};

typedef std::pair<int, int> pixPos; //definition of pixPos

typedef std::pair<float, int> new_pair_type; //for convenience

bool isImgConsistent(
        const std::vector<pixPos> &pixPositions,
        int &imgRemoval
);

bool isSilhouetteConsistent(
        const std::vector<pixPos> &pixPositions,
        int &silhouetteRemoval
);

bool isColorConsistent_plane(
        const glm::vec4 &voxPosition,
        const std::vector<pixPos> &pixPositions,
        glm::vec3 &outputColor,
        const int axis,
        int &colorRemoval
);

void transform1(Vec3b &vec1,glm::vec3 &vec2);


// global variables
std::string path;
std::string out;

// space carving
int gridRes = 50;

int numInput = 3;
std::vector<int> imgWidth;
std::vector<int> imgHeight;
std::vector<voxel> result;
std::vector<glm::mat4> textureMatrices;
std::vector<glm::mat4> modelView;
std::vector<glm::mat4> projection;
std::vector<Mat> imgs;
std::vector<glm::vec3> camPos;
int diffThresh = 100;


/**
 * Aufgabe: Bildkonsistenz (5 Punkte)
 *
 * Iteriere über alle Voxel, indem du zuerst alle
 * Voxel einer Ebene mit gleicher z-Koordinate betrachtest und dann die Ebene
 * entlang der positiven z-Achse verschiebst.  Benutze \code{textureMatrices},
 * um die Voxelmittelpunkte in die einzelnen Bilder zu projizieren.  Ist die
 * berechnete Position innerhalb der Bildes, behalte sie, ansonsten wird dieser
 * Voxel aus dem Gitter entfernt. Dies führt zu einer Menge von Voxeln, die
 * möglicherweise (abhängig vom Objekt in der Szene) in allen Bildern sichtbar
 * sind. Setzt man alle Voxel, die diese Bedingung erfüllen schwarz, so sollte
 * das Ergebnis aussehen wie im linken Bild.
 */

void consistencyCheck(int x, int y, int z, //coordinates
                      int axis, // current axis 0-5
                      int &imgRemoval, int &silhouetteRemoval, int &colorRemoval, // removal counts
                      std::vector<pixPos> &pixPositions,
                      std::set<voxel> &consistentVoxels,
                      float step) {

/* TODO */
    voxel voxel_temp;

    for (int i = 0; i < axis; i++){
        std::cout<<"i  "<<i<<std::endl;
        switch (i){
        //along the Z positive axis
        case 0:
            for(z = 0 ; z < gridRes; z++){
               // std::cout<<"z"<<z<<std::endl;
                for(y = 0; y < gridRes; y++){
                    for(x = 0;x < gridRes; x++){
                        voxel_temp.pos = glm::vec3(-0.5+step*x,-0.5+step*y,-0.5+step*z);
                        voxel_temp.color = glm::vec3(0.0f,0.0f,0.0f);
                        pixPositions.clear();
                        glm::vec4 pos;
                        glm::vec3 col;
                        for(int j = 0; j < numInput; j++){
                            pos = glm::vec4(voxel_temp.pos,1.0f);
                            glm::vec4 pos_temp = textureMatrices.at(j) * pos_temp;
                            //std::cout<<"matrices"<< textureMatrices.at(j)<<std::endl;
                            glm::vec2 pos_in_img = glm::vec2(pos_temp.x/pos_temp.w,pos_temp.y/pos_temp.w);
                            pixPos pispos;
                            int img_x = round(pos_in_img.x);
                            //std::cout<<"imgx  "<<img_x<<std::endl;
                            int img_y = round(pos_in_img.y);
                            //std::cout<<"imgy  "<<img_y<<std::endl;
                            pispos.first = img_x;
                            pispos.second = img_y;
                            pixPositions.push_back(pispos);
                            //std::cout<<"pix_size"<<pixPositions.size()<<std::endl;

                        }

                        if(isImgConsistent(pixPositions,imgRemoval)
                            && isSilhouetteConsistent(pixPositions,silhouetteRemoval)
                           && isColorConsistent_plane(pos,pixPositions,col,0,colorRemoval)

                                )

                        {
                            //std::cout<<"imgRemoval"<<imgRemoval<<std::endl;
                            voxel_temp.color = col;
                            consistentVoxels.insert(voxel_temp);
                           // std::cout<<"position"<< voxel_temp.pos<<std::endl;
                        }
                    }
                }
            }


            break;
        //along the Z negative axis
        case 1:
            for(z = gridRes -1; z >= 0; z--){
               // std::cout<<"z"<<z<<std::endl;
                for(y = 0; y < gridRes; y++){
                    for(x = 0;x < gridRes; x++){
                        voxel_temp.pos = glm::vec3(-0.5+step*x,-0.5+step*y,-0.5+step*z);
                        voxel_temp.color = glm::vec3(0.0f,0.0f,0.0f);
                        pixPositions.clear();
                        glm::vec4 pos;
                        glm::vec3 col;
                        for(int j = 0; j < numInput; j++){
                            pos = glm::vec4(voxel_temp.pos,1.0f);
                            glm::vec4 pos_temp = textureMatrices.at(j) * pos_temp;
                            //std::cout<<"matrices"<< textureMatrices.at(j)<<std::endl;
                            glm::vec2 pos_in_img = glm::vec2(pos_temp.x/pos_temp.w,pos_temp.y/pos_temp.w);
                            pixPos pispos;
                            int img_x = round(pos_in_img.x);
                            //std::cout<<"imgx  "<<img_x<<std::endl;
                            int img_y = round(pos_in_img.y);
                            //std::cout<<"imgy  "<<img_y<<std::endl;
                            pispos.first = img_x;
                            pispos.second = img_y;
                            pixPositions.push_back(pispos);
                            //std::cout<<"pix_size"<<pixPositions.size()<<std::endl;

                        }

                        if(isImgConsistent(pixPositions,imgRemoval)
                            && isSilhouetteConsistent(pixPositions,silhouetteRemoval)
                           && isColorConsistent_plane(pos,pixPositions,col,1,colorRemoval)

                                )

                        {
                            //std::cout<<"imgRemoval"<<imgRemoval<<std::endl;
                            voxel_temp.color = col;
                            consistentVoxels.insert(voxel_temp);
                           // std::cout<<"position"<< voxel_temp.pos<<std::endl;
                        }
                    }
                }
            }

            break;
        //along the X positive axis
        case 2:
            for(x = 0; x < gridRes; x++){
               // std::cout<<"z"<<z<<std::endl;
                for(y = 0; y < gridRes; y++){
                    for(z = 0;z < gridRes; z++){
                        voxel_temp.pos = glm::vec3(-0.5+step*x,-0.5+step*y,-0.5+step*z);
                        voxel_temp.color = glm::vec3(0.0f,0.0f,0.0f);
                        pixPositions.clear();
                        glm::vec4 pos;
                        glm::vec3 col;
                        for(int j = 0; j < numInput; j++){
                            pos = glm::vec4(voxel_temp.pos,1.0f);
                            glm::vec4 pos_temp = textureMatrices.at(j) * pos_temp;
                            //std::cout<<"matrices"<< textureMatrices.at(j)<<std::endl;
                            glm::vec2 pos_in_img = glm::vec2(pos_temp.x/pos_temp.w,pos_temp.y/pos_temp.w);
                            pixPos pispos;
                            int img_x = round(pos_in_img.x);
                            //std::cout<<"imgx  "<<img_x<<std::endl;
                            int img_y = round(pos_in_img.y);
                            //std::cout<<"imgy  "<<img_y<<std::endl;
                            pispos.first = img_x;
                            pispos.second = img_y;
                            pixPositions.push_back(pispos);
                            //std::cout<<"pix_size"<<pixPositions.size()<<std::endl;

                        }

                        if(isImgConsistent(pixPositions,imgRemoval)
                            && isSilhouetteConsistent(pixPositions,silhouetteRemoval)
                           && isColorConsistent_plane(pos,pixPositions,col,2,colorRemoval)

                                )

                        {
                            //std::cout<<"imgRemoval"<<imgRemoval<<std::endl;
                            voxel_temp.color = col;
                            consistentVoxels.insert(voxel_temp);
                           // std::cout<<"position"<< voxel_temp.pos<<std::endl;
                        }
                    }
                }
            }

            break;
        //along the x negative axis
        case 3:
            for(x = gridRes - 1; x >= 0; x--){
               // std::cout<<"z"<<z<<std::endl;
                for(y = 0; y < gridRes; y++){
                    for(z = 0;z < gridRes; z++){
                        voxel_temp.pos = glm::vec3(-0.5+step*x,-0.5+step*y,-0.5+step*z);
                        voxel_temp.color = glm::vec3(0.0f,0.0f,0.0f);
                        pixPositions.clear();
                        glm::vec4 pos;
                        glm::vec3 col;
                        for(int j = 0; j < numInput; j++){
                            pos = glm::vec4(voxel_temp.pos,1.0f);
                            glm::vec4 pos_temp = textureMatrices.at(j) * pos_temp;
                            //std::cout<<"matrices"<< textureMatrices.at(j)<<std::endl;
                            glm::vec2 pos_in_img = glm::vec2(pos_temp.x/pos_temp.w,pos_temp.y/pos_temp.w);
                            pixPos pispos;
                            int img_x = round(pos_in_img.x);
                            //std::cout<<"imgx  "<<img_x<<std::endl;
                            int img_y = round(pos_in_img.y);
                            //std::cout<<"imgy  "<<img_y<<std::endl;
                            pispos.first = img_x;
                            pispos.second = img_y;
                            pixPositions.push_back(pispos);
                            //std::cout<<"pix_size"<<pixPositions.size()<<std::endl;

                        }

                        if(isImgConsistent(pixPositions,imgRemoval)
                            && isSilhouetteConsistent(pixPositions,silhouetteRemoval)
                           && isColorConsistent_plane(pos,pixPositions,col,3,colorRemoval)

                                )

                        {
                            //std::cout<<"imgRemoval"<<imgRemoval<<std::endl;
                            voxel_temp.color = col;
                            consistentVoxels.insert(voxel_temp);
                           // std::cout<<"position"<< voxel_temp.pos<<std::endl;
                        }
                    }
                }
            }
            break;
        //along the Y positive axis
        case 4:
            for(y = 0; y < gridRes; y++){
               // std::cout<<"z"<<z<<std::endl;
                for(z = 0; z < gridRes; z++){
                    for(x = 0;x < gridRes; x++){
                        voxel_temp.pos = glm::vec3(-0.5+step*x,-0.5+step*y,-0.5+step*z);
                        voxel_temp.color = glm::vec3(0.0f,0.0f,0.0f);
                        pixPositions.clear();
                        glm::vec4 pos;
                        glm::vec3 col;
                        for(int j = 0; j < numInput; j++){
                            pos = glm::vec4(voxel_temp.pos,1.0f);
                            glm::vec4 pos_temp = textureMatrices.at(j) * pos_temp;
                            //std::cout<<"matrices"<< textureMatrices.at(j)<<std::endl;
                            glm::vec2 pos_in_img = glm::vec2(pos_temp.x/pos_temp.w,pos_temp.y/pos_temp.w);
                            pixPos pispos;
                            int img_x = round(pos_in_img.x);
                            //std::cout<<"imgx  "<<img_x<<std::endl;
                            int img_y = round(pos_in_img.y);
                            //std::cout<<"imgy  "<<img_y<<std::endl;
                            pispos.first = img_x;
                            pispos.second = img_y;
                            pixPositions.push_back(pispos);
                            //std::cout<<"pix_size"<<pixPositions.size()<<std::endl;

                        }

                        if(isImgConsistent(pixPositions,imgRemoval)
                            && isSilhouetteConsistent(pixPositions,silhouetteRemoval)
                           && isColorConsistent_plane(pos,pixPositions,col,4,colorRemoval)

                                )

                        {
                            //std::cout<<"imgRemoval"<<imgRemoval<<std::endl;
                            voxel_temp.color = col;
                            consistentVoxels.insert(voxel_temp);
                           // std::cout<<"position"<< voxel_temp.pos<<std::endl;
                        }
                    }
                }
            }
            break;
        //along the y negative axis
        case 5:
            for(y = gridRes -1; y >= 0; y--){
               // std::cout<<"z"<<z<<std::endl;
                for(z = 0; z < gridRes; z++){
                    for(x = 0;x < gridRes; x++){
                        voxel_temp.pos = glm::vec3(-0.5+step*x,-0.5+step*y,-0.5+step*z);
                        voxel_temp.color = glm::vec3(0.0f,0.0f,0.0f);
                        pixPositions.clear();
                        glm::vec4 pos;
                        glm::vec3 col;
                        for(int j = 0; j < numInput; j++){
                            pos = glm::vec4(voxel_temp.pos,1.0f);
                            glm::vec4 pos_temp = textureMatrices.at(j) * pos_temp;
                            //std::cout<<"matrices"<< textureMatrices.at(j)<<std::endl;
                            glm::vec2 pos_in_img = glm::vec2(pos_temp.x/pos_temp.w,pos_temp.y/pos_temp.w);
                            pixPos pispos;
                            int img_x = round(pos_in_img.x);
                            //std::cout<<"imgx  "<<img_x<<std::endl;
                            int img_y = round(pos_in_img.y);
                            //std::cout<<"imgy  "<<img_y<<std::endl;
                            pispos.first = img_x;
                            pispos.second = img_y;
                            pixPositions.push_back(pispos);
                            //std::cout<<"pix_size"<<pixPositions.size()<<std::endl;

                        }

                        if(isImgConsistent(pixPositions,imgRemoval)
                            && isSilhouetteConsistent(pixPositions,silhouetteRemoval)
                           && isColorConsistent_plane(pos,pixPositions,col,5,colorRemoval)

                                )

                        {
                            //std::cout<<"imgRemoval"<<imgRemoval<<std::endl;
                            voxel_temp.color = col;
                            consistentVoxels.insert(voxel_temp);
                           // std::cout<<"position"<< voxel_temp.pos<<std::endl;
                        }
                    }
                }
            }
            break;
        default:
            break;


        }


    }



}

//
void carve(float step) {

    std::set<voxel> consistentVoxels;

    int imgRemoval = 0;
    int silhouetteRemoval = 0;
    int colorRemoval = 0;

    std::vector<pixPos> pixPositions;
//Resizes the container so that it contains n elements.
	pixPositions.resize(numInput);

/* TODO */
    consistencyCheck(0,0,0,6,imgRemoval,silhouetteRemoval,colorRemoval,pixPositions,consistentVoxels,step);
    // remove doubled elements
    for (std::set<voxel>::iterator iter = consistentVoxels.begin(); iter != consistentVoxels.end(); ++iter) {
        result.push_back(*iter);
    }

    std::cout << "\n\nTotal amount of consistent voxels: " << result.size() << std::endl << std::endl;

}


vector<glm::vec3> cube(float x, float y, float z, float w) {

    vector<glm::vec3> vertices = {
            {x,     y,     z},
            {x + w, y,     z},
            {x + w, y,     z + w},
            {x,     y,     z + w}, // 4 lower points
            {x,     y + w, z},
            {x + w, y + w, z},
            {x + w, y + w, z + w},
            {x,     y + w, z + w}  // 4 upper points
    };

    return vertices;

}


void applySpaceCarving() {

    // the midpoints range from [-0.5, -0.5, -0.5] to [0.5, 0.5, 0.5]
	//nomorlized the pixel
    std::cout<<"start applySpaceCarving"<<std::endl;
    float step = 1.0f / (gridRes - 1);

    carve(step);




    // save result to .obj file
    static vector<glm::vec3> faces = {
            {0, 2, 3},
            {0, 1, 2}, // bottom
            {0, 5, 1},
            {0, 4, 5}, // front
            {0, 7, 4},
            {0, 3, 7}, // left
            {6, 4, 7},
            {6, 5, 4}, // top
            {6, 3, 2},
            {6, 7, 3}, // back
            {6, 1, 5},
            {6, 2, 1}  // right
    };

    std::cout << "Writing to file " << out << "...";

    std::ofstream obj;
    obj.open(out);

    int num_vertices = 1;
    for (std::vector<voxel>::const_iterator iter = result.begin(); iter != result.end(); ++iter) {
        glm::vec3 pos = iter->pos;
        glm::vec3 color = iter->color;

        vector<glm::vec3> vertices = cube(pos[0], pos[1], pos[2], step);
        for (int i = 0; i < vertices.size(); ++i) {
            obj << "v " << std::fixed << std::setprecision(6) << vertices[i].x << " " << vertices[i].y << " "
                << vertices[i].z << " " << color.z << " " << color.y << " " << color.x << std::endl;
        }
        for (int i = 0; i < faces.size(); ++i) {
            obj << "f " << std::fixed << std::setprecision(0) << faces[i].x + num_vertices << " "
                << faces[i].y + num_vertices << " " << faces[i].z + num_vertices << std::endl;
        }
        num_vertices += 8;
    }

    for (int j = 0; j < camPos.size(); ++j) {
        glm::vec3 cam = camPos[j];
        vector<glm::vec3> vertices = cube(cam.x, cam.y, cam.z, .1f);
        glm::vec3 color = glm::vec3(0.f, 0.f, 1.f);

        for (int i = 0; i < vertices.size(); ++i) {
            obj << "v " << std::fixed << std::setprecision(6) << vertices[i].x << " " << vertices[i].y << " "
                << vertices[i].z << " " << color.x << " " << color.y << " " << color.z << std::endl;
        }
        for (int i = 0; i < faces.size(); ++i) {
            obj << "f " << std::fixed << std::setprecision(0) << faces[i].x + num_vertices << " "
                << faces[i].y + num_vertices << " " << faces[i].z + num_vertices << std::endl;
        }
        num_vertices += 8;
    }

    obj.close();

    std::cout << " Done." << std::endl;
}

//check the consistent of position,must be called for every toxel 
bool isImgConsistent(
        const std::vector<pixPos> &pixPositions,
        int &imgRemoval
) {
    assert(pixPositions.size() == static_cast<unsigned int>(numInput));

/* TODO */
	//in the bild space
    //iterate all 12 image, if all consistent, then return ture
		for (int i=0; i<numInput; i++){
			if (pixPositions.at(i).first < 0 || pixPositions.at(i).first > imgWidth.at(i) || pixPositions.at(i).second < 0 || pixPositions.at(i).second > imgHeight.at(i)){
            imgRemoval++;
            //std::cout<<"imgRemoval"<<imgRemoval<<std::endl;
            return false;
        }
    }
	return true;
}

/**
 * Aufgabe: Silhouettenkonsistenz (5 Punkte)
 *
 * Es wird eine weitere Bedingung hinzugefügt: Alle gegebenen Bilder haben
 * einen weißen Hintergrund. Wenn also ein Voxel in irgendeinem der Bilder auf
 * einen weißen Pixel projiziert wird, wird er aus dem Voxelgitter entfernt.
 * Nimmt man die Bildkonsistenz hinzu, sollte
 * das Ergebnis aussehen wie im mittleren Bild.
 */
//transform the type vec3b to the type glm::vec3, dont know whats the fuction for the length for type vec3b
void transform1(Vec3b &vec1,glm::vec3 &vec2){
    vec2.x = vec1[0];
    vec2.y = vec1[1];
    vec2.z = vec1[2];
}

bool isSilhouetteConsistent(
        const std::vector<pixPos> &pixPositions,
        int &silhouetteRemoval
) {
    assert(pixPositions.size() == static_cast<unsigned int>(numInput));

/* TODO */
    for (int i=0; i<numInput; i++){
         glm::vec3 color_vec3;
         Vec3b color_temp = imgs.at(i).at<Vec3b>(pixPositions.at(i).second,pixPositions.at(i).first);
         transform1(color_temp,color_vec3);
         //std::cout<<"color_vec3_x  "<<color_vec3.x<<"color_vec3_y  "<<color_vec3.y<<"color_vec3_z  "<<color_vec3.z<<std::endl;
        if((color_vec3.x == 255 && color_vec3.y == 255 && color_vec3.z == 255))
        {
        silhouetteRemoval++;
        //std::cout<<"silhouetteRemoval"<<silhouetteRemoval<<std::endl;
        return false;
    }
}
    return true;
}
//compare operation for the queue
struct cmp{
    bool operator()(new_pair_type a ,new_pair_type b){
        return a.first > b.first;
    }
};
/**
 * Aufgabe: Space Carving (10 Punkte)
 *
 * - Erweitere das bisher entstandene Framework um eine weitere Bedingung:
 *   Vergleiche die Farbe eines Voxels in allen Eingabebildern. Nur wenn sich
 *   die Farben paarweise um weniger als \code{diffThresh} pro Farbkanal
 *   unterscheiden, werden sie im Gitter belassen. Weise diesen Voxeln die
 *   mittlere Farbe aller Pixelfarben zu. Die 3D-Szene wird in einer .obj-Datei
 *   gespeichert. Farbwerte liegen dort im Bereich $[0,1]$, sie müssen also
 *   entsprechend umgewandelt werden.
 * - Aus numerischen Gründen vergleiche nun nicht die Farbwerte aller
 *   Eingabebilder, sondern nur der $3$ Bilder, die am nächsten am Mittelwert
 *   liegen. Weise dem Voxel den Mittelwert dieser $3$ Farbwerte zu.  Eine
 *   \code{priority\_queue} und \code{pair} können die Arbeit hier erleichtern.
 * - Erweitere das Programm so, dass es mehrere Durchläufe (\emph{sweeps})
 *   benutzt: Betrachte nicht nur eine Ebene, die sich entlang der positiven
 *   z-Achse bewegt, sondern auch Ebenen entlang der positiven $x$- und
 *   $y$-Achse und entlang der negativen Achsen.
 * - Ziehe nun zum Farbvergleich nur Kameras hinzu, die sich in Bezug auf die
 *   Bewegungsrichtung hinter der Ebene befinden.
 */
bool isColorConsistent_plane(
        const glm::vec4 &voxPosition,
        const std::vector<pixPos> &pixPositions,
        glm::vec3 &outputColor,
        const int axis,
        int &colorRemoval
) {
    assert(pixPositions.size() == static_cast<unsigned int>(numInput));
    assert(camPos.size() == static_cast<unsigned int>(numInput));

/* TODO */
    glm::vec3 voxel_position = voxPosition.xyz();
    outputColor = glm::vec3 (0,0,0);
    std::priority_queue<new_pair_type,vector<new_pair_type>,cmp> queue;
    glm::vec3 mean_color(0,0,0);
    std::vector<glm::vec3> color_end;
    color_end.clear();

    //remove some camera
    std::vector<int> camera_id;
    for (int i = 0; i<numInput; ++i){
        glm::vec3 cam_position = camPos[i];

        switch (axis){
        case 0:// z positive
            if (voxel_position.z > cam_position.z){ camera_id.push_back(i); }
            break;
        case 1: //z negativ
            if (voxel_position.z < cam_position.z){ camera_id.push_back(i); }
            break;
        case 2: // x positiv
            if (voxel_position.x > cam_position.x){ camera_id.push_back(i); }
            break;
        case 3: // x negativ
            if (voxel_position.x < cam_position.x){ camera_id.push_back(i); }
            break;
        case 4: // y positv
            if (voxel_position.y > cam_position.y){ camera_id.push_back(i); }
            break;
        case 5: // y negativ
            if (voxel_position.y < cam_position.y){ camera_id.push_back(i); }
            break;
        default:
            break;
        }
    }
    if (camera_id.size() == 0){
        colorRemoval++;
        return false;
    }


    //calculate the mean color from all image
    for (int i=0; i<camera_id.size(); i++){
         Vec3b color_temp = imgs.at(camera_id.at(i)).at<Vec3b>(pixPositions.at(camera_id.at(i)).second,pixPositions.at(camera_id.at(i)).first);
         glm::vec3 vec3_temp;
         transform1(color_temp,vec3_temp);
        //std::cout<<"color "<<static_cast< int >(color_temp[0])<<"  "<<static_cast< int >(color_temp[1])<<"  "<<static_cast< int >(color_temp[2])<<std::endl;
        mean_color += vec3_temp;
    }
    mean_color /=camera_id.size();
    std::cout<<" mean_color "<<mean_color.x<<"  "<<mean_color.y<<"  "<<mean_color.z<<std::endl;

  //reoder the image oder in the queue according to the color differnece
    for (int i=0; i<camera_id.size(); i++){
        Vec3b color_temp = imgs.at(camera_id.at(i)).at<Vec3b>(pixPositions.at(camera_id.at(i)).second,pixPositions.at(camera_id.at(i)).first);
        glm::vec3 vec3_temp;
        transform1(color_temp,vec3_temp);
        new_pair_type pair;
        pair.first = glm::length(vec3_temp - mean_color);
        //std::cout<<"  difference  "<<pair.first <<std::endl;
        pair.second = i;
        queue.push(pair);
    }
  //only use the first 3 image
    for(int i = 0;i < 3; i++) {
        new_pair_type temp = queue.top();
        queue.pop();
        int image_id = temp.second;
        std::cout<<"difference"<<i<<"  "<<queue.top().first<< "     index   "<<queue.top().second<<std::endl;
        Vec3b color_temp = imgs.at(image_id).at<Vec3b>(pixPositions.at(image_id).second,pixPositions.at(image_id).first);
        glm::vec3 vec3_temp;
        transform1(color_temp,vec3_temp);
        color_end.push_back(vec3_temp);
        std::cout<<" color_temp"<<i<<"  "<<vec3_temp.x<<"  "<<vec3_temp.y<<"  "<<vec3_temp.z<<std::endl;

}
    glm::vec3 difference0 = glm::abs(color_end.at(0) - color_end.at(1));
    glm::vec3 difference1 = glm::abs(color_end.at(1) - color_end.at(2));
    glm::vec3 difference2 = glm::abs(color_end.at(2) - color_end.at(0));
    if(difference0.x >= diffThresh && difference0.y >= diffThresh && difference0.z >= diffThresh
            && difference1.x >= diffThresh && difference1.y >= diffThresh && difference1.z >= diffThresh
            && difference2.x >= diffThresh && difference2.y >= diffThresh && difference2.z >= diffThresh){
        colorRemoval++;
        return false;
     }

        outputColor = (color_end.at(0) + color_end.at(1) +color_end.at(2)) / 3.0f / 255.0f;

    return true;
}





///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Code for I/O; don't bother.
glm::mat4 createTextureMatrix(const glm::mat4 &modelviewmat, const glm::mat4 &projectionmat, const Mat &img) {
    glm::mat4 flip(glm::vec4(1, 0, 0, 0), glm::vec4(0, 1, 0, 0), glm::vec4(0, 0, 1, 0), glm::vec4(0, img.rows, 0, 1));
    glm::mat4 imgscale(glm::vec4(img.cols, 0, 0, 0), glm::vec4(0, -img.rows, 0, 0), glm::vec4(0, 0, 1, 0),
                       glm::vec4(0, 0, 0, 1));
    glm::mat4 scale(glm::vec4(0.5f, 0, 0, 0), glm::vec4(0, 0.5f, 0, 0), glm::vec4(0, 0, 0.5f, 0),
                    glm::vec4(0, 0, 0, 1));
    glm::mat4 translate(glm::vec4(1, 0, 0, 0), glm::vec4(0, 1, 0, 0), glm::vec4(0, 0, 1, 0),
                        glm::vec4(0.5f, 0.5f, 0.5f, 1));

    return flip* imgscale * translate * scale * projectionmat * modelviewmat;
}

void createTextureMatrices() {

    for (int i = 0; i < numInput; ++i) {
        glm::mat4 tex = createTextureMatrix(modelView[i], projection[i], imgs.at(i));

        textureMatrices.push_back(tex);
    }
}

void findCameraPositions() {
    for (std::vector<glm::mat4>::iterator iter = modelView.begin(); iter != modelView.end(); ++iter) {
        glm::mat4 m = glm::inverse(*iter);

        glm::vec4 origin(0, 0, 0, 1);
        glm::vec3 pos = (m * origin).xyz();

        camPos.push_back(pos);
    }
}

void loadData() {
    std::stringstream p;
    p << path << "/num.txt";

    // find out number of images to be loaded
    std::ifstream file(p.str().c_str());
    if (!file) {
        std::cerr << "Could not find scene \"" << path << "\"." << std::endl;
        exit(1);
    }

    file >> numInput;
    std::cout << "Number of Images: " << numInput << std::endl;

    modelView.resize(numInput);
    projection.resize(numInput);

    // load camera data
    int count = 0;

    for (int i = 0; i < numInput; ++i) {
        std::stringstream s;
        s << path << "/cam" << count++ << ".txt";
        std::ifstream file(s.str().c_str());

        float arr1[16];
        for (int j = 0; j < 16; ++j) {
            file >> arr1[j];

        }
        modelView[i] = glm::make_mat4(arr1);

        float arr2[16];
        for (int j = 0; j < 16; ++j) {
            file >> arr2[j];
        }
        projection[i] = glm::make_mat4(arr2);

    }

    // load image data
    count = 0;
    for (int i = 0; i < numInput; ++i) {
        std::stringstream s;
        s << path << "/img" << count++ << ".png";
        Mat img = imread(s.str().c_str());
        imgs.push_back(img);
        imgWidth.push_back(img.cols);
        imgHeight.push_back(img.rows);
    }

    createTextureMatrices();
    findCameraPositions();

}


void parseInput(int argc, char **argv) {
    if (argc < 3) {
        std::cerr << "usage: " << argv[0] << " <path> <outfile> (<diffThresh>) (<gridRes>)" << std::endl;
        exit(1);
    }
    path = argv[1];
    out = argv[2];

    if (argc > 3) {
        diffThresh = atoi(argv[3]);
    }
    if (argc > 4) {
        gridRes = atoi(argv[4]);
    }
}


int main(int argc, char **argv) {

    //compare arguments
    parseInput(argc, argv);
    // load the data
    loadData();
    // apply space carving
    applySpaceCarving();

    return 0;

}
