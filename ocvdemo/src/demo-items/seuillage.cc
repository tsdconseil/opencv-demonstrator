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

int InpaintDemo::proceed(OCVDemoItemInput &input, OCVDemoItemOutput &output)
{
  output.nout = 2;
  auto I = input.images[0];
  output.images[0] = I.clone();
  if(input.mask.data != nullptr)
    cv::inpaint(I, input.mask, output.images[1], 3, CV_INPAINT_TELEA);
  else
    output.images[1] = I.clone();
  return 0;
}


WShedDemo::WShedDemo()
{
  props.id = "watershed";
  output.nout = 3;
}

int WShedDemo::proceed(OCVDemoItemInput &input, OCVDemoItemOutput &output)
{
  Mat gray, nb, ret;
  cvtColor(input.images[0], gray, CV_BGR2GRAY);
  threshold(gray,nb,0,255,CV_THRESH_BINARY_INV | CV_THRESH_OTSU);
  //Execute morphological-open
  morphologyEx(nb,ret,MORPH_OPEN,Mat::ones(9,9,CV_8SC1),Point(4,4),2);
  Mat distTransformed(ret.rows,ret.cols,CV_32F);
  distanceTransform(ret,distTransformed,CV_DIST_L2,3);
  //normalize the transformed image in order to display
  normalize(distTransformed, distTransformed, 0.0, 255.0, NORM_MINMAX);

  output.nout = 4;
  output.images[0] = input.images[0];
  output.images[1] = distTransformed.clone();
  output.names[1] = "Distance Transformation";

  //threshold the transformed image to obtain markers for watershed
  threshold(distTransformed,distTransformed,0.7 * 255,255,CV_THRESH_BINARY);
  distTransformed.convertTo(distTransformed,CV_8UC1);
  output.images[2] = distTransformed.clone();
  output.names[2] = "Thresholded dist. trans.";
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
  watershed(input.images[0], markers);

  output.images[3] = Mat(markers.size(), CV_8UC3);
  journal.trace("paint the watershed image...");
  for(auto i = 0; i < markers.rows; i++)
  {
    for(auto j = 0; j < markers.cols; j++)
    {
      int index = markers.at<int>(i,j);
      if(index == -1)
        output.images[3].at<Vec3b>(i,j) = Vec3b(255,255,255);
      else if((index <= 0) || (index > compCount))
        output.images[3].at<Vec3b>(i,j) = Vec3b(0,0,0);
      else
        output.images[3].at<Vec3b>(i,j) = colorTab[index - 1];
    }
  }
  output.images[3] = output.images[3] * 0.5 + input.images[0] * 0.5;
  return 0;
}

GrabCutDemo::GrabCutDemo()
{
  props.id = "grabcut";
  props.requiert_roi = true;
  input.roi.x = 137;
  input.roi.y = 5;
  input.roi.width = 251 - 137;
  input.roi.height = 130 - 5;
}

int GrabCutDemo::proceed(OCVDemoItemInput &input, OCVDemoItemOutput &output)
{
  Mat mask, bgmodel, fgmodel;
  Mat I = input.images[0];
  mask = Mat::zeros(I.size(), CV_8UC1);
  journal.verbose("grabcut...");
  Mat Id;
  cv::pyrDown(I, Id);
  cv::Rect roi2;
  roi2.x = input.roi.x / 2;
  roi2.y = input.roi.y / 2;
  roi2.width = input.roi.width / 2;
  roi2.height = input.roi.height / 2;
  cv::grabCut(Id, mask, roi2, bgmodel, fgmodel, 1, GC_INIT_WITH_RECT);
  //cv::pyrUp(mask, mask);

  journal.verbose("masque...");
  output.nout = 2;
  output.images[0] = I.clone();
  output.images[1] = I.clone();


  //Point3_<uchar> *p = O.ptr<Point3_<uchar>>(0,0);
  for(auto y = 0; y < I.rows; y++)
  {
    for(auto x = 0; x < I.cols; x++)
    {
      uint8_t msk = mask.at<uint8_t>(Point(x/2,y/2));
      if((msk == GC_BGD) || (msk == GC_PR_BGD))
      {
        Vec3b &pix = output.images[1].at<Vec3b>(Point(x,y));
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

int Seuillage::proceed(OCVDemoItemInput &input, OCVDemoItemOutput &output)
{
  int sel = input.model.get_attribute_as_int("sel");

  auto I = input.images[0];
  Mat Ig;
  cvtColor(I, Ig, CV_BGR2GRAY);

  // Fixe
  if(sel == 0)
  {
    int seuil = input.model.get_attribute_as_int("seuillage-fixe/seuil");
    threshold(Ig, output.images[0], seuil, 255, THRESH_BINARY_INV);
  }
  // Otsu
  else if(sel == 1)
  {
    threshold(Ig, output.images[0], 0 /* non utilisé */,
              255, THRESH_BINARY_INV | THRESH_OTSU);
  }
  // Adaptatif
  else if(sel == 2)
  {
    int taille_bloc = input.model.get_attribute_as_int("seuillage-adaptatif/taille-bloc");
    int seuil = input.model.get_attribute_as_int("seuillage-adaptatif/seuil");
    if((taille_bloc & 1) == 0)
      taille_bloc++;
    cv::adaptiveThreshold(Ig, output.images[0], 255,
        ADAPTIVE_THRESH_GAUSSIAN_C, THRESH_BINARY_INV,
        taille_bloc, seuil);
    // ou ADAPTIVE_THRESH_MEAN_C
  }
  return 0;
}

DTransDemo::DTransDemo()
{
  props.id = "dtrans";
}

int DTransDemo::proceed(OCVDemoItemInput &input, OCVDemoItemOutput &output)
{
  Mat Ig, tmp;
  cvtColor(input.images[0], Ig, CV_BGR2GRAY);
  threshold(Ig, output.images[0], 0 /* non utilisé */,
                255, THRESH_BINARY_INV | THRESH_OTSU);
  journal.trace("dtrans...");
  cv::distanceTransform(output.images[0], tmp, CV_DIST_L2, 3);
  journal.trace("ok.");
  normalize(tmp, output.images[0], 0, 255, NORM_MINMAX, CV_8UC1);
  return 0;
}


