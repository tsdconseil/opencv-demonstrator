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

class DecimalSpinView: public AttributeView 
{
public:
  DecimalSpinView(Attribute *model, int compatibility_mode = 0);
  virtual ~DecimalSpinView() {}
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

class FloatView: public AttributeView {
public:
  FloatView(Attribute *model);
  virtual ~FloatView() {
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

class StringView: public AttributeView
{
public:
  StringView(Attribute *model, bool small_ = false);
  virtual ~StringView();

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

class FixedStringView: public AttributeView
{
public:
  FixedStringView(Attribute *model);
  virtual ~FixedStringView()
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

class TxtView: public AttributeView {
public:
  TxtView(Attribute *model, bool small_);
  virtual ~TxtView() {
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

class FolderView: public AttributeView {
public:
  FolderView(Attribute *model);
  virtual ~FolderView() {
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
  Gtk::FileChooserButton *button;
  Gtk::FileChooserDialog *fcd;
};

class FileView: public AttributeView {
public:
  FileView(Attribute *model);
  virtual ~FileView(); 


  void set_sensitive(bool b);
  unsigned int get_nb_widgets();
  Gtk::Widget *get_widget(int index);
  Gtk::Widget *get_gtk_widget();
  void update_langue();
private:
  bool on_focus_in(GdkEventFocus *gef);
  bool on_frame_event(GdkEvent *gef);
  void on_event(const ChangeEvent &ce);
  void on_file_changed();
  std::string class_name() const {
    return "file-view";
  }
  Gtk::Label label;
  Gtk::FileChooserButton *button;
  Gtk::FileChooserDialog *fcd;
};

class HexaView: public AttributeView {
public:
  HexaView(Attribute *model);
  virtual ~HexaView() {
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

class BytesView: public AttributeView {
public:
  BytesView(Attribute *model);
  virtual ~BytesView() {
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

class BooleanView: public AttributeView {
public:
  BooleanView(Attribute *model);
  virtual ~BooleanView();

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

class LedView: public AttributeView {
public:
  LedView(Attribute *model);
  virtual ~LedView();

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

class ComboView: public AttributeView {
public:
  ComboView(Attribute *model);
  virtual ~ComboView() {
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

class SerialView: public AttributeView {
public:
  SerialView(Attribute *model);
  virtual ~SerialView() {
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

class ColorView: public AttributeView {
public:
  ColorView(Attribute *model);
  virtual ~ColorView() {
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

class DateView: public AttributeView {
public:
  DateView(Attribute *model);
  virtual ~DateView() {
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

class ChoiceView: public AttributeView, private CListener<KeyPosChangeEvent> {
public:
  ChoiceView(Attribute *model, Node parent, NodeViewConfiguration config);
  virtual ~ChoiceView();
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
  void on_radio_activate();
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

