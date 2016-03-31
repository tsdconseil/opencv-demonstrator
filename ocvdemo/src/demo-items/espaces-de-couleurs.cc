/** @file color-demo.cc
 *  @brief Gestion des espaces de couleurs

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

#include "demo-items/espaces-de-couleurs.hpp"


DFTDemo::DFTDemo()
{
  props.id = "dft";
  output.names[0] = "DFT";
}

int DFTDemo::proceed(OCVDemoItemInput &input, OCVDemoItemOutput &output)
{
  Mat Ig, padded;                            //expand input image to optimal size

  cvtColor(input.images[0], Ig, CV_BGR2GRAY);

  int m = getOptimalDFTSize( Ig.rows );
  int n = getOptimalDFTSize( Ig.cols ); // on the border add zero values
  copyMakeBorder(Ig, padded, 0, m - Ig.rows, 0, n - Ig.cols, BORDER_CONSTANT, Scalar::all(0));

  Mat planes[] = {Mat_<float>(padded), Mat::zeros(padded.size(), CV_32F)};
  Mat complexI;
  merge(planes, 2, complexI);         // Add to the expanded another plane with zeros

  dft(complexI, complexI);            // this way the result may fit in the source matrix

  // compute the magnitude and switch to logarithmic scale
  // => log(1 + sqrt(Re(DFT(I))^2 + Im(DFT(I))^2))
  split(complexI, planes);                   // planes[0] = Re(DFT(I), planes[1] = Im(DFT(I))
  magnitude(planes[0], planes[1], planes[0]);// planes[0] = magnitude
  Mat magI = planes[0];

  magI += Scalar::all(1);                    // switch to logarithmic scale
  log(magI, magI);

  // crop the spectrum, if it has an odd number of rows or columns
  magI = magI(Rect(0, 0, magI.cols & -2, magI.rows & -2));

  // rearrange the quadrants of Fourier image  so that the origin is at the image center
  int cx = magI.cols/2;
  int cy = magI.rows/2;

  Mat q0(magI, Rect(0, 0, cx, cy));   // Top-Left - Create a ROI per quadrant
  Mat q1(magI, Rect(cx, 0, cx, cy));  // Top-Right
  Mat q2(magI, Rect(0, cy, cx, cy));  // Bottom-Left
  Mat q3(magI, Rect(cx, cy, cx, cy)); // Bottom-Right

  Mat tmp;                           // swap quadrants (Top-Left with Bottom-Right)
  q0.copyTo(tmp);
  q3.copyTo(q0);
  tmp.copyTo(q3);

  q1.copyTo(tmp);                    // swap quadrant (Top-Right with Bottom-Left)
  q2.copyTo(q1);
  tmp.copyTo(q2);

  normalize(magI, magI, 0, 255, NORM_MINMAX); // Transform the matrix with float values into a
                                          // viewable image form (float between values 0 and 1).
  output.images[0] = magI;

  return 0;
}

HSVDemo::HSVDemo()
{
  props.id = "hsv";
  output.names[0] = "Couleur";
}

int HSVDemo::proceed(OCVDemoItemInput &input, OCVDemoItemOutput &output)
{
  int T = input.model.get_attribute_as_int("T");
  int S = input.model.get_attribute_as_int("S");
  int V = input.model.get_attribute_as_int("V");

  auto I = input.images[0];
  int sx = I.cols;
  int sy = I.rows;

  //I.setTo(Scalar(0,0,0));
  I = Mat::zeros(Size(sx,sy),CV_8UC3);

  Mat tsv = Mat::zeros(Size(sx,sy),CV_8UC3);
  tsv.setTo(Scalar(T,S,V));
  cvtColor(tsv, output.images[0], CV_HSV2BGR);
  return 0;
}

DemoBalleTennis::DemoBalleTennis()
{
  props.id = "tennis";
}

int DemoBalleTennis::proceed(OCVDemoItemInput &input,
                             OCVDemoItemOutput &output)
{
  Mat &I = input.images[0];

  // Solutions :
  // (1) Mahalanobis distance, puis seuillage
  // (2) Projection arrière d'histogramme
  // (3) Seuillage simple sur teinte / saturation

  Mat tsv, masque;
  cvtColor(I, tsv, CV_BGR2HSV);
  inRange(tsv, Scalar(input.model.get_attribute_as_int("T0"),
      input.model.get_attribute_as_int("S0"),
      input.model.get_attribute_as_int("V0")),
      Scalar(input.model.get_attribute_as_int("T1"),
          input.model.get_attribute_as_int("S1"),
          input.model.get_attribute_as_int("V1")),
          masque);

  output.images[0] = masque;

  return 0;
}

