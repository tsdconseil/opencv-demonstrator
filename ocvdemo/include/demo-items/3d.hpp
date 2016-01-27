/** @file 3d.hpp

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

#ifndef CAM_DEMO_HPP
#define CAM_DEMO_HPP

#include "ocvdemo-item.hpp"


/** @brief Résultat de la calibration stéréo */
struct StereoCalResultats
{
  /** Calibration effectuée avec succès oui / non */
  bool valide;

  /** Paramètres intrinséques de chaque caméra */
  cv::Mat matrices_cameras[2];

  /** Paramètres de distortion (correction des non linéarités de chaque caméra) */
  cv::Mat dcoefs[2];

  /** Rotation (R: 3x3) et translation (T: 3x1) entre les 2 caméras */
  cv::Mat R, T;

  /** Matrice essentielle */
  cv::Mat E;

  /** Matrice fondamentale */
  cv::Mat F;

  /** Matrices de projection 3x4 vers le système de coordonnées rectifié */
  cv::Mat rectif_P[2];

  /** Transformées de rectification 3x3 (matrices de rotation) */
  cv::Mat rectif_R[2];

  /** LUTs (x,y) pour la fonction remap (rectification) */
  cv::Mat rmap[2][2];

  /** Matrice 4x4 de mapping disparité vers profondeur */
  cv::Mat Q;
};


/****************************************/
/** @brief Stereo calibration demonstration */
class StereoCalDemo: public OCVDemoItem
{
public:
  StereoCalDemo();
  int proceed(OCVDemoItemInput &input, OCVDemoItemOutput &output);


  static StereoCalResultats res;
};

/****************************************/
/** @brief Rectification demonstration */
class RectificationDemo: public OCVDemoItem
{
public:
  RectificationDemo();
  int proceed(OCVDemoItemInput &input, OCVDemoItemOutput &output);
};

/****************************************/
/** @brief Epipolar lines demonstration */
class EpiDemo: public OCVDemoItem
{
public:
  EpiDemo();
  int proceed(OCVDemoItemInput &input, OCVDemoItemOutput &output);
};

/***************************************/
/** @brief Disparity map demonstration
 *  Compute the disparity map between 2 rectified images. */
class DispMapDemo: public OCVDemoItem
{
public:
  DispMapDemo();
  int proceed(OCVDemoItemInput &input, OCVDemoItemOutput &output);
};

/********************************************/
/** @brief Camera calibration demonstration
 *  Detect a chessboard, and correct camera distortions. */
class CamCalDemo: public OCVDemoItem
{
public:
  CamCalDemo();
  int proceed(OCVDemoItemInput &input, OCVDemoItemOutput &output);
};


#endif
