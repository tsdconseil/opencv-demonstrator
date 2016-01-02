/** @file boutils-image.cc

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

#include "tools/boutils-image.hpp"
#include "cutil.hpp"

MasqueBOutils::MasqueBOutils()
{
  tools.set_icon_size(Gtk::ICON_SIZE_SMALL_TOOLBAR);
  tools.set_has_tooltip(false);
  tools.add(b_raz);
  tools.add(b_gomme);
  //tools.add(b_remplissage);

  b_gomme.set_active(true);

  b_raz.set_stock_id(Gtk::Stock::REDO);

  Gtk::Image *buttonImage = new Gtk::Image(utils::get_fixed_data_path() + "/img/gomme.png");
  b_gomme.set_icon_widget(*buttonImage);
  //b_gomme.set_image(*buttonImage);
  //b_gomme.set_icon_widget(*buttonImage);
  buttonImage = new Gtk::Image(utils::get_fixed_data_path() + "/img/remp.png");
  b_remplissage.set_icon_widget(*buttonImage);
  //b_exit.signal_clicked().connect(sigc::mem_fun(*this, &OCVDemo::on_b_exit));
  wnd.add(vbox);

  vbox.pack_start(tools, Gtk::PACK_SHRINK);

  wnd.show_all_children(true);
  maj_lang();
  wnd.set_size_request(370,50);
}

void MasqueBOutils::montre()
{
  wnd.show_all_children(true);
  wnd.show_now();
}

void MasqueBOutils::cache()
{
  wnd.hide();
}

void MasqueBOutils::maj_lang()
{
  b_raz.set_label(utils::langue.get_item("b_raz"));
  b_gomme.set_label(utils::langue.get_item("b_gomme"));
  b_remplissage.set_label(utils::langue.get_item("b_remplissage"));
  wnd.set_title(utils::langue.get_item("wnd-tools"));
}

