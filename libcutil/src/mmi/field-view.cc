
#include "mmi/stdview.hpp"
#include "mmi/ColorCellRenderer2.hpp"
#include "mmi/renderers.hpp"
#include "mmi/stdview-fields.hpp"

#include "comm/serial.hpp"

#include <string.h>
#include <stdlib.h>
#include <limits.h>

using namespace std;

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
//#   ifdef WIN
    w.override_color(Gdk::RGBA("#000000"), Gtk::STATE_FLAG_NORMAL);
//#   else
//    w.override_color(Gdk::RGBA("#ffffff"), Gtk::STATE_FLAG_NORMAL);
//#   endif
  }
  else
    w.override_color(Gdk::RGBA("#ff0000"), Gtk::STATE_FLAG_NORMAL);
}

/*******************************************************************
 *               BYTES VIEW IMPLEMENTATION                          *
 *******************************************************************/

VueOctets::VueOctets(Attribute *model) {
  this->model = model;
  valid = model->schema->is_valid(model->get_string());
  lock = false;
  label.set_use_markup(true);
  update_langue();
  entry.set_editable(true);
  entry.set_width_chars(20);
  entry.set_text(model->get_string());
  entry.signal_changed().connect(
      sigc::mem_fun(*this, &VueOctets::on_signal_changed));
  entry.signal_focus_in_event().connect(
      sigc::mem_fun(*this, &AttributeView::on_focus_in));
}

void VueOctets::update_langue() {
  label.set_markup("<b>" + NodeView::mk_label_colon(model->schema->name) + "</b>");
}

unsigned int VueOctets::get_nb_widgets() {
  return 2;
}

Gtk::Widget *VueOctets::get_widget(int index) {
  if (index == 0)
    return &label;
  else
    return &entry;
}

Gtk::Widget *VueOctets::get_gtk_widget()
{
  return &entry;
}

void VueOctets::set_sensitive(bool b) {
  entry.set_sensitive(b);
  label.set_sensitive(b);
}

void VueOctets::on_signal_changed() {
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

void VueOctets::on_event(const ChangeEvent &ce) {
  if (!lock) {
    lock = true;
    entry.set_text(model->get_string());
    lock = false;
  }
}

/*******************************************************************
 *               HEXA VIEW IMPLEMENTATION                          *
 *******************************************************************/

VueHexa::VueHexa(Attribute *model) {
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
      sigc::mem_fun(*this, &VueHexa::on_signal_changed));
}

void VueHexa::update_langue() {
  label.set_markup("<b>" + NodeView::mk_label_colon(model->schema->name) + "</b>");
}

unsigned int VueHexa::get_nb_widgets() {
  return 2;
}

Gtk::Widget *VueHexa::get_widget(int index) {
  if (index == 0)
    return &label;
  else
    return &entry;
}

Gtk::Widget *VueHexa::get_gtk_widget()
{
  return &entry;
}

void VueHexa::set_sensitive(bool b) {
  entry.set_sensitive(b);
  label.set_sensitive(b);
}




void VueHexa::on_signal_changed() {
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

void VueHexa::on_event(const ChangeEvent &ce) {
  if (!lock) {
    lock = true;
    entry.set_text(model->get_string());
    lock = false;
  }
}

/*******************************************************************
 *               TEXT VIEW IMPLEMENTATION                        *
 *******************************************************************/

VueTexte::VueTexte(Attribute *model, bool small_) {
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
      sigc::mem_fun(*this, &VueTexte::on_signal_changed));

  scroll.set_policy(Gtk::POLICY_NEVER, Gtk::POLICY_AUTOMATIC);
  scroll.add(view);
  scroll.set_size_request(350, 150); //225,75);
  frame.add(scroll);
}

void VueTexte::update_langue() {
  label.set_markup("<b>" + NodeView::mk_label(model->schema->name) + "</b>");
}

unsigned int VueTexte::get_nb_widgets() {
  return 2;
}

Gtk::Widget *VueTexte::get_widget(int index) {
  if (index == 0)
    return &label;
  else
    return &frame;
}

Gtk::Widget *VueTexte::get_gtk_widget()
{
  return &frame;
}

void VueTexte::set_sensitive(bool b) {
  view.set_sensitive(b);
  label.set_sensitive(b);
}

void VueTexte::on_signal_changed() {
  if (!lock) {
    lock = true;
    model->set_value(view.get_buffer()->get_text());
    lock = false;
  }
}

void VueTexte::on_event(const ChangeEvent &ce) {
  if (!lock) {
    lock = true;
    view.get_buffer()->set_text(model->get_string());
    lock = false;
  }
}

/*******************************************************************
 *               DECIMAL SPIN VIEW IMPLEMENTATION                  *
 *******************************************************************/

VueDecimal::VueDecimal(Attribute *model, int compatibility_mode)
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

  //infos("decimal view(%s): min = %d, max = %d.", model->name.c_str(), model->schema.get_min(), model->schema.get_max());

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
      sigc::mem_fun(*this, &VueDecimal::on_signal_changed));
  spin.signal_editing_done().connect(
      sigc::mem_fun(*this, &VueDecimal::on_signal_changed));
  spin.signal_changed().connect(
      sigc::mem_fun(*this, &VueDecimal::on_signal_changed));
  spin.signal_focus_out_event().connect(
      sigc::mem_fun(*this, &VueDecimal::on_signal_focus_out));
  spin.signal_focus_in_event().connect(
      sigc::mem_fun(*this, &AttributeView::on_focus_in));
  update_valid();
}

void VueDecimal::update_langue() {
  set_sensitive(is_sensitive);
  //label.set_markup("<b>" + mk_label(&(model->schema)) + "</b>");
}

unsigned int VueDecimal::get_nb_widgets() {
  //if(model->schema.has_unit())
  //return 3;
  return 2;
}

Gtk::Widget *VueDecimal::get_widget(int index) {
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

Gtk::Widget *VueDecimal::get_gtk_widget()
{
  return &spin;
}

void VueDecimal::set_sensitive(bool b) {
  is_sensitive = b;
  spin.set_sensitive(b);
  label.set_sensitive(b);
  label_unit.set_sensitive(b);
  label.set_markup("<b>" + NodeView::mk_label_colon(model->schema->name) + "</b>");
}

bool VueDecimal::on_signal_focus_out(GdkEventFocus *gef) {
  //infos("focus out.");
  //on_signal_changed();
  return true;
}

void VueDecimal::update_valid()
{
  update_text_color(spin, valid);
}

bool VueDecimal::is_valid() {
  return valid;
}

void VueDecimal::on_signal_changed() {
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

void VueDecimal::on_event(const ChangeEvent &ce) {
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

VueFloat::VueFloat(Attribute *model) {
  lock = false;
  this->model = model;
  label.set_use_markup(true);
  label.set_markup("<b>" + NodeView::mk_label_colon(model->schema->name) + "</b>");
  spin.set_editable(true);

  int digits = model->schema->digits;

  //utils::infos("float-view[%s]: digits = %d", model->schema->name.get_id().c_str(), digits);

  if(digits <= 0)
    digits = 6;

  float increment = 1.0;
  for(auto i = 0u; i < (unsigned int) digits; i++)
    increment /= 10;

  spin.set_increments(increment, 1);
  spin.set_digits(digits);
  //infos("decimal view(%s): min = %d, max = %d.", model->name.c_str(), model->schema.get_min(), model->schema.get_max());

  spin.set_range(model->schema->get_min(), model->schema->get_max());
  //infos("Setting spin value = %f, str = %s", atof(model->value.c_str()), model->value.c_str());
  spin.set_value(model->get_float());   //atof(model->value.c_str()));
  if (model->schema->has_unit())
  {
    std::string unit = model->schema->unit;
    if (langue.has_item(unit))
      unit = langue.get_item(unit);
    label_unit.set_text(std::string("  ") + unit + "  ");
  }
  spin.signal_value_changed().connect(
      sigc::mem_fun(*this, &VueFloat::on_signal_changed));
}

void VueFloat::update_langue() {
  label.set_markup("<b>" + NodeView::mk_label(model->schema->name) + "</b>");
}

unsigned int VueFloat::get_nb_widgets() {
  if (model->schema->has_unit())
    return 3;
  return 2;
}

Gtk::Widget *VueFloat::get_widget(int index) {
  if (index == 0)
    return &label;
  else if (index == 1)
    return &spin;
  else
    return &label_unit;
}

Gtk::Widget *VueFloat::get_gtk_widget()
{
  return &spin;
}

void VueFloat::set_sensitive(bool b) {
  spin.set_sensitive(b);
  label.set_sensitive(b);
  label_unit.set_sensitive(b);
}

void VueFloat::on_signal_changed() {
  if (!lock) {
    lock = true;
    //printf("Spin change: %s\n", Util::int2str(spin.get_value_as_float()).c_str());
    model->set_value((float) spin.get_value());
    lock = false;
  }
}

void VueFloat::on_event(const ChangeEvent &ce) {
  if (!lock) {
    lock = true;
    spin.set_value(model->get_float());//atof(model->value.c_str()));
    lock = false;
  }
}

/*******************************************************************
 *               BOOLEAN VIEW IMPLEMENTATION                       *
 *******************************************************************/
VueBouleen::~VueBouleen() {

}

VueBouleen::VueBouleen(Attribute *model) {
  lock = false;
  this->model = model;
  //Gtk::Label *lab = new Gtk::Label();
  lab.set_use_markup(true);
  std::string s = NodeView::mk_label(model->schema->name);
  lab.set_markup("<b>" + s + "</b>");
  check.add(lab);
  check.set_active(model->get_boolean());
  check.signal_toggled().connect(
      sigc::mem_fun(*this, &VueBouleen::on_signal_toggled));
  check.signal_focus_in_event().connect(
      sigc::mem_fun(*this, &AttributeView::on_focus_in));
}

void VueBouleen::update_langue()
{
  std::string s = NodeView::mk_label(model->schema->name);
  lab.set_markup("<b>" + s + "</b>");
}

unsigned int VueBouleen::get_nb_widgets() {
  return 1;
}

Gtk::Widget *VueBouleen::get_widget(int index) {
  return &check;
}

Gtk::Widget *VueBouleen::get_gtk_widget()
{
  return &check;
}

void VueBouleen::set_sensitive(bool b) {
  check.set_sensitive(b);
}

void VueBouleen::on_signal_toggled() {
  if (!lock) {
    lock = true;
    model->set_value(check.get_active());
    lock = false;
  }
}

void VueBouleen::on_event(const ChangeEvent &ce) {
  if (!lock) {
    lock = true;
    check.set_active(model->get_boolean());
    lock = false;
  }
}

/*******************************************************************
 *               COMBO VIEW IMPLEMENTATION                         *
 *******************************************************************/
VueCombo::VueCombo(Attribute *model) {
  lock = false;
  //infos("comboview(%s)..", model->schema.name.get_id().c_str());
  this->model = model;
  label.set_use_markup(true);

  tree_model = Gtk::ListStore::create(columns);
  combo.set_model(tree_model);

  combo.pack_start(columns.m_col_name);
  if (model->schema->has_unit())
    combo.pack_start(columns.m_col_unit);

  update_langue();

  combo.signal_changed().connect(
      sigc::mem_fun(*this, &VueCombo::on_combo_changed));
  combo.signal_focus_in_event().connect(
      sigc::mem_fun(*this, &AttributeView::on_focus_in));
}

void VueCombo::update_langue() {
  unsigned int imax;
  bool old_lock = lock;

  auto schema = model->schema;

  lock = true;
  //infos("combo: update langue...");
  label.set_markup("<b>" + NodeView::mk_label_colon(schema->name) + "</b>");
  tree_model->clear();

  //imin = 0;
  imax = schema->constraints.size();

  if ((imax == 0) && (model->schema->has_max)) {
    imax = schema->max - schema->min + 1;
  }



  unsigned int nb_enum = model->schema->enumerations.size();

  /*if(nb_enum > 0)
    infos("vue combo: %d enumerations (prem : %s)",
      nb_enum, schema->enumerations[0].name.get_localized().c_str());*/

  for (unsigned int i = 0; i < imax; i++)
  {
    std::string valeur;
    if(schema->constraints.size() > i)
      valeur = schema->constraints[i];
    else
      valeur = str::int2str(i);
    std::string nom = valeur;
    for (unsigned int j = 0; j < nb_enum; j++)
    {
      Enumeration e;

      if (i >= schema->enumerations.size()) {
        avertissement("enumeration %d not defined: attribute %s.", i,
            schema->name.get_id().c_str());

        infos("schema is: %s.\n", schema->to_string().c_str());

        break;
      }

      e = schema->enumerations[j];
      if ((e.value == valeur) || (e.name.get_id() == valeur))
      {
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

    if(schema->has_unit()) {
      row[columns.m_col_unit] = "";
      std::string unit = schema->unit;
      if (langue.has_item(unit))
        unit = langue.get_item(unit);
      row[columns.m_col_unit] = unit;
    }
  }
  unsigned int i;
  for (i = 0; i < imax; i++) {
    std::string valeur;
    if (schema->constraints.size() > i)
      valeur = schema->constraints[i];
    else
      valeur = str::int2str(i);

    //std::string valeur = model->schema.constraints[i];
    std::string nom = valeur;
    for (unsigned int j = 0; j < schema->enumerations.size(); j++)
    {
      if (i >= schema->enumerations.size())
      {
        erreur("enumeration.");
        break;
      }

      Enumeration e = schema->enumerations[i];
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
    avertissement("Not found current value (%s, %s).",
            schema->name.get_id().c_str(), s.c_str());
    for (i = 0; i < schema->constraints.size(); i++) {
      std::string valeur = schema->constraints[i];
      infos("constraint[%d] = %s.", i, valeur.c_str());
    }
  }
  //infos("combo: update langue done.");
  lock = old_lock;
}

unsigned int VueCombo::get_nb_widgets() {
  return 2;
}

Gtk::Widget *VueCombo::get_widget(int index) {
  if (index == 0)
    return &label;
  else
    return &combo;
}

Gtk::Widget *VueCombo::get_gtk_widget()
{
  return &combo;
}

void VueCombo::set_sensitive(bool b) {
  label.set_sensitive(b);
  combo.set_sensitive(b);
}

void VueCombo::on_combo_changed() {
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
      erreur("combo view: none selected.");
    lock = false;
  }
}

//    check.set_active(model->get_boolean());

void VueCombo::on_event(const ChangeEvent &ce) {
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

VueDate::VueDate(Attribute *model)
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
      sigc::mem_fun(*this, &VueDate::on_date_changed));
  month.signal_changed().connect(
      sigc::mem_fun(*this, &VueDate::on_date_changed));
  year.signal_changed().connect(
      sigc::mem_fun(*this, &VueDate::on_date_changed));

  day.signal_editing_done().connect(
      sigc::mem_fun(*this, &VueDate::on_date_changed));
  day.signal_focus_out_event().connect(
      sigc::mem_fun(*this, &VueDate::on_signal_focus_out));
  month.signal_editing_done().connect(
      sigc::mem_fun(*this, &VueDate::on_date_changed));
  month.signal_focus_out_event().connect(
      sigc::mem_fun(*this, &VueDate::on_signal_focus_out));
  year.signal_editing_done().connect(
      sigc::mem_fun(*this, &VueDate::on_date_changed));
  year.signal_focus_out_event().connect(
      sigc::mem_fun(*this, &VueDate::on_signal_focus_out));

  day.signal_focus_in_event().connect(
      sigc::mem_fun(*this, &AttributeView::on_focus_in));
  month.signal_focus_in_event().connect(
      sigc::mem_fun(*this, &AttributeView::on_focus_in));
  year.signal_focus_in_event().connect(
      sigc::mem_fun(*this, &AttributeView::on_focus_in));
}

void VueDate::update_langue() {
  label.set_markup("<b>" + NodeView::mk_label_colon(model->schema->name) + "</b>");
  if (appli_view_prm.use_touchscreen) {
    day.set_snap_to_ticks(false);
    month.set_snap_to_ticks(false);
    year.set_snap_to_ticks(false);
  }
}

unsigned int VueDate::get_nb_widgets() {
  return 2;
}

Gtk::Widget *VueDate::get_widget(int index) {
  if (index == 0)
    return &label;
  else
    return &hbox;
}

Gtk::Widget *VueDate::get_gtk_widget()
{
  return &hbox;
}

void VueDate::set_sensitive(bool b) {
  label.set_sensitive(b);
  year.set_sensitive(b);
  month.set_sensitive(b);
  day.set_sensitive(b);
}

bool VueDate::on_signal_focus_out(GdkEventFocus *gef) {
  on_date_changed();
  return true;
}

void VueDate::on_date_changed() {
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

    //infos("new date: %s.", s.c_str());

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

void VueDate::on_event(const ChangeEvent &ce) {
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

bool VueDossier::on_focus_in(GdkEventFocus *gef) {

  infos("focus in");
  //target_window->present();
  if (appli_view_prm.fixed_size) {
    fcd->resize(appli_view_prm.dx, appli_view_prm.dy);
    fcd->set_size_request(appli_view_prm.dx, appli_view_prm.dy);
    fcd->move(appli_view_prm.ox, appli_view_prm.oy);
    fcd->resize(appli_view_prm.dx, appli_view_prm.dy);
  }

  return true;
}

bool VueDossier::on_frame_event(GdkEvent *gef) {

  infos("frame event");
  //target_window->present();
  if (appli_view_prm.fixed_size)
  {
    fcd->resize(appli_view_prm.dx, appli_view_prm.dy);
    fcd->move(appli_view_prm.ox, appli_view_prm.oy);
  }

  return true;
}

VueDossier::VueDossier(Attribute *model)
{
  fcd = new Gtk::FileChooserDialog(langue.get_item("select-folder"),
      Gtk::FILE_CHOOSER_ACTION_SELECT_FOLDER);
  fcd->add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
  fcd->add_button(Gtk::Stock::OPEN, Gtk::RESPONSE_OK);

  bouton = new Gtk::FileChooserButton(*fcd);

  if (appli_view_prm.fixed_size) {
    fcd->set_position(Gtk::WIN_POS_NONE);
    fcd->resize(appli_view_prm.dx, appli_view_prm.dy);
    fcd->move(appli_view_prm.ox, appli_view_prm.oy);
    fcd->set_resizable(false);
    fcd->set_size_request(appli_view_prm.dx, appli_view_prm.dy);
    //fcd->set_resize_mode(Gtk::RESIZE_QUEUE);
    fcd->signal_focus_in_event().connect(
        sigc::mem_fun(*this, &VueDossier::on_focus_in));
    //TODO fcd->signal_frame_event().connect(
    //    sigc::mem_fun(*this, &FolderView::on_frame_event));
  } else {
    fcd->set_position(Gtk::WIN_POS_CENTER_ALWAYS);
  }

  lock = false;
  this->model = model;
  label.set_use_markup(true);
  label.set_markup("<b>" + NodeView::mk_label_colon(model->schema->name) + "</b>");

  bouton->set_action(Gtk::FILE_CHOOSER_ACTION_SELECT_FOLDER);

  ChangeEvent ce;
  on_event(ce);

# if ((GTKMM_MAJOR_VERSION * 100 + GTKMM_MINOR_VERSION) < 218)
# warning GTKMM version too old for signal_file_set
  bouton->signal_selection_changed().connect(
      sigc::mem_fun(*this, &VueDossier::on_folder_changed));
  on_folder_changed();
# else
  bouton->signal_file_set().connect(sigc::mem_fun(*this, &VueDossier::on_folder_changed));
# endif
}

void VueDossier::update_langue() {
  label.set_markup("<b>" + NodeView::mk_label_colon(model->schema->name) + "</b>");
}

unsigned int VueDossier::get_nb_widgets() {
  return 2;
}

Gtk::Widget *VueDossier::get_widget(int index) {
  if (index == 0)
    return &label;
  else
    return bouton;
}

Gtk::Widget *VueDossier::get_gtk_widget()
{
  return bouton;
}

void VueDossier::set_sensitive(bool b) {
  label.set_sensitive(b);
  bouton->set_sensitive(b);
}

void VueDossier::on_folder_changed() {
  if (!lock) {
    lock = true;
    Glib::ustring s = bouton->get_filename();
    std::string s2 = s;
    std::string s3 = str::utf8_to_latin(s2);
    infos("Folder view changed: '%s'.", s2.c_str());
    model->set_value(s3);
    lock = false;
  }
}

void VueDossier::on_event(const ChangeEvent &ce) {
  if (!lock) {
    lock = true;
    bouton->set_current_folder(model->get_string());
    lock = false;
  }
}

/*******************************************************************
 *               FILE VIEW IMPLEMENTATION                        *
 *******************************************************************/

bool VueFichier::on_focus_in(GdkEventFocus *gef) {

  infos("focus in");
  //target_window->present();
  if (appli_view_prm.fixed_size) {
    fcd->resize(appli_view_prm.dx, appli_view_prm.dy);
    fcd->set_size_request(appli_view_prm.dx, appli_view_prm.dy);
    fcd->move(appli_view_prm.ox, appli_view_prm.oy);
    fcd->resize(appli_view_prm.dx, appli_view_prm.dy);
  }

  return true;
}

bool VueFichier::on_frame_event(GdkEvent *gef) {

  infos("frame event");
  //target_window->present();
  if (appli_view_prm.fixed_size) {
    fcd->resize(appli_view_prm.dx, appli_view_prm.dy);
    fcd->move(appli_view_prm.ox, appli_view_prm.oy);
  }

  return true;
}

VueFichier::~VueFichier()
{

  delete bouton;
  delete fcd;
}

bool VueFichier::gere_bouton(GdkEventButton *non_ut)
{
  infos("Detecte clic sur bouton vue fichier.");


  //fcd->set_action(Gtk::FILE_CHOOSER_ACTION_SAVE);
  if(fcd->run() == Gtk::RESPONSE_OK)
  {
    auto s = fcd->get_filename();
    infos("Mise a jour du texte bouton [%s]...", s.c_str());

    auto e = utils::files::get_extension(s);
    if(e.size() == 0)
    {
      avertissement("Pas d'extension precisee.");
      std::string ext = model->schema->extension;
      if(ext.size() > 0)
      {
        s += "." + ext;
        infos("ajoute extension [%s] >> ", ext.c_str(), s.c_str());
      }
    }

#   ifdef ANCIEN_BOUTON
    bouton->set_filename(s);
#   else
    auto s_resume = utils::str::get_filename_resume(s);
    infos("Nouvelle valeur de fichier : [%s], RES = [%s]", s.c_str(), s_resume.c_str());
    bouton->set_label(s_resume);
    model->set_value(s);
#   endif
  }
  fcd->hide();
  infos("retour gere bouton.");
  return true;
}

void VueFichier::BoutonFichier::gere_clic()
{
  parent->gere_bouton(nullptr);
}

VueFichier::BoutonFichier::BoutonFichier(Gtk::FileChooserDialog *fcd, VueFichier *parent)
 //: Gtk::FileChooserButton(*fcd)
{
  this->fcd = fcd;
  this->parent = parent;
  this->signal_clicked().connect(sigc::mem_fun(*this, &VueFichier::BoutonFichier::gere_clic));
}


VueFichier::VueFichier(Attribute *model, bool fichier_existant)
{
  this->fichier_existant = fichier_existant;
  fcd = new Gtk::FileChooserDialog(langue.get_item("select-fichier"),
      fichier_existant ? Gtk::FILE_CHOOSER_ACTION_OPEN : Gtk::FILE_CHOOSER_ACTION_SAVE);
  fcd->add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
  fcd->add_button(Gtk::Stock::OPEN, Gtk::RESPONSE_OK);

  bouton = new /*Gtk::FileChooserButton(*fcd)*/BoutonFichier(fcd, this);

  std::string ext = model->schema->extension;
  if (ext.size() > 0)
  {
    Glib::RefPtr<Gtk::FileFilter> filter = Gtk::FileFilter::create();
    filter->set_name(ext + " file");
    filter->add_pattern(std::string("*.") + ext);
    infos("Extension choisie : [%s]", ext.c_str());
    fcd->add_filter(filter);
  }

  // TODO
  //bouton->set_image()

  if (appli_view_prm.fixed_size) {
    fcd->set_position(Gtk::WIN_POS_NONE);
    fcd->resize(appli_view_prm.dx, appli_view_prm.dy);
    fcd->move(appli_view_prm.ox, appli_view_prm.oy);
    fcd->set_resizable(false);
    fcd->set_size_request(appli_view_prm.dx, appli_view_prm.dy);
    //fcd->set_resize_mode(Gtk::RESIZE_QUEUE);
    fcd->signal_focus_in_event().connect(
        sigc::mem_fun(*this, &VueFichier::on_focus_in));
    //TODO fcd->signal_frame_event().connect(
    //    sigc::mem_fun(*this, &FileView::on_frame_event));
  } else {
    fcd->set_position(Gtk::WIN_POS_CENTER_ALWAYS);
  }

  lock = false;
  this->model = model;
  label.set_use_markup(true);
  label.set_markup("<b>" + NodeView::mk_label_colon(model->schema->name) + "</b>");

  maj_chemin(model->get_string());

# ifdef ANCIEN_BOUTON
  bouton->set_action(Gtk::FILE_CHOOSER_ACTION_OPEN); // Ou Save impossible
  bouton->set_width_chars(20);
  bouton->signal_button_press_event().connect(sigc::mem_fun(*this, &VueFichier::gere_bouton));



  std::string ext = model->schema->extension;

  if (ext.size() > 0) {
    Glib::RefPtr<Gtk::FileFilter> filter = Gtk::FileFilter::create();
    filter->set_name(ext + " file");
    filter->add_pattern(std::string("*.") + ext);
    bouton->add_filter(filter);
  }
# endif

  //ChangeEvent ce;
  //on_event(ce);
  //button->set_filename(model->get_string());
  maj_chemin(model->get_string());

# ifdef ANCIEN_BOUTON
  bouton->signal_selection_changed().connect(
      sigc::mem_fun(*this, &VueFichier::gere_changement_fichier));
# endif
}

void VueFichier::maj_chemin(const std::string &s)
{
  auto s2 = s;
  //if(s.find("$DATA") != std::string::npos)
  if(s.substr(0, 5) == "$DATA")
  {
    s2 = utils::get_fixed_data_path() + s.substr(5, s.size() - 5);
  }
# ifdef ANCIEN_BOUTON
  bouton->set_filename(s2);
# else
  s2 = utils::str::get_filename_resume(s2);
  bouton->set_label(s2);
# endif
}

void VueFichier::maj_langue() {
  label.set_markup("<b>" + NodeView::mk_label_colon(model->schema->name) + "</b>");
}

unsigned int VueFichier::get_nb_widgets() {
  return 2;
}

Gtk::Widget *VueFichier::get_widget(int index) {
  if (index == 0)
    return &label;
  else
    return bouton;
}

Gtk::Widget *VueFichier::get_gtk_widget()
{
  return bouton;
}

void VueFichier::set_sensitive(bool b) {
  label.set_sensitive(b);
  bouton->set_sensitive(b);
}

void VueFichier::gere_changement_fichier()
{
# if 0
  if (!lock) {
    lock = true;
# ifdef ANCIEN_BOUTON
    Glib::ustring s = bouton->get_filename();
# else
    Glib::ustring s = bouton->get_label();
# endif
    std::string s2 = s;
    infos("Changement nom fichier sur bouton: [%s].", s2.c_str());
    if(s2.size() > 0)
      model->set_value(s);
    lock = false;
  }
# endif
}

void VueFichier::on_event(const ChangeEvent &ce) {
  if (!lock)
  {
    lock = true;
    auto s = model->get_string();
    infos("Changement modele --> bouton [%s].", s.c_str());
    maj_chemin(s);
    //bouton->set_filename(s);
    lock = false;
  }
}

/*******************************************************************
 *               SERIAL VIEW IMPLEMENTATION                        *
 *******************************************************************/
VueSelPortCOM::VueSelPortCOM(Attribute *model) {
  lock = false;
  this->model = model;
  label.set_use_markup(true);

  tree_model = Gtk::ListStore::create(columns);
  combo.set_model(tree_model);

  combo.pack_start(columns.m_col_name);
  if (model->schema->has_unit())
    combo.pack_start(columns.m_col_unit);

  if(comm::Serial::enumerate(serials))
  {
    comm::SerialInfo si;
    for(auto i = 0; i < 70; i++)
    {
      si.name = "COM" + utils::str::int2str(i);
      si.complete_name = si.name;
      si.techno = "";
      serials.push_back(si);
    }
  }

  update_langue();

  combo.signal_changed().connect(
      sigc::mem_fun(*this, &VueSelPortCOM::on_combo_changed));
}

void VueSelPortCOM::update_langue() {
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

unsigned int VueSelPortCOM::get_nb_widgets() {
  return 2;
}

Gtk::Widget *VueSelPortCOM::get_widget(int index) {
  if (index == 0)
    return &label;
  else
    return &combo;
}

Gtk::Widget *VueSelPortCOM::get_gtk_widget()
{
  return &combo;
}

void VueSelPortCOM::set_sensitive(bool b) {
  label.set_sensitive(b);
  combo.set_sensitive(b);
}

void VueSelPortCOM::on_combo_changed() {
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
      erreur("combo view: none selected.");
    lock = false;
  }
}

void VueSelPortCOM::on_event(const ChangeEvent &ce) {
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

VueChoixCouleur::VueChoixCouleur(Attribute *model) {
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
      sigc::mem_fun(*this, &VueChoixCouleur::on_color_changed));

  //if(appli_view_prm.use_touchscreen)
  {
    cb = new ColorButton(model);
  }
  /*else
   {
   cb = nullptr;
   }*/

}

void VueChoixCouleur::update_langue() {
  label.set_markup("<b>" + NodeView::mk_label_colon(model->schema->name) + "</b>");
}

unsigned int VueChoixCouleur::get_nb_widgets() {
  return 2;
}

Gtk::Widget *VueChoixCouleur::get_widget(int index) {
  if (index == 0)
    return &label;
  else {
    if (cb == nullptr)
      return &color;
    else
      return cb;
  }
}

Gtk::Widget *VueChoixCouleur::get_gtk_widget()
{
  if (cb == nullptr)
    return &color;
  else
    return cb;
}

void VueChoixCouleur::set_sensitive(bool b) {
  label.set_sensitive(b);
  color.set_sensitive(b);
}

void VueChoixCouleur::on_color_changed() {
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

void VueChoixCouleur::on_event(const ChangeEvent &ce) {
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

VueChaine::VueChaine(Attribute *model, bool small_)
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
      sigc::mem_fun(*this, &VueChaine::on_signal_changed));
  entry.signal_focus_in_event().connect(
      sigc::mem_fun(*this, &AttributeView::on_focus_in));
  update_valid();
}

VueChaine::~VueChaine()
{
  //model->CProvider<ChangeEvent>::remove_listener(this);
}

void VueChaine::update_langue() {
  label.set_markup("<b>" + NodeView::mk_label_colon(model->schema->name) + "</b>");
}

unsigned int VueChaine::get_nb_widgets() {
  return 2;
}

Gtk::Widget *VueChaine::get_widget(int index) {
  if (index == 0)
    return &label;
  else
    return &entry;
}

Gtk::Widget *VueChaine::get_gtk_widget()
{
  return &entry;
}

void VueChaine::set_sensitive(bool b)
{
  entry.set_sensitive(b);
  label.set_sensitive(b);
}

void VueChaine::update_valid()
{
  update_text_color(entry, valid);
}

void VueChaine::on_signal_changed()
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

bool VueChaine::is_valid() {
  //infos("is_valid = %s.", valid ? "true" : "false");
  return valid;
}

void VueChaine::on_event(const ChangeEvent &ce)
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

VueChaineConstante::VueChaineConstante(Attribute *model)
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

void VueChaineConstante::update_langue() {
  label.set_markup("<b>" + NodeView::mk_label_colon(model->schema->name) + "</b>");
}

unsigned int VueChaineConstante::get_nb_widgets() {
  return 2;
}

Gtk::Widget *VueChaineConstante::get_widget(int index) {
  if (index == 0)
    return &label;
  else
    return &entry;
}

Gtk::Widget *VueChaineConstante::get_gtk_widget()
{
  return &entry;
}

void VueChaineConstante::set_sensitive(bool b)
{
  entry.set_sensitive(b);
  label.set_sensitive(b);
}

void VueChaineConstante::update_valid()
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

bool VueChaineConstante::is_valid() {
  //infos("is_valid = %s.", valid ? "true" : "false");
  return valid;
}

void VueChaineConstante::on_event(const ChangeEvent &ce)
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

    std::string s = utils::str::latin_to_utf8(nom);
    if(model->schema->has_unit())
      s += " " + model->schema->unit;
    //if(model->schema->is_hexa)
     // s = "0x" + s;

    entry.set_markup(s);
    lock = false;
  }
}

}

}
}

