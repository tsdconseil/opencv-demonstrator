/**
 *  This file is part of LIBCUTIL.
 *
 *  LIBCUTIL is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  LIBCUTIL is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with LIBCUTIL.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  Copyright 2007-2011 J. A.
 */

#ifndef GTKUTIL_HPP
#define GTKUTIL_HPP

//#include <gtkmm.h>


#include <cstddef>
using std::ptrdiff_t;


#include <gtkmm/button.h>
#include <gtkmm/window.h>
#include <gtkmm/frame.h>
#include <gtkmm/alignment.h>
#include <gtkmm/buttonbox.h>
#include <gtkmm/cellrenderer.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/colorbutton.h>
#include <gtkmm/colorselection.h>
#include <gtkmm/combobox.h>
#include <gtkmm/dialog.h>
#include <gtkmm/drawingarea.h>
#include <gtkmm/entry.h>
#include <gtkmm/filechooser.h>
#include <gtkmm/filechooserbutton.h>
#include <gtkmm/filechooserdialog.h>
#include <gtkmm/image.h>
#include <gtkmm/label.h>
#include <gtkmm/liststore.h>
#include <gtkmm/menu.h>
#include <gtkmm/menutoolbutton.h>
#include <gtkmm/notebook.h>
#include <gtkmm/paned.h>
#include <gtkmm/radiobutton.h>
#include <gtkmm/scrollbar.h>
#include <gtkmm/spinbutton.h>
#include <gtkmm/table.h>
#include <gtkmm/fixed.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/treestore.h>
#include <gtkmm/toolbar.h>
#include <gtkmm/separatortoolitem.h>
#include <gtkmm/uimanager.h>
#include <gtkmm/separator.h>
#include <gtkmm/main.h>
#include <gtkmm/aboutdialog.h>
#include <gtkmm/progressbar.h>

#include <gtkmm/stock.h>
#include <gtkmm/messagedialog.h>
#include <gtkmm/eventbox.h>
#include <gtkmm/textview.h>
#include <gtkmm/arrow.h>

#include <glibmm/dispatcher.h>
#include <gdkmm/cursor.h>



#include "cutil.hpp"
#include "trace.hpp"
#include "modele.hpp"

namespace utils
{
namespace mmi
{

using namespace model;


//////////////////////////////////////////////////
// Global functions                             //
//////////////////////////////////////////////////

extern void show_frame_window(Gtk::Widget *frame, std::string name);
extern Glib::ustring request_user_string(Gtk::Window *parent,
                                       Glib::ustring mainMessage,
                                       Glib::ustring subMessage);

/** The theme must be localated in a folder with
 *  the same name as the theme one. */
extern void set_theme(std::string theme_name);

extern Gtk::Window *mainWindow;

namespace dialogs
{
  extern string save_dialog(string title, string filter, string filter_name,
                              string default_name = "", string default_dir = "");
  extern string open_dialog(string title, string filter, string filter_name,
                              string default_name = "", string default_dir = "");
  extern string new_dialog(string title, string filter, string filter_name,
                             string default_name = "", string default_dir = "");
  extern bool check_dialog(string title,
                             string short_description = "",
                             string description = "");
  extern void show_info(string title, string short_description = "", string description = "");
  extern void show_error(string title, string short_description = "", string description = "");
  extern void show_warning(string title, string short_description = "", string description = "", bool blocking = true);
}


//////////////////////////////////////////////////
// Objects                                      //
//////////////////////////////////////////////////


class VideoView: public Gtk::DrawingArea
{
public:
  VideoView(uint16_t dx = 150, uint16_t dy = 100);
  void update(void *img, uint16_t sx, uint16_t sy);

private:
  ///////////////////////////////////
  // Data protected by a mutex.
  void *new_img;
  uint16_t new_sx, new_sy;
  ///////////////////////////////////

  Logable log;
  bool realized;
  uint16_t csx, csy;
  Cairo::RefPtr<Cairo::ImageSurface> image_surface;
  Glib::Dispatcher signal_video_update;
  utils::hal::Mutex mutex_video_update;

  void change_dim(uint16_t sx, uint16_t sy);
  void on_video_update();
  void do_update_view();
  virtual bool on_draw(const Cairo::RefPtr<Cairo::Context> &cr);
  void on_the_realisation();
};


class GColor
{
public:
  uint8_t red, green, blue;
  GColor(){red=0;green=0;blue=0;}
  GColor(uint8_t r, uint8_t g, uint8_t b){red=r;green=g;blue=b;}
  GColor(const Gdk::Color &col)
  {
    red = col.get_red() >> 8;
    green = col.get_green() >> 8;
    blue = col.get_blue() >> 8;
  }
  GColor gain(float a) const
  {
    GColor res;

    float val = red * a;
    if(val > 255)
      val = 255;
    res.red = (uint8_t) val;

    val = green * a;
    if(val > 255)
      val = 255;
    res.green = (uint8_t) val;

    val = blue * a;
    if(val > 255)
      val = 255;
    res.blue = (uint8_t) val;
    return res;
  }

  Gdk::Color to_gdk() const
  {
    Gdk::Color res;
    res.set_red(256 * red);
    res.set_green(256 * green);
    res.set_blue(256 * blue);
    return res;
  }
  uint32_t to_int() const
  {
    uint32_t res = (blue << 16) | (green << 8) | red;
    return res;
  }
  GColor(std::string desc)
  {
    std::vector<int> vec;
    utils::str::parse_int_list(desc, vec);
    if(vec.size() < 3)
      return;
    red = vec[0];
    green = vec[1];
    blue = vec[2];
  }
  std::string to_string() const
  {
    return utils::str::int2str(red) + "."
        + utils::str::int2str(green) + "."
        + utils::str::int2str(blue);
  }
  std::string to_html_string() const
  {
    return "#" + utils::str::int2strhexa(red, 8)
      + utils::str::int2strhexa(green, 8)
      + utils::str::int2strhexa(blue, 8);
  }
};


// To deprecate
namespace AntiliasingDrawing
{
  extern void draw_line(uint32_t *img,
                        uint32_t img_width,
                        uint32_t img_height,
                        int x1, int y1,
                        int x2, int y2,
                        GColor  color,
                        bool    mat_on_white);
}

class JFrame : public Gtk::Frame
{
public:
    JFrame(){lab = nullptr;}
    JFrame(std::string label);
    void set_label(const Glib::ustring &s);
private:
    Gtk::Label *lab;
};

class JComboBox: public Gtk::ComboBox
{
public:
    JComboBox();
    void add_key(std::string key);
    std::string get_current_key();
    void set_current_key(std::string s);
    void remove_all();
private:
    std::vector<std::string> keys;
    class ModelColumns : public Gtk::TreeModel::ColumnRecord
    {
    public:
        ModelColumns()
        { add(m_col_id); add(m_col_name); }
        Gtk::TreeModelColumn<Glib::ustring> m_col_id;
        Gtk::TreeModelColumn<Glib::ustring> m_col_name;
    };
    ModelColumns m_Columns;
    Glib::RefPtr<Gtk::ListStore> m_refTreeModel;
};

class GtkLed;

class LedEvent
{
public:
  GtkLed *source;
};


class GtkLed:
  public CProvider<LedEvent>,
  public Gtk::DrawingArea
{
public:
  GtkLed(unsigned int size = 13, bool is_red = false, bool is_mutable = true);
  void update_view();
  void light(bool on);
  bool is_on();
  void set_red(bool is_red);
  void set_yellow();
  void set_sensitive(bool sensistive);
  void set_mutable(bool is_mutable);
private:
  bool is_sensitive;
  unsigned int size;
  bool is_mutable;
  bool is_red, is_yellow;
  bool realized;
  bool is_lighted;
  Glib::RefPtr<Gdk::Window> wnd;
  Cairo::RefPtr<Cairo::Context> gc;
  bool on_expose_event(GdkEventExpose* event);
  void on_the_realisation();
  Glib::RefPtr<Pango::Layout> lay;
  bool on_mouse(GdkEventButton *event );
};


class GtkKey;

class KeyChangeEvent
{
public:
  GtkKey *source;
  std::string key;
  bool active;
};

class GtkKey:
  public Gtk::DrawingArea,
  public CProvider<KeyChangeEvent>
{
public:
    GtkKey(unsigned short size_x = 40, unsigned short size_y = 40, std::string s = "", bool toggle = false);
    void set_text(std::string s);
    void set_text(char c);
    std::string get_text();
    void set_sensitive(bool sensitive);
private:
    bool on_expose_event(GdkEventExpose* event);
    void on_the_realisation();
    bool on_mouse(GdkEventButton *event);

    void update_view();

    std::string text;
    Cairo::RefPtr<Cairo::Context> ctx;
    Glib::RefPtr<Gdk::Window> wnd;
    Cairo::RefPtr<Cairo::Context> gc;
    Glib::RefPtr<Pango::Layout> lay;
    unsigned short size_x, size_y;
    bool realized;
    bool clicking;
    bool toggle;
    bool sensitive;
    static Logable log;
};

class VirtualKeyboard
  : public  JFrame,
    private CListener<KeyChangeEvent>
{
public:
  VirtualKeyboard(Gtk::Window *target_window);
  ~VirtualKeyboard();
  
  /** @brief Setup currently valid characters (as UTF-8 strings) */
  void set_valid_chars(const std::vector<std::string> &vchars);

  std::deque<GtkKey *> keys;
  Gtk::Fixed fixed;
  Gtk::HBox hbox;
  Gtk::Window *target_window;
private:
  bool maj_on;
  bool currently_active;
  static Logable log;
  bool on_key(GdkEventKey *event);
  void on_event(const KeyChangeEvent &kce);
  unsigned long cx, cy;
  unsigned long kw;
  GtkKey *add_key(std::string s);
  std::string class_name() const {return "graphic keyboard";}
  void update_keyboard();  
};


class Wizard;

class WizardStep
{
public:
  std::string name;
  std::string title;
  std::string description;
  Gtk::Frame *get_frame();
  Wizard *parent;
  virtual ~WizardStep(){}
  virtual void validate() = 0;
  virtual void enter() = 0;
  virtual bool enable_next(){return false;}
  virtual bool enable_end(){return false;}
  virtual bool enable_prec(){return true;}
private:
};

/** @brief A wizard */
class Wizard
{
  friend class WizardStep;
public:
  Wizard() : wizlab("./img/wiznew.png"){}
  virtual ~Wizard(){}
  std::string title;
  void set_icon(const string &ipath);
  void start();
  void update_view();
  virtual void on_next_step(std::string current) = 0;
  virtual void on_prec_step(std::string current) = 0;
  void set_current(std::string name);
protected:
  // Wizard customization
  WizardStep *current, *first;
  void add_step(WizardStep *step);
  WizardStep *get_step(std::string name);

  // non-zero if user cancelation
  int state;

  int current_index;

private:

  void on_cancel();
  void on_prec();
  void on_next();

  Gtk::Window dialog;
  Gtk::VBox vbox, hvbox;
  Gtk::Image wizlab;
  Gtk::HBox hhvbox;
  Gtk::HButtonBox low_buts;
  Gtk::Frame high, mid;
  Gtk::Label main_label, description_label;
  Gtk::Button b_cancel, b_prec, b_next;
  std::vector<WizardStep *> steps;
};

class PageChange
{
public:
  Gtk::Widget *previous_widget, *new_widget;
};

class PageClose
{
public:
  Gtk::Widget *closed_widget;
};

class NotebookManager:
  public Gtk::Notebook,
  public CProvider<PageChange>,
  public CProvider<PageClose>
{
public:
  NotebookManager();

  static const int POS_FIRST;
  static const int POS_LAST;

  /** @returns page index if > 0 */
  int add_page(int position, Gtk::Widget &widget, std::string name, std::string icon_path, std::string description = "");
  int remove_page(Gtk::Widget &widget);
  Gtk::Widget *get_current_page();
  int set_current_page(Gtk::Widget &widget);
  unsigned int nb_pages() const;


private:
  class Page
  {
  public:
    ~Page();
    Gtk::Widget *widget;
    Gtk::Alignment *align;
    int index;
    std::string name;
    std::string icon_path;
    Gtk::ScrolledWindow scroll;
  private:
  };

  int current_page;
  std::deque<Page *> pages;
  Logable log;
  std::string class_name() const {return "notebook-manager";}
  void on_switch_page(Gtk::Widget *page, int page_num);
  void on_b_close(Page *page);
};


class SelectionChangeEvent
{
public:
  Node old_selection, new_selection;
};


class VarEventFunctor
{
public:
  virtual ~VarEventFunctor(){}
  virtual int call(Node target) = 0;
  std::string action;
};

template <class A>
class SpecificVarEventFunctor: public VarEventFunctor
{
public:
  SpecificVarEventFunctor(A *object, int(A::*m_function)(Node target))
  {
    this->object = object;
    this->m_function = m_function;
  }

  virtual int call(Node target)
  {
    return (*object.*m_function)(target);
  }
private:
  int (A::*m_function)(Node target);
  A *object;
};

class TreeManager
  : public  JFrame,
    public  CProvider<SelectionChangeEvent>,
    private CListener<ChangeEvent>
{
public:
  TreeManager();
  void set_model(Node model);
  TreeManager(Node model);
  Node get_selection();
  int set_selection(Node elt);
  void maj_langue();
  template<class A>
    int add_action_listener(std::string action, A *target, int (A:: *fun)(Node target));
private:
  Glib::RefPtr<Gdk::Pixbuf> get_pics(NodeSchema *schema);
  bool has_pic(NodeSchema *schema);
  void load_pics(NodeSchema *sc);
  void populate();
  void populate(Node m, Gtk::TreeModel::Row row);
  void update_view();
  void clear_table();
  void on_selection_changed();
  void on_event(const ChangeEvent &pce);
  void set_selection(Gtk::TreeModel::Row &root, Node reg);
  void setup_row_view(Node elt);
  void on_treeview_row_activated(const Gtk::TreeModel::Path &path, Gtk::TreeViewColumn *);
  void on_menu(Node elt, std::string action);
  void on_menup(std::pair<Node, std::string> pr);

  std::string class_name() const {return "tree-mng";}

  class ModelColumns : public Gtk::TreeModel::ColumnRecord
  {
  public:
      ModelColumns(){add(m_col_name); add(m_col_ptr); add(m_col_pic); add(m_col_desc);}
      Gtk::TreeModelColumn<Glib::ustring> m_col_name;
      Gtk::TreeModelColumn<Node> m_col_ptr;
      Gtk::TreeModelColumn<Glib::ustring> m_col_desc;
      Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf> > m_col_pic;
  };
  class MyTreeView : public Gtk::TreeView
  {
  public:
        MyTreeView(TreeManager *parent);
        virtual bool on_button_press_event(GdkEventButton *ev);
  private:
        TreeManager *parent;
  };

  static Logable log;
  Node model;
  bool lock;
  Gtk::ScrolledWindow scroll;
  MyTreeView tree_view;
  Glib::RefPtr<Gtk::TreeStore> tree_model;
  ModelColumns columns;
  Gtk::Menu popup_menu;
  std::deque<std::pair<Glib::RefPtr<Gdk::Pixbuf>, NodeSchema *> > pics;
  std::deque<NodeSchema *> pics_done;
  std::deque<VarEventFunctor *> menu_functors;
};

template<class A>
int TreeManager::add_action_listener(std::string action, A *target,
                                     int (A:: *fun)(Node target))
{
  /* TODO: check if not already registered */
  SpecificVarEventFunctor<A> *f = new SpecificVarEventFunctor<A>(target, fun);
  f->action = action;
  menu_functors.push_back(f);
  return 0;
}

/** @brief Class to manage the position on the screen
 *  of a window / dialog, decide whether this window should use
 *  scroll bars (screen too small), and the position of the
 *  virtual keyboard if necessary (touch screen mode). */
class DialogManager: private Logable
{
public:

  class Placable
  {
  public:
    virtual Gtk::Window *get_window() = 0;
    virtual void force_scroll(__attribute__((unused)) int dx, __attribute__((unused)) int dy){}
    virtual void unforce_scroll(){}
  };

  static void get_screen(Gtk::Window *wnd, uint32_t &ox, uint32_t &oy, uint32_t &x, uint32_t &y);

  /** @brief Place a window on the screen,
   *  optionnaly enabling its scrolling bars if the screen is too small. */
  static void setup_window(Placable *wnd, bool fullscreen = false);
  static void setup_window(Gtk::Window *wnd, bool fullscreen = false);
private:
};


class ColorRectangle: public Gtk::DrawingArea,
                      private Logable
{
public:
  ColorRectangle(const GColor &col, uint16_t width, uint16_t height);
  void update_color(const GColor &col);
  ~ColorRectangle();
private:
  GColor col;
  uint16_t width, height;
  bool realized;
  Glib::RefPtr<Gdk::Window> wnd;
  Cairo::RefPtr<Cairo::Context> gc;
  bool on_expose_event(const Cairo::RefPtr<Cairo::Context> &cr);
  void on_the_realisation();
  void update_view();
  bool on_timer(int unused);
};

class ColorButton: public Gtk::Button,
                   private Logable,
                   private CListener<ChangeEvent>
{
public:
  ColorButton(Attribute *model);
  ~ColorButton();
private:
  void on_event(const ChangeEvent &ce);
  void on_b_pressed();
  Gtk::Alignment al;
  Gtk::VBox box;
  Attribute *model;
  ColorRectangle *crec;
  bool lock;
};

/** Dialog with custom toolbar in touchscreen mode */
class GenDialog: public Gtk::Dialog,
                 protected Logable,
                 public DialogManager::Placable
{
public:
  int display_modal();
  virtual Gtk::Window *get_window();

protected:
  typedef enum GenDialogTypeEnum
  {
    GEN_DIALOG_APPLY_CANCEL = 0,
    GEN_DIALOG_VALID_CANCEL = 1,
    GEN_DIALOG_CANCEL       = 2,
    GEN_DIALOG_CLOSE        = 3
  } GenDialogType;

  GenDialog(GenDialogType type,
            std::string title = "");

  void enable_validation(bool enable);

  virtual void on_cancel() {};
  virtual void on_close()  {};

  virtual void on_valid()  {};
  virtual void on_apply()  {};

  Gtk::Box *vbox;

private:
  void on_b_apply_valid();
  void on_b_cancel_close();
  GenDialogType type;
  bool result_ok;
  Gtk::Toolbar toolbar;
  Gtk::ToolButton tool_valid, tool_cancel;
  Gtk::SeparatorToolItem sep2;
  Gtk::Button b_valid, b_cancel, b_apply;
  Gtk::Label label_title;
  Gtk::HButtonBox hbox;
};

class ColorDialog: public GenDialog
{
public:
  ColorDialog();
  int display(GColor initial_color, std::vector<std::string> constraints);
  void force_scroll(int dx, int dy);
  void unforce_scroll();
  GColor get_color();
private:
  void on_valid();
  void on_cancel();
  Gtk::ColorSelection cs;
  GColor selected;
};

class ColorPalette: public Gtk::DrawingArea, private Logable
{
public:
  ColorPalette(const std::vector<GColor> &colors, uint32_t initial_color = 0);
  ~ColorPalette();

  uint32_t current_color;

private:
  int ncols, nrows;
  std::vector<GColor> colors;
  uint32_t width, height;
  uint32_t c_width, c_height;
  uint32_t spacing;
  bool realized;
  virtual bool on_draw(const Cairo::RefPtr<Cairo::Context>& cr);
  bool on_expose_event(GdkEventExpose *event);
  void on_the_realisation();
  void update_view();
  bool on_mouse(GdkEventButton *event);
};

class ColorPaletteDialog: public GenDialog
{
public:
  ColorPaletteDialog(const std::vector<GColor> &colors,
                     uint32_t initial_color);
  ~ColorPaletteDialog();
  uint32_t color_index;
protected:
  virtual void on_valid();
  virtual void on_cancel();
private:
  std::vector<GColor> colors;
  ColorPalette palette;
};

/** A one end, listen to asynchronous generic 'A' type events
 *  At the other end, provides the same 'A' type events, synchronously
 *  inside the GTK thread. */
template<class A>
class GtkDispatcher: public CListener<A>, public CProvider<A>
{
public:
  GtkDispatcher(uint32_t fifo_capacity = 4): fifo(4)
  {
    gtk_dispatcher.connect(sigc::mem_fun(*this, &GtkDispatcher::on_dispatcher));
  }

  void on_event(const A &a)
  {
    fifo.push(a);
    gtk_dispatcher();
  }

private:
  Glib::Dispatcher gtk_dispatcher;
  hal::Fifo<A> fifo;

  void on_dispatcher()
  {
    if(fifo.empty())
    {
      /* spurious dispatch ! */
    }
    else
    {
      A a = fifo.pop();
      CProvider<A>::dispatch(a);
    }
  }
private:
};


struct GtkVoid2{};

class GtkVoidDispatcher: public CListener<GtkVoid2>, public CProvider<GtkVoid2>
{
public:
  GtkVoidDispatcher(uint32_t fifo_capacity = 4): fifo(4)
  {
    gtk_dispatcher.connect(sigc::mem_fun(*this, &GtkVoidDispatcher::on_dispatcher));
  }

  void on_event(const GtkVoid2 &a)
  {
    fifo.push(a);
    gtk_dispatcher();
  }

  void do_dispatch()
  {
    GtkVoid2 a;
    fifo.push(a);
    gtk_dispatcher();
  }

private:
  Glib::Dispatcher gtk_dispatcher;
  hal::Fifo<GtkVoid2> fifo;

  void on_dispatcher()
  {
    if(fifo.empty())
    {
      /* spurious dispatch ! */
    }
    else
    {
      GtkVoid2 a = fifo.pop();
      CProvider<GtkVoid2>::dispatch(a);
    }
  }
private:
};



struct LabelClick
{
  std::string path;
  enum
  {
    SEL_CLICK,
    VAL_CLICK
  } type;
};

class SensitiveLabel: public Gtk::EventBox,
                      public CProvider<LabelClick>
{
public:
  SensitiveLabel(std::string path);
  Gtk::Label label;
private:
  bool on_button_press_event(GdkEventButton *event);
  std::string path;
};


class NullFunctor
{
public:
  virtual ~NullFunctor(){}
  virtual void call() = 0;
};

template <class A>
class SpecificNullFunctor: public NullFunctor
{
public:
  SpecificNullFunctor(A *object, void(A::*m_function)())
  {
    this->object = object;
    this->m_function = m_function;
  }

  virtual void call()
  {
    (*object.*m_function)();
  }
private:
  void (A::*m_function)();
  A *object;
};

class ProgressDialog: public Gtk::Dialog
{
public:
  ProgressDialog();
  ~ProgressDialog();

  /* To be called from IHM thread */
  template<class A>
    void start_progress(std::string title, std::string text,
                        A *target_class,
                        void (A:: *target_function)());
private:


  enum Event
  {
    START,
    EXIT,
  };

  utils::hal::Fifo<Event> event_fifo;

  void setup(std::string title, std::string text);
  void thread_entry();
  bool on_timer(int index);
  void on_b_cancel();
  struct Bidon{};
  int on_event(const Bidon &bidon);


  utils::mmi::GtkDispatcher<Bidon> gtk_dispatcher;

  NullFunctor *functor;
  utils::hal::Signal callback_done, can_start, exit_done;
  Gtk::ProgressBar progress;
  Gtk::Label label;
  Gtk::ToolButton bt;
  Gtk::Toolbar toolbar;
  bool canceled;
  Gtk::SeparatorToolItem sep;
  utils::Logable log;
  bool in_progress;
};

template<class A>
void ProgressDialog::start_progress(std::string title, std::string text,
                                    A *target_class,
                                    void (A:: *target_function)())
{
  callback_done.clear();
  functor = new SpecificNullFunctor<A>(target_class, target_function);
  setup(title, text);
}


}
}

#endif





