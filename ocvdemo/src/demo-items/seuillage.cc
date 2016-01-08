/** @file seuillage.cc

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

#include "demo-items/seuillage.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/photo/photo.hpp"

InpaintDemo::InpaintDemo()
{
  props.id = "inpaint";
  props.requiert_masque = true;
}

int InpaintDemo::calcul(Node &model, cv::Mat &I)
{
  sortie.nout = 2;
  if(params.masque.data != nullptr)
    cv::inpaint(I, params.masque, sortie.O[1], 3, CV_INPAINT_TELEA);
  else
    sortie.O[1] = I.clone();
  return 0;
}


WShedDemo::WShedDemo()
{
  props.id = "watershed";
  sortie.nout = 3;
}

int WShedDemo::calcul(Node &model, cv::Mat &I)
{
  Mat gris, nb, ret;
  cvtColor(I, gris, CV_BGR2GRAY);
  threshold(gris,nb,0,255,CV_THRESH_BINARY_INV | CV_THRESH_OTSU);
  //Execute morphological-open
  morphologyEx(nb,ret,MORPH_OPEN,Mat::ones(9,9,CV_8SC1),Point(4,4),2);
  Mat distTransformed(ret.rows,ret.cols,CV_32F);
  distanceTransform(ret,distTransformed,CV_DIST_L2,3);
  //normalize the transformed image in order to display
  normalize(distTransformed, distTransformed, 0.0, 255.0, NORM_MINMAX);

  sortie.nout = 4;
  sortie.O[1] = distTransformed.clone();

  sortie.outname[1] = "Distance Transformation";
  //imshow("Distance Transformation",distTransformed);
  //threshold the transformed image to obtain markers for watershed
  threshold(distTransformed,distTransformed,0.7 * 255,255,CV_THRESH_BINARY);

  distTransformed.convertTo(distTransformed,CV_8UC1);
  sortie.O[2] = distTransformed.clone();
  sortie.outname[2] = "Thresholded dist. trans.";
  //imshow("Thresholded Distance Transformation",distTransformed);


  vector<vector<Point> > contours;
  vector<Vec4i> hierarchy;
  findContours(distTransformed, contours, hierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE);

  if(contours.empty())
    return -1;
  Mat markers(distTransformed.size(), CV_32S);
  markers = Scalar::all(0);
  int idx, compCount = 0;
  //draw contours

  for(idx = 0; idx >= 0; idx = hierarchy[idx][0], compCount++)
    drawContours(markers, contours, idx, Scalar::all(compCount+1), -1, 8, hierarchy, INT_MAX);

  vector<Vec3b> colorTab;
  for(auto i = 0; i < compCount; i++ )
  {
    int b = theRNG().uniform(0, 255);
    int g = theRNG().uniform(0, 255);
    int r = theRNG().uniform(0, 255);
    colorTab.push_back(Vec3b((uchar)b, (uchar)g, (uchar)r));
  }

  //apply watershed with the markers as seeds

  watershed(I, markers);

  sortie.O[3] = Mat(markers.size(), CV_8UC3);
  journal.trace("paint the watershed image...");
  for(auto i = 0; i < markers.rows; i++)
  {
    for(auto j = 0; j < markers.cols; j++)
    {
      int index = markers.at<int>(i,j);
      if(index == -1)
        sortie.O[3].at<Vec3b>(i,j) = Vec3b(255,255,255);
      else if((index <= 0) || (index > compCount))
        sortie.O[3].at<Vec3b>(i,j) = Vec3b(0,0,0);
      else
        sortie.O[3].at<Vec3b>(i,j) = colorTab[index - 1];
    }
  }
  sortie.O[3] = sortie.O[3] * 0.5 + I * 0.5;
  return 0;
}

GrabCutDemo::GrabCutDemo()
{
  props.id = "grabcut";
  props.requiert_roi = true;
  params.roi.x = 137;
  params.roi.y = 5;
  params.roi.width = 251 - 137;
  params.roi.height = 130 - 5;
}

int GrabCutDemo::calcul(Node &model, cv::Mat &I)
{
  Mat mask, bgmodel, fgmodel;
  mask = Mat::zeros(I.size(), CV_8UC1);
  journal.verbose("grabcut...");
  Mat Id;
  cv::pyrDown(I, Id);
  cv::Rect roi2;
  roi2.x = params.roi.x / 2;
  roi2.y = params.roi.y / 2;
  roi2.width = params.roi.width / 2;
  roi2.height = params.roi.height / 2;
  cv::grabCut(Id, mask, roi2, bgmodel, fgmodel, 1, GC_INIT_WITH_RECT);
  //cv::pyrUp(mask, mask);

  journal.verbose("masque...");
  sortie.O[1] = I.clone();
  sortie.nout = 2;

  //Point3_<uchar> *p = O.ptr<Point3_<uchar>>(0,0);
  for(auto y = 0; y < I.rows; y++)
  {
    for(auto x = 0; x < I.cols; x++)
    {
      uint8_t msk = mask.at<uint8_t>(Point(x/2,y/2));
      if((msk == GC_BGD) || (msk == GC_PR_BGD))
      {
        Vec3b &pix = sortie.O[1].at<Vec3b>(Point(x,y));
        pix[0] = 0;
        pix[1] = 0;
        pix[2] = 0;
      }
    }
  }
  journal.verbose("terminé.");

  return 0;
}

Seuillage::Seuillage()
{
  props.id = "seuillage";
}

int Seuillage::calcul(Node &model, cv::Mat &I)
{
  int sel = model.get_attribute_as_int("sel");

  sortie.nout = 2;

  Mat Ig;
  cvtColor(I, Ig, CV_BGR2GRAY);
  //I = Ig;

  // Fixe
  if(sel == 0)
  {
    int seuil = model.get_attribute_as_int("seuillage-fixe/seuil");
    threshold(Ig, sortie.O[1], seuil, 255, THRESH_BINARY_INV);
  }
  // Otsu
  else if(sel == 1)
  {
    threshold(Ig, sortie.O[1], 0 /* non utilisé */,
              255, THRESH_BINARY_INV | THRESH_OTSU);
  }
  // Adaptatif
  else if(sel == 2)
  {
    int taille_bloc = model.get_attribute_as_int("seuillage-adaptatif/taille-bloc");
    int seuil = model.get_attribute_as_int("seuillage-adaptatif/seuil");
    if((taille_bloc & 1) == 0)
      taille_bloc++;
    cv::adaptiveThreshold(Ig, sortie.O[1], 255,
        ADAPTIVE_THRESH_GAUSSIAN_C, THRESH_BINARY_INV,
        taille_bloc, seuil);
    // ou ADAPTIVE_THRESH_MEAN_C
  }
  //cv::bitwise_and(Ig, O, O);
  cvtColor(sortie.O[1], sortie.O[1], CV_GRAY2BGR);
  return 0;
}

DTransDemo::DTransDemo()
{
  props.id = "dtrans";
}

int DTransDemo::calcul(Node &model, cv::Mat &I)
{
  Mat Ig, tmp;
  cvtColor(I, Ig, CV_BGR2GRAY);
  threshold(Ig, sortie.O[1], 0 /* non utilisé */,
                255, THRESH_BINARY_INV | THRESH_OTSU);
  journal.trace("dtrans...");
  sortie.nout = 2;
  cv::distanceTransform(sortie.O[1], tmp, CV_DIST_L2, 3);
  journal.trace("ok.");
  normalize(tmp, sortie.O[1], 0, 255, NORM_MINMAX, CV_8UC1);
  return 0;
}


