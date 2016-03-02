/** @file seuillage.hpp

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

#ifndef SEUILLAGE_HPP
#define SEUILLAGE_HPP

#include "ocvdemo-item.hpp"


/** @brief Démonstration réparation d'image */
class InpaintDemo: public OCVDemoItem
{
public:
  InpaintDemo();
  int proceed(OCVDemoItemInput &input, OCVDemoItemOutput &output);
private:
  
};

/** @brief Démonstration seuillage */
class Seuillage: public OCVDemoItem
{
public:
  Seuillage();
  int proceed(OCVDemoItemInput &input, OCVDemoItemOutput &output);
};



/** @brief Démonstration transformée de distance */
class DTransDemo: public OCVDemoItem
{
public:
  DTransDemo();
  int proceed(OCVDemoItemInput &input, OCVDemoItemOutput &output);
};


#endif
