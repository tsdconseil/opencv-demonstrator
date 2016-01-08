/** @file gestion-souris.cc

    Copyright 2016 J.A. / http://www.tsdconseil.fr

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

#include "ocvdemo.hpp"



void OCVDemo::mouse_callback(int image, int event, int x, int y, int flags)
{
  mutex.lock();
  journal.trace("ocvdemo mouse callback img = %d, x = %d, y = %d.", image, x, y);
  switch (event)
  {
    case CV_EVENT_LBUTTONDOWN:
    {
      journal.trace("LB DOWN x = %d, y = %d.", x, y);
      //Point seedPoint = cvPoint(x,y); //setting mouse clicked location as seed point

      rdi0.x = x;
      rdi0.y = y;
      rdi1 = rdi0;
      etat_souris = 1;

      if((demo_en_cours != nullptr) && (demo_en_cours->props.requiert_masque))
      {
        masque_clic(x,y);
        compute_Ia();
        update_Ia();
      }

      //floodFill(I0, seedPoint, Scalar(0,255,0));
      //update();
      break;
    }
    case CV_EVENT_MOUSEMOVE:
    {
      if(etat_souris == 1)
      {
        //Ia = I1.clone();
        rdi1.x = x;
        rdi1.y = y;

        if((demo_en_cours != nullptr) && (demo_en_cours->props.requiert_masque))
          masque_clic(x,y);

        //cv::rectangle(Ia, roi0, roi1, Scalar(0,255,0), 3);
        compute_Ia();
        update_Ia();
        journal.trace("updated ia.");
      }
      break;
    }
    case CV_EVENT_LBUTTONUP:
    {
      journal.trace("LB UP x = %d, y = %d.", x, y);
      if(etat_souris == 1)
      {
        //Ia = I1.clone();
        rdi1.x = x;
        rdi1.y = y;
        compute_Ia();
        //cv::rectangle(Ia, roi0, roi1, Scalar(0,255,0), 3);
        etat_souris = 0;
        if(demo_en_cours != nullptr)
        {
          int minx = min(rdi0.x, rdi1.x);
          int miny = min(rdi0.y, rdi1.y);
          int maxx = max(rdi0.x, rdi1.x);
          int maxy = max(rdi0.y, rdi1.y);
          Rect rdi(minx, miny, maxx - minx, maxy - miny);
          journal.trace_major("Set rdi(%d,%d,%d,%d).",
                              rdi.x, rdi.y, rdi.width, rdi.height);
          demo_en_cours->set_roi(I0, rdi);
        }
        update();
      }
      break;
    }
  }
  mutex.unlock();
}


