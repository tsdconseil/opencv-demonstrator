/** @file filtrage.cc
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

#include "demo-items/filtrage.hpp"
#include <random>





DemoFiltreGabor::DemoFiltreGabor()
{
  props.id = "gabor";
}

int DemoFiltreGabor::proceed(OCVDemoItemInput &input, OCVDemoItemOutput &output)
{
  int taille_noyau = input.model.get_attribute_as_int("taille-noyau");
  float sigma = input.model.get_attribute_as_float("sigma");
  float theta = input.model.get_attribute_as_float("theta");
  float lambda = input.model.get_attribute_as_float("lambda");
  float gamma = input.model.get_attribute_as_float("gamma");
  float psi = input.model.get_attribute_as_float("psi");

  if((taille_noyau & 1) == 0)
    taille_noyau++;

  cv::Mat K = cv::getGaborKernel(cv::Size(taille_noyau, taille_noyau), sigma, theta, lambda, gamma, psi);

  cv::Mat tmp;
  cv::cvtColor(input.images[0], tmp, CV_BGR2GRAY);
  cv::filter2D(tmp, output.images[0], -1, K);

  return 0;
}

DemoRedim::DemoRedim()
{
  props.id = "redim";
}

int DemoRedim::proceed(OCVDemoItemInput &input, OCVDemoItemOutput &output)
{
  auto tp = input.model.get_attribute_as_int("type");
  auto qt = input.model.get_attribute_as_int("qt");
  auto algo = input.model.get_attribute_as_int("algo");

  cv::Mat I = input.images[0];
  cv::Mat O = I.clone();

  cv::Size osize = I.size();
  if(tp == 0)
  {
    for(auto i = 0; i < qt; i++)
      osize *= 2;
  }
  else
  {
    for(auto i = 0; i < qt; i++)
      osize /= 2;
  }

  if(algo == 0) // pyrdown
  {
    for(auto i = 0; i < qt; i++)
    {
      if(tp == 0) // Agrandissement
        cv::pyrUp(O, O);
      else
        cv::pyrDown(O, O);
    }
  }
  else
  {
    cv::resize(O, O, osize);
  }

  output.images[0] = O;

  return 0;
}

DemoFiltrage::DemoFiltrage()
{
  props.id = "filtrage";
}

int DemoFiltrage::proceed(OCVDemoItemInput &input, OCVDemoItemOutput &output)
{
  DemoFiltrageConfig config;
  auto model = input.model;
  config.type_filtre = (DemoFiltrageConfig::filtre_t) model.get_attribute_as_int("sel");
  config.bruit_blanc_actif = model.get_attribute_as_boolean("gaussian");
  config.poivre_et_sel_actif = model.get_attribute_as_boolean("sp");
  config.sigma_bb  = model.get_attribute_as_float("sigma");
  config.sigma_ps  = model.get_attribute_as_float("sigma2");
  config.p_ps = model.get_attribute_as_float("p");

  config.ma.taille_noyau = model.get_attribute_as_int("ma/taille-noyau");
  config.gaussien.taille_noyau = model.get_attribute_as_int("gaussian/taille-noyau");

  if((config.gaussien.taille_noyau % 2) == 0)
  {
    config.gaussien.taille_noyau++;
    model.set_attribute("gaussian/taille-noyau", config.gaussien.taille_noyau);
  }

  config.gaussien.sigma = model.get_attribute_as_float("gaussian/sigma");
  config.median.taille_noyau = model.get_attribute_as_int("median/taille-noyau");
  if((config.median.taille_noyau % 2) == 0)
  {
    config.median.taille_noyau++;
    model.set_attribute("median/taille-noyau", config.median.taille_noyau);
  }
  config.bilateral.taille_noyau = model.get_attribute_as_int("bilateral/taille-noyau");
  config.bilateral.sigma_couleur = model.get_attribute_as_float("bilateral/sigma-color");
  config.bilateral.sigma_espace = model.get_attribute_as_float("bilateral/sigma-space");
  return proceed(config, input.images[0], output);
}

int DemoFiltrage::proceed(const DemoFiltrageConfig &conf, cv::Mat &I, OCVDemoItemOutput &output)
{
  cv::Mat Ib, If;
  I.convertTo(I, CV_32F, 1.0); // intervalle de sortie = 0..255

  Ib = I.clone();

  uint32_t n = I.cols * I.rows;
  float *ptr = Ib.ptr<float>();

  
  std::default_random_engine generator;
  std::normal_distribution<float> distri1(0, conf.sigma_bb);
  std::normal_distribution<float> distri2(0, conf.sigma_ps);

  // bruit blanc gaussien
  if(conf.bruit_blanc_actif)
  {
    for(uint32_t i = 0; i < n * 3; i++)
      ptr[i] += distri1(generator);
  }

  if(conf.poivre_et_sel_actif)
  {
    std::uniform_real_distribution<float> distri3(0,1.0);

    for(uint32_t i = 0; i < n * 3; i++)
    {
      // bruit poivre et sel
      float f = distri3(generator);
      if(f < conf.p_ps)
        ptr[i] += distri2(generator);
    }
  }

  I.convertTo(I, CV_8UC3, 1.0); // intervalle de sortie = 0..255
  Ib.convertTo(Ib, CV_8UC3, 1.0); // intervalle de sortie = 0..255
  If.convertTo(If, CV_8UC3, 1.0); // intervalle de sortie = 0..255



  if(conf.type_filtre == DemoFiltrageConfig::FILTRE_MA)
    cv::blur(Ib, If, cv::Size(conf.ma.taille_noyau,conf.ma.taille_noyau));
  else if(conf.type_filtre == DemoFiltrageConfig::FILTRE_GAUSSIEN)
    cv::GaussianBlur(Ib, If, cv::Size(conf.gaussien.taille_noyau,conf.gaussien.taille_noyau), 0);
  else if(conf.type_filtre == DemoFiltrageConfig::FILTRE_MEDIAN)
    cv::medianBlur(Ib, If, conf.median.taille_noyau);
  else if(conf.type_filtre == DemoFiltrageConfig::FILTRE_BILATERAL)
    cv::bilateralFilter(Ib, If, conf.bilateral.taille_noyau, conf.bilateral.sigma_couleur, conf.bilateral.sigma_espace);

  output.nout = 2;
  output.images[0] = Ib;
  output.images[1] = If;
  output.names[0] = "Noisy image";
  output.names[1] = "Filtered image";

  return 0;
}
