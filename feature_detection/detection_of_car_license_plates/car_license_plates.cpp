#include <stdio.h>
#include <iostream>
#include <iostream>
#include "opencv2/core.hpp"
#include "opencv2/features2d.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/features2d/features2d.hpp"
#include "opencv2/calib3d/calib3d.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/xfeatures2d.hpp"
#include <opencv2/imgproc/imgproc.hpp>
#include <tesseract/baseapi.h>
#include <leptonica/allheaders.h>

using namespace std;
using namespace cv;
using namespace cv::xfeatures2d;
using namespace tesseract;

const int WIDTH_SIZE = 640;
const int HEIGHT_SIZE = 480;
const int MIN_HESSIAN = 400;
const int MAX_LOW_THRESHOLD = 100;
const int RATIO = 3;
const int KERNEL_SIZE = 3;

Mat loadImage(string path) 
{
    // Imagen a consultar
    Mat queryImg = imread(path, IMREAD_GRAYSCALE);

    if(!queryImg.data) { 
      std::cout<< " --(!) Error reading image " << std::endl; 
      exit(-1);
    }

    return queryImg;
}

Mat boundingPattern(Mat imgPattern, Mat imgScene, Mat h)
{
    std::cout<< " --(!) Recortando imagen " << std::endl;
    std::vector<Point2f> obj_corners(4);
    obj_corners[0] = cvPoint(0,0); 
    obj_corners[1] = cvPoint( imgPattern.cols, 0 );
    obj_corners[2] = cvPoint( imgPattern.cols, imgPattern.rows ); 
    obj_corners[3] = cvPoint( 0, imgPattern.rows );
    std::vector<Point2f> scene_corners(4);
    perspectiveTransform( obj_corners, scene_corners, h);
    // Get crop image
    Rect box = boundingRect(Mat(scene_corners));
    Mat roi = Mat(imgScene,box);
    return roi;
}

/*
    Función que elimina todos aquellos puntos característicos detectados por la Función
    SURF, pero que se encuentran fuera de la matrícula. Para ello determina la mínima distancia
    entre puntos y elimina todos aquellos que no tienen al menos 5 puntos al doble de la distancia 
    mínima hallada.

*/
std::vector<Point2f> removeOutsidePoints(std::vector<Point2f> scene, double min_dist, Mat img2){
    double distx, disty;
    int cont;
    std::vector<Point2f> result;
    std::cout<< " --(!) Eliminando puntos fuera de la matrícula " << std::endl; 
    // Compara todos los puntos para hallar la distancia mínima
    for(int i = 0; i < scene.size(); i++)
    {
        // Variable que cuenta cuántos puntos hay más cerca del doble de la mínima distancia.
        cont=0;
        for(int j = 0; j < scene.size(); j++)
        {
            if(i!=j && scene[i].y > (img2.rows/3))
            {
                distx = std::abs(scene[i].x - scene[j].x);
                disty = std::abs(scene[i].y - scene[j].y);
                if(distx < 2*min_dist && disty < 2*min_dist) cont++;
            }
        }
        // Si hay más de 5 puntos a menos del doble de la mínima distancia.
        if(cont > 5) result.push_back(scene[i]);
    }
    return result;
}

// Función encargada de detectar los puntos característicos de la imagen.
Mat getHomography(Mat imgPattern, Mat imgScene){
    // Se pretende determinar todas las coincidencias entre la imagen de entrada  y el patrón
    Ptr<SURF> detector = SURF::create();
    detector->setHessianThreshold(MIN_HESSIAN);
    FlannBasedMatcher matcher;
    std::vector<KeyPoint> keypointsPattern, keypointsScene;
    Mat descriptorsPattern, descriptorsScene;
    std::vector<DMatch> matches;
    std::vector<DMatch> good_matches;
    // variables para discriminar puntos característicos en las imagenes
    double max_dist = 0, min_dist = 100, dist;
    std::cout<< " --(!) Buscando puntos característicos " << std::endl; 
    std::cout<< " --(!) Enlazamos coincidencias " << std::endl; 
    // Buscamos puntos característicos un número determinado de veces para descartar los malos
    do{
        /*
            1 -> Detecta puntos característicos de las imágenes
            2 -> Calcula los descriptores (Puntos característicos que más información contienen).
        */
        detector->detectAndCompute(imgPattern, Mat(), keypointsPattern, descriptorsPattern);
        detector->detectAndCompute(imgScene, Mat(), keypointsScene, descriptorsScene);
        // Enlazamos coincidencias de los descriptores SURF con el FLANN matcher.
        matcher.match( descriptorsPattern, descriptorsScene, matches);
        //calculo de la máxima y mínima distancia entre puntos característicos
        for( int i = 0; i < descriptorsPattern.rows; i++)
        {
            dist = matches[i].distance;
            if(dist < min_dist) min_dist = dist;
            if(dist > max_dist) max_dist = dist;
        }
        // Buscamos coincidencias buenas, aquellas cuyo distancia es menor que 1.5 veces
        // la mínima distancia entre puntos característicos.
        for(int i = 0; i < descriptorsPattern.rows; i++)
        {
            if(matches[i].distance < 1.5*min_dist)
                good_matches.push_back(matches[i]);
        } 
    }while(good_matches.size() < 100);

    printf("-- Max dist : %f \n", max_dist );
    printf("-- Min dist : %f \n", min_dist );
    printf("-- Matches: %lu \n", matches.size());
    printf("-- Good Matches: %lu \n", good_matches.size());
    Mat img_matches;
    drawMatches(imgPattern, keypointsPattern, imgScene, keypointsScene,
               good_matches, img_matches, Scalar::all(-1), Scalar::all(-1),
               vector<char>(), DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS );
    
    imshow("Good Matches & Object detection", img_matches);

    std::vector<Point2f> obj;
    std::vector<Point2f> scene;
    std::cout<< " --(!) Obteniendo puntos clave de las coincidencias buenas " << std::endl; 
    for(int i = 0; i < good_matches.size(); i++)
    {
        //Obtenemos los keypointsScene de las coincidencias buenas
        obj.push_back( keypointsPattern[ good_matches[i].queryIdx ].pt );
        scene.push_back( keypointsScene[ good_matches[i].trainIdx ].pt );
    }    
    // eliminamos aquellos puntos que se encuentren fuera de la matrícula
    
    scene = removeOutsidePoints(scene, 0.20, imgPattern);
    printf("-- obj size : %lu \n", obj.size());
    printf("-- scene size : %lu \n", scene.size() );
    std::cout<< " --(!) Obteniendo homografía" << std::endl; 
    Mat h = findHomography( obj, scene, CV_RANSAC );
    return h;
}

int main(int argc, char** argv)
{

    Mat img, pattern;
    // Carga imagen 
    img = loadImage(argv[1]);
    // Cargamos imagen del patrón
    pattern = loadImage("../img/pattern.jpg");
    //GaussianBlur(pattern, pattern, Size(21, 21), 0);
    if(!img.data || !pattern.data)
    {
        std::cout<< " --(!) Error al cargar las imagenes " << std::endl; 
        exit(-1);
    }
    else
    {
        std::cout<< " --(!) Imagenes cargadas con éxito " << std::endl;
    }
    // Redimensiona imagen de entrada para trabajar siempre con las mismas dimensiones
    resize(img, img, Size(WIDTH_SIZE, HEIGHT_SIZE));
    Mat h = getHomography(pattern, img);
    if(!h.empty())
    {
        Mat cropImage = boundingPattern(pattern, img, h);
        imshow("Crop Image", cropImage);
    }
        
    std::cout<< " --(!) Operación finalizada " << std::endl;
    waitKey(0);
}