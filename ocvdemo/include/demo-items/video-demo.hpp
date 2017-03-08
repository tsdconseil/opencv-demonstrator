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

#include "ocvdemo-item.hpp"
#include "opencv2/video/video.hpp"

/** @brief Démonstration flux optique */
class OptFlowDemo: public OCVDemoItem
{
public:
  OptFlowDemo();
  int proceed(OCVDemoItemInput &input, OCVDemoItemOutput &output);
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
  int proceed(OCVDemoItemInput &input, OCVDemoItemOutput &output);
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
  int proceed(OCVDemoItemInput &input, OCVDemoItemOutput &output);
  void set_roi(const cv::Mat &I, const cv::Rect &new_roi);
private:
  cv::MatND hist;
  cv::Rect trackwindow;
  bool bp_init_ok;
};

class DemoVideoStab: public OCVDemoItem
{
public:
  DemoVideoStab();
  int proceed(OCVDemoItemInput &input, OCVDemoItemOutput &output);
};


#endif
