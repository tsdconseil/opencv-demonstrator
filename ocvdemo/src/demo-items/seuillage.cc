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
  {
    infos("Le masque n'est pas défini.");
    output.images[1] = I.clone();
  }
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
  infos("dtrans...");
  cv::distanceTransform(output.images[0], tmp, CV_DIST_L2, 3);
  infos("ok.");
  normalize(tmp, output.images[0], 0, 255, NORM_MINMAX, CV_8UC1);
  return 0;
}


