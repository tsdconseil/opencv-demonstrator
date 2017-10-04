#ifndef STDVIEW_FIELDS_H
#define STDVIEW_FIELDS_H

#include "mmi/stdview.hpp"
#include "comm/serial.hpp"

namespace utils
{
namespace mmi
{

namespace fields
{

class VueDecimal: public AttributeView 
{
public:
  VueDecimal(Attribute *model, int compatibility_mode = 0);
  virtual ~VueDecimal() {}
  bool is_valid();
  void set_sensitive(bool b);

  // Deprecated
  unsigned int get_nb_widgets();
  // Deprecated
  Gtk::Widget *get_widget(int index);

  Gtk::Widget *get_gtk_widget();

  void update_langue();
private:
  void update_valid();
  void on_event(const ChangeEvent &ce);
  void on_signal_changed();
  bool on_signal_focus_out(GdkEventFocus *gef);
  std::string class_name() const {
    return "decimal-spin-view";
  }
  Gtk::SpinButton spin;
  Gtk::Label label, label_unit;
  Gtk::Alignment align;
  Gtk::HBox hbox;
  bool is_sensitive;
  bool valid;
};

class VueFloat: public AttributeView {
public:
  VueFloat(Attribute *model);
  virtual ~VueFloat() {
  }

  void set_sensitive(bool b);
  // Deprecated
  unsigned int get_nb_widgets();
  // Deprecated
  Gtk::Widget *get_widget(int index);
  void update_langue();

  Gtk::Widget *get_gtk_widget();

private:
  void on_event(const ChangeEvent &ce);
  void on_signal_changed();
  std::string class_name() const {
    return "float-view";
  }
  Gtk::SpinButton spin;
  Gtk::Label label, label_unit;
};

class VueChaine: public AttributeView
{
public:
  VueChaine(Attribute *model, bool small_ = false);
  virtual ~VueChaine();

  void set_sensitive(bool b);
  unsigned int get_nb_widgets();
  Gtk::Widget *get_widget(int index);
  void update_langue();
  bool is_valid();

  Gtk::Widget *get_gtk_widget();
private:
  bool valid;
  void on_event(const ChangeEvent &ce);
  void on_signal_changed();
  void update_valid();
  std::string class_name() const {
    return "string-view";
  }
  Gtk::Label label, label_unit;
  Gtk::Entry entry;
};

class VueChaineConstante: public AttributeView
{
public:
  VueChaineConstante(Attribute *model);
  virtual ~VueChaineConstante()
  {
  }

  void set_sensitive(bool b);
  unsigned int get_nb_widgets();
  Gtk::Widget *get_widget(int index);
  void update_langue();
  bool is_valid();

  Gtk::Widget *get_gtk_widget();
private:
  bool valid;
  void on_event(const ChangeEvent &ce);
  //void on_signal_changed();
  void update_valid();
  std::string class_name() const {
    return "string-view";
  }
  Gtk::Label label, label_unit;
  Gtk::Label entry;
};

class VueTexte: public AttributeView {
public:
  VueTexte(Attribute *model, bool small_);
  virtual ~VueTexte() {
  }

  void set_sensitive(bool b);
  unsigned int get_nb_widgets();
  Gtk::Widget *get_widget(int index);
  void update_langue();
  Gtk::Widget *get_gtk_widget();
private:
  void on_event(const ChangeEvent &ce);
  void on_signal_changed();
  std::string class_name() const {
    return "txt-view";
  }
  Gtk::Label label;
  Gtk::TextView view;
  Gtk::ScrolledWindow scroll;
  Gtk::Frame frame;
};

class VueDossier: public AttributeView {
public:
  VueDossier(Attribute *model);
  virtual ~VueDossier() {
  }

  void set_sensitive(bool b);
  unsigned int get_nb_widgets();
  Gtk::Widget *get_widget(int index);
  void update_langue();
  Gtk::Widget *get_gtk_widget();
private:
  bool on_focus_in(GdkEventFocus *gef);
  bool on_frame_event(GdkEvent *gef);
  void on_event(const ChangeEvent &ce);
  void on_folder_changed();
  std::string class_name() const {
    return "folder-view";
  }
  Gtk::Label label;
  Gtk::FileChooserButton *bouton;
  Gtk::FileChooserDialog *fcd;
};

class VueFichier: public AttributeView
{
public:
  VueFichier(Attribute *model, bool fichier_existant = true);
  virtual ~VueFichier(); 


  void set_sensitive(bool b);
  unsigned int get_nb_widgets();
  Gtk::Widget *get_widget(int index);
  Gtk::Widget *get_gtk_widget();
  void maj_langue();
private:

  bool fichier_existant;

  class BoutonFichier: public Gtk::Button//Gtk::FileChooserButton
  {
  public:
    BoutonFichier(Gtk::FileChooserDialog *fcd, VueFichier *parent);
  private:
    void gere_clic();
    //bool on_button_press_event(GdkEventButton* event);
    Gtk::FileChooserDialog *fcd;
    VueFichier *parent;
  };


  bool gere_bouton(GdkEventButton *non_ut);
  void maj_chemin(const std::string &s);
  bool on_focus_in(GdkEventFocus *gef);
  bool on_frame_event(GdkEvent *gef);
  void on_event(const ChangeEvent &ce);
  void gere_changement_fichier();

  Gtk::Label label;
  BoutonFichier *bouton;
  Gtk::FileChooserDialog *fcd;
  friend class BoutonFichier;
};

class VueHexa: public AttributeView {
public:
  VueHexa(Attribute *model);
  virtual ~VueHexa() {
  }

  void set_sensitive(bool b);
  unsigned int get_nb_widgets();
  Gtk::Widget *get_widget(int index);
  Gtk::Widget *get_gtk_widget();
  void update_langue();
private:
  void on_event(const ChangeEvent &ce);
  void on_signal_changed();
  std::string class_name() const {
    return "hexa-view";
  }
  Gtk::Label label, label_unit;
  Gtk::Entry entry;
  bool valid;
};

class VueOctets: public AttributeView {
public:
  VueOctets(Attribute *model);
  virtual ~VueOctets() {
  }

  void set_sensitive(bool b);
  unsigned int get_nb_widgets();
  Gtk::Widget *get_widget(int index);
  Gtk::Widget *get_gtk_widget();
  void update_langue();
private:
  void on_event(const ChangeEvent &ce);
  void on_signal_changed();
  std::string class_name() const {
    return "bytes-view";
  }
  Gtk::Label label, label_unit;
  Gtk::Entry entry;
  bool valid;
};

class VueBouleen: public AttributeView {
public:
  VueBouleen(Attribute *model);
  virtual ~VueBouleen();

  void set_sensitive(bool b);
  unsigned int get_nb_widgets();
  Gtk::Widget *get_widget(int index);
  Gtk::Widget *get_gtk_widget();
  void update_langue();
private:
  void on_event(const ChangeEvent &ce);
  void on_signal_toggled();
  Gtk::Label label, lab;

  Gtk::CheckButton check;
  bool lock;
  std::string class_name() const {
    return "boolean-view";
  }
};

class VueLed: public AttributeView {
public:
  VueLed(Attribute *model);
  virtual ~VueLed();

  void set_sensitive(bool b);
  unsigned int get_nb_widgets();
  Gtk::Widget *get_widget(int index);
  Gtk::Widget *get_gtk_widget();
  void update_langue();
private:
  void on_event(const ChangeEvent &ce);
  int on_signal_toggled(const LedEvent &le);
  Gtk::Label label, lab;
  GtkLed led;
  Gtk::HBox vbox;
  Gtk::Alignment align;
  bool lock;
  std::string class_name() const {
    return "led-view";
  }
};

class VueCombo: public AttributeView {
public:
  VueCombo(Attribute *model);
  virtual ~VueCombo() {
  }

  void set_sensitive(bool b);
  unsigned int get_nb_widgets();
  Gtk::Widget *get_widget(int index);
  Gtk::Widget *get_gtk_widget();
  void update_langue();
private:
  void on_event(const ChangeEvent &ce);
  void on_combo_changed();
  std::string class_name() const {
    return "combo-view";
  }
  Gtk::Label label;
  Gtk::ComboBox combo;

  class ModelColumns: public Gtk::TreeModel::ColumnRecord {
  public:
    ModelColumns() {
      add(m_col_name);
      add(m_col_real_name);
      add(m_col_unit);
    }
    Gtk::TreeModelColumn<Glib::ustring> m_col_name;
    Gtk::TreeModelColumn<Glib::ustring> m_col_unit;
    Gtk::TreeModelColumn<Glib::ustring> m_col_real_name;
  };

  ModelColumns columns;

  Glib::RefPtr<Gtk::ListStore> tree_model;
};

class VueSelPortCOM: public AttributeView {
public:
  VueSelPortCOM(Attribute *model);
  virtual ~VueSelPortCOM() {
  }

  void set_sensitive(bool b);
  unsigned int get_nb_widgets();
  Gtk::Widget *get_widget(int index);
  Gtk::Widget *get_gtk_widget();
  void update_langue();
private:
  std::vector<comm::SerialInfo> serials;
  void on_event(const ChangeEvent &ce);
  void on_combo_changed();
  std::string class_name() const {
    return "serial-view";
  }
  Gtk::Label label;
  Gtk::ComboBox combo;

  class ModelColumns: public Gtk::TreeModel::ColumnRecord {
  public:
    ModelColumns() {
      add(m_col_name);
      add(m_col_real_name);
      add(m_col_unit);
    }
    Gtk::TreeModelColumn<Glib::ustring> m_col_name;
    Gtk::TreeModelColumn<Glib::ustring> m_col_unit;
    Gtk::TreeModelColumn<Glib::ustring> m_col_real_name;
  };

  ModelColumns columns;

  Glib::RefPtr<Gtk::ListStore> tree_model;
};

class VueChoixCouleur: public AttributeView {
public:
  VueChoixCouleur(Attribute *model);
  virtual ~VueChoixCouleur() {
  }

  void set_sensitive(bool b);
  unsigned int get_nb_widgets();
  Gtk::Widget *get_widget(int index);
  Gtk::Widget *get_gtk_widget();
  void update_langue();
private:
  void on_event(const ChangeEvent &ce);
  void on_color_changed();
  std::string class_name() const {
    return "color-view";
  }
  Gtk::Label label;
  Gtk::ColorButton color;
  ColorButton *cb;
};

class VueDate: public AttributeView {
public:
  VueDate(Attribute *model);
  virtual ~VueDate() {
  }

  void set_sensitive(bool b);
  unsigned int get_nb_widgets();
  Gtk::Widget *get_widget(int index);
  Gtk::Widget *get_gtk_widget();
  void update_langue();
private:
  void on_event(const ChangeEvent &ce);
  void on_date_changed();
  bool on_signal_focus_out(GdkEventFocus *gef);
  std::string class_name() const {
    return "date-view";
  }
  Gtk::Label label;
  Gtk::HBox hbox;
  Gtk::Alignment a_month;
  Glib::RefPtr<Gtk::Adjustment> adj_year, adj_month, adj_day;
  Gtk::SpinButton year, month, day;
  bool valid;
};

class VueChoix: public AttributeView, private CListener<KeyPosChangeEvent> {
public:
  VueChoix(Attribute *model, Node parent, NodeViewConfiguration config);
  virtual ~VueChoix();
  void update_langue();
  Gtk::Widget *get_widget(int index);
  Gtk::Widget *get_gtk_widget();
  unsigned int get_nb_widgets();
  void set_sensitive(bool b);
  bool is_valid();
private:
  void on_event(const KeyPosChangeEvent &kpce) {
    CProvider < KeyPosChangeEvent > ::dispatch(kpce);
  }
  void update_sub_view();
  void on_event(const ChangeEvent &ce);
  void on_radio_activate(unsigned int num);
  std::string class_name() const {
    return "choice-view";
  }
  Gtk::RadioButton **radios;
  Gtk::RadioButton::Group group;
  unsigned int nb_choices;
  Gtk::VBox vbox;
  JFrame frame;
  NodeView *current_view;
  Node model_parent;
  NodeViewConfiguration config;
};

}
}
}

#endif

