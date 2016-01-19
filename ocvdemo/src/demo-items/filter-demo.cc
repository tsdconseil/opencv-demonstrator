/** @file filter-demo.cc
 *  @brief Filtrage

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

#include "demo-items/filter-demo.hpp"
#include <random>

FilterDemo::FilterDemo()
{
  props.id = "filtrage";
  out.nout = 3;
}

int FilterDemo::calcul(Node &model, cv::Mat &I)
{
  FilterDemoConfig config;
  config.filter_type = (FilterDemoConfig::filter_t) model.get_attribute_as_int("sel");
  config.has_wn = model.get_attribute_as_boolean("gaussian");
  config.has_sp = model.get_attribute_as_boolean("sp");
  config.sigma_wn  = model.get_attribute_as_float("sigma");
  config.sigma_sp  = model.get_attribute_as_float("sigma2");
  config.p_sp = model.get_attribute_as_float("p");

  config.ma.ksize = model.get_attribute_as_int("ma/ksize");
  config.gaussian.ksize = model.get_attribute_as_int("gaussian/ksize");
  config.gaussian.sigma = model.get_attribute_as_int("gaussian/sigma");
  config.median.ksize = model.get_attribute_as_int("median/ksize");
  if((config.median.ksize % 2) == 0)
  {
    config.median.ksize++;
    model.set_attribute("median/ksize", config.median.ksize);
  }
  config.bilateral.ksize = model.get_attribute_as_int("bilateral/ksize");
  config.bilateral.sigma_color = model.get_attribute_as_int("bilateral/sigma-color");
  config.bilateral.sigma_space = model.get_attribute_as_int("bilateral/sigma-space");
  return proceed(config, I);
}

int FilterDemo::proceed(const FilterDemoConfig &conf, cv::Mat &I)
{
  out.O[0] = I;
  cv::Mat Ib, If;
  I.convertTo(I, CV_32F, 1.0); // intervalle de sortie = 0..255

  Ib = I.clone();

  uint32_t n = I.cols * I.rows;
  float *ptr = Ib.ptr<float>();

  
  std::default_random_engine generator;
  std::normal_distribution<float> distri1(0, conf.sigma_wn);
  std::normal_distribution<float> distri2(0, conf.sigma_sp);

  // bruit blanc gaussien
  if(conf.has_wn)
  {
    for(uint32_t i = 0; i < n * 3; i++)
      ptr[i] += distri1(generator);
  }

  if(conf.has_sp)
  {
    std::uniform_real_distribution<float> distri3(0,1.0);

    for(uint32_t i = 0; i < n * 3; i++)
    {
      // bruit poivre et sel
      float f = distri3(generator);
      if(f < conf.p_sp)
        ptr[i] += distri2(generator);
    }
  }

  I.convertTo(I, CV_8UC3, 1.0); // intervalle de sortie = 0..255
  Ib.convertTo(Ib, CV_8UC3, 1.0); // intervalle de sortie = 0..255
  If.convertTo(If, CV_8UC3, 1.0); // intervalle de sortie = 0..255



  if(conf.filter_type == FilterDemoConfig::FILTER_MA)
    cv::blur(Ib, If, cv::Size(conf.ma.ksize,conf.ma.ksize));
  else if(conf.filter_type == FilterDemoConfig::FILTER_GAUSSIAN)
    cv::GaussianBlur(Ib, If, cv::Size(conf.gaussian.ksize,conf.gaussian.ksize), 0);
  else if(conf.filter_type == FilterDemoConfig::FILTER_MEDIAN)
    cv::medianBlur(Ib, If, conf.median.ksize);
  else if(conf.filter_type == FilterDemoConfig::FILTER_BILATERAL)
    cv::bilateralFilter(Ib, If, conf.bilateral.ksize, conf.bilateral.sigma_color, conf.bilateral.sigma_space);

  out.O[1] = Ib;
  out.O[2] = If;

  return 0;
}
