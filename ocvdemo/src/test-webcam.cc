/** @file test-webcam.cc

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

#include <opencv2/opencv.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/features2d/features2d.hpp>
#include "mmi/gtkutil.hpp"
#include "cutil.hpp"

using namespace cv;


int main(int argc, char **argv)
{
  utils::CmdeLine cmdeline(argc, argv);
  utils::init(cmdeline, "ocvdemo", "test-webcam");
  utils::TraceManager::set_global_min_level(utils::TraceManager::TraceTarget::TRACE_TARGET_FILE, utils::TraceLevel::AL_VERBOSE);

  VideoCapture cam;

  utils::langue.load("./data/lang.xml");
  Gtk::Main kit(argc, argv);

  if(!cam.isOpened())
  {
    utils::mmi::dialogs::show_error(utils::langue.get_item("cam-err-1"), utils::langue.get_item("cam-err-2"), utils::langue.get_item("cam-err-3"));
    return -1;
  }

  Mat I;
  do
  {
    cam >> I;

    if(I.data == nullptr)
      break;

    // A FAIRE : diviser la résolution par 2 et passer en niveaux de gris
    // [...]
    cv::pyrDown(I, I);

    imshow("Camera #0", I);
  } while (waitKey(30) == -1); // Sortie dès que appui sur une touche
  return 0;
}




