#include "model-editor.hpp"
#include "mmi/stdview.hpp"
#include "cutil.hpp"
#include <stdio.h>
#include <iostream>


ModelEditor *ModelEditor::instance = nullptr;

ModelEditor *ModelEditor::get_instance()
{
  if(instance == nullptr)
    instance = new ModelEditor();
  return instance;
}

void ModelEditor::on_b_infos()
{
  Gtk::AboutDialog ad;
  ad.set_copyright("(C) 2012 J.A.");
  Glib::RefPtr<Gdk::Pixbuf>  pix = Gdk::Pixbuf::create_from_file(utils::get_img_path() + "/TODO.png");
  ad.set_logo(pix);
  ad.set_name(langue.get_item("main-wnd-title") + "\n");
  ad.set_version("revision 0.00");
  std::vector<Glib::ustring> v;
  v.push_back("(C) 2012 J.A.");
  ad.set_authors(v);
  ad.set_position(Gtk::WIN_POS_CENTER);
  ad.run();
}

void ModelEditor::on_b_save_as()
{

}

void ModelEditor::on_b_new()
{
  std::string path, name;
  files::split_path_and_filename(model_path, path, name);

  std::string fn = dialogs::new_dialog("Nouvelle configuration", "*.xml", "Fichier XML", name, path);
  if(fn.size() == 0)
    return;


  Node mod  = Node::create_ram_node(root_schema);

  if(ev != nullptr)
  {
    log.trace("Delete old widget...");
    model.remove_listener(this);
    vboxi.remove(*(ev->get_widget()));
    delete ev;
  }

  model_path = fn;

  log.trace("Model switch..");
  model = mod;
  model.add_listener(this);

  model.save(fn);

  log.trace("Model view creation..");
  NodeViewConfiguration vconfig;
  //vconfig.show_children = true;
  //vconfig.show_separator = false;
  //vconfig.nb_attributes_by_column = 20;
  //vconfig.nb_columns = 2;
  ev = new utils::mmi::NodeView(this, model, vconfig);
  log.trace("Adding to current view..");
  vboxi.pack_start(*(ev->get_widget()), Gtk::PACK_EXPAND_WIDGET);
  this->show_all_children(true);
  config.set_attribute("last-file", fn);
  update_view();

  DialogManager::setup_window(this);
}

void ModelEditor::save_as(const string &filename)
{

}

void ModelEditor::on_b_save()
{
  if(!files::file_exists(model_path))
  {
    return;
  }
  model.save(model_path, true);
  model_saved = true;
  update_view();
}

void ModelEditor::on_b_exit()
{
  if(!model_saved)
  {
    if(!dialogs::check_dialog("Quitter",
        "Certaines modifications n'ont pas été sauvegardées.",
        "Voulez-vous vraiment fermer l'application et\nperdre les dernières modifications ?"))
    {
      return;
    }
  }
  std::string cfg_file = utils::get_current_user_path() + PATH_SEP + "cfg.xml";
  config.save(cfg_file);
  log.trace("Bye.");
  exit(0);
}

void ModelEditor::on_b_open()
{
  std::string path, name;
  files::split_path_and_filename(config.get_attribute_as_string("last-file"), path, name);
  std::string fn = dialogs::open_dialog("Ouvrir", /*"*.xml"*/ext, /*"Fichier XML"*/extname, name, path);
  if(fn.size() > 0)
    this->open(fn);
}

void ModelEditor::on_b_param()
{
  if(NodeDialog::display_modal(config) == 0)
  {
    std::string cfg_file = utils::get_current_user_path() + PATH_SEP + "cfg.xml";
    config.save(cfg_file);
  }
}

void ModelEditor::on_b_comp()
{
}

void ModelEditor::on_event(const ChangeEvent &ce)
{
  if(ce.type == ChangeEvent::COMMAND_EXECUTED)
  {

  }
  else
  {
    model_saved = false;
    //trace("Change event detected: %s", ce.to_string().c_str());
    update_view();
  }
}

void ModelEditor::update_view()
{
  std::string s;

  if(ev == nullptr)
  {
    s = "Edition modéle XML";
  }
  else
  {
    s = "Edition [";
    s += model_path;
    s += "]";
  }

  set_title(s);
  b_save.set_sensitive(!model_saved);
}

void ModelEditor::on_event(const EVSelectionChangeEvent &evse)
{
  log.trace("Sel change detected.");

  #if 0

  if(!model_saved)
  {
    if(Gide::check_dialog("Changement de sélection",
        "Voulez-vous enregistrer les modification effectuées ?",
        "Les modifications n'ont pas été sauvegardées."))
    {
      on_b_save();
    }
    else
    {
      model_saved = true;
      update_view();
    }
  }


  std::string type    = evse.selection.schema()->name.get_id();
  std::string dbpath  = config.get_attribute_as_string("db-path");
  std::string name    = evse.selection.get_attribute_as_string("name");

  if(type.compare("card") == 0)
  {
    trace("Selection = card %s.", name.c_str());
    open(dbpath
         + Util::get_path_separator()
         + "cards"
         + Util::get_path_separator()
         + name
         + ".xml");
  }
  else if(type.compare("fpga-lib") == 0)
  {
    trace("Selection = fpga lib %s.", name.c_str());
    open(dbpath
         + Util::get_path_separator()
         + "fpga"
         + Util::get_path_separator()
         + name
         + ".xml");
  }
  else if(type.compare("mod") == 0)
  {
    trace("Selection = module %s.", name.c_str());
    open(dbpath
         + Util::get_path_separator()
         + "modules"
         + Util::get_path_separator()
         + name
         + ".xml");
  }
  #endif

}

ModelEditor::ModelEditor()
{
  log.setup("cutil", "model-editor");
  ev = nullptr;
  mainWindow = this;
  set_title("Model editor");
  model_saved = true;

  appli_view_prm.use_touchscreen  = false;
  appli_view_prm.inverted_colors  = false;

  
  //root_fs = new FileSchema(exec_dir + Util::get_path_separator() + "std-schema.xml");
  FileSchema *fs = new FileSchema(utils::get_fixed_data_path() + PATH_SEP + "model-editor-config-schema.xml");

  std::string cfg_file = utils::get_current_user_path() + PATH_SEP + "cfg.xml";
  if(!files::file_exists(cfg_file))
  {
    config = Node::create_ram_node(fs->get_schema("model-editor"));
    config.save(cfg_file);
  }
  else
  {
    config = Node::create_ram_node(fs->get_schema("model-editor"), cfg_file);
    if(config.is_nullptr())
    {
      config = Node::create_ram_node(fs->get_schema("model-editor"));
      config.save(cfg_file);
    }
  }

  log.trace("Application configuration:\n%s\n", config.to_xml().c_str());

  add(vbox);

  vbox.pack_start(tools, Gtk::PACK_SHRINK);


  /*NodeViewConfiguration cfg;
  cfg.show_children       = true;
  cfg.display_only_tree   = false;
  cfg.expand_all          = false;
  Node nv;
  ev_root = new NodeView(this, nv, cfg);

  ev_root->Provider<EVSelectionChangeEvent>::add_listener(this);
  scroll.add(*ev_root->get_widget());*/

  //vbox.pack_start(scroll, Gtk::PACK_EXPAND_WIDGET);
  vbox.pack_start(vboxi, Gtk::PACK_SHRINK);
  //scroll.add(vboxi);

  tools.add(b_new);
  tools.add(b_open);
  tools.add(b_save);
  tools.add(b_param);
  tools.add(b_infos);
  tools.add(b_exit);

  b_new.set_stock_id(Gtk::Stock::NEW);
  b_open.set_stock_id(Gtk::Stock::OPEN);
  b_save.set_stock_id(Gtk::Stock::SAVE);
  b_exit.set_stock_id(Gtk::Stock::QUIT);
  b_infos.set_stock_id(Gtk::Stock::ABOUT);
  b_param.set_stock_id(Gtk::Stock::PREFERENCES);

  b_infos.set_label("A propos");
  b_infos.set_tooltip_markup("A propos");

  tools.set_icon_size(Gtk::ICON_SIZE_LARGE_TOOLBAR);
  tools.set_toolbar_style(Gtk::TOOLBAR_ICONS);//TOOLBAR_BOTH);

  b_new.set_tooltip_markup(langue.get_item("new"));
  b_open.set_tooltip_markup(langue.get_item("open"));
  b_save.set_tooltip_markup(langue.get_item("save"));
  b_exit.set_tooltip_markup(langue.get_item("exit"));
  b_param.set_tooltip_markup(langue.get_item("params"));

  b_new.signal_clicked().connect(sigc::mem_fun(*this, &ModelEditor::on_b_new));
  b_open.signal_clicked().connect(sigc::mem_fun(*this, &ModelEditor::on_b_open));
  b_save.signal_clicked().connect(sigc::mem_fun(*this, &ModelEditor::on_b_save));
  b_exit.signal_clicked().connect(sigc::mem_fun(*this, &ModelEditor::on_b_exit));
  b_param.signal_clicked().connect(sigc::mem_fun(*this, &ModelEditor::on_b_param));
  b_infos.signal_clicked().connect(sigc::mem_fun(*this, &ModelEditor::on_b_infos));


  log.trace("Construction terminée.");
  show_all_children(true);
  update_view();
  //set_size_request(1000,780);
  DialogManager::setup_window(this);
  // TODO
  //nv.add_listener(this);
}


ModelEditor::~ModelEditor()
{
}

int ModelEditor::open(std::string filename)
{
  MXml mx;

  if(!files::file_exists(filename))
  {

    // CREATION DU FICHIER SI IL N'EXISTE PAS ?
    log.anomaly("File not found: %s.", filename.c_str());
    dialogs::show_error("Ouverture", "Le fichier n'existe pas.", filename + " n'est pas accessible.");
    return -1;
  }


  if(mx.from_file(filename))
  {
    log.anomaly("Parse error in %s.", filename.c_str());
    dialogs::show_error("Ouverture", "Le format du fichier est invalide", "");
    return -2;
  }

  model_path = filename;

  log.trace("Loading model...");
  Node mod = Node::create_ram_node(root_fs->get_schema(mx.name), filename);

  if(ev != nullptr)
  {
    log.trace("Delete old widget...");
    model.remove_listener(this);
    vboxi.remove(*(ev->get_widget()));
    delete ev;
  }
  log.trace("Model switch..");
  model = mod;
  model.add_listener(this);
  log.trace("Model view creation..");
  NodeViewConfiguration vconfig;
  vconfig.display_only_tree = false;
  vconfig.show_children = true;
  vconfig.expand_all = true;
  //vconfig.show_children = true;
  //vconfig.show_separator = false;
  //vconfig.nb_attributes_by_column = 20;
  //vconfig.nb_columns = 2;
  ev = new NodeView(this, model, vconfig);
  log.trace("Adding to current view..");
  vboxi.pack_start(*(ev->get_widget()), Gtk::PACK_EXPAND_WIDGET);
  this->show_all_children(true);
  config.set_attribute("last-file", filename);
  update_view();

  DialogManager::setup_window(this);
  return 0;
}

static void usage()
{
  cout << "Usage:" << endl;
  cout << "model-editor.exe [-f schema.xml -s root-node [-d data-file.xml]]" << endl << endl;
}

int ModelEditor::main(CmdeLine &cmdeline)
{
  langue.load(utils::get_fixed_data_path() + PATH_SEP + "std-lang.xml");


  //GtkUtil::set_theme("dark-nimbus");

  std::string filename;

  std::string schema_path;

  if(cmdeline.has_option("--help"))
  {
    usage();
    return 0;
  }

  if(cmdeline.has_option("-f"))
  {
    schema_path = cmdeline.get_option("-f");
  }
  else
  {
    log.warning("no schema specified.");
    
    schema_path = dialogs::open_dialog(str::latin_to_utf8("Sch�ma de donn�es"),
				    "*.xml", 
				    "Fichier XML",
				    "", 
				    last_schema_dir);

    if(schema_path.size() == 0)
      return -1;
  }
  
  if(!files::file_exists(schema_path))
  {
    log.warning("Schema file not found: %s.", schema_path.c_str());
    return -1;
  }

  root_fs = new FileSchema(schema_path);

  string rschema;
  if(cmdeline.has_option("-s"))
  {
    rschema = cmdeline.get_option("-s");
  }
  else
  {
    log.anomaly("TODO: root schema selection.");
    return -1;
  }

  root_schema = root_fs->get_schema(rschema);

  ext = std::string("*.") + cmdeline.get_option("-e", "xml");
  extname = cmdeline.get_option("-n", "Fichier XML (*.xml)");

  if(cmdeline.has_option("-d"))
    open(cmdeline.get_option("-d"));
  else
  {
    log.warning("no file specified.");



    /*string file_path = Gide::open_dialog(Util::latin_to_utf8("Fichier de parametres"),
            ext,
            extname,
            "",
            last_schema_dir);

    if(file_path.size() == 0)
      return -1;
    open(file_path);*/
  }

  Gtk::Main::run(*this);

  return 0;
}

int main(int argc, char **argv)
{
  CmdeLine cmdeline(argc, argv);
  utils::init(cmdeline, "utils", "model-editor");

  //Glib::thread_init();
  Gtk::Main kit(argc, argv);


  ModelEditor *editor = ModelEditor::get_instance();
  return editor->main(cmdeline);
}





