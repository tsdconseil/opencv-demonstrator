#ifndef RENDERERS_HPP
#define RENDERERS_HPP


#include <glibmm/property.h>

#include "../journal.hpp"
#include "modele.hpp"
#include "cutil.hpp"
#include "mmi/gtkutil.hpp"


namespace utils
{
namespace mmi
{


class RefCellEditable:
  public Gtk::EventBox, 
  public Gtk::CellEditable
{
public:
  // Ctor/Dtor
  RefCellEditable(const Glib::ustring& path);//, Node model, std::string ref_name);
  virtual ~RefCellEditable();

  void setup_model(Node model, std::string ref_name);
	
  // Return creation path
  Glib::ustring get_path() const;

  // Get and set text
  void set_text(const Glib::ustring& text);
  Glib::ustring get_text() const;
	
  // Editing cancelled
  bool get_editing_cancelled() const { return editing_cancelled_; };
	
  // Return button width
  static int get_button_width();
	
  // Return ColorArea width
  static int get_color_area_width();
	
  // Signal for editing done
  typedef sigc::signal<void> signal_editing_done_t;
  signal_editing_done_t& signal_editing_done() { return signal_editing_done_; };
	
protected:
  //std::vector<std::string> constraints;
  Glib::ustring path_;
  Node model;
  std::string ref_name;
  //ColorArea* color_area_ptr_;
  Gtk::Label* entry_ptr_;
  Gtk::Button* button_ptr_;
  //Gdk::Color color_;
  bool editing_cancelled_;
  signal_editing_done_t signal_editing_done_;
  /* override */virtual void start_editing_vfunc(GdkEvent* event);
  /* override */virtual void on_editing_done();
  /* override */virtual void on_entry_activate();
  
  // Manage button_clicked signal
  virtual void on_button_clicked();
	
  // Manage entry_key_press_event signal
  bool on_entry_key_press_event(GdkEventKey* event);
};

class RefCellRenderer
  : public Gtk::CellRenderer
{
public:
  RefCellRenderer();//Node model, std::string ref_name);
  virtual ~RefCellRenderer();

  // Properties
  Glib::PropertyProxy< Glib::ustring > property_text();
  Glib::PropertyProxy< bool > property_editable();
  
  // Edited signal
  typedef sigc::signal<void, const Glib::ustring&, const Glib::ustring&> signal_edited_t;
  signal_edited_t& signal_edited() { return signal_edited_; };
	
protected:
  //Node model;
  //std::string ref_name;
  Glib::Property< Glib::ustring > property_text_;
  Glib::Property< bool > property_editable_;
  signal_edited_t signal_edited_;
  RefCellEditable *color_cell_edit_ptr_;
  mutable int button_width_; //mutable because it is just a cache for get_size_vfunc

  // Raise the edited event
  void edited(const Glib::ustring& path, const Glib::ustring& new_text);
  
  virtual void get_size_vfunc (Gtk::Widget& widget, const Gdk::Rectangle* cell_area, int* x_offset, int* y_offset, int* width, int* height) const;

  // TODO
  //virtual void render_vfunc (const Glib::RefPtr<Gdk::Drawable>& window, Gtk::Widget& widget, const Gdk::Rectangle& background_area, const Gdk::Rectangle& cell_area, const Gdk::Rectangle& expose_area, Gtk::CellRendererState flags);
  virtual bool activate_vfunc (GdkEvent*, Gtk::Widget&, const Glib::ustring& path, const Gdk::Rectangle& background_area, const Gdk::Rectangle& cell_area, Gtk::CellRendererState flags);
  virtual Gtk::CellEditable* start_editing_vfunc(GdkEvent* event, Gtk::Widget& widget, const Glib::ustring& path, const Gdk::Rectangle& background_area, const Gdk::Rectangle& cell_area, Gtk::CellRendererState flags);
  
  // Manage editing_done event for color_cell_edit_ptr_
  void on_editing_done();
};


class RefExplorerChange
{
  
};

class RefExplorer: public Gtk::Frame, 
                   public CProvider<RefExplorerChange>
{
public:
  RefExplorer();
  RefExplorer(Node model, std::string ref_name, const std::string &title = "");
  void setup(Node model, std::string ref_name, const std::string &title = "");
  Node get_selection();
private:
  bool is_valid();
  void populate();
  void populate(Node m, Gtk::TreeModel::Row row);
  void update_view();
  void clear_table();
  void on_treeview_row_activated(const Gtk::TreeModel::Path &path, Gtk::TreeViewColumn *);
  void on_selection_changed();

  class ModelColumns : public Gtk::TreeModel::ColumnRecord
  {
  public:
    ModelColumns(){add(m_col_name); add(m_col_ptr);} //add(m_col_pic); add(m_col_desc);}
      Gtk::TreeModelColumn<Glib::ustring> m_col_name;
      Gtk::TreeModelColumn<Node> m_col_ptr;
  };

  bool lock;
  Gtk::ScrolledWindow scroll;
  Gtk::TreeView tree_view;
  Glib::RefPtr<Gtk::TreeStore> tree_model;
  ModelColumns columns;
  Node model;
  std::string ref_name;
  bool valid;
  bool init_done;
};


/** A window to let the user select an object in a tree model */
class RefExplorerWnd: public GenDialog
{
public:
  /** Create the window (do not show it).
   *  @param model:  the tree to be explored. 
   *  @param type:   The type of node that the user can select. */
  RefExplorerWnd(Node model, const std::string &type, const std::string &wnd_title = "");

  /** Display the window (blocking call)
   *  @returns 0 if the user has not canceled. */
  int display();

  Node get_selection();

  void update_view();

  void on_change(const RefExplorerChange &change);

private:
  RefExplorer explorer;
  Node model;
  std::string ref_name;
};

}
}

#endif
