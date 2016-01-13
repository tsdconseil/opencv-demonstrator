/** @file demo-skeleton.cc

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



#include "demo-items/canny-test.hpp" // REPLACE BY NAME OF YOUR INCLUDE FILE


// Replace "MyDemo" by the name of your class
int Canny_test::calcul(Node &model, cv::Mat &I)
{
  // Place your code here:
  // The input image is I
  // The output image(s) should be stored in sortie.O[0], sortie.O[1], ...

  // For instance, a dummy copy of the input image

  sortie.O[0] = I;


  // Return code: 0 if computing is successfull
  return 0;
}

