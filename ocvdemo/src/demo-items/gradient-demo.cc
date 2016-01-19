/** @file gradient-demo.cc

    Copyright 2015 J.A. / http://www.tsdconseil.fr

    Project web page: http://www.tsdconseil.fr/log/opencv/demo/index-en.html

    This file is part of OCVDemo.

    OCVDemo is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    OCVDemo is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with OCVDemo.  If not, see <http://www.gnu.org/licenses/>.
 **/

#include "demo-items/gradient-demo.hpp"
#include "gl.hpp"
#include "hough.hpp"
#include "opencv2/core/core.hpp"


ContourDemo::ContourDemo()
{
  props.id = "contours";
}

int ContourDemo::calcul(Node &model, cv::Mat &I)
{
  Mat detected_edges;
  int seuil_bas  = model.get_attribute_as_int("seuil-bas");
  int seuil_haut = model.get_attribute_as_int("seuil-haut");
  int norme      = model.get_attribute_as_int("norme");
  //int ratio = 3;
  int taille_noyau = 3;
  Mat tmp;
  cvtColor(I,tmp,CV_BGR2GRAY);
  out.O[0] = tmp;
  I = tmp;
  blur(tmp,tmp,Size(3,3));
  Canny(tmp, detected_edges, seuil_bas, seuil_haut, taille_noyau, norme == 1);
  /// Using Canny's output as a mask, we display our result
  out.O[2].create(I.size(), I.type());
  out.O[2] = Scalar::all(0);
  //I.copyTo(O[0], detected_edges);
  out.outname[1] = "Canny";
  out.outname[2] = "Contours";
  out.nout = 3;

  int kernel_width = 5;
  int kernel_type = MORPH_RECT;
  Mat K = getStructuringElement(kernel_type,
                                      Size(2*kernel_width + 1, 2*kernel_width+1),
                                      Point(kernel_width, kernel_width));


  //morphologyEx(detected_edges, detected_edges, MORPH_CLOSE, K);
  out.O[1] = detected_edges;

  int typCont = model.get_attribute_as_int("typcont");

  int mode = RETR_EXTERNAL;

  if(typCont == 1)
    mode = RETR_TREE;

  journal.trace("findcountours...");
  vector<vector<Point> > contours0;
  vector<Vec4i> hierarchy;
  findContours(detected_edges, contours0,
               hierarchy,
               mode,
               CHAIN_APPROX_SIMPLE,
               Point(0,0));
  journal.trace("ok.");

  RNG rng(12345);
  /// Draw contours
  Mat drawing = Mat::zeros(detected_edges.size(), CV_8UC3 );
  for(auto i = 0u; i < contours0.size(); i++ )
  {
   Scalar color = Scalar( rng.uniform(0, 255), rng.uniform(0,255), rng.uniform(0,255) );
   drawContours(drawing, contours0, i, color, 2, 8, hierarchy, 0, Point() );
  }

# if 0
  vector<vector<Point> > contours;
  contours.resize(contours0.size());
  for(size_t k = 0; k < contours0.size(); k++)
    cv::approxPolyDP(Mat(contours0[k]), contours[k], 3, true);

  const int w = 500;
  int nlev = model.get_attribute_as_int("nlev");
  //int levels = 3;
  Mat cnt_img = Mat::zeros(I.size(), CV_8UC3);
  //int _levels = nlev - 3;
  journal.trace("drawcountours...");
  drawContours(cnt_img, contours, /*_levels <= 0 ? 3 : */-1, Scalar(128,255,255),
                3, /*LINE_AA*/CV_AA, hierarchy, std::abs(nlev/*_levels*/) );
  journal.trace("ok.");
# endif
  out.O[2] = drawing;
  return 0;
}

NetDemo::NetDemo()
{
  props.id = "net";
  out.nout = 2;
}


int NetDemo::calcul(Node &model, cv::Mat &I)
{
  Mat I32, lap;
  out.O[0] = I;
  float c = model.get_attribute_as_float("coef");
  I.convertTo(I32, CV_32F);
  Laplacian(I, lap, CV_32F, 3);
  Mat sharp_image_32bit = I32 - c * lap;
  sharp_image_32bit.convertTo(out.O[1], CV_8U);
  return 0;
}

GradientDemo::GradientDemo()
{
  props.id = "gradient";
}


int GradientDemo::calcul(Node &model, cv::Mat &I)
{
  Mat gx, gy, agx, agy,tmp,grad;

  float sigma = model.get_attribute_as_float("gaussian-sigma/sigma");
  float gamma = model.get_attribute_as_float("deriche-gamma/gamma");
  int sel = model.get_attribute_as_int("sel");
  int sel2 = model.get_attribute_as_int("sel2");
  int sel3 = model.get_attribute_as_int("sel3");

  out.O[0] = I;

  //GaussianBlur(I,tmp, Size(3,3),0);
  if(sigma > 0)
  {
    if(sel3 == 0)
      GaussianBlur(I,tmp, Size(0,0),sigma);
    else
    {
      DericheBlur(I, tmp, gamma);
    }
  }
  else
    tmp = I.clone();
  cvtColor(tmp,tmp,CV_BGR2GRAY);
  if(sel2 == 0)
  {
    Sobel(tmp,gx,CV_16S,1,0,3,1,0);
    Sobel(tmp,gy,CV_16S,0,1,3,1,0);
  }
  else
  {
    Scharr(tmp,gx,CV_16S,1,0);
    Scharr(tmp,gy,CV_16S,0,1);
  }
  convertScaleAbs(gx,agx);
  convertScaleAbs(gy,agy);
  if(sel == 0)
  {
    addWeighted(agx, .5, agy, .5, 0, out.O[1]);
    out.nout = 2;
    out.outname[1] = langue.get_item("gabs");//"Gradient (valeur absolue)";
  }
  else
  {
    out.O[1] = agx;
    out.O[2] = agy;
    out.nout = 3;
    out.outname[1] = "GX";
    out.outname[2] = "GY";
  }
  return 0;
}

LaplaceDemo::LaplaceDemo()
{
  props.id = "laplace";
}


int LaplaceDemo::calcul(Node &model, cv::Mat &I)
{
  float sigma = model.get_attribute_as_float("sigma");

  Mat tmp,tmpg,tmp2;

  out.O[0] = I;

  if(sigma > 0)
    GaussianBlur(I,tmp, Size(0,0),sigma);
  else
    tmp = I.clone();

  out.nout = 2;
  out.outname[1] = "Laplacien";

  cvtColor(tmp,tmpg,CV_BGR2GRAY);
  Laplacian(tmpg, tmp2, CV_16S, 3, 1, 0);
  normalize(tmp2,tmp2,0,255,NORM_MINMAX);
  convertScaleAbs(tmp2, out.O[1]); // Conversion vers CV_8U
  return 0;
}


CannyDemo::CannyDemo()
{
  props.id = "canny";
  out.nout = 2;
}


int CannyDemo::calcul(Node &model, cv::Mat &I)
{
  Mat detected_edges;
  int seuil_bas  = model.get_attribute_as_int("seuil-bas");
  int seuil_haut = model.get_attribute_as_int("seuil-haut");
  int norme      = model.get_attribute_as_int("norme");
  //int ratio = 3;
  int taille_noyau = 3;
  Mat tmp;
  cvtColor(I,tmp,CV_BGR2GRAY);
  blur(tmp,tmp,Size(3,3));
  Canny(tmp, detected_edges, seuil_bas, seuil_haut, taille_noyau, norme == 1);
  /// Using Canny's output as a mask, we display our result
  out.O[0] = I;
  out.O[1].create(I.size(), I.type());
  out.O[1] = Scalar::all(0);
  I.copyTo(out.O[1], detected_edges);
  out.outname[1] = "Contours";
  return 0;
}

HoughDemo::HoughDemo()
{
  props.id = "hough";
}


int HoughDemo::calcul(Node &model, cv::Mat &I)
{
  float seuil = model.get_attribute_as_float("seuil");
  float seuil_canny = model.get_attribute_as_float("seuil-canny");
  Mat dst, bw;
  int ratio = 3;

  cv::cvtColor(I, bw, CV_BGR2GRAY);
  cv::blur(bw, bw, cv::Size(3, 3));
  Canny(bw, bw, seuil_canny, seuil_canny * ratio, 3);
  cvtColor(bw, out.O[1], CV_GRAY2BGR);

  int sel = model.get_attribute_as_int("sel");

  if(sel == 0)
  {
    out.O[2] = I.clone();
    out.nout = 3;
    vector<Vec2f> lines;
    HoughLines(bw, lines, 1, CV_PI/180, seuil, 0, 0);
    printf("Détecté %d lignes.\n", lines.size());
    for(size_t i = 0; i < lines.size(); i++ )
    {
      float rho = lines[i][0], theta = lines[i][1];
      Point pt1, pt2;
      double a = cos(theta), b = sin(theta);
      double x0 = a * rho, y0 = b * rho;
      pt1.x = cvRound(x0 + 1000 * (-b));
      pt1.y = cvRound(y0 + 1000 * (a));
      pt2.x = cvRound(x0 - 1000 * (-b));
      pt2.y = cvRound(y0 - 1000 * (a));
      line(out.O[2], pt1, pt2, Scalar(0,0,255), 3, CV_AA);
    }
  }
  else if(sel == 1)
  {
    out.O[2] = I.clone();
    out.nout = 3;
    std::vector<cv::Vec4i> lines;
    cv::HoughLinesP(bw, lines, 1, CV_PI/180, seuil,
          30, // min line length
          10); // max line gap
    for(size_t i = 0; i < lines.size(); i++ )
    {
      //float rho = lines[i][0], theta = lines[i][1];
      Point pt1, pt2;
      pt1.x = lines[i].val[0];
      pt1.y = lines[i].val[1];
      pt2.x = lines[i].val[2];
      pt2.y = lines[i].val[3];
      line(out.O[2], pt1, pt2, Scalar(0,0,255), 3, CV_AA);
    }
  }
  else if(sel == 2)
  {
    out.nout = 2;
    out.O[1] = I.clone();
    vector<Vec2f> lines;
    HoughLinesWithGradientDir(I, lines, 1, CV_PI/180);
    printf("Détecté %d lignes.\n", lines.size());
    for(size_t i = 0; i < lines.size(); i++ )
    {
      float rho = lines[i][0], theta = lines[i][1];
      Point pt1, pt2;
      double a = cos(theta), b = sin(theta);
      double x0 = a * rho, y0 = b * rho;
      pt1.x = cvRound(x0 + 1000 * (-b));
      pt1.y = cvRound(y0 + 1000 * (a));
      pt2.x = cvRound(x0 - 1000 * (-b));
      pt2.y = cvRound(y0 - 1000 * (a));
      line(out.O[1], pt1, pt2, Scalar(0,0,255), 3, CV_AA);
    }
  }

  return 0;
}

HoughCDemo::HoughCDemo()
{
  props.id = "houghc";
  out.nout = 2;
}


int HoughCDemo::calcul(Node &model, cv::Mat &I)
{
  Mat gris;
  //int ratio = 3;
  double seuil_canny = model.get_attribute_as_int("seuil-canny");
  //Canny(I, dst, seuil_canny, seuil_canny * ratio, 3);
  cvtColor(I, gris, CV_BGR2GRAY);
  out.O[1] = I.clone();
  vector<Vec3f> cercles;
  double seuil = model.get_attribute_as_int("seuil");
  int rmin = model.get_attribute_as_int("rmin");
  int rmax = model.get_attribute_as_int("rmax");
  HoughCircles(gris, cercles, CV_HOUGH_GRADIENT, 2 /* dp = 2 */,
      20 /* min dist */,
      seuil_canny,
      seuil,
      rmin,
      rmax);
  printf("Détecté %d cercles.\n", cercles.size());
  for(size_t i = 0; i < cercles.size(); i++ )
  {
    float xc = cercles[i][0], yc = cercles[i][1], r = cercles[i][2];
    cv::circle(out.O[1], Point(xc,yc), r, Scalar(0,255,0), 2);
  }

  return 0;
}

RectDemo::RectDemo()
{
  props.id = "hough-rec";
}


Point2f computeIntersect(cv::Vec4i a,
                         cv::Vec4i b)
{
  int x1 = a[0], y1 = a[1], x2 = a[2], y2 = a[3], x3 = b[0], y3 = b[1], x4 = b[2], y4 = b[3];
  //float denom;

  if (float d = ((float)(x1 - x2) * (y3 - y4)) - ((y1 - y2) * (x3 - x4)))
  {
    cv::Point2f pt;
    pt.x = ((x1 * y2 - y1 * x2) * (x3 - x4) - (x1 - x2) * (x3 * y4 - y3 * x4)) / d;
    pt.y = ((x1 * y2 - y1 * x2) * (y3 - y4) - (y1 - y2) * (x3 * y4 - y3 * x4)) / d;
    return pt;
  }
  else
    return cv::Point2f(-1, -1);
}


void sortCorners(std::vector<cv::Point2f>& corners,
                 cv::Point2f center)
{
  std::vector<cv::Point2f> top, bot;

  for(auto i = 0u; i < corners.size(); i++)
  {
    if (corners[i].y < center.y)
      top.push_back(corners[i]);
    else
      bot.push_back(corners[i]);
  }
  corners.clear();

  if (top.size() == 2 && bot.size() == 2){
    cv::Point2f tl = top[0].x > top[1].x ? top[1] : top[0];
    cv::Point2f tr = top[0].x > top[1].x ? top[0] : top[1];
    cv::Point2f bl = bot[0].x > bot[1].x ? bot[1] : bot[0];
    cv::Point2f br = bot[0].x > bot[1].x ? bot[0] : bot[1];


    corners.push_back(tl);
    corners.push_back(tr);
    corners.push_back(br);
    corners.push_back(bl);
  }
}


// D'après http://opencv-code.com/tutorials/automatic-perspective-correction-for-quadrilateral-objects/
int RectDemo::calcul(Node &model, cv::Mat &I)
{
  cv::Mat bw;
  cv::cvtColor(I, bw, CV_BGR2GRAY);
  cv::blur(bw, bw, cv::Size(3, 3));
  cv::Canny(bw, bw, 100, 100, 3);

  Mat I0 = I.clone();

  std::vector<cv::Vec4i> lines;
  cv::HoughLinesP(bw, lines, 1, CV_PI/180, 70, 30, 10);

  out.nout = 2;
  out.outname[0] = "Localisation quadrilatere";
  out.outname[1] = "Correction de perspective";

  // Expand the lines
  for(auto i = 0u; i < lines.size(); i++)
  {
    cv::Vec4i v = lines[i];
    lines[i][0] = 0;
    lines[i][1] = ((float)v[1] - v[3]) / (v[0] - v[2]) * -v[0] + v[1];
    lines[i][2] = I.cols;
    lines[i][3] = ((float)v[1] - v[3]) / (v[0] - v[2]) * (I.cols - v[2]) + v[3];
  }

  std::vector<cv::Point2f> corners;
  for(auto i = 0u; i < lines.size(); i++)
  {
    for (unsigned int j = i+1; j < lines.size(); j++)
    {
      cv::Point2f pt = computeIntersect(lines[i], lines[j]);
      if (pt.x >= 0 && pt.y >= 0)
        corners.push_back(pt);
    }
  }


  std::vector<cv::Point2f> approx;
  cv::approxPolyDP(cv::Mat(corners), approx, cv::arcLength(cv::Mat(corners), true) * 0.02, true);


  if (approx.size() != 4)
  {
    //errmsg = "obj-pas-quadri";
    //return -1;
    out.O[1] = Mat::zeros(I.size(), CV_8UC3);
    return 0;
  }

  cv::Point2f center(0,0);
  // Get mass center
  for (auto i = 0u; i < corners.size(); i++)
    center += corners[i];
  center *= (1. / corners.size());


  sortCorners(corners, center);
  if (corners.size() == 0)
  {
    //errmsg = "coins-non-tries-correctement";
    //return -1;
    out.O[1] = Mat::zeros(I.size(), CV_8UC3);
    return 0;
  }

  // Draw lines
  for(auto i = 0u; i < lines.size(); i++)
  {
    cv::Vec4i v = lines[i];
    cv::line(out.O[0], cv::Point(v[0], v[1]), cv::Point(v[2], v[3]), CV_RGB(0,255,0));
  }


  // Draw corner points
  cv::circle(out.O[0], corners[0], 3, CV_RGB(255,0,0), 2);
  cv::circle(out.O[0], corners[1], 3, CV_RGB(0,255,0), 2);
  cv::circle(out.O[0], corners[2], 3, CV_RGB(0,0,255), 2);
  cv::circle(out.O[0], corners[3], 3, CV_RGB(255,255,255), 2);

  // Draw mass center
  cv::circle(out.O[0], center, 3, CV_RGB(255,255,0), 2);


  cv::Mat quad = cv::Mat::zeros(300, 220, CV_8UC3);

  std::vector<cv::Point2f> quad_pts;
  quad_pts.push_back(cv::Point2f(0, 0));
  quad_pts.push_back(cv::Point2f(quad.cols, 0));
  quad_pts.push_back(cv::Point2f(quad.cols, quad.rows));
  quad_pts.push_back(cv::Point2f(0, quad.rows));


  cv::Mat transmtx = cv::getPerspectiveTransform(corners, quad_pts);
  cv::warpPerspective(I0, out.O[1], transmtx, quad.size());

  return 0;
}
