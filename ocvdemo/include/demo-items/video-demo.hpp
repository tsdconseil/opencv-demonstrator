/** @file video-demo.hpp

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

#ifndef OPTFLOWDEMO_HPP
#define OPTFLOWDEMO_HPP

#include "ocvdemo.hpp"
#include "opencv2/video/video.hpp"

/** @brief Démonstration flux optique */
class OptFlowDemo: public OCVDemoItem
{
public:
  OptFlowDemo();
  int calcul(Node &model, cv::Mat &I);
private:
  cv::Mat Iprec;
  bool reset;
  cv::Ptr<cv::DenseOpticalFlow> algo;
};

/** @brief Démonstration soustraction d'arrière-plan */
class SousArrierePlanDemo: public OCVDemoItem
{
public:
  SousArrierePlanDemo();
  int calcul(Node &model, cv::Mat &I);
private:
  //cv::BackgroundSubtractorMOG mog;
  Ptr<cv::BackgroundSubtractor> algo;
  Mat mhi;
  int nframes;
  int osel;
  void update_sel(int nsel);
};

/** Démonstration algorithme camshift */
class CamShiftDemo: public OCVDemoItem
{
public:
  CamShiftDemo();
  int calcul(Node &model, cv::Mat &I);
  void set_roi(const cv::Mat &I, const cv::Rect &new_roi);
private:
  cv::MatND hist;
  cv::Rect trackwindow;
  bool bp_init_ok;
};


#endif
