/** @file gradient-demo.hpp

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

#ifndef GRADIENT_DEMO_HPP_
#define GRADIENT_DEMO_HPP_

#include "ocvdemo-item.hpp"

class NetDemo: public OCVDemoItem
{
public:
  NetDemo();
  int calcul(Node &model, cv::Mat &I);
};

class GradientDemo: public OCVDemoItem
{
public:
  GradientDemo();
  int calcul(Node &model, cv::Mat &I);
};

class LaplaceDemo: public OCVDemoItem
{
public:
  LaplaceDemo();
  int calcul(Node &model, cv::Mat &I);
};


class CannyDemo: public OCVDemoItem
{
public:
  CannyDemo();
  int calcul(Node &model, cv::Mat &I);
};

class ContourDemo: public OCVDemoItem
{
public:
  ContourDemo();
  int calcul(Node &model, cv::Mat &I);
};

class HoughDemo: public OCVDemoItem
{
public:
  HoughDemo();
  int calcul(Node &model, cv::Mat &I);
};

class HoughCDemo: public OCVDemoItem
{
public:
  HoughCDemo();
  int calcul(Node &model, cv::Mat &I);
};

class RectDemo: public OCVDemoItem
{
public:
  RectDemo();
  int calcul(Node &model, cv::Mat &I);
};

#endif /* MORPHO_DEMO_HPP_ */
