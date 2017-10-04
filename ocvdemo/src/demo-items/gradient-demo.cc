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


int calcule_canny(const cv::Mat &I, cv::Mat &masque_canny,
                  const utils::model::Node &modele)
{
  Mat tmp;
  cvtColor(I, tmp, CV_BGR2GRAY);

  int seuil_methode = modele.get_attribute_as_int("seuil-methode");
  int seuil_bas     = modele.get_attribute_as_int("seuil-bas");
  int seuil_haut    = modele.get_attribute_as_int("seuil-haut");
  int norme         = modele.get_attribute_as_int("norme");
  int taille_noyau  = modele.get_attribute_as_int("taille-noyau");
  bool prefiltrage  = modele.get_attribute_as_boolean("prefiltrage");
  int taille_prefilt = modele.get_attribute_as_int("taille-noyau-prefiltrage");


  if(prefiltrage)
    blur(tmp, tmp, Size(taille_prefilt,taille_prefilt));

  if(seuil_methode == 1)
  {
    cv::Scalar moyenne, sigma;
    cv::meanStdDev(tmp, moyenne, sigma);
    seuil_bas = moyenne[0] - sigma[0];
    seuil_haut = moyenne[0] + sigma[0];
  }

  //infos("Canny: seuils = %d, %d.", seuil_bas, seuil_haut);
  if(taille_noyau < 3)
    taille_noyau = 3;

  if((taille_noyau & 1) == 0)
    taille_noyau++;

  Canny(tmp, masque_canny, seuil_bas, seuil_haut, taille_noyau, norme == 1);

  return 0;
}

int ContourDemo::proceed(OCVDemoItemInput &input, OCVDemoItemOutput &output)
{
  Mat masque_canny;
  calcule_canny(input.images[0], masque_canny, input.model);

  output.names[0] = "Canny";
  output.names[1] = "Contours";
  output.nout = 2;

  /*int kernel_width = 5;
  int kernel_type = MORPH_RECT;
  Mat K = getStructuringElement(kernel_type,
                                      Size(2*kernel_width + 1, 2*kernel_width+1),
                                      Point(kernel_width, kernel_width));


  morphologyEx(detected_edges, detected_edges, MORPH_CLOSE, K);*/

  output.images[0] = masque_canny;

  int type_contour = input.model.get_attribute_as_int("typcont");

  int mode = RETR_EXTERNAL;
  if(type_contour == 1)
    mode = RETR_TREE;

  std::vector<std::vector<cv::Point> > contours;
  std::vector<Vec4i> hierarchie;
  findContours(masque_canny, contours,
               hierarchie,
               mode,
               CHAIN_APPROX_SIMPLE,
               Point(0,0));

  RNG rng(12345);
  // Dessine les contours
  Mat dessin = Mat::zeros(masque_canny.size(), CV_8UC3 );
  for(auto i = 0u; i < contours.size(); i++ )
  {
   Scalar couleur = Scalar(rng.uniform(0, 255), rng.uniform(0,255), rng.uniform(0,255));
   drawContours(dessin, contours, i, couleur, 2, 8, hierarchie, 0, Point());
  }

  output.images[1] = dessin;
  return 0;
}

NetDemo::NetDemo()
{
  props.id = "net";
}


int NetDemo::proceed(OCVDemoItemInput &input, OCVDemoItemOutput &output)
{
  Mat I32, lap;
  auto I = input.images[0].clone();
  float c = input.model.get_attribute_as_float("coef");
  I.convertTo(I32, CV_32F);
  Laplacian(I, lap, CV_32F, 3);
  Mat sharp_image_32bit = I32 - c * lap;
  sharp_image_32bit.convertTo(output.images[0], CV_8U);
  return 0;
}

GradientDemo::GradientDemo()
{
  props.id = "gradient";
}


int GradientDemo::proceed(OCVDemoItemInput &input, OCVDemoItemOutput &output)
{
  Mat gx, gy, agx, agy,tmp,grad;

  auto model = input.model;
  auto I = input.images[0];
  float sigma = model.get_attribute_as_float("gaussian-sigma/sigma");
  float gamma = model.get_attribute_as_float("deriche-gamma/gamma");
  int sel = model.get_attribute_as_int("sel");
  int sel2 = model.get_attribute_as_int("sel2");
  int sel3 = model.get_attribute_as_int("sel3");
  int tnoyau = model.get_attribute_as_int("taille-noyau");

  if((tnoyau & 1) == 0)
    tnoyau++;

  //GaussianBlur(I,tmp, Size(3,3),0);
  if(sigma > 0)
  {
    if(sel3 == 0)
      GaussianBlur(I,tmp, Size(0,0),sigma);
    else
    {
      ocvext::Deriche_blur(I, tmp, gamma);
    }
  }
  else
    tmp = I.clone();


  bool preconv = input.model.get_attribute_as_boolean("preconv");
  bool gradient_couleur = input.model.get_attribute_as_boolean("couleur");

  if(preconv)
    cvtColor(tmp,tmp,CV_BGR2GRAY);

  if(sel2 == 0)
  {
    Sobel(tmp,gx,CV_32F,1,0,tnoyau,1,0);
    Sobel(tmp,gy,CV_32F,0,1,tnoyau,1,0);
  }
  else
  {
    Scharr(tmp,gx,CV_32F,1,0);
    Scharr(tmp,gy,CV_32F,0,1);
  }

  if(!preconv && !gradient_couleur)
  {
    infos("Gradient couleur -> abs...");
    gx = cv::abs(gx);
    gy = cv::abs(gy);
    gx = gx / 255;
    gy = gy / 255;
    cvtColor(gx,gx,CV_BGR2GRAY);
    cvtColor(gy,gy,CV_BGR2GRAY);
  }

  if(sel == 0)
  {
    output.nout = 1;
    cv::Mat mag, angle;

    cv::cartToPolar(gx, gy, mag, angle);
    //addWeighted(agx, .5, agy, .5, 0, output.images[0]);
    cv::normalize(mag, output.images[0], 0, 255, cv::NORM_MINMAX);
    output.names[0] = utils::langue.get_item("gabs");
  }
  else
  {
    convertScaleAbs(gx,agx);
    convertScaleAbs(gy,agy);
    output.nout = 2;
    output.images[0] = agx;
    output.images[1] = agy;
    output.names[0] = "GX";
    output.names[1] = "GY";
  }
  return 0;
}

DetFlouDemo::DetFlouDemo()
{
  props.id = "det-flou";
}


int DetFlouDemo::proceed(OCVDemoItemInput &input, OCVDemoItemOutput &output)
{
  int taille_noyau = input.model.get_attribute_as_int("taille-noyau");
  int BS = input.model.get_attribute_as_int("taille-bloc");

  if((taille_noyau & 1) == 0)
    taille_noyau++;

  Mat tmp, lap;
  cv::cvtColor(input.images[0], tmp, CV_BGR2GRAY);

  cv::Laplacian(tmp, lap, CV_32F, taille_noyau, 1, 0);

  //lap = cv::abs(lap);

  cv::Scalar m, d;
  cv::meanStdDev(lap, m, d);
  //float score_global = d[0];

  cv::Mat res = cv::Mat::zeros(lap.rows / BS, lap.cols / BS, CV_32F);

  unsigned int yo = 0;
  for(auto y = 0u; y + BS < (unsigned int) lap.rows; y += BS)
  {
    unsigned int xo = 0;
    for(auto x = 0u; x + BS < (unsigned int) lap.cols; x += BS)
    {
      cv::meanStdDev(lap(cv::Rect(x,y,BS,BS)), m, d);
      res.at<float>(yo,xo) = d[0];
      xo++;
    }
    yo++;
  }

  cv::normalize(res, res, 0, 255, cv::NORM_MINMAX);

  /*while()
  {
    cv::pyrUp(res, res);
  }*/
  cv::resize(res, res, cv::Size(lap.cols, lap.rows));

  output.images[0] = res;

  //cv::normalize(lap, output.images[0], 0, 255, NORM_MINMAX);
  //convertScaleAbs(lap, output.images[0]); // Conversion vers CV_8U
  output.names[0] = "Laplacien";
  return 0;
}

LaplaceDemo::LaplaceDemo()
{
  props.id = "laplace";
}


int LaplaceDemo::proceed(OCVDemoItemInput &input, OCVDemoItemOutput &output)
{
  bool pref = input.model.get_attribute_as_boolean("prefiltrer");
  float sigma = input.model.get_attribute_as_float("sigma");
  int aff = input.model.get_attribute_as_int("aff");
  int taille_noyau = input.model.get_attribute_as_int("taille-noyau");

  if((taille_noyau & 1) == 0)
    taille_noyau++;

  Mat tmp, lap;
  cv::cvtColor(input.images[0], tmp, CV_BGR2GRAY);

  if(pref && (sigma > 0))
    GaussianBlur(tmp, tmp, Size(0,0), sigma);

  cv::Laplacian(tmp, lap, CV_32F, taille_noyau, 1, 0);

  if(aff == 0)
    lap = cv::abs(lap);

  cv::normalize(lap, output.images[0], 0, 255, NORM_MINMAX);
  //convertScaleAbs(lap, output.images[0]); // Conversion vers CV_8U
  output.names[0] = "Laplacien";
  return 0;
}


CannyDemo::CannyDemo()
{
  props.id = "canny";
}


int CannyDemo::proceed(OCVDemoItemInput &input, OCVDemoItemOutput &output)
{
  calcule_canny(input.images[0], output.images[0], input.model);
  output.names[0] = "Contours";
  return 0;
}


HoughDemo::HoughDemo()
{
  props.id = "hough";
}


static void dessine_ligne(cv::Mat &I, float rho, float theta)
{
  Point pt1, pt2;
  double a = cos(theta), b = sin(theta);
  double x0 = a * rho, y0 = b * rho;
  pt1.x = cvRound(x0 + 1000 * (-b));
  pt1.y = cvRound(y0 + 1000 * (a));
  pt2.x = cvRound(x0 - 1000 * (-b));
  pt2.y = cvRound(y0 - 1000 * (a));
  line(I, pt1, pt2, Scalar(0,0,255), 3, CV_AA);
}


int HoughDemo::proceed(OCVDemoItemInput &input, OCVDemoItemOutput &output)
{
  float reso_rho = input.model.get_attribute_as_float("reso-rho");
  float reso_theta = input.model.get_attribute_as_float("reso-theta") * CV_PI / 180.0;
  float seuil = input.model.get_attribute_as_float("seuil");
  float seuilg = input.model.get_attribute_as_float("seuilg");
  float seuil_canny = input.model.get_attribute_as_float("seuil-canny");
  Mat dst, bw;
  int ratio = 3;

  if((reso_theta <= 0) || (reso_rho <= 0))
  {
    output.nout = 0;
    output.errmsg = "Les resolutions en rho et theta doivent etre positives.";
    return -1;
  }

  auto I = input.images[0];
  cv::cvtColor(I, bw, CV_BGR2GRAY);
  cv::blur(bw, bw, cv::Size(3, 3));
  Canny(bw, bw, seuil_canny, seuil_canny * ratio, 3);
  cvtColor(bw, output.images[0], CV_GRAY2BGR);

  int sel = input.model.get_attribute_as_int("sel");

  if(sel == 0)
  {
    output.images[1] = I.clone();
    output.nout = 2;
    std::vector<Vec2f> lignes;
    HoughLines(bw, lignes, reso_rho, reso_theta, seuil, 0, 0);
    infos("Détecté %d lignes.\n", (int) lignes.size());
    for(const auto &l: lignes)
      dessine_ligne(output.images[1], l[0], l[1]);
  }
  else if(sel == 1)
  {
    output.images[1] = I.clone();
    output.nout = 2;
    std::vector<cv::Vec4i> lines;
    cv::HoughLinesP(bw, lines, reso_rho, reso_theta, seuil,
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
      line(output.images[1], pt1, pt2, Scalar(0,0,255), 3, CV_AA);
    }
  }
  else if(sel == 2)
  {
    output.nout = 3;
    output.images[0] = I.clone();

    std::vector<Vec2f> lignes;
    cv::Mat deb;
    ocvext::Hough_lines_with_gradient_dir(I, lignes, deb, reso_rho, reso_theta, 0.6, seuilg);
    cv::pyrUp(deb, deb);
    output.images[1] = deb.t();
    output.names[1] = "Espace parametrique";
    infos("Détecté %d lignes.\n", (int) lignes.size());
    output.images[2] = I.clone();
    for(const auto &l: lignes)
      dessine_ligne(output.images[2], l[0], l[1]);
  }

  return 0;
}

HoughCDemo::HoughCDemo()
{
  props.id = "houghc";
}


int HoughCDemo::proceed(OCVDemoItemInput &input, OCVDemoItemOutput &output)
{
  Mat gris;
  double seuil_canny = input.model.get_attribute_as_int("seuil-canny");

  auto I = input.images[0];
  cvtColor(I, gris, CV_BGR2GRAY);
  output.images[0] = I.clone();
  std::vector<Vec3f> cercles;
  double seuil = input.model.get_attribute_as_int("seuil");
  int rmin = input.model.get_attribute_as_int("rmin");
  int rmax = input.model.get_attribute_as_int("rmax");
  HoughCircles(gris, cercles, CV_HOUGH_GRADIENT, 2 /* dp = 2 */,
      20 /* min dist */,
      seuil_canny,
      seuil,
      rmin,
      rmax);
  infos("Détecté %d cercles.\n", (int) cercles.size());
  for(size_t i = 0; i < cercles.size(); i++ )
  {
    float xc = cercles[i][0], yc = cercles[i][1], r = cercles[i][2];
    cv::circle(output.images[0], Point(xc,yc), r, Scalar(0,255,0), 2);
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
int RectDemo::proceed(OCVDemoItemInput &input, OCVDemoItemOutput &output)
{
  cv::Mat bw;
  auto I = input.images[0];
  cv::cvtColor(I, bw, CV_BGR2GRAY);
  cv::blur(bw, bw, cv::Size(3, 3));
  cv::Canny(bw, bw, 100, 100, 3);

  Mat I0 = I.clone();

  std::vector<cv::Vec4i> lines;
  cv::HoughLinesP(bw, lines, 1, CV_PI/180, 70, 30, 10);

  output.nout = 2;
  output.names[0] = "Localisation quadrilatere";
  output.names[1] = "Correction de perspective";

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
    output.images[0] = Mat::zeros(I.size(), CV_8UC3);
    return 0;
  }

  cv::Point2f center(0,0);
  // Get mass center
  for (auto i = 0u; i < corners.size(); i++)
    center += corners[i];
  center *= (1. / corners.size());

  output.images[0] = input.images[0].clone();
  output.images[1] = Mat::zeros(I.size(), CV_8UC3);

  sortCorners(corners, center);
  if (corners.size() == 0)
  {
    //errmsg = "coins-non-tries-correctement";
    //return -1;
    return 0;
  }

  // Draw lines
  for(auto i = 0u; i < lines.size(); i++)
  {
    cv::Vec4i v = lines[i];
    cv::line(output.images[0], cv::Point(v[0], v[1]), cv::Point(v[2], v[3]), CV_RGB(0,255,0));
  }

  // Draw corner points
  cv::circle(output.images[0], corners[0], 3, CV_RGB(255,0,0), 2);
  cv::circle(output.images[0], corners[1], 3, CV_RGB(0,255,0), 2);
  cv::circle(output.images[0], corners[2], 3, CV_RGB(0,0,255), 2);
  cv::circle(output.images[0], corners[3], 3, CV_RGB(255,255,255), 2);

  // Draw mass center
  cv::circle(output.images[0], center, 3, CV_RGB(255,255,0), 2);


  cv::Mat quad = cv::Mat::zeros(300, 220, CV_8UC3);

  std::vector<cv::Point2f> quad_pts;
  quad_pts.push_back(cv::Point2f(0, 0));
  quad_pts.push_back(cv::Point2f(quad.cols, 0));
  quad_pts.push_back(cv::Point2f(quad.cols, quad.rows));
  quad_pts.push_back(cv::Point2f(0, quad.rows));


  cv::Mat transmtx = cv::getPerspectiveTransform(corners, quad_pts);
  cv::warpPerspective(I0, output.images[1], transmtx, quad.size());

  return 0;
}
