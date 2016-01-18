/**
 *  This file is part of LIBSERIAL.
 *
 *  LIBSERIAL is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  LIBSERIAL is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with LIBSERIAL.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  Copyright 2007-2011 J. A.
 */

#ifndef SERIAL_UI_H
#define SERIAL_UI_H

#include "mmi/gtkutil.hpp"
#include "cutil.hpp"
#include "serial.hpp"

namespace utils
{
namespace comm
{

class SerialFrame: public JFrame
{
public:
  SerialFrame(std::vector<SerialInfo> &infos, const SerialConfig &sc);
  SerialConfig config;
  void update_view();
private:
  Gtk::Label l_com, l_deb;
  Gtk::Table table;
  class ModelColumns : public Gtk::TreeModel::ColumnRecord
  {
  public:
    ModelColumns()
    { add(m_col_name); add(m_col_lgname); }
    Gtk::TreeModelColumn<Glib::ustring> m_col_name;
    Gtk::TreeModelColumn<Glib::ustring> m_col_lgname;
  };
  ModelColumns m_Columns;
  Glib::RefPtr<Gtk::ListStore> m_refTreeModel;
  Gtk::ComboBox combo;
  void on_combo_change();

  class ModelColumns2 : public Gtk::TreeModel::ColumnRecord
  {
  public:
    ModelColumns2()
    { add(m_col_val); }
    Gtk::TreeModelColumn<int> m_col_val;
  };
  ModelColumns2 m_Columns2;
  Glib::RefPtr<Gtk::ListStore> m_refTreeModel2;
  Gtk::ComboBox combo2;
  void on_combo2_change();
};
}
}

#endif
