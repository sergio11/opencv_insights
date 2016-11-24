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

const int MIN_HESSIAN = 400;

VideoCapture openCamera()
{
    VideoCapture camera;
    camera.open(0);
    if(!camera.isOpened()) {
        std::cout<<"Camera did not open";
        exit(1);
    }
    camera.set(CV_CAP_PROP_FRAME_WIDTH, 640);
    camera.set(CV_CAP_PROP_FRAME_HEIGHT, 480);
    return camera;
}

Mat loadQueryImage() 
{
    // Imagen a consultar
    Mat queryImg = imread( "../img/spring_cuata_edicion.jpg", IMREAD_GRAYSCALE );

    if( !queryImg.data) { 
      std::cout<< " --(!) Error reading query iamge " << std::endl; 
      exit(-1);
    }

    return queryImg;
}

Mat getNextFrame(VideoCapture camera)
{
    Mat cframe;
    bool success = camera.read(cframe);
    if (!success) {
        std::cerr << "ERROR: Couldn't grab a camera frame." <<std::endl;
        exit(1);
    }
    cvtColor(cframe, cframe, CV_8U); //Converting image to 8-bit
    Mat grayImage; //creating a canvas for the grayscale image
    cvtColor(cframe, grayImage, CV_RGB2GRAY); //creating the grayscale image
    return grayImage;
}

std::vector<DMatch> matchDescriptors(Mat descriptorsImg1, Mat descriptorsImg2)
{
    std::vector<DMatch> matches;
    double max_dist = 0; double min_dist = 100;
    FlannBasedMatcher matcher;
    matcher.match( descriptorsImg1, descriptorsImg2, matches);
    // Cálculo rápido de las distancias máximas y mínimas entre ambos puntos claves
    for( int i = 0; i < descriptorsImg1.rows; i++ )
    { 
        double dist = matches[i].distance;
        if( dist < min_dist ) min_dist = dist;
        if( dist > max_dist ) max_dist = dist;
    }
    printf("-- Max dist : %f \n", max_dist );
    printf("-- Min dist : %f \n", min_dist );
    std::vector<DMatch> good_matches;
    for( int i = 0; i < descriptorsImg1.rows; i++ ) { 
        if( matches[i].distance <= max(2*min_dist, 0.02) ) { 
            good_matches.push_back( matches[i]); 
        }
    }
    return good_matches;
}

int main( int argc, char** argv )
{
    // creamos el detecto SURF
    Ptr<SURF> detector = SURF::create();
    detector->setHessianThreshold(MIN_HESSIAN);
    // cargamos la imagen a consultar
    Mat queryImg = loadQueryImage();
    std::vector<KeyPoint> queryImageKeyPoints;
    Mat queryImageDescriptors;
    // obtenemos puntos clave y descriptores para la imagen a consultar.
    detector->detectAndCompute( queryImg, Mat(), queryImageKeyPoints, queryImageDescriptors);
    // Inciamos la cámara
    VideoCapture camera = openCamera();

    while (true)
    {
      if (waitKey(30) >= 0) break;
      // obtenemos el siguiente frame
      Mat frame = getNextFrame(camera);
      std::vector<KeyPoint> frameKeyPoints;
      Mat frameDescriptors;
      // obtenemos puntos clave y descriptores para el frame actual.
      detector->detectAndCompute(frame, Mat(), frameKeyPoints, frameDescriptors);
      // buscamos coincidencias entre ambos descriptores
      std::vector<DMatch> matches = matchDescriptors(queryImageDescriptors, frameDescriptors);
      Mat img_matches;
      drawMatches( queryImg, queryImageKeyPoints, frame, frameKeyPoints,
               matches, img_matches, Scalar::all(-1), Scalar::all(-1),
               vector<char>(), DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS );
      imshow("Buscando Libro de Spring",img_matches);
    }

    std::cout<<"Good By!";
    return 0;
}