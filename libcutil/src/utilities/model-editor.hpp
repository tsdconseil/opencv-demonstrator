#ifndef MODEL_EDITOR_HPP
#define MODEL_EDITOR_HPP


#include "mmi/stdview.hpp"

using namespace utils;
using namespace utils::model;
using namespace utils::mmi;

class ModelEditor: public Gtk::Window,
                   private CListener<ChangeEvent>,
                   private CListener<EVSelectionChangeEvent>
{
public:
  static ModelEditor *get_instance();
  virtual ~ModelEditor();
  int open(std::string filename);
  int main(CmdeLine &cmdline);
  ModelEditor();

private:

  std::string ext, extname;
  void on_event(const EVSelectionChangeEvent &evse);

  /** Main horizontal separation */
  Gtk::HPaned hpane;

  /** View for the database */
  NodeView *ev_root;

  Gtk::VBox vbox, vboxi;

  Gtk::Toolbar tools;
  Gtk::ToolButton b_new, b_open, b_save, b_exit, b_param, b_about, b_infos;

  /** View for the currently edited item */
  NodeView *ev;

  Gtk::ScrolledWindow scroll;


  Node model;
  static ModelEditor *instance;

  bool model_saved;

  /** Application configuration (path, etc.) */
  Node config;

  /** root-schema.xml */
  FileSchema *root_fs;
  NodeSchema *root_schema;

  std::string model_path, schema_path;

  std::string last_schema_dir;
  
  Logable log;

  void on_b_open();
  void on_b_save();
  void on_b_save_as();
  void save_as(const string &filename);
  void on_b_new();
  void on_b_exit();
  void on_b_comp();
  void on_b_param();
  void on_b_infos();
  void update_view();
  void on_event(const ChangeEvent &ce);
};



#endif

