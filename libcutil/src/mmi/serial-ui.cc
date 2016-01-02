#include "mmi/serial-ui.hpp"
#include "comm/serial.hpp"

namespace utils
{
namespace comm
{

SerialFrame::SerialFrame(std::vector<SerialInfo> &infos, const SerialConfig &sc)
  : table(1,2)
{
  config = sc;
  m_refTreeModel = Gtk::ListStore::create(m_Columns);
  combo.set_model(m_refTreeModel);
  combo.pack_start(m_Columns.m_col_lgname, Gtk::PACK_SHRINK);
  Gtk::TreeModel::Row row;
  int is = 0;
  for(unsigned int i = 0; i < infos.size(); i++)
  {
    row = *(m_refTreeModel->append());
    row[m_Columns.m_col_name] = infos[i].name;
    row[m_Columns.m_col_lgname] = infos[i].complete_name;
    if(config.port.compare(infos[i].name) == 0)
      is = i;
  }
  combo.set_active(is);


  m_refTreeModel2 = Gtk::ListStore::create(m_Columns2);
  combo2.set_model(m_refTreeModel2);
  combo2.pack_start(m_Columns2.m_col_val, Gtk::PACK_SHRINK);
  row = *(m_refTreeModel2->append());
  row[m_Columns2.m_col_val] = 9600;
  row = *(m_refTreeModel2->append());
  row[m_Columns2.m_col_val] = 19200;
  row = *(m_refTreeModel2->append());
  row[m_Columns2.m_col_val] = 38400;
  row = *(m_refTreeModel2->append());
  row[m_Columns2.m_col_val] = 57600;
  row = *(m_refTreeModel2->append());
  row[m_Columns2.m_col_val] = 115200;
  row = *(m_refTreeModel2->append());
  row[m_Columns2.m_col_val] = 460800;

  switch(config.baud_rate)
    {
    case 9600: combo2.set_active(0); break;
    case 19200: combo2.set_active(1); break;
    case 38400: combo2.set_active(2); break;
    case 57600: combo2.set_active(3); break;
    case 115200: combo2.set_active(4); break;
    case 460800: combo2.set_active(5); break;
    }

  

  //combo.set_active(0);
  //combo2.set_active(4);


  table.attach(l_com, 0, 1, 0, 1);
  table.attach(combo, 1, 2, 0, 1);
  table.attach(l_deb, 0, 1, 1, 2);
  table.attach(combo2, 1, 2, 1, 2);
  add(table);
  table.set_border_width(7);
  update_view();
  combo.signal_changed().connect( sigc::mem_fun(*this, &SerialFrame::on_combo_change));
  combo2.signal_changed().connect( sigc::mem_fun(*this, &SerialFrame::on_combo2_change));
  set_border_width(7);
}

void SerialFrame::on_combo_change()
{
  std::string name = "";
  Gtk::TreeModel::iterator iter = combo.get_active();
  if(iter)
  {
    Gtk::TreeModel::Row row = *iter;
    if(row)
    {
      Glib::ustring res = row[m_Columns.m_col_name];
      name = res;
    }
  }
  if(name.size() > 0)
  {
    config.port = name; 
    printf("Selected %s.\n", name.c_str());
    fflush(stdout);
  }
}

void SerialConfig::dump()
{
  printf("Serial configuration: port = \"%s\", baud rate = %d.\n", port.c_str(), baud_rate);
  fflush(stdout);
}

void SerialFrame::on_combo2_change()
{
  int speed = 115200;
  Gtk::TreeModel::iterator iter = combo2.get_active();
  if(iter)
  {
    Gtk::TreeModel::Row row = *iter;
    if(row)
    {
      speed = row[m_Columns2.m_col_val];
      config.baud_rate = speed;
    }
  }
  printf("Selected %d bauds.\n", speed);
  fflush(stdout);
}

void SerialFrame::update_view()
{
  set_label(langue.get_item("com config"));
  l_com.set_label(langue.get_item("Liaison :"));
  l_deb.set_label(langue.get_item("debit"));
}

}
}


