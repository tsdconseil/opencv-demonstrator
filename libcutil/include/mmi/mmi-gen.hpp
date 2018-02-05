#ifndef MY_MMI_GEN_H
#define MY_MMI_GEN_H




#include "cutil.hpp"
#include "mmi/gtkutil.hpp"
#include "mmi/stdview.hpp"
#include "modele.hpp"
#include <gtkmm/scale.h>
#include <gtkmm/menubar.h>
#include "../journal.hpp"

namespace utils { namespace mmi {

struct MMIGenSection
{
  MMIGenSection(utils::model::Node modele);
  JFrame frame;
  utils::model::Node modele;
  utils::mmi::NodeView *vue;
  Gtk::VBox vbox;
  Gtk::HButtonBox bbox;
};

class MMIGen:
  public Gtk::Window
{
public:
  MMIGen();
  MMIGen(utils::CmdeLine &cmdline, utils::model::Node modele, FileSchema *root);
  int setup(utils::CmdeLine &cmdline, utils::model::Node modele, FileSchema *root);
  static MMIGen *get_instance();
  MMIGenSection *lookup(std::string nom);


protected:

  struct ActionEvent{};

  struct Action: public utils::CProvider<ActionEvent>
  {
    std::string id;
    Gtk::ToolButton bouton;
  };

  std::vector<Action *> actions;

  Action *recherche_action(const std::string &id);

  virtual void gere_fin_application() {}

  Gtk::Toolbar barre_outils;
  unsigned int ncolonnes;
  CmdeLine cmdline;
  utils::model::Node modele_mmi; // Modèle statique, pour construire l'IHM
  utils::model::Node modele; // Modèle de donnée dynamique
  utils::model::NodeSchema *schema_vue; // Schéma construit
  std::vector<MMIGenSection *> sections;
  Gtk::VBox  vbox_princ;
  Gtk::Frame frame_menu;
  std::vector<Gtk::VBox *> vboxes;
  Gtk::HBox hbox;//, hb1, hb2;
  Gtk::ProgressBar progress;
  Glib::RefPtr<Gtk::TextBuffer> text_buffer;
  Gtk::ScrolledWindow text_scroll;
  Gtk::TextView text_view;
  std::string historique;

  // Barre d'outils

  Gtk::ToolButton b_open, b_infos, b_exit, b_save;
  Gtk::MenuBar barre_menu;

  void gere_bouton(std::string id);
  void on_b_save();
  void on_b_open();
  void on_b_infos();
  void on_b_exit();
  void on_b_non_gere(){}

  virtual void maj_vue();
  void maj_langue();
  void set_histo(std::string text);
  void put_histo(std::string text);
  void put_histo_temp(std::string text);

  int lock;
private:
  void init();
};

}}



#endif
