/** @file filtrage.hpp

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

#ifndef FILTRAGE_DEMO_HPP
#define FILTRAGE_DEMO_HPP

#include "ocvdemo-item.hpp"

/** Configuration filtrage */
struct DemoFiltrageConfig
{
  bool bruit_blanc_actif;
  float sigma_bb;
  bool poivre_et_sel_actif;
  float sigma_ps;
  float p_ps;

  typedef enum
  {
    FILTRE_MA         = 0,
    FILTRE_GAUSSIEN   = 1,
    FILTRE_MEDIAN     = 2,
    FILTRE_BILATERAL  = 3
  } filtre_t;

  filtre_t type_filtre;

  struct
  {
    int taille_noyau;
  } ma;

  struct
  {
    int taille_noyau;
    float sigma;
  } gaussien;

  struct
  {
    int taille_noyau;
  } median;

  struct
  {
    int taille_noyau;
    float sigma_couleur, sigma_espace;
  } bilateral;

};


class DemoFiltrage: public OCVDemoItem
{
public:
  DemoFiltrage();
  int proceed(OCVDemoItemInput &input, OCVDemoItemOutput &output);
private:
  int proceed(const DemoFiltrageConfig &conf, cv::Mat &I, OCVDemoItemOutput &output);
};

class DemoRedim: public OCVDemoItem
{
public:
  DemoRedim();
  int proceed(OCVDemoItemInput &input, OCVDemoItemOutput &output);
};

class DemoFiltreGabor: public OCVDemoItem
{
public:
  DemoFiltreGabor();
  int proceed(OCVDemoItemInput &input, OCVDemoItemOutput &output);
};



#endif
