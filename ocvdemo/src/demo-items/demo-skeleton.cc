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



#include "demo-items/demo-skeleton.hpp" // REPLACE WITH THE NAME OF YOUR INCLUDE FILE


// Replace "SkeletonDemo" by the name of your class
int SkeletonDemo::proceed(OCVDemoItemInput &input, OCVDemoItemOutput &output)
{
  // - The input image(s) are in the vector input.images (of type: vector<cv::Mat>).
  //   You can assume that input images are in BGR format, 8 bits.
  // - The output image(s) should be stored in output.images[0], output.images[1], ... (of type: array of cv::Mat)
  //   Output images can be in BGR or grayscale format, 8 bits or floating point (in this case from 0 to 1.0).

  // In this simple case we do the minimum. 
  // set number of images out to one.
  output.nout = 1;
  //clone the input to the output.
  output.images[0]=input.images[0].clone();
  //set a name.
  output.names[0] = "The same";
  
  // Return code: 0 if computing is successful (return any other value to indicate a failure)
  return 0;
}

