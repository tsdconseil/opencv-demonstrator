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
  uint32_t n = entree.model.get_attribute_as_int("napp"),
      ntraits = 2u;
  Mat mat_entrainement(Size(ntraits,n), CV_32F);

  uint32_t sx = 512, sy = 512;
  uint32_t sx2 = 32, sy2 = 32;
  Mat O(Size(sx, sy), CV_8UC3, Scalar(255,255,255));
  Mat O2(Size(sx2, sy2), CV_8UC3);

  RNG rng;
  Mat labels(Size(1,n), CV_32S);
  auto ptr = labels.ptr<int>();

  auto *optr = mat_entrainement.ptr<float>();

  uint16_t dl = (5 * 512) / sx;

  journal.verbose("Generation jeu de donnees...");
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

    cv::line(O, Point(xi-dl,yi), Point(xi+dl,yi), couleur, 1, CV_AA);
    cv::line(O, Point(xi,yi-dl), Point(xi,yi+dl), couleur, 1, CV_AA);

    *optr++ = x;
    *optr++ = y;
    //mat_entrainement.at<float>(i,0) = x;
    //mat_entrainement.at<float>(i,1) = y;

    *ptr++ = classe;
  }
  journal.verbose("ok.");

  // (2) Entrainement SVM

  float gamma = input.model.get_attribute_as_float("svm/svm-rbf/gamma");
  if(gamma == 0)
    gamma = 1.0e-10;
  float C = input.model.get_attribute_as_float("svm/C");
  if(C <= 0)
    C = 1.0e-10;

  int kernel = input.model.get_attribute_as_int("svm/kernel");
  int degre = input.model.get_attribute_as_int("svm/svm-poly/degre");

  journal.verbose("Gamma = %f, C = %f, kernel = %d.", gamma, C, kernel);

  Ptr<SVM> svm = SVM::create();
  svm->setTermCriteria(TermCriteria(CV_TERMCRIT_ITER | CV_TERMCRIT_EPS, 1000, 1e-3));
  svm->setGamma(gamma);
  svm->setKernel((SVM::KernelTypes) kernel); //SVM::RBF);
  svm->setNu(0.5);
  svm->setP(0.1);
  svm->setC(C);//0.01);
  svm->setType(SVM::C_SVC);
  svm->setDegree(degre);

  journal.verbose("Entrainement...");
  svm->train(mat_entrainement, ROW_SAMPLE, labels);
  journal.verbose("Ok.");

  //Mat sv = svm->getSupportVectors();


  // (3) Affichage des vecteurs de support





  journal.verbose("Echantillonnage du plan...");
  // (4) Classification (échantillonnage du plan)



  //Mat O3 = Mat(Size(sx,sy),CV_32F);
  //float *o3_ptr = O3.ptr<float>();

  Vec3b *o2ptr = O2.ptr<Vec3b>();

  Vec3b c[2];
  c[0][0] = 255;
  c[0][1] = 0;
  c[0][2] = 0;
  c[1][0] = 0;
  c[1][1] = 255;
  c[1][2] = 0;



# if 0
  journal.verbose("Creation matrice de traits...");
  Mat traits(Size(2,sx2*sy2),CV_32F);
  float *tptr = traits.ptr<float>();
  for(auto y = 0u; y < sy2; y++)
  {
    for(auto x = 0u; x < sx2; x++)
    {
      *tptr++ = ((float) x * 2 * 3.1415926) / sx2;
      *tptr++ = (((float) y) - sy2/2) / (sy2 / 2);
    }
  }

  journal.verbose("Prediction SVM...");


  cv::Mat res(Size(1,sx2*sy2), CV_32S);
  svm->predict(traits, res);

  journal.verbose("Mise en forme des resultats...");
  int *iptr = res.ptr<int>();
  for(auto y = 0u; y < sy2; y++)
  {
    for(auto x = 0u; x < sx2; x++)
    {
      int res = *iptr++;
      assert((res == 0) || (res == 1));
      *o2ptr++ = c[res];
    }
  }
# endif

# if 1

  Mat traits(Size(2,1),CV_32F);
  float *tptr = traits.ptr<float>();
  for(auto y = 0u; y < sy2; y++)
  {
    for(auto x = 0u; x < sx2; x++)
    {
      tptr[0] = ((float) x * 2 * 3.1415926) / sx2;
      tptr[1] = (((float) y) - sy2/2) / (sy2 / 2);
      int res = svm->predict(traits);
      *o2ptr++ = c[res];
    }
  }
# endif
  journal.verbose("Ok.");

  //double minv, maxv;
  //cv::minMaxLoc(O3, &minv, &maxv, nullptr, nullptr);
  //journal.verbose("Min class = %f, max class = %f.", minv, maxv);
  //cv::normalize(O3, O3, 0, 1, NORM_MINMAX);

  sortie.nout = 2;
  sortie.images[0] = O;
  sortie.names[0]  = "Entrainement";
  sortie.images[1] = O2;
  sortie.names[1]  = "Classication";
  //sortie.images[2] = O2;
  //sortie.names[2]  = "SVM (classication du plan)";

  return 0;
}





