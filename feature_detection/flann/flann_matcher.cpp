#include <stdio.h>
#include <iostream>
#include <iostream>
#include "opencv2/core.hpp"
#include "opencv2/features2d.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/xfeatures2d.hpp"

using namespace std;
using namespace cv;
using namespace cv::xfeatures2d;


int main( int argc, char** argv )
{
  // Imagen a consultar
  Mat img_1 = imread( "../img/2093GSW.jpg", IMREAD_GRAYSCALE );
  // Imagen de entrenamiento
  Mat img_2 = imread( "../img/matricula-zoom.jpg", IMREAD_GRAYSCALE );

  if( !img_1.data || !img_2.data ) { 
      std::cout<< " --(!) Error reading images " << std::endl; 
      return -1;
  }
  // Obtenemos los puntos clave y los descriptores de ambas imagenes usando el detector SURF
  int minHessian = 400;
  std::vector<KeyPoint> keypoints_1, keypoints_2;
  Mat descriptors_1, descriptors_2;
  std::vector<DMatch> matches;
  double max_dist = 0; double min_dist = 100;

  Ptr<SURF> detector = SURF::create();
  detector->setHessianThreshold(minHessian);
  detector->detectAndCompute( img_1, Mat(), keypoints_1, descriptors_1 );
  detector->detectAndCompute( img_2, Mat(), keypoints_2, descriptors_2 );
  //Usamos el FLANN matcher para buscar coincidencias entre los descriptores SURF de ambas imagenes
  FlannBasedMatcher matcher;
  matcher.match( descriptors_1, descriptors_2, matches );
  // Cálculo rápido de las distancias máximas y mínimas entre ambos puntos claves
  for( int i = 0; i < descriptors_1.rows; i++ )
  { double dist = matches[i].distance;
    if( dist < min_dist ) min_dist = dist;
    if( dist > max_dist ) max_dist = dist;
  }
  printf("-- Max dist : %f \n", max_dist );
  printf("-- Min dist : %f \n", min_dist );

  std::vector<DMatch> good_matches;
  for( int i = 0; i < descriptors_1.rows; i++ ) { 
    if( matches[i].distance <= max(2*min_dist, 0.02) ) { 
      good_matches.push_back( matches[i]); 
    }
  }
  Mat img_matches;
  drawMatches( img_1, keypoints_1, img_2, keypoints_2,
               good_matches, img_matches, Scalar::all(-1), Scalar::all(-1),
               vector<char>(), DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS );
  imshow( "Good Matches", img_matches );
  for( int i = 0; i < (int)good_matches.size(); i++ ) { 
    printf( "-- Good Match [%d] Keypoint 1: %d  -- Keypoint 2: %d  \n", i, good_matches[i].queryIdx, good_matches[i].trainIdx ); 
  }
  waitKey(0);
  return 0;
}

