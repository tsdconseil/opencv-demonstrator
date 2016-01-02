/** @file histo.hpp

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


#ifndef HISTO_HPP_
#define HISTO_HPP_

#include "ocvdemo.hpp"


extern int calc_hist(const cv::Mat &I, cv::Rect &roi, cv::MatND &hist);
extern int calc_bp(const cv::Mat &I, cv::Rect &roi, cv::MatND &backproj);
extern int calc_bp(const cv::Mat &I, const cv::MatND &hist, cv::MatND &backproj);


// Egalisation d'histogramme
class HistoDemo: public OCVDemoItem
{
public:
  HistoDemo();
  int calcul(Node &model, cv::Mat &I);
};

class HistoCalc: public OCVDemoItem
{
public:
  HistoCalc();
  int calcul(Node &model, cv::Mat &I);
};

// Backprojection
class HistoBP: public OCVDemoItem
{
public:
  HistoBP();
  int calcul(Node &model, cv::Mat &I);
};

#endif /* MORPHO_DEMO_HPP_ */
