/** @file ocvext/test/tests.cc
 *  @brief Programme de test pour hough.h et gl.h
 *  @author J.A. 2015 / www.tsdconseil.fr
 *  @license LGPL V 3.0 */

#include "gl.hpp"
#include "hough.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#ifdef OCV240
#include <opencv2/contrib/contrib.hpp>
#endif
#include <stdio.h>
#include <math.h>

#define trace(...) printf(__VA_ARGS__); fflush(0);


/* Code for this function taken from the OpenCV tutorial on page:
 * http://docs.opencv.org/doc/tutorials/imgproc/imgtrans/hough_lines/hough_lines.html */
static void plot_lines(cv::Mat &img,
                       const std::vector<cv::Vec2f> lines,
                       const cv::Scalar &color)
{
  for(size_t i = 0; i < lines.size(); i++ )
  {
    float rho = lines[i][0], theta = lines[i][1];
    cv::Point pt1, pt2;
    double a = cos(theta), b = sin(theta);
    double x0 = a * rho, y0 = b * rho;
    pt1.x = cvRound(x0 + 1000 * (-b));
    pt1.y = cvRound(y0 + 1000 * (a));
    pt2.x = cvRound(x0 - 1000 * (-b));
    pt2.y = cvRound(y0 - 1000 * (a));
    cv::line(img, pt1, pt2, color, 1, CV_AA);
  }
}

static float normal_law(float m, float sigma)
{
  float x1;
  float x2;

  do
  {
    x1 = ((float) rand()) / ((float) RAND_MAX);
    x2 = ((float) rand()) / ((float) RAND_MAX);
  } while((x1 < 0.001) || (x2 < 0.001));


  float y1 = std::sqrt(-2.0*log(x1)) * cos(2.0 * 3.1415926f * x2);

  float res = m + y1 * sigma;

  return res;
}

static void add_noise(uint8_t &value)
{
  int v = value;
  v += normal_law(0, 30);
  if(v < 0)
    value = 0;
  else if(v > 255)
    value = 255;
  else
    value = (uint8_t) v;
}



void test_grad(cv::Mat &gx, cv::Mat &gy, const std::string &name)
{
  cv::Mat gabs, mag, angle;

  cv::cartToPolar(gx, gy, mag, angle);

  cv::convertScaleAbs(gx,gx);
  cv::convertScaleAbs(gy,gy);
  cv::addWeighted(gx, .5, gy, .5, 0, gabs);
  cv::normalize(gabs, gabs, 0, 255, cv::NORM_MINMAX);
  cv::imwrite("./build/gabs-" + name + ".jpg", gabs);

  // Image au format TSV, seule la luminosité est fixée
  cv::Mat hsv;
  cv::normalize(mag,mag,0,1.0,cv::NORM_MINMAX);
  cv::Mat _hsv[3], bgr;
  // Teinte (couleur) = angle
  _hsv[0] = angle * ((360.0 / (2 * 3.141592)));
  // Chromaticité = 1
  _hsv[1] = 1.0 * cv::Mat::ones(angle.size(), CV_32F);
  // Luminance
  _hsv[2] = mag;
  merge(_hsv, 3, hsv);
  cv::Mat hsv2 = cv::Mat(hsv.size(), CV_8UC3);
  cvtColor(hsv, hsv2, cv::COLOR_HSV2BGR);
  hsv2.convertTo(hsv2, CV_8UC3, 255);
  cv::imwrite("./build/hsv-" + name + ".jpg", hsv2);
}


int tests_gradient(const cv::Mat &img)
{
  cv::Mat gx, gy, imgf;
  //////////////////////////////////
  // Pas de pré-lissage         ////
  //////////////////////////////////
  cv::Sobel(img, gx, CV_32F, 1, 0, 1);
  cv::Sobel(img, gy, CV_32F, 0, 1, 1);
  test_grad(gx, gy, "sobel");

  //////////////////////////////////
  // Pré-lissage Deriche        ////
  //////////////////////////////////
  DericheGradient(img, gx, gy, 0.5);
  test_grad(gx, gy, "deriche");

  //////////////////////////////////
  // Pré-lissage gaussien       ////
  //////////////////////////////////
  cv::GaussianBlur(img, imgf, cv::Size(15,15), 0, 0);
  cv::Sobel(imgf, gx, CV_32F, 1, 0, 1);
  cv::Sobel(imgf, gy, CV_32F, 0, 1, 1);
  test_grad(gx, gy, "canny");

  //////////////////////////////////
  // Pré-lissage moy. mobile    ////
  //////////////////////////////////
  cv::blur(img, imgf, cv::Size(11,11));
  cv::Sobel(imgf, gx, CV_32F, 1, 0, 1);
  cv::Sobel(imgf, gy, CV_32F, 0, 1, 1);
  test_grad(gx, gy, "mob");
  return 0;
}


int tests_hough(const cv::Mat &img)
{
  printf("test hough...\n"); fflush(0);
  //////////////////////////////////
  // Hough avec gradient        ////
  //////////////////////////////////
  std::vector<cv::Vec2f> lines;
  HoughLinesWithGradientDir(img, lines);
  printf("Détecté %d lignes.\n", lines.size());
  printf("sx = %d, sy = %d.\n", img.cols, img.rows);
  cv::Mat img2, img3;
  cv::cvtColor(img, img2, CV_GRAY2BGR);
  plot_lines(img2, lines, cv::Scalar(0,255,0));
  cv::imwrite("./build/hough-grad.jpg", img2);

  //////////////////////////////////
  // Hough natif OpenCV         ////
  //////////////////////////////////
  lines.clear();
  cv::Mat bw = img.clone();
  cv::blur(bw, bw, cv::Size(3, 3));
  int seuil_canny = 120;
  int seuil_hough = 50;
  cv::Canny(bw, bw, seuil_canny, seuil_canny * 2, 3);
  cv::HoughLines(bw, lines, 1, CV_PI/180, seuil_hough, 0, 0);
  printf("Détecté %d lignes.\n", lines.size());
  printf("sx = %d, sy = %d.\n", img.cols, img.rows);
  cv::cvtColor(img, img3, CV_GRAY2BGR);
  plot_lines(img3, lines, cv::Scalar(0,0,255));
  cv::imwrite("./build/hough-nat.jpg", img3);
  return 0;
}

int main(int argc, const char **argv)
{
  cv::Mat img, imgf, gris;
  trace("lecture image...\n");
  if(argc > 1)
    img = cv::imread(argv[1]);
  else
  {
    img = cv::imread(/*argv[1]*/"data/test-gradient.png");
    uint16_t sx = img.cols, sy = img.rows;
    for(auto y = 0u; y < sy; y++)
    {
      for(auto x = 0u; x < sx; x++)
      {
        add_noise(img.at<cv::Vec3b>(y,x).val[0]);
        add_noise(img.at<cv::Vec3b>(y,x).val[1]);
        add_noise(img.at<cv::Vec3b>(y,x).val[2]);
      }
    }
    cv::imwrite("./build/img-bruit.jpg", img);
  }
  uint16_t sx = img.cols, sy = img.rows;

  cv::cvtColor(img, gris, CV_BGR2GRAY);
  tests_gradient(gris);
  tests_hough(gris);


  std::vector<cv::Vec2f> lines;
  img = cv::imread("data/card.jpg");
  HoughLinesWithGradientDir(img, lines);
  cv::Mat img2 = img.clone();
  plot_lines(img2, lines, cv::Scalar(0,255,0));

  cv::Mat hough;
  HoughWithGradientDir(img, hough);
  hough.convertTo(hough, CV_8U);
  cv::applyColorMap(hough, hough, cv::COLORMAP_JET);
  cv::imwrite("./build/card-lines.jpg", img2);
  cv::imwrite("./build/card-hough.jpg", hough);
  return 0;
}



