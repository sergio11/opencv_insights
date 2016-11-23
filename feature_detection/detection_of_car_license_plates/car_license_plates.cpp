#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/nonfree/features2d.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/opencv.hpp"
#include "opencv2/calib3d/calib3d.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <math.h>
#include <string.h>

using namespace std;
using namespace cv;
using namespace cv::xfeatures2d;

const int WIDTH_SIZE = 640;
const int HEIGHT_SIZE = 480;
const int MIN_HESSIAN = 400;

Mat loadImage(string path) 
{
    // Imagen a consultar
    Mat queryImg = imread( ath, IMREAD_GRAYSCALE);

    if( !queryImg.data) { 
      std::cout<< " --(!) Error reading image " << std::endl; 
      exit(-1);
    }

    return queryImg;
}

/*
    Función que elimina todos aquellos puntos característicos detectados por la Función
    SURF, pero que se encuentran fuera de la matrícula. Para ello determina la mínima distancia
    entre puntos y elimina todos aquellos que no tienen al menos 5 puntos al doble de la distancia 
    mínima hallada.

*/
cv::vector<Point2f> removeOutsidePoints(std::vector<Point2f> scene, double min_dist, Mat img2){
    double distx, disty;
    int cont;
    std::vector<Point2f> result;
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
                if(distx < 2*min_dist && disty < 2*min_dist)
                    cont++
            }
        }
        // Si hay más de 5 puntos a menos del doble de la mínima distancia.
        if(cont > 5) result.push_back(scene[i]);
    }
    return result;
}

// Función encargada de detectar los puntos característicos de la imagen.
std::vector<Point2f> getKeyPoints(Mat img_1, Mat img_2){
    // Se pretende determinar todas las coincidencias entre la imagen de entrada  y el patrón
    Ptr<SURF> detector = SURF::create();
    detector->setHessianThreshold(MIN_HESSIAN);
    FlannBasedMatcher matcher;
    std::vector<KeyPoint> keypoints_1, keypoints_2;
    Mat descriptors_1, descriptors_2;
    std::vector<DMatch> matches;
    std::vector<DMatch> good_matches;
    // variables para discriminar puntos característicos en las imagenes
    double max_dist = 0, min_dist = 100, dist;
    // Buscamos puntos característicos un número determinado de veces para descartar los malos
    do{
        /*
            1 -> Detecta puntos característicos de las imágenes
            2 -> Calcula los descriptores (Puntos característicos que más información contienen).
        */
        detector->detectAndCompute(img_1, Mat(), keypoints_1, descriptors_1);
        detector->detectAndCompute(img_2, Mat(), keypoints_2, descriptors_2);
        // Enlazamos coincidencias de los descriptores SURF con el FLANN matcher.
        matcher.match( descriptors_1, descriptors_2, matches);
        //calculo de la máxima y mínima distancia entre puntos característicos
        for( int i = 0; i < descriptors_1.rows; i++)
        {
            dist = matches[i].distance;
            if(dist < min_dist) min_dist = dist;
            if(dist > max_dist) max_dist = dist;
        }
        // Buscamos coincidencias buenas, aquellas cuyo distancia es menor que 1.5 veces
        // la mínima distancia entre puntos característicos.
        for(int i = 0; i < descriptors_1.rows; i++)
        {
            if(matches[i].distance < 1.5*min_dist)
                good_matches.push_back(matches[i]);
        } 
    }while(good_matches.size() < 100);

    std::vector<Point2f> obj;
    std::vector<Point2f> scene;
    for(int i = 0; i < good_matches.size(); i++)
    {
        //Obtenemos los keypoints de las coincidencias buenas
        Point2f objPt = keypoints_1[good_matches[i].queryIdx].pt;
        Point2f scenePt = keypoints_2[good_matches[i].trainIdx].pt;
        obj.push_back(objPt);
        scene.push_back(scenePt);
    }
    // eliminamos aquellos puntos que se encuentren fuera de la matrícula
    std::vector<Point2f> obj2 = removeOutsidePoints(scene, 0.20, img_2);
    return obj2;
}

int main(int argc, char** argv)
{

    Mat img, pattern;
    //Carga imagen
    img = loadImage(argv[1]);
    pattern = loadImage("pattern.jpg");
    if(!img.data || !pattern.data){
        printf("Error al cargar las imagenes");
        exit(-1);
    }
    // Redimensiona imagen de entrada para trabajar siempre con las mismas dimensiones
    resize(img, img Size(WIDTH_SIZE, HEIGHT_SIZE));
    std::vector<Point2f> points = getKeyPoints(img, pattern);
    
}