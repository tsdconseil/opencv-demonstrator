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
#include "demo-items/thinning.hpp"


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
  Mat A, squelette;

  cv::cvtColor(input.images[0], A, CV_BGR2GRAY);

  //cv::threshold(A, A, 128, 255, cv::THRESH_OTSU);
  //A = 255 - A;



  int sel = input.model.get_attribute_as_int("sel");

  // Gradient morphologique
  // Algorithme d'après page web
  // http://felix.abecassis.me/2011/09/opencv-morphological-skeleton/
  if(sel == 0)
  {
    thinning_morpho(A, squelette);
  }
  // Zhang-Suen
  // D'après https://web.archive.org/web/20160322113207/http://opencv-code.com/quick-tips/implementation-of-thinning-algorithm-in-opencv/
  else if(sel == 1)
  {
    thinning_Zhang_Suen(A, squelette);
  }
  // Guo-Hall
  // D'après https://web.archive.org/web/20160314104646/http://opencv-code.com/quick-tips/implementation-of-guo-hall-thinning-algorithm/
  else if(sel == 2)
  {
    thinning_Guo_Hall(A, squelette);
  }

  int aff = input.model.get_attribute_as_int("aff");
  if(aff == 0)
  {
    output.nout = 1;
    output.names[0] = "Squelitisation";
    output.images[0] = squelette;
  }
  else if(aff == 1)
  {
    output.nout = 2;
    output.names[0] = "Binarisation";
    output.names[1] = "Squelitisation";
    output.images[0] = A;
    output.images[1] = squelette;
  }
  else if(aff == 2)
  {
    output.nout = 1;
    output.names[0] = "Squelitisation";
    output.images[0] = input.images[0].clone();
    output.images[0].setTo(Scalar(0,0,255), squelette);
  }

  return 0;
}



