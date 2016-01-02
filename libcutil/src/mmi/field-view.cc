
#include "mmi/stdview.hpp"
#include "mmi/ColorCellRenderer2.hpp"
#include "mmi/renderers.hpp"
#include "mmi/stdview-fields.hpp"

#include "comm/serial.hpp"

#include <string.h>
#include <stdlib.h>
#include <limits.h>

namespace utils
{
namespace mmi
{

namespace fields
{

static void update_text_color(Gtk::Widget &w, bool valid)
{
  if (valid)
  {
    //GdkColor *tmp = nullptr;
    //w.override_color(Gdk::RGBA("#000000"), Gtk::STATE_FLAG_NORMAL);
#   ifdef WIN
    w.override_color(Gdk::RGBA("#000000"), Gtk::STATE_FLAG_NORMAL);
#   else
    w.override_color(Gdk::RGBA("#ffffff"), Gtk::STATE_FLAG_NORMAL);
#   endif
    //entry.modify_text(Gtk::STATE_NORMAL, Gdk::Color(tmp, false));
  }
  else
  {
    w.override_color(Gdk::RGBA("#ff0000"), Gtk::STATE_FLAG_NORMAL);
    //entry.modify_text(Gtk::STATE_NORMAL, Gdk::Color("#ff0000"));
  }
}

/*******************************************************************
 *               BYTES VIEW IMPLEMENTATION                          *
 *******************************************************************/

BytesView::BytesView(Attribute *model) {
  this->model = model;
  valid = model->schema->is_valid(model->get_string());
  lock = false;
  label.set_use_markup(true);
  update_langue();
  entry.set_editable(true);
  entry.set_width_chars(20);
  entry.set_text(model->get_string());
  entry.signal_changed().connect(
      sigc::mem_fun(*this, &BytesView::on_signal_changed));
  entry.signal_focus_in_event().connect(
      sigc::mem_fun(*this, &AttributeView::on_focus_in));
}

void BytesView::update_langue() {
  label.set_markup("<b>" + NodeView::mk_label_colon(model->schema->name) + "</b>");
}

unsigned int BytesView::get_nb_widgets() {
  return 2;
}

Gtk::Widget *BytesView::get_widget(int index) {
  if (index == 0)
    return &label;
  else
    return &entry;
}

Gtk::Widget *BytesView::get_gtk_widget()
{
  return &entry;
}

void BytesView::set_sensitive(bool b) {
  entry.set_sensitive(b);
  label.set_sensitive(b);
}

void BytesView::on_signal_changed() {
  if (!lock) {
    lock = true;
    //model->set_value(entry.get_text());

    std::string s = entry.get_text();

    valid = model->schema->is_valid(s);
    model->set_value(entry.get_text());

    update_text_color(entry, valid);

    model->forward_change_event();

    lock = false;
  }
}

void BytesView::on_event(const ChangeEvent &ce) {
  if (!lock) {
    lock = true;
    entry.set_text(model->get_string());
    lock = false;
  }
}

/*******************************************************************
 *               HEXA VIEW IMPLEMENTATION                          *
 *******************************************************************/

HexaView::HexaView(Attribute *model) {
  this->model = model;
  lock = false;
  valid = model->schema->is_valid(model->get_string());
  label.set_use_markup(true);
  update_langue();
  entry.set_editable(true);
  entry.set_width_chars(20);

  unsigned short offset = 0;
  char buf[500];
  unsigned int vl = model->get_int();

  buf[offset++] = '0';
  buf[offset++] = 'x';
  unsigned char nb_digits = model->schema->size * 2;
  if (vl > 0) {
    unsigned long temp = vl;
    while (temp > 0) {
      temp = temp >> 4;
      nb_digits--;
    }
  } else
    nb_digits--;
  for (unsigned char i = 0; i < nb_digits; i++)
    buf[offset++] = '0';
  sprintf(&(buf[offset]), "%x", vl);
  //printf("Text : %s\n", buf);
  entry.set_text(std::string(buf));

  //entry.set_text(model->value);
  entry.signal_changed().connect(
      sigc::mem_fun(*this, &HexaView::on_signal_changed));
}

void HexaView::update_langue() {
  label.set_markup("<b>" + NodeView::mk_label_colon(model->schema->name) + "</b>");
}

unsigned int HexaView::get_nb_widgets() {
  return 2;
}

Gtk::Widget *HexaView::get_widget(int index) {
  if (index == 0)
    return &label;
  else
    return &entry;
}

Gtk::Widget *HexaView::get_gtk_widget()
{
  return &entry;
}

void HexaView::set_sensitive(bool b) {
  entry.set_sensitive(b);
  label.set_sensitive(b);
}




void HexaView::on_signal_changed() {
  if (!lock) {
    lock = true;
    //model->set_value(entry.get_text());

    std::string s = entry.get_text();

    valid = model->schema->is_valid(s);
    model->set_value(entry.get_text());

    update_text_color(entry, valid);

    model->forward_change_event();

    lock = false;
  }
}

void HexaView::on_event(const ChangeEvent &ce) {
  if (!lock) {
    lock = true;
    entry.set_text(model->get_string());
    lock = false;
  }
}

/*******************************************************************
 *               TEXT VIEW IMPLEMENTATION                        *
 *******************************************************************/

TxtView::TxtView(Attribute *model, bool small_) {
  lock = false;
  this->model = model;
  label.set_use_markup(true);
  update_langue();
  view.set_editable(true);
  //entry.set_text(Util::latin_to_utf8(model->value));

  //Glib::RefPtr< TextBuffer >
  view.get_buffer()->set_text(model->get_string());
  view.set_wrap_mode(Gtk::WRAP_WORD);

  frame.set_shadow_type(Gtk::SHADOW_ETCHED_IN);

  /*if(small_)
   view.set_width_chars(12);
   else
   view.set_width_chars(30);*/
  view.get_buffer()->signal_changed().connect(
      sigc::mem_fun(*this, &TxtView::on_signal_changed));

  scroll.set_policy(Gtk::POLICY_NEVER, Gtk::POLICY_AUTOMATIC);
  scroll.add(view);
  scroll.set_size_request(350, 150); //225,75);
  frame.add(scroll);
}

void TxtView::update_langue() {
  label.set_markup("<b>" + NodeView::mk_label(model->schema->name) + "</b>");
}

unsigned int TxtView::get_nb_widgets() {
  return 2;
}

Gtk::Widget *TxtView::get_widget(int index) {
  if (index == 0)
    return &label;
  else
    return &frame;
}

Gtk::Widget *TxtView::get_gtk_widget()
{
  return &frame;
}

void TxtView::set_sensitive(bool b) {
  view.set_sensitive(b);
  label.set_sensitive(b);
}

void TxtView::on_signal_changed() {
  if (!lock) {
    lock = true;
    model->set_value(view.get_buffer()->get_text());
    lock = false;
  }
}

void TxtView::on_event(const ChangeEvent &ce) {
  if (!lock) {
    lock = true;
    view.get_buffer()->set_text(model->get_string());
    lock = false;
  }
}

/*******************************************************************
 *               DECIMAL SPIN VIEW IMPLEMENTATION                  *
 *******************************************************************/

DecimalSpinView::DecimalSpinView(Attribute *model, int compatibility_mode)
{
  tp = "decimal-spin";
  is_sensitive = true;
  lock = false;
  this->model = model;
  label.set_use_markup(true);
  update_langue();
  spin.set_editable(true);
  spin.set_increments(1, 1);

  valid = model->schema->is_valid(model->get_string());

  //trace("decimal view(%s): min = %d, max = %d.", model->name.c_str(), model->schema.get_min(), model->schema.get_max());

  spin.set_range(model->schema->get_min(), model->schema->get_max());
  spin.set_value((double) model->get_float()); //atoi(model->get_string().c_str()));
  if (model->schema->has_unit())
  {
    std::string unit = model->schema->unit;
    if (langue.has_item(unit))
      unit = langue.get_item(unit);
    label_unit.set_text(std::string("  ") + unit + "  ");

    // To deprecate
    if(compatibility_mode)
      hbox.pack_start(spin, Gtk::PACK_SHRINK);
    hbox.pack_start(align, Gtk::PACK_SHRINK);
    align.add(label_unit);
    align.set_padding(0, 0, 5, 0);
  }

  spin.signal_value_changed().connect(
      sigc::mem_fun(*this, &DecimalSpinView::on_signal_changed));
  spin.signal_editing_done().connect(
      sigc::mem_fun(*this, &DecimalSpinView::on_signal_changed));
  spin.signal_changed().connect(
      sigc::mem_fun(*this, &DecimalSpinView::on_signal_changed));
  spin.signal_focus_out_event().connect(
      sigc::mem_fun(*this, &DecimalSpinView::on_signal_focus_out));
  spin.signal_focus_in_event().connect(
      sigc::mem_fun(*this, &AttributeView::on_focus_in));
  update_valid();
}

void DecimalSpinView::update_langue() {
  set_sensitive(is_sensitive);
  //label.set_markup("<b>" + mk_label(&(model->schema)) + "</b>");
}

unsigned int DecimalSpinView::get_nb_widgets() {
  //if(model->schema.has_unit())
  //return 3;
  return 2;
}

Gtk::Widget *DecimalSpinView::get_widget(int index) {
  if (index == 0)
    return &label;
  else if (index == 1)
  {
    if (model->schema->has_unit())
      return &hbox;
    else
      return &spin;
  }
  return nullptr;
}

Gtk::Widget *DecimalSpinView::get_gtk_widget()
{
  return &spin;
}

void DecimalSpinView::set_sensitive(bool b) {
  is_sensitive = b;
  spin.set_sensitive(b);
  label.set_sensitive(b);
  label_unit.set_sensitive(b);
  label.set_markup("<b>" + NodeView::mk_label(model->schema->name) + "</b>");
}

bool DecimalSpinView::on_signal_focus_out(GdkEventFocus *gef) {
  //trace("focus out.");
  //on_signal_changed();
  return true;
}

void DecimalSpinView::update_valid()
{
  update_text_color(spin, valid);
}

bool DecimalSpinView::is_valid() {
  return valid;
}

void DecimalSpinView::on_signal_changed() {
  if (!lock) {
    lock = true;

    std::string s = spin.get_text();
    valid = model->schema->is_valid(s);
    model->set_value(s);
    update_valid();
    model->forward_change_event();
    lock = false;
  }
}

void DecimalSpinView::on_event(const ChangeEvent &ce) {
  if (!lock) {
    lock = true;
    spin.set_range(model->schema->get_min(), model->schema->get_max());
    spin.set_value(model->get_int());
    lock = false;
  }
}

/*******************************************************************
 *               FLOAT VIEW IMPLEMENTATION                         *
 *******************************************************************/

FloatView::FloatView(Attribute *model) {
  lock = false;
  this->model = model;
  label.set_use_markup(true);
  label.set_markup("<b>" + NodeView::mk_label(model->schema->name) + "</b>");
  spin.set_editable(true);
  spin.set_increments(0.01, 1);
  spin.set_digits(6);
  //trace("decimal view(%s): min = %d, max = %d.", model->name.c_str(), model->schema.get_min(), model->schema.get_max());

  spin.set_range(model->schema->get_min(), model->schema->get_max());
  //trace("Setting spin value = %f, str = %s", atof(model->value.c_str()), model->value.c_str());
  spin.set_value(model->get_float());   //atof(model->value.c_str()));
  if (model->schema->has_unit()) {
    std::string unit = model->schema->unit;
  if (langue.has_item(unit))
    unit = langue.get_item(unit);
    label_unit.set_text(std::string("  ") + unit + "  ");
  }
  spin.signal_value_changed().connect(
      sigc::mem_fun(*this, &FloatView::on_signal_changed));
}

void FloatView::update_langue() {
  label.set_markup("<b>" + NodeView::mk_label(model->schema->name) + "</b>");
}

unsigned int FloatView::get_nb_widgets() {
  if (model->schema->has_unit())
    return 3;
  return 2;
}

Gtk::Widget *FloatView::get_widget(int index) {
  if (index == 0)
    return &label;
  else if (index == 1)
    return &spin;
  else
    return &label_unit;
}

Gtk::Widget *FloatView::get_gtk_widget()
{
  return &spin;
}

void FloatView::set_sensitive(bool b) {
  spin.set_sensitive(b);
  label.set_sensitive(b);
  label_unit.set_sensitive(b);
}

void FloatView::on_signal_changed() {
  if (!lock) {
    lock = true;
    //printf("Spin change: %s\n", Util::int2str(spin.get_value_as_float()).c_str());
    model->set_value((float) spin.get_value());
    lock = false;
  }
}

void FloatView::on_event(const ChangeEvent &ce) {
  if (!lock) {
    lock = true;
    spin.set_value(model->get_float());//atof(model->value.c_str()));
    lock = false;
  }
}

/*******************************************************************
 *               BOOLEAN VIEW IMPLEMENTATION                       *
 *******************************************************************/
BooleanView::~BooleanView() {

}

BooleanView::BooleanView(Attribute *model) {
  lock = false;
  this->model = model;
  //Gtk::Label *lab = new Gtk::Label();
  lab.set_use_markup(true);
  std::string s = NodeView::mk_label(model->schema->name);
  lab.set_markup("<b>" + s + "</b>");
  check.add(lab);
  check.set_active(model->get_boolean());
  check.signal_toggled().connect(
      sigc::mem_fun(*this, &BooleanView::on_signal_toggled));
  check.signal_focus_in_event().connect(
      sigc::mem_fun(*this, &AttributeView::on_focus_in));
}

void BooleanView::update_langue()
{
  std::string s = NodeView::mk_label(model->schema->name);
  lab.set_markup("<b>" + s + "</b>");
}

unsigned int BooleanView::get_nb_widgets() {
  return 1;
}

Gtk::Widget *BooleanView::get_widget(int index) {
  return &check;
}

Gtk::Widget *BooleanView::get_gtk_widget()
{
  return &check;
}

void BooleanView::set_sensitive(bool b) {
  check.set_sensitive(b);
}

void BooleanView::on_signal_toggled() {
  if (!lock) {
    lock = true;
    model->set_value(check.get_active());
    lock = false;
  }
}

void BooleanView::on_event(const ChangeEvent &ce) {
  if (!lock) {
    lock = true;
    check.set_active(model->get_boolean());
    lock = false;
  }
}

/*******************************************************************
 *               COMBO VIEW IMPLEMENTATION                         *
 *******************************************************************/
ComboView::ComboView(Attribute *model) {
  lock = false;
  //trace("comboview(%s)..", model->schema.name.get_id().c_str());
  this->model = model;
  label.set_use_markup(true);

  tree_model = Gtk::ListStore::create(columns);
  combo.set_model(tree_model);

  combo.pack_start(columns.m_col_name);
  if (model->schema->has_unit())
    combo.pack_start(columns.m_col_unit);

  update_langue();

  combo.signal_changed().connect(
      sigc::mem_fun(*this, &ComboView::on_combo_changed));
  combo.signal_focus_in_event().connect(
      sigc::mem_fun(*this, &AttributeView::on_focus_in));
}

void ComboView::update_langue() {
  unsigned int imax;
  bool old_lock = lock;

  lock = true;
  //trace("combo: update langue...");
  label.set_markup("<b>" + NodeView::mk_label_colon(model->schema->name) + "</b>");
  tree_model->clear();

  //imin = 0;
  imax = model->schema->constraints.size();

  if ((imax == 0) && (model->schema->has_max)) {
    imax = model->schema->max - model->schema->min + 1;
  }

  for (unsigned int i = 0; i < imax; i++) {
    std::string valeur;
    if (model->schema->constraints.size() > i)
      valeur = model->schema->constraints[i];
    else
      valeur = str::int2str(i);
    std::string nom = valeur;
    for (unsigned int j = 0; j < model->schema->enumerations.size(); j++) {
      Enumeration e;

      if (i >= model->schema->enumerations.size()) {
        log.warning("enumeration %d not defined: attribute %s.", i,
            model->schema->name.get_id().c_str());

        log.trace("schema is: %s.\n", model->schema->to_string().c_str());

        break;
      }

      e = model->schema->enumerations[i];
      if ((e.value.compare(valeur) == 0)
          || (e.name.get_id().compare(valeur) == 0)) {
        nom = e.name.get_localized();
        break;
      }
    }
    if (langue.has_item(nom))
      nom = langue.get_item(nom);
    Gtk::TreeModel::Row row = *(tree_model->append());

    if ((nom[0] >= 'a') && (nom[0] <= 'z'))
      nom[0] += ('A' - 'a');

    row[columns.m_col_name] = nom;
    row[columns.m_col_real_name] = valeur;

    if (model->schema->has_unit()) {
      row[columns.m_col_unit] = "";
      std::string unit = model->schema->unit;
      if (langue.has_item(unit))
        unit = langue.get_item(unit);
      row[columns.m_col_unit] = unit;
    }
  }
  unsigned int i;
  for (i = 0; i < imax; i++) {
    std::string valeur;
    if (model->schema->constraints.size() > i)
      valeur = model->schema->constraints[i];
    else
      valeur = str::int2str(i);

    //std::string valeur = model->schema.constraints[i];
    std::string nom = valeur;
    for (unsigned int j = 0; j < model->schema->enumerations.size(); j++) {
      if (i >= model->schema->enumerations.size()) {
        log.anomaly("enumeration.");
        break;
      }

      Enumeration e = model->schema->enumerations[i];
      if (e.value.compare(valeur) == 0) {
        nom = e.name.get_localized();
        break;
      }
    }
    if ((model->get_string().compare(nom) == 0)
        || (model->get_string().compare(valeur) == 0))
    {
      combo.set_active(i);
      break;
    }
  }
  if (i == /*model->schema.constraints.size()*/imax)
    {
      string s = model->get_string();
    log.anomaly("Not found current value (%s, %s).",
            model->schema->name.get_id().c_str(), s.c_str());
    for (i = 0; i < model->schema->constraints.size(); i++) {
      std::string valeur = model->schema->constraints[i];
      log.trace("constraint[%d] = %s.", i, valeur.c_str());
    }
  }
  //log.trace("combo: update langue done.");
  lock = old_lock;
}

unsigned int ComboView::get_nb_widgets() {
  return 2;
}

Gtk::Widget *ComboView::get_widget(int index) {
  if (index == 0)
    return &label;
  else
    return &combo;
}

Gtk::Widget *ComboView::get_gtk_widget()
{
  return &combo;
}

void ComboView::set_sensitive(bool b) {
  label.set_sensitive(b);
  combo.set_sensitive(b);
}

void ComboView::on_combo_changed() {
  if (!lock) {
    lock = true;

    Gtk::TreeModel::iterator iter = combo.get_active();
    if (iter) {
      Gtk::TreeModel::Row row = *iter;
      if (row) {
        Glib::ustring name = row[columns.m_col_name];
        Glib::ustring real_name = row[columns.m_col_real_name];
        model->set_value(real_name);
      }
    } else
      log.anomaly("combo view: none selected.");
    lock = false;
  }
}

//    check.set_active(model->get_boolean());

void ComboView::on_event(const ChangeEvent &ce) {
  if (!lock) {
    lock = true;

    unsigned int imax = model->schema->constraints.size();

    if ((imax == 0) && (model->schema->has_max)) {
      imax = model->schema->max - model->schema->min + 1;
    }

    unsigned int i;
    for (i = 0; i < imax; i++) {
      std::string valeur;
      if (model->schema->constraints.size() > i)
        valeur = model->schema->constraints[i];
      else
        valeur = str::int2str(i);

      std::string nom = valeur;
      for (unsigned int j = 0; j < model->schema->enumerations.size(); j++) {
        Enumeration &e = model->schema->enumerations[i];
        if (e.value.compare(valeur) == 0) {
          nom = e.name.get_localized();
          break;
        }
      }
      if ((model->get_string().compare(nom) == 0)
          || (model->get_string().compare(valeur) == 0)) {
        combo.set_active(i);
        break;
      }
    }

    lock = false;
  }
}

/*******************************************************************
 *               DATE  VIEW IMPLEMENTATION                         *
 *******************************************************************/

DateView::DateView(Attribute *model)
{
  adj_year  = Gtk::Adjustment::create(2000,0,2100);
  adj_month = Gtk::Adjustment::create(1,1,12);
  adj_day   = Gtk::Adjustment::create(1,1,31);

  year.set_adjustment(adj_year);
  month.set_adjustment(adj_month);
  day.set_adjustment(adj_day);

  lock = false;
  this->model = model;
  valid = model->schema->is_valid(model->get_string());
  label.set_use_markup(true);
  label.set_markup("<b>" + NodeView::mk_label_colon(model->schema->name) + "</b>");

  hbox.pack_start(day, Gtk::PACK_SHRINK);
  a_month.add(month);
  a_month.set_padding(0, 0, 6, 6);
  hbox.pack_start(a_month, Gtk::PACK_SHRINK);
  hbox.pack_start(year, Gtk::PACK_SHRINK);

  ChangeEvent ce;
  on_event(ce);

  if (appli_view_prm.use_touchscreen) {
    day.set_snap_to_ticks(false);
    month.set_snap_to_ticks(false);
    year.set_snap_to_ticks(false);
  }

  day.signal_changed().connect(
      sigc::mem_fun(*this, &DateView::on_date_changed));
  month.signal_changed().connect(
      sigc::mem_fun(*this, &DateView::on_date_changed));
  year.signal_changed().connect(
      sigc::mem_fun(*this, &DateView::on_date_changed));

  day.signal_editing_done().connect(
      sigc::mem_fun(*this, &DateView::on_date_changed));
  day.signal_focus_out_event().connect(
      sigc::mem_fun(*this, &DateView::on_signal_focus_out));
  month.signal_editing_done().connect(
      sigc::mem_fun(*this, &DateView::on_date_changed));
  month.signal_focus_out_event().connect(
      sigc::mem_fun(*this, &DateView::on_signal_focus_out));
  year.signal_editing_done().connect(
      sigc::mem_fun(*this, &DateView::on_date_changed));
  year.signal_focus_out_event().connect(
      sigc::mem_fun(*this, &DateView::on_signal_focus_out));

  day.signal_focus_in_event().connect(
      sigc::mem_fun(*this, &AttributeView::on_focus_in));
  month.signal_focus_in_event().connect(
      sigc::mem_fun(*this, &AttributeView::on_focus_in));
  year.signal_focus_in_event().connect(
      sigc::mem_fun(*this, &AttributeView::on_focus_in));
}

void DateView::update_langue() {
  label.set_markup("<b>" + NodeView::mk_label_colon(model->schema->name) + "</b>");
  if (appli_view_prm.use_touchscreen) {
    day.set_snap_to_ticks(false);
    month.set_snap_to_ticks(false);
    year.set_snap_to_ticks(false);
  }
}

unsigned int DateView::get_nb_widgets() {
  return 2;
}

Gtk::Widget *DateView::get_widget(int index) {
  if (index == 0)
    return &label;
  else
    return &hbox;
}

Gtk::Widget *DateView::get_gtk_widget()
{
  return &hbox;
}

void DateView::set_sensitive(bool b) {
  label.set_sensitive(b);
  year.set_sensitive(b);
  month.set_sensitive(b);
  day.set_sensitive(b);
}

bool DateView::on_signal_focus_out(GdkEventFocus *gef) {
  on_date_changed();
  return true;
}

void DateView::on_date_changed() {
  if (!lock) {
    lock = true;

    /*int vy = year.get_value_as_int();
     int vm = month.get_value_as_int();
     int vd = day.get_value_as_int();*/
    int vy = atoi(year.get_text().c_str());
    int vm = atoi(month.get_text().c_str());
    int vd = atoi(day.get_text().c_str());

    std::string s = str::int2str(vd) + "." + str::int2str(vm) + "."
        + str::int2str(vy);

    //trace("new date: %s.", s.c_str());

    //model->set_value(s);
    valid = model->schema->is_valid(s);
    model->set_value(s);

    update_text_color(year, valid);
    update_text_color(month, valid);
    update_text_color(day, valid);

    model->forward_change_event();

    lock = false;
  }
}

void DateView::on_event(const ChangeEvent &ce) {
  if (!lock) {
    lock = true;

    Gdk::Color c;

    std::vector<int> lst;
    str::parse_int_list(model->get_string(), lst);

    while (lst.size() < 3)
      lst.push_back(0);

    year.set_value(lst[2]);
    month.set_value(lst[1]);
    day.set_value(lst[0]);

    lock = false;
  }
}

/*******************************************************************
 *               FOLDER VIEW IMPLEMENTATION                        *
 *******************************************************************/

bool FolderView::on_focus_in(GdkEventFocus *gef) {

  log.trace("focus in");
  //target_window->present();
  if (appli_view_prm.fixed_size) {
    fcd->resize(appli_view_prm.dx, appli_view_prm.dy);
    fcd->set_size_request(appli_view_prm.dx, appli_view_prm.dy);
    fcd->move(appli_view_prm.ox, appli_view_prm.oy);
    fcd->resize(appli_view_prm.dx, appli_view_prm.dy);
  }

  return true;
}

bool FolderView::on_frame_event(GdkEvent *gef) {

  log.trace("frame event");
  //target_window->present();
  if (appli_view_prm.fixed_size)
  {
    fcd->resize(appli_view_prm.dx, appli_view_prm.dy);
    fcd->move(appli_view_prm.ox, appli_view_prm.oy);
  }

  return true;
}

FolderView::FolderView(Attribute *model)
{
  fcd = new Gtk::FileChooserDialog(langue.get_item("select-folder"),
      Gtk::FILE_CHOOSER_ACTION_SELECT_FOLDER);
  fcd->add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
  fcd->add_button(Gtk::Stock::OPEN, Gtk::RESPONSE_OK);

  button = new Gtk::FileChooserButton(*fcd);

  if (appli_view_prm.fixed_size) {
    fcd->set_position(Gtk::WIN_POS_NONE);
    fcd->resize(appli_view_prm.dx, appli_view_prm.dy);
    fcd->move(appli_view_prm.ox, appli_view_prm.oy);
    fcd->set_resizable(false);
    fcd->set_size_request(appli_view_prm.dx, appli_view_prm.dy);
    //fcd->set_resize_mode(Gtk::RESIZE_QUEUE);
    fcd->signal_focus_in_event().connect(
        sigc::mem_fun(*this, &FolderView::on_focus_in));
    //TODO fcd->signal_frame_event().connect(
    //    sigc::mem_fun(*this, &FolderView::on_frame_event));
  } else {
    fcd->set_position(Gtk::WIN_POS_CENTER_ALWAYS);
  }

  lock = false;
  this->model = model;
  label.set_use_markup(true);
  label.set_markup("<b>" + NodeView::mk_label_colon(model->schema->name) + "</b>");

  button->set_action(Gtk::FILE_CHOOSER_ACTION_SELECT_FOLDER);

  ChangeEvent ce;
  on_event(ce);

# if ((GTKMM_MAJOR_VERSION * 100 + GTKMM_MINOR_VERSION) < 218)
# warning GTKMM version too old for signal_file_set
  button->signal_selection_changed().connect(
      sigc::mem_fun(*this, &FolderView::on_folder_changed));
  on_folder_changed();
# else
  button->signal_file_set().connect(sigc::mem_fun(*this, &FolderView::on_folder_changed));
# endif
}

void FolderView::update_langue() {
  label.set_markup("<b>" + NodeView::mk_label_colon(model->schema->name) + "</b>");
}

unsigned int FolderView::get_nb_widgets() {
  return 2;
}

Gtk::Widget *FolderView::get_widget(int index) {
  if (index == 0)
    return &label;
  else
    return button;
}

Gtk::Widget *FolderView::get_gtk_widget()
{
  return button;
}

void FolderView::set_sensitive(bool b) {
  label.set_sensitive(b);
  button->set_sensitive(b);
}

void FolderView::on_folder_changed() {
  if (!lock) {
    lock = true;
    Glib::ustring s = button->get_filename();
    std::string s2 = s;
    std::string s3 = str::utf8_to_latin(s2);
    log.trace("Folder view changed: '%s'.", s2.c_str());
    model->set_value(s3);
    lock = false;
  }
}

void FolderView::on_event(const ChangeEvent &ce) {
  if (!lock) {
    lock = true;
    button->set_current_folder(model->get_string());
    lock = false;
  }
}

/*******************************************************************
 *               FILE VIEW IMPLEMENTATION                        *
 *******************************************************************/

bool FileView::on_focus_in(GdkEventFocus *gef) {

  log.trace("focus in");
  //target_window->present();
  if (appli_view_prm.fixed_size) {
    fcd->resize(appli_view_prm.dx, appli_view_prm.dy);
    fcd->set_size_request(appli_view_prm.dx, appli_view_prm.dy);
    fcd->move(appli_view_prm.ox, appli_view_prm.oy);
    fcd->resize(appli_view_prm.dx, appli_view_prm.dy);
  }

  return true;
}

bool FileView::on_frame_event(GdkEvent *gef) {

  log.trace("frame event");
  //target_window->present();
  if (appli_view_prm.fixed_size) {
    fcd->resize(appli_view_prm.dx, appli_view_prm.dy);
    fcd->move(appli_view_prm.ox, appli_view_prm.oy);
  }

  return true;
}

FileView::~FileView()
{

  delete button;
  delete fcd;
}

FileView::FileView(Attribute *model)
{
  fcd = new Gtk::FileChooserDialog(langue.get_item("select-folder"),
      Gtk::FILE_CHOOSER_ACTION_SELECT_FOLDER);
  fcd->add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
  fcd->add_button(Gtk::Stock::OPEN, Gtk::RESPONSE_OK);

  button = new Gtk::FileChooserButton(*fcd);

  if (appli_view_prm.fixed_size) {
    fcd->set_position(Gtk::WIN_POS_NONE);
    fcd->resize(appli_view_prm.dx, appli_view_prm.dy);
    fcd->move(appli_view_prm.ox, appli_view_prm.oy);
    fcd->set_resizable(false);
    fcd->set_size_request(appli_view_prm.dx, appli_view_prm.dy);
    //fcd->set_resize_mode(Gtk::RESIZE_QUEUE);
    fcd->signal_focus_in_event().connect(
        sigc::mem_fun(*this, &FileView::on_focus_in));
    //TODO fcd->signal_frame_event().connect(
    //    sigc::mem_fun(*this, &FileView::on_frame_event));
  } else {
    fcd->set_position(Gtk::WIN_POS_CENTER_ALWAYS);
  }

  lock = false;
  this->model = model;
  label.set_use_markup(true);
  label.set_markup("<b>" + NodeView::mk_label_colon(model->schema->name) + "</b>");

  button->set_action(Gtk::FILE_CHOOSER_ACTION_OPEN);

  button->set_width_chars(20);

  std::string ext = model->schema->extension;

  if (ext.size() > 0) {
    Glib::RefPtr<Gtk::FileFilter> filter = Gtk::FileFilter::create();
    filter->set_name(ext + " file");
    filter->add_pattern(std::string("*.") + ext);
    button->add_filter(filter);
  }

  //ChangeEvent ce;
  //on_event(ce);
  button->set_filename(model->get_string());

  button->signal_selection_changed().connect(
      sigc::mem_fun(*this, &FileView::on_file_changed));
}

void FileView::update_langue() {
  label.set_markup("<b>" + NodeView::mk_label_colon(model->schema->name) + "</b>");
}

unsigned int FileView::get_nb_widgets() {
  return 2;
}

Gtk::Widget *FileView::get_widget(int index) {
  if (index == 0)
    return &label;
  else
    return button;
}

Gtk::Widget *FileView::get_gtk_widget()
{
  return button;
}

void FileView::set_sensitive(bool b) {
  label.set_sensitive(b);
  button->set_sensitive(b);
}

void FileView::on_file_changed()
{
  if (!lock) {
    lock = true;
    Glib::ustring s = button->get_filename();

    std::string s2 = s;
    printf("**** on_file_changed: %s.\n", s2.c_str());

    // Here: convert absolute path to relative path
    // --> How to find the root directory ?
    if(s2.size() > 0)
      model->set_value(s);
    lock = false;
  }
}

void FileView::on_event(const ChangeEvent &ce) {
  if (!lock) {
    lock = true;
    button->set_filename(model->get_string());
    lock = false;
  }
}

/*******************************************************************
 *               SERIAL VIEW IMPLEMENTATION                        *
 *******************************************************************/
SerialView::SerialView(Attribute *model) {
  lock = false;
  this->model = model;
  label.set_use_markup(true);

  tree_model = Gtk::ListStore::create(columns);
  combo.set_model(tree_model);

  combo.pack_start(columns.m_col_name);
  if (model->schema->has_unit())
    combo.pack_start(columns.m_col_unit);

  comm::Serial::enumerate(serials);

  update_langue();

  combo.signal_changed().connect(
      sigc::mem_fun(*this, &SerialView::on_combo_changed));
}

void SerialView::update_langue() {
  bool old_lock = lock;

  lock = true;
  label.set_markup("<b>" + NodeView::mk_label_colon(model->schema->name) + "</b>");
  tree_model->clear();

  for (unsigned int i = 0; i < serials.size(); i++) {
    std::string valeur = serials[i].name;
    std::string nom = serials[i].complete_name;
    Gtk::TreeModel::Row row = *(tree_model->append());
    row[columns.m_col_name] = str::latin_to_utf8(nom);
    row[columns.m_col_real_name] = valeur;
  }
  for (unsigned int i = 0; i < serials.size(); i++) {
    std::string valeur = serials[i].name;
    std::string nom = serials[i].complete_name;
    if ((model->get_string().compare(nom) == 0)
        || (model->get_string().compare(valeur) == 0)) {
      combo.set_active(i);
      break;
    }
  }
  //if(i == serials.size())
  //combo.set_active(0);
  lock = old_lock;
}

unsigned int SerialView::get_nb_widgets() {
  return 2;
}

Gtk::Widget *SerialView::get_widget(int index) {
  if (index == 0)
    return &label;
  else
    return &combo;
}

Gtk::Widget *SerialView::get_gtk_widget()
{
  return &combo;
}

void SerialView::set_sensitive(bool b) {
  label.set_sensitive(b);
  combo.set_sensitive(b);
}

void SerialView::on_combo_changed() {
  if (!lock) {
    lock = true;

    Gtk::TreeModel::iterator iter = combo.get_active();
    if (iter) {
      Gtk::TreeModel::Row row = *iter;
      if (row) {
        Glib::ustring name = row[columns.m_col_name];
        Glib::ustring real_name = row[columns.m_col_real_name];
        model->set_value(real_name);
      }
    } else
      log.anomaly("combo view: none selected.");
    lock = false;
  }
}

void SerialView::on_event(const ChangeEvent &ce) {
  if (!lock) {
    lock = true;
    for (unsigned int i = 0; i < serials.size(); i++) {
      std::string valeur = serials[i].name;
      std::string nom = serials[i].complete_name;
      if ((model->get_string().compare(nom) == 0)
          || (model->get_string().compare(valeur) == 0)) {
        combo.set_active(i);
        break;
      }
    }
    lock = false;
  }
}

/*******************************************************************
 *               COLOR VIEW IMPLEMENTATION                         *
 *******************************************************************/

ColorView::ColorView(Attribute *model) {
  lock = false;
  this->model = model;
  label.set_use_markup(true);
  update_langue();

  Gdk::Color c;

  std::vector<int> vec;
  if(str::parse_int_list(model->get_string(), vec) == 0)
  {
    if(vec.size() == 3)
    {
      c.set_red(256 * vec[0]);
      c.set_green(256 * vec[1]);
      c.set_blue(256 * vec[2]);
    }
  }

  color.set_color(c);
  color.signal_color_set().connect(
      sigc::mem_fun(*this, &ColorView::on_color_changed));

  //if(appli_view_prm.use_touchscreen)
  {
    cb = new ColorButton(model);
  }
  /*else
   {
   cb = nullptr;
   }*/

}

void ColorView::update_langue() {
  label.set_markup("<b>" + NodeView::mk_label_colon(model->schema->name) + "</b>");
}

unsigned int ColorView::get_nb_widgets() {
  return 2;
}

Gtk::Widget *ColorView::get_widget(int index) {
  if (index == 0)
    return &label;
  else {
    if (cb == nullptr)
      return &color;
    else
      return cb;
  }
}

Gtk::Widget *ColorView::get_gtk_widget()
{
  if (cb == nullptr)
    return &color;
  else
    return cb;
}

void ColorView::set_sensitive(bool b) {
  label.set_sensitive(b);
  color.set_sensitive(b);
}

void ColorView::on_color_changed() {
  if (!lock) {
    lock = true;

    Gdk::Color c = color.get_color();
    char buf[100];
    sprintf(buf, "%d.%d.%d", c.get_red() / 256, c.get_green() / 256,
        c.get_blue() / 256);

    model->set_value(std::string(buf));
    lock = false;
  }
}

void ColorView::on_event(const ChangeEvent &ce) {
  if (!lock) {
    lock = true;

    Gdk::Color c;

    std::vector<int> vec;
    str::parse_int_list(model->get_string(), vec);

    if(vec.size() == 3)
    {
      c.set_red(256 * vec[0]);
      c.set_green(256 * vec[1]);
      c.set_blue(256 * vec[2]);
      color.set_color(c);
    }

    lock = false;
  }
}




/*******************************************************************
 *               STRING VIEW IMPLEMENTATION                        *
 *******************************************************************/

StringView::StringView(Attribute *model, bool small_)
{
  lock = false;
  valid = model->schema->is_valid(model->get_string());
  this->model = model;
  label.set_use_markup(true);
  update_langue();
  entry.set_editable(true);

  entry.set_text(model->get_string());
  if (small_)
    entry.set_width_chars(12);
  else
    entry.set_width_chars(30);
  entry.signal_changed().connect(
      sigc::mem_fun(*this, &StringView::on_signal_changed));
  entry.signal_focus_in_event().connect(
      sigc::mem_fun(*this, &AttributeView::on_focus_in));
  update_valid();
}

StringView::~StringView()
{
  //model->CProvider<ChangeEvent>::remove_listener(this);
}

void StringView::update_langue() {
  label.set_markup("<b>" + NodeView::mk_label_colon(model->schema->name) + "</b>");
}

unsigned int StringView::get_nb_widgets() {
  return 2;
}

Gtk::Widget *StringView::get_widget(int index) {
  if (index == 0)
    return &label;
  else
    return &entry;
}

Gtk::Widget *StringView::get_gtk_widget()
{
  return &entry;
}

void StringView::set_sensitive(bool b)
{
  entry.set_sensitive(b);
  label.set_sensitive(b);
}

void StringView::update_valid()
{
  update_text_color(entry, valid);
}

void StringView::on_signal_changed()
{
  if (!lock) {
    lock = true;

    std::string s = entry.get_text();
    valid = model->schema->is_valid(s);
    model->set_value(entry.get_text());
    update_valid();
    model->forward_change_event();

    lock = false;
  }
}

bool StringView::is_valid() {
  //trace("is_valid = %s.", valid ? "true" : "false");
  return valid;
}

void StringView::on_event(const ChangeEvent &ce)
{
  if (!lock)
  {
    lock = true;
    entry.set_text(utils::str::latin_to_utf8(model->get_string()));
    lock = false;
  }
}

/*******************************************************************
 *       FIXED   STRING VIEW IMPLEMENTATION                        *
 *******************************************************************/

FixedStringView::FixedStringView(Attribute *model)
{
  lock = false;
  valid = model->schema->is_valid(model->get_string());
  this->model = model;
  label.set_use_markup(true);
  update_langue();
  //entry.set_editable(true);

  ChangeEvent ce;
  on_event(ce);

  //entry.set_markup(model->get_string());
  /*if (small_)
    entry.set_width_chars(12);
  else
    entry.set_width_chars(30);*/
  /*entry.signal_changed().connect(
      sigc::mem_fun(*this, &StringView::on_signal_changed));
  entry.signal_focus_in_event().connect(
      sigc::mem_fun(*this, &AttributeView::on_focus_in));*/
  update_valid();
}

void FixedStringView::update_langue() {
  label.set_markup("<b>" + NodeView::mk_label_colon(model->schema->name) + "</b>");
}

unsigned int FixedStringView::get_nb_widgets() {
  return 2;
}

Gtk::Widget *FixedStringView::get_widget(int index) {
  if (index == 0)
    return &label;
  else
    return &entry;
}

Gtk::Widget *FixedStringView::get_gtk_widget()
{
  return &entry;
}

void FixedStringView::set_sensitive(bool b)
{
  entry.set_sensitive(b);
  label.set_sensitive(b);
}

void FixedStringView::update_valid()
{
  //update_text_color(entry, valid);
}

/*void FixedStringView::on_signal_changed()
{
  if (!lock) {
    lock = true;

    std::string s = entry.get_text();
    valid = model->schema->is_valid(s);
    model->set_value(entry.get_text());
    update_valid();
    model->forward_change_event();

    lock = false;
  }
}*/

bool FixedStringView::is_valid() {
  //trace("is_valid = %s.", valid ? "true" : "false");
  return valid;
}

void FixedStringView::on_event(const ChangeEvent &ce)
{
  if (!lock)
  {
    lock = true;


    std::string nom = model->get_string();
    for (unsigned int j = 0; j < model->schema->enumerations.size(); j++)
    {
      Enumeration &e = model->schema->enumerations[j];
      if (e.value.compare(nom) == 0) {
        nom = e.name.get_localized();
        break;
      }
    }

    entry.set_markup(utils::str::latin_to_utf8(nom));
    lock = false;
  }
}

}

}
}

