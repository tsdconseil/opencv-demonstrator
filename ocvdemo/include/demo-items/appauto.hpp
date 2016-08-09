/** @file appauto.hpp

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

#ifndef APPAUTO_DEMO_HPP
#define APPAUTO_DEMO_HPP

#include "ocvdemo-item.hpp"

//for new demo change SkeletonDemo to your class name

class DemoAppAuto: public OCVDemoItem
{
public:
  DemoAppAuto();
  int proceed(OCVDemoItemInput &input, OCVDemoItemOutput &output);
};

/*class DemoMultiClasses: public OCVDemoItem
{
public:
  DemoMultiClasses();
  int proceed(OCVDemoItemInput &input, OCVDemoItemOutput &output);
};*/


#endif
