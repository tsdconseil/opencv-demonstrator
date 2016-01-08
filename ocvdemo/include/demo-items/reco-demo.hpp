/** @file reco-demo.hpp

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

#ifndef RECO_DEMO_HPP_
#define RECO_DEMO_HPP_

#include "ocvdemo-item.hpp"
#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/features2d/features2d.hpp"

class MatchDemo: public OCVDemoItem
{
public:
  MatchDemo();
  int calcul(Node &model, cv::Mat &I);
  void setup_model(Node &model);
private:
  bool lock;
  std::vector<cv::Mat> imgs;
};


class PanoDemo: public OCVDemoItem
{
public:
  PanoDemo();
  int calcul(Node &model, cv::Mat &I);
private:
  bool lock;
};

class CornerDemo: public OCVDemoItem
{
public:
  CornerDemo();
  int calcul(Node &model, cv::Mat &I);
private:
};

class VisageDemo: public OCVDemoItem
{
public:
  VisageDemo();
  int calcul(Node &model, cv::Mat &I);
private:
  CascadeClassifier face_cascade, eyes_cascade;
  RNG rng;
};


class CascGenDemo: public OCVDemoItem
{
public:
  CascGenDemo(std::string id);
  int calcul(Node &model, cv::Mat &I);
private:
  std::vector<std::string> cnames;
  CascadeClassifier cascade[3];
  RNG rng;
  bool cascade_ok;
};





#endif /* MORPHO_DEMO_HPP_ */
