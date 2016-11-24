#include <stdio.h>
#include <iostream>
#include <iostream>
#include "opencv2/core.hpp"
#include "opencv2/features2d.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/features2d/features2d.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/xfeatures2d.hpp"
#include <opencv2/imgproc/imgproc.hpp>

using namespace std;
using namespace cv;
using namespace cv::xfeatures2d;

const int WIDTH_SIZE = 640;
const int HEIGHT_SIZE = 480;
const int MIN_HESSIAN = 400;

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
    std::cout<< " --(!) Buscando puntos característicos " << std::endl; 
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
    std::cout<< " --(!) Obteniendo puntos clave de las coincidencias buenas " << std::endl; 
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

/*
    Hallamos el rectángulo de corte. Para ello determina la posición del punto más 
    a la izquierda. En función de su ubicación crea diferentes rectángulos de corte.
*/
Mat findRectangle(std::vector<Point2f> obj, Mat img)
{
    float minx, miny, maxx, maxy;
    std::cout<< " --(!) Hallando rectángulo de corte " << std::endl; 
    // Obtenemos posiciones de los puntos extremos. Aquellos que tienen los mayores y los
    // menores valores en x e y
    for(int i = 0; i < obj.size(); i++)
    {
        if(i == 0)
        {
            minx = obj[i].x;
            miny = obj[i].y;
            maxx = obj[i].x;
            maxy = obj[i].y;
        } 
        else
        {
            if(obj[i].x < minx) minx = obj[i].x;
            if(obj[i].y < miny) miny = obj[i].y;
            if(obj[i].x > maxx) maxx = obj[i].x;
            if(obj[i].y > maxy) maxy = obj[i].y;
        }
    }
    double tol = 60.0;
    float cols = img.cols;
    float div = minx/cols;
    float minxx, maxyy = maxy + tol, minyy = miny - tol;
    if(div > 0 && div < 0.25)
    {
        minxx = minx;
        maxx=minx+(minx*3);
    }
    else if(div >= 0.25 && div < 0.35)
    {
        minxx = minx-(minx*0.2);
        maxx=minx+(minx*1.6);
    }
    else if(div >= 0.35 && div < 0.45)
    {
        minxx = minx-(minx*0.35);
        maxx=minx+(minx*1.2);
    }
    else if(div >= 0.45 && div < 0.55)
    {
        minxx = minx-(minx*0.6);
        maxx=minx+(minx*0.6);
    }
    else if(div >= 0.55 && div < 0.65)
    {
        minxx = minx-(minx*0.65);
        maxx=minx+(minx*0.4);
    }
    else if(div >= 0.65 && div < 0.75)
    {
        minxx = minx-(minx*0.7);
        maxx=minx;
    }
    else
    {
        minxx = minx-(minx*0.8);
        maxx=minx+minx;
    }

    //Vector para guardar los puntos de corte de la imagen
    std::vector<Point2f> obj_corners(4);
    obj_corners[0] = Point2f(minxx, minyy);
    obj_corners[1] = Point2f(maxx, minyy);
    obj_corners[2] = Point2f(maxx, maxyy);
    obj_corners[3] = Point2f(minxx, maxyy);

    Rect rec(
        obj_corners[0].x,
        obj_corners[0].y,
        obj_corners[1].x - obj_corners[0].x,
        obj_corners[2].y - obj_corners[1].y
    );

    Mat crop  = img(rec);
    // Dibujamos las líneas del rectángulo de corte
    cvtColor(img, img, CV_GRAY2BGR);
    line(img, obj_corners[0], obj_corners[1], Scalar(0, 255, 0), 4);
    line(img, obj_corners[1], obj_corners[2], Scalar(0, 255, 0), 4);
    line(img, obj_corners[2], obj_corners[3], Scalar(0, 255, 0), 4);
    line(img, obj_corners[3], obj_corners[0], Scalar(0, 255, 0), 4);

    return img;

}

int main(int argc, char** argv)
{

    Mat img, pattern;
    //Carga imagen
    img = loadImage(argv[1]);
    pattern = loadImage("../img/pattern_2.jpeg");
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
    std::vector<Point2f> points = getKeyPoints(img, pattern);
    // Buscamos rectángulo
    Mat imageCrop = findRectangle(points, img);
    imshow("Image Crop", imageCrop);
    // Algoritmo para acotar espacio de búsqueda
    Point p1, p2;
    Mat rectash, rec_col;
    // Vector que contiene las rectas detectadas
    std::vector<cv::Vec2f> lineash;
    // Se aplica Canny a la imagen para extraer los bordes.
    Canny(imageCrop, rectash, 100, 250, 3);
    std::cout<< " --(!) Operación finalizada " << std::endl;
    waitKey(0);
}