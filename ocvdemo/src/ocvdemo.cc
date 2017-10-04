/** @file ocvdemo-main.cc

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
#include "mmi/theme.hpp"


int main(int argc, char **argv)
{
  utils::CmdeLine cmdeline(argc, argv);
  utils::init(cmdeline, "ocvdemo", "ocvdemo");
  utils::journal::set_global_min_level(utils::journal::TRACE_TARGET_FILE, utils::journal::AL_VERBOSE);
  //utils::TraceManager::set_global_min_level(TraceManager::TraceTarget::TRACE_TARGET_STD, TraceLevel::AL_NONE);
  std::string dts = utils::get_current_date_time();
  trace_majeure("\nFichier journal pour l'application OCVDEMO, version %d.%02d\nDate / heure lancement application : %s\n**************************************\n**************************************\n**************************************",
      VMAJ, VMIN, dts.c_str());
  langue.load("./data/odemo-lang.xml");
  Gtk::Main kit(argc, argv);

  utils::mmi::charge_themes();
  utils::mmi::installe_theme("dark");

  OCVDemo demo(cmdeline);
  demo.demarre_interface();
  return 0;
}

