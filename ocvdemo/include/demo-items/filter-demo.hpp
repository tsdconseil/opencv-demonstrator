/** @file filter-demo.hpp

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

#ifndef FILTER_DEMO_HPP
#define FILTER_DEMO_HPP

#include "ocvdemo.hpp"

struct FilterDemoConfig
{
  bool has_wn;
  float sigma_wn;
  bool has_sp;
  float sigma_sp;
  float p_sp;
  typedef enum
  {
    FILTER_MA         = 0,
    FILTER_GAUSSIAN   = 1,
    FILTER_MEDIAN     = 2,
    FILTER_BILATERAL  = 3
  } filter_t;
  filter_t filter_type;

  struct
  {
    int ksize;
  } ma;

  struct
  {
    int ksize;
    float sigma;
  } gaussian;

  struct
  {
    int ksize;
  } median;

  struct
  {
    int ksize;
    float sigma_color, sigma_space;
  } bilateral;

};


class FilterDemo: public OCVDemoItem
{
public:
  FilterDemo();
  int calcul(Node &model, cv::Mat &I);
  int proceed(const FilterDemoConfig &conf, cv::Mat &I);
};

//extern int filter_demo(const FilterDemoConfig &conf);



#endif
