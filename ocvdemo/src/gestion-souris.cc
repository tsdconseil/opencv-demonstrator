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



//void OCVDemo::mouse_callback(int image, int event, int x, int y, int flags)
void OCVDemo::on_event(const OCVMouseEvent &me)
{
  mutex.lock();
  //infos("ocvdemo mouse callback img = %d, x = %d, y = %d.",
	//	me.image, me.x, me.y);
  switch (me.event)
  {
    case CV_EVENT_LBUTTONDOWN:
    {
      //trace_verbeuse("LB DOWN x = %d, y = %d.", me.x, me.y);
      rdi0.x = me.x;
      rdi0.y = me.y;
      rdi1 = rdi0;
      etat_souris = 1;

      if((demo_en_cours != nullptr) && (demo_en_cours->props.requiert_masque))
      {
        masque_clic(me.x,me.y);
        compute_Ia();
        update_Ia();
      }
      break;
    }
    case CV_EVENT_MOUSEMOVE:
    {
      if(etat_souris == 1)
      {
        rdi1.x = me.x;
        rdi1.y = me.y;

        if((demo_en_cours != nullptr) && (demo_en_cours->props.requiert_masque))
          masque_clic(me.x,me.y);

        compute_Ia();
        update_Ia();
        infos("updated ia.");
      }
      break;
    }
    case CV_EVENT_LBUTTONUP:
    {
      //trace_verbeuse("LB UP x = %d, y = %d.", me.x, me.y);
      if(etat_souris == 1)
      {
        rdi1.x = me.x;
        rdi1.y = me.y;
        compute_Ia();
        etat_souris = 0;
        if(demo_en_cours != nullptr)
        {
          if(demo_en_cours->props.requiert_roi)
          {
            int minx = min(rdi0.x, rdi1.x);
            int miny = min(rdi0.y, rdi1.y);
            int maxx = max(rdi0.x, rdi1.x);
            int maxy = max(rdi0.y, rdi1.y);
            Rect rdi(minx, miny, maxx - minx, maxy - miny);
            trace_majeure("Set rdi(%d,%d,%d,%d).",
                                rdi.x, rdi.y, rdi.width, rdi.height);
            demo_en_cours->set_roi(I0, rdi);
            update();
          }
          else if(demo_en_cours->props.requiert_masque)
          {
            update();
          }
          else
            demo_en_cours->on_mouse(me.x, me.y, me.event, me.image);
        }
      }
      break;
    }
  }
  mutex.unlock();
}


