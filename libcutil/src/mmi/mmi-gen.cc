
#include "mmi/mmi-gen.hpp"
#include <stdio.h>
#ifdef WIN
#include <process.h>
#endif
#include <cairomm/context.h>
#include <gdkmm/pixbuf.h>
#include <gdkmm/general.h>
#include <gtkmm/drawingarea.h>
#include <gtkmm/imagemenuitem.h>
#include <gtkmm/uimanager.h>


using namespace std;
using namespace utils;
using namespace utils::model;

namespace utils { namespace mmi {

static MMIGen *instance = NULL;
Node configuration;
FileSchema *fs;

MMIGen *MMIGen::get_instance()
{
  return instance;
}


MMIGenSection::MMIGenSection(Node modele)
{
  this->modele = modele;
  frame.set_border_width(5);
  frame.add(vbox);
  bbox.set_spacing(4);
  bbox.set_border_width(4);
  bbox.set_layout(Gtk::BUTTONBOX_END);

  if(modele.is_nullptr())
  {
    erreur("MMIGenSection : sans modele ?");
  }
  else
  {
    trace_majeure("toxml...");
    auto s = modele.to_xml(true, true);
    trace_majeure("Creation vue noeud :\n%s\n", s.c_str());
  }

  vue = new NodeView(instance, modele);

  vbox.pack_start(*(vue->get_widget()), Gtk::PACK_SHRINK);
  vbox.pack_start(bbox, Gtk::PACK_SHRINK);

  vbox.show_all_children(true);

  frame.set_label(modele.schema()->get_localized());
}

MMIGen::Action *MMIGen::recherche_action(const std::string &id)
{
  for(auto act: actions)
    if(act->id == id)
      return act;
  erreur("Action non trouvee : %s.", id.c_str());
  return nullptr;
}

void MMIGen::init()
{
}

MMIGen::MMIGen()
{
  init();
}

void MMIGen::gere_bouton(std::string id)
{
  auto act = recherche_action(id);
  if(act == nullptr)
    return;
  ActionEvent ae;
  act->dispatch(ae);
}

int MMIGen::setup(utils::CmdeLine &cmdline, utils::model::Node modele_mmi, FileSchema *root)
{
  std::string s1 = modele_mmi.to_xml(true, true);
  infos("Construction MMI GEN (modèle [%s])", s1.c_str());
  this->sections = sections;
  lock = 0;
  this->modele_mmi = modele_mmi;
  this->cmdline = cmdline;
  instance = this;
  set_title(modele_mmi.get_localized_name());
  set_border_width(5);
  set_default_size(modele_mmi.get_attribute_as_int("largeur-par-defaut"),
                   modele_mmi.get_attribute_as_int("hauteur-par-defaut"));


  //configuration.add_listener(this);

  barre_outils.set_icon_size(Gtk::ICON_SIZE_SMALL_TOOLBAR);
  //barre_outils.set_icon_size(Gtk::ICON_SIZE_SMALL_TOOLBAR);
  barre_outils.set_toolbar_style(Gtk::TOOLBAR_BOTH);
  barre_outils.set_has_tooltip(false);

  vbox_princ.pack_start(frame_menu, Gtk::PACK_SHRINK);
  vbox_princ.pack_start(barre_outils, Gtk::PACK_SHRINK);
  barre_outils.add(b_open);
  barre_outils.add(b_save);


  for(auto &ch: modele_mmi.children("mmi-gen-action"))
  {
    Action *a = new Action();
    a->id = ch.get_attribute_as_string("name");
    a->bouton.set_label(ch.get_localized_name());
    auto icp = ch.get_attribute_as_string("icone");
    if(icp.size() > 0)
    {
      auto s = utils::get_fixed_data_path() + "/img/" + icp;
      if(!utils::files::file_exists(s))
      {
        erreur("Fichier icone non trouve : %s", s.c_str());
      }
      else
      {
        auto img = new Gtk::Image(s);
        a->bouton.set_icon_widget(*img);
      }
    }
    barre_outils.add(a->bouton);
    a->bouton.signal_clicked().connect(sigc::bind(sigc::mem_fun(*this, &MMIGen::gere_bouton), a->id));
    actions.push_back(a);
  }



  barre_outils.add(b_infos);
  barre_outils.add(b_exit);

  b_open.set_stock_id(Gtk::Stock::OPEN);
  b_save.set_stock_id(Gtk::Stock::SAVE);
  b_infos.set_stock_id(Gtk::Stock::ABOUT);
  b_exit.set_stock_id(Gtk::Stock::QUIT);

  //auto img = new Gtk::Image(utils::get_fixed_data_path() + "/img/zones.png");
  //b_zones.set_icon_widget(*img);


  b_open.signal_clicked().connect(sigc::mem_fun(*this, &MMIGen::on_b_open));
  b_save.signal_clicked().connect(sigc::mem_fun(*this, &MMIGen::on_b_save));
  b_exit.signal_clicked().connect(sigc::mem_fun(*this, &MMIGen::on_b_exit));
  b_infos.signal_clicked().connect(sigc::mem_fun(*this, &MMIGen::on_b_infos));


  add(vbox_princ);

  vbox_princ.pack_start(hbox, Gtk::PACK_EXPAND_WIDGET);

  ncolonnes = modele_mmi.get_attribute_as_int("ncolonnes");
  for(auto i = 0u; i < ncolonnes; i++)
  {
    vboxes.push_back(new Gtk::VBox());
    hbox.pack_start(*(vboxes[i]), Gtk::PACK_EXPAND_WIDGET);//Gtk::PACK_SHRINK);
  }

  schema_vue = new utils::model::NodeSchema();
  schema_vue->name.set_value(Localized::LANG_ID, "modele");
  for(auto &s: modele_mmi.children("mmi-gen-section"))
  {
    utils::model::SubSchema ss;
    utils::model::NodeSchema *sschema;
    auto nom_modele = s.get_attribute_as_string("modele");
    if(nom_modele.size() > 0)
    {
      sschema = root->get_schema(nom_modele);
      if(sschema == nullptr)
        return -1;
      ss.child_str = nom_modele;
    }
    else
    {
      sschema = new utils::model::NodeSchema(s, root);
      ss.child_str = s.get_attribute_as_string("name");
    }
    ss.ptr = sschema;
    ss.min = ss.max = 1;
    ss.name.set_value(Localized::LANG_ID, ss.child_str);
    schema_vue->add_sub_node(ss);
  }
  schema_vue->update_size_info();
  //schema->serialize()

  modele = utils::model::Node::create_ram_node(schema_vue);

  std::string s = modele.to_xml(true, true);
  infos("Création schema :\n%s\n", s.c_str());
  s = schema_vue->to_string();
  infos("Schema :\n%s\n", s.c_str());
  for(auto s: modele_mmi.children("mmi-gen-section"))
  {
    unsigned int col = s.get_attribute_as_int("colonne");
    if(col >= ncolonnes)
    {
      erreur("Numéro de colonne invalide (%d / %d)", col, ncolonnes);
      continue;
    }

    MMIGenSection *mgs = new MMIGenSection(modele.get_child(s.get_attribute_as_string("name")));
    sections.push_back(mgs);
    vboxes[col]->pack_start(mgs->frame, Gtk::PACK_EXPAND_WIDGET);
  }

  //vboxes[ncolonnes-1].pack_start(frame_infos, Gtk::PACK_SHRINK);

  vboxes[ncolonnes-1]->pack_end(progress, Gtk::PACK_SHRINK);

  /*frame_infos.set_label(utils::str::latin_to_utf8("Informations"));
  frame_infos.add(text_scroll);*/
  text_scroll.add(text_view);
  text_scroll.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_ALWAYS);
  text_buffer = Gtk::TextBuffer::create();
  text_buffer->create_tag("bold")->property_weight() = Pango::WEIGHT_BOLD;

  text_view.set_editable(false);
  text_view.set_buffer(text_buffer);

  show_all_children(true);

  maj_vue();
  return 0;
}

MMIGenSection *MMIGen::lookup(std::string nom)
{
  for(auto &e: this->sections)
    if(e->modele.schema()->name.get_id() == nom)
      return e;
  erreur("MMIGen::%s: non trouvé (%s).", __func__, nom.c_str());
  return nullptr;
}

MMIGen::MMIGen(CmdeLine &cmdline,
               utils::model::Node modele_mmi,
               FileSchema *root)
{
  init();
  setup(cmdline, modele_mmi, root);
}


void MMIGen::on_b_save()
{
/*  std::string fichier = utils::mmi::dialogs::save_dialog(
      langue.get_item("dlg-sauve-titre"),
      "*.xml",
      "Fichier XML");
  if(fichier.size() > 0)
    configuration.save(fichier, true);*/
}

void MMIGen::on_b_open()
{
/*  std::string fichier = utils::mmi::dialogs::open_dialog(
      langue.get_item("dlg-ouvre-titre"),
      "*.xml",
      "Fichier XML");
  if(fichier.size() > 0)
    configuration.load(fichier);*/
}

void MMIGen::on_b_infos()
{
  infos("on_b_infos gen");
  Gtk::AboutDialog ad;
  ad.set_copyright("(C) 2017 TSD CONSEIL");
  //Glib::RefPtr<Gdk::Pixbuf>  pix = Gdk::Pixbuf::create_from_file(utils::get_img_path() + "/todo.png");
  //ad.set_logo(pix);
  ad.set_name(langue.get_item("titre-principal") + "\n");
  ad.set_program_name(langue.get_item("titre-principal"));
  ad.set_version(modele_mmi.get_attribute_as_string("version"));
  ad.set_position(Gtk::WIN_POS_CENTER);
  ad.run();
}

void MMIGen::on_b_exit()
{
  trace_majeure("Fin normale de l'application.");
  //utils::files::delete_file(lockfile);
  hide();
  gere_fin_application();
  exit(0);
}





void MMIGen::set_histo(std::string text)
{
  text = utils::str::latin_to_utf8(text);
  historique = text;
  text_buffer->set_text(historique);
}

void MMIGen::put_histo(std::string text)
{
  text = utils::str::latin_to_utf8(text);
  historique += text;
  text_buffer->set_text(historique);
  Gtk::TextBuffer::iterator it = text_buffer->end();
  text_view.scroll_to(it);
}

void MMIGen::put_histo_temp(std::string text)
{
  text = utils::str::latin_to_utf8(text);
  text_buffer->set_text(historique + text);
}

void MMIGen::maj_langue()
{
  b_open.set_label(langue.get_item("open"));
  b_open.set_tooltip_markup(langue.get_item("open-tt"));
  b_save.set_label(langue.get_item("save"));
  b_save.set_tooltip_markup(langue.get_item("save-tt"));
  b_exit.set_label(langue.get_item("quitter"));
  b_exit.set_tooltip_markup(langue.get_item("quitter-tt"));
  b_infos.set_label(langue.get_item("apropos"));
  b_infos.set_tooltip_markup(langue.get_item("apropos-tt"));
}

void MMIGen::maj_vue()
{
  bool connected = false;
  bool ope_ok = connected;
  b_save.set_sensitive(ope_ok);
  b_open.set_sensitive(ope_ok);
  maj_langue();
}


}}


