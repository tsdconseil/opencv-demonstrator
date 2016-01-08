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


int MorphoDemo::calcul(Node &model, cv::Mat &I)
{
  int sel = model.get_attribute_as_int("type");
  int kersel = model.get_attribute_as_int("kernel");

  int kernel_type;
  if(kersel == 0)
    kernel_type = MORPH_RECT;
  else if(kersel == 1 )
    kernel_type = MORPH_CROSS;
  else if(kersel == 2)
    kernel_type = MORPH_ELLIPSE;
  else
    assert(0);

  int kernel_width = model.get_attribute_as_int("kernel-width");

  printf("Proceed k = %d, kw = %d, sel = %d.\n", kersel, kernel_width, sel);
  fflush(0);

  Mat K = getStructuringElement(kernel_type,
                                      Size(2*kernel_width + 1, 2*kernel_width+1 ),
                                      Point(kernel_width, kernel_width));

  sortie.nout = 2;

  if(sel == 0)
    dilate(I,sortie.O[1],K);
  else if(sel == 1)
    erode(I,sortie.O[1],K);
  else
    morphologyEx(I, sortie.O[1], sel, K);

  return 0;
}


