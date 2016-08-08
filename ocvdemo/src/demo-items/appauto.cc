/** @file appauto.cc

    Copyright 2016 J.A. / http://www.tsdconseil.fr

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



#include "demo-items/appauto.hpp"
#include <iostream>
#include <vector>
#include <assert.h>
#include <stdio.h>
#include <cmath>
#include <opencv2/opencv.hpp>

using namespace cv;
using namespace cv::ml;

DemoAppAuto::DemoAppAuto()
{
  props.id = "2d-2classes";
  props.input_min = 0;
  props.input_max = 0;
}


int DemoAppAuto::proceed(OCVDemoItemInput &entree, OCVDemoItemOutput &sortie)
{
  // (1) Génération d'un jeu de données
  auto n = 500u, ntraits = 2u;
  Mat mat_entrainement(Size(ntraits,n), CV_32F);

  uint32_t sx = 512, sy = 512;
  Mat O(Size(sx, sy), CV_8UC3, Scalar(255,255,255));
  Mat O2 = O.clone();

  RNG rng;
  Mat labels(Size(1,n), CV_32S);
  auto ptr = labels.ptr<int>();
  for(auto i = 0u; i < n; i++)
  {
    uint8_t classe = 0;
    float x = rng.uniform(0.0f, 2 * 3.1415926f);
    float y = rng.uniform(-1.0f, 1.0f);
    if(y >= std::sin(x))
      classe = 1;
    //x = x + rng.gaussian(0.02f);
    //y = y + rng.gaussian(0.02f);

    Scalar couleur = Scalar(255, 0, 0);
    if(classe == 1)
      couleur = Scalar(0, 255, 0);

    int xi = (x * sx / (2 * 3.1415926f));
    int yi = sy/2 + y * sy / 2;
    cv::line(O, Point(xi-5,yi), Point(xi+5,yi), couleur, 1, CV_AA);
    cv::line(O, Point(xi,yi-5), Point(xi,yi+5), couleur, 1, CV_AA);

    mat_entrainement.at<float>(i,0) = x;
    mat_entrainement.at<float>(i,1) = y;

    *ptr++ = classe;
  }

  // (2) Entrainement SVM

  Ptr<SVM> svm = SVM::create();
  svm->setTermCriteria(TermCriteria(CV_TERMCRIT_ITER | CV_TERMCRIT_EPS, 1000, 1e-3));
  svm->setGamma(1);
  svm->setKernel(SVM::RBF);
  svm->setNu(0.5);
  svm->setP(0.1);
  svm->setC(0.01);
  svm->setType(SVM::C_SVC);

  printf("Entrainement...\n"); fflush(stdout);
  svm->train(mat_entrainement, ROW_SAMPLE, labels);
  printf("Ok.\n"); fflush(stdout);

  Mat sv = svm->getSupportVectors();


  // (3) Affichage des vecteurs de support





  journal.verbose("échantillonnage du plan...");
  // (4) Classification (échantillonnage du plan)

  Mat traits(Size(2,1),CV_32F);

  Mat O3 = Mat(Size(sx,sy),CV_32F);

  for(auto y = 0u; y < sy; y++)
  {
    for(auto x = 0u; x < sx; x++)
    {
      traits.at<float>(0,0) = ((float) x * 2 * 3.1415926) / sx;
      traits.at<float>(0,1) = (((float) y) - sy/2) / (sy / 2);
      float res = svm->predict(traits);
      Scalar couleur = Scalar(255, 0, 0);
      if(res != 0)
        couleur = Scalar(0, 255, 0);
      for(auto j = 0u; j < 3u; j++)
        O2.at<Vec3b>(y,x)[j] = couleur[j];
      O3.at<float>(y,x) = res;
    }
  }

  double minv, maxv;
  cv::minMaxLoc(O3, &minv, &maxv, nullptr, nullptr);
  journal.verbose("Min class = %f, max class = %f.", minv, maxv);

  cv::normalize(O3, O3, 0, 1, NORM_MINMAX);

  sortie.nout = 2;
  sortie.images[0] = O;
  sortie.names[0]  = "Entrainement";
  sortie.images[1] = O2;
  sortie.names[1]  = "SVM (classication du plan)";
  //sortie.images[2] = O2;
  //sortie.names[2]  = "SVM (classication du plan)";

  return 0;
}





