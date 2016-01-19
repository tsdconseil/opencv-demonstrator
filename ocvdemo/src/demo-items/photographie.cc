/** @file photography.cc

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

#include "demo-items/photographie.hpp"
#include <opencv2/photo.hpp>

HDRDemo::HDRDemo()
{
  props.id = "hdr";
  props.input_max = -1;
  out.nout = 1;
}


int HDRDemo::proceed(OCVDemoItemInput &input, OCVDemoItemOutput &output)
{
  Ptr<MergeMertens> merge_mertens = createMergeMertens();
  Mat tmp, tmp2;
  merge_mertens->process(input.images, tmp);
  tmp.convertTo(tmp2, CV_8UC3, 255, 0);
  out.images[0] = tmp2;
  return 0;
}
