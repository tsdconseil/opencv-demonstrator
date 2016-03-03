/** @file morpho-demo.cc
 *  @brief Démonstrations autour des opérateurs morphologiques

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

#include "demo-items/morpho-demo.hpp"


MorphoDemo::MorphoDemo()
{
  props.id = "morpho";
}


int MorphoDemo::proceed(OCVDemoItemInput &input, OCVDemoItemOutput &output)
{
  int sel = input.model.get_attribute_as_int("type");
  int kersel = input.model.get_attribute_as_int("kernel");

  int kernel_type;
  if(kersel == 0)
    kernel_type = MORPH_RECT;
  else if(kersel == 1 )
    kernel_type = MORPH_CROSS;
  else if(kersel == 2)
    kernel_type = MORPH_ELLIPSE;
  else
    assert(0);

  int kernel_width = input.model.get_attribute_as_int("kernel-width");

  printf("Proceed k = %d, kw = %d, sel = %d.\n", kersel, kernel_width, sel);
  fflush(0);

  Mat K = getStructuringElement(kernel_type,
                                      Size(2*kernel_width + 1, 2*kernel_width+1 ),
                                      Point(kernel_width, kernel_width));

  auto I = input.images[0];

  if(sel == 0)
    dilate(I,output.images[0],K);
  else if(sel == 1)
    erode(I,output.images[0],K);
  else
    morphologyEx(I, output.images[0], sel, K);

  return 0;
}

DemoSqueletisation::DemoSqueletisation()
{
  props.id = "squeletisation";
}

int DemoSqueletisation::proceed(OCVDemoItemInput &input, OCVDemoItemOutput &output)
{
  //Mat A = input.images[0];
  Mat A;

  output.nout = 2;

  cv::cvtColor(input.images[0], A, CV_BGR2GRAY);

  cv::threshold(A, A, 128, 255, cv::THRESH_OTSU);
  A = 255 - A;

  output.images[0] = A.clone();

  cv::Mat K = cv::getStructuringElement(cv::MORPH_CROSS, cv::Size(3, 3));

  cv::Mat squelette(A.size(), CV_8UC1, cv::Scalar(0));
  cv::Mat temp(A.size(), CV_8UC1);

  bool done;
  do
  {
    cv::morphologyEx(A, temp, cv::MORPH_OPEN, K);
    cv::bitwise_not(temp, temp);
    cv::bitwise_and(A, temp, temp);
    cv::bitwise_or(squelette, temp, squelette);
    cv::erode(A, A, K);

    double max;
    cv::minMaxLoc(A, 0, &max);
    done = (max == 0);
  } while (!done);

  output.images[1] = squelette;
  return 0;
}



