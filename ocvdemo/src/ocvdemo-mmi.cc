
/** @file ocvdemo-mmi.cc

    Copyright 2015 J.A. / http://www.tsdconseil.fr

    Project web page: http://www.tsdconseil.fr/log/opencv/demo/index-en.html

    This file is part of OCVDemo.

    OCVDemo is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    OCVDemo is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with OCVDemo.  If not, see <http://www.gnu.org/licenses/>.
 **/


#include "ocvdemo.hpp"
#include <glibmm.h>

void OCVDemo::setup_menu()
{
  infos("menu setup-ocvdemo-mmi");
  agroup = Gtk::ActionGroup::create();
  agroup->add( Gtk::Action::create("MenuMain", "_Menu") );
  agroup->add( Gtk::Action::create("Input", langue.get_item("menu-entree")),
    sigc::mem_fun(*this, &OCVDemo::on_menu_entree) );

  agroup->add( Gtk::Action::create("Save", langue.get_item("save")),
    sigc::mem_fun(*this, &OCVDemo::on_b_save ) );


  agroup->add( Gtk::Action::create("Open", langue.get_item("Ouvrier")),
    sigc::mem_fun(*this, &OCVDemo::on_b_open) );

  agroup->add( Gtk::Action::create("Quit", langue.get_item("menu-quitter")),
    sigc::mem_fun(*this, &OCVDemo::on_menu_quitter) );
  Glib::RefPtr<Gtk::UIManager> m_refUIManager =
      Gtk::UIManager::create();
  m_refUIManager->insert_action_group(agroup);
  wnd.add_accel_group(m_refUIManager->get_accel_group());

  Glib::ustring ui_info =
      "<ui>"
      "  <menubar name='MenuBar'>"
      "    <menu action='MenuMain'>"
      "      <menuitem action='Input'/>"
      "      <menuitem action='Save'/>"
      "      <menuitem action='Open'/>"
      "      <menuitem action='Quit'/>"
      "    </menu>"
      "  </menubar>"
      "</ui>";

  infos("add-ui ocvdemo-mmi");

  m_refUIManager->add_ui_from_string(ui_info);
  Gtk::Widget *pMenuBar = m_refUIManager->get_widget("/MenuBar");
  vbox.pack_start(*pMenuBar, Gtk::PACK_SHRINK);
  infos("After add-ui ocvdemo-mmi");

}

void OCVDemo::on_menu_entree()
{
  infos("on menu entree. ocvdemo-mmi");

  auto cp = utils::model::Node::create_ram_node(modele_global.schema());

  cp.copy_from(modele_global);

  if(utils::mmi::NodeDialog::display_modal(cp) == 0)
    modele_global.copy_from(cp);
}

void OCVDemo::on_menu_quitter()
{
  on_b_exit();
}

bool OCVDemo::on_delete_event(GdkEventAny *event)
{
  on_b_exit();
  return true;
}

void OCVDemo::on_b_infos()
{
  infos("on_b_infos ocvdemo-mmi");
  Gtk::AboutDialog ad;
  ad.set_copyright("(C) 2015 - 2017 TSD Conseil / J.A.");
  Glib::RefPtr<Gdk::Pixbuf>  pix = Gdk::Pixbuf::create_from_file(utils::get_img_path() + "/logo.png");
  ad.set_logo(pix);
  //ad.set_logo_icon_name("OpenCV demonstrator");
  ad.set_name(langue.get_item("main-title") + "\n");
  ad.set_program_name(langue.get_item("main-title"));




  if(modele_global.get_attribute_as_boolean("mode-appli-ext"))
  {
    ad.set_copyright("(C) 2015 - 2017 TSD Conseil");
  }
  else
  {
    char buf[500];
    std::string s = langue.get_item("rev");
    sprintf(buf, s.c_str(), VMAJ, VMIN, VPATCH, OCV_VMAJ, OCV_VMIN, OCV_VPATCH);
    ad.set_version(buf);
    ad.set_copyright("(C) 2015 - 2016 TSD Conseil / J.A. and contributors");
    ad.set_license("LGPL");
    ad.set_license_type(Gtk::LICENSE_LGPL_3_0);
    ad.set_website("http://www.tsdconseil.fr/log/opencv/demo/index-en.html");
    ad.set_website_label("http://www.tsdconseil.fr/index-en.html");
    std::vector<Glib::ustring> v;
    v.push_back("J.A. / TSD Conseil");
    ad.set_authors(v);
    std::string cmts;

    cmts += langue.get_item("credit-image") + "\n";
    cmts += langue.get_item("credit-image-opencv") + "\n";
    cmts += langue.get_item("credit-image-carte") + "\n";
    cmts += langue.get_item("credit-image-autre");

    ad.set_comments(cmts);
  }


  ad.set_wrap_license(true);

  ad.set_position(Gtk::WIN_POS_CENTER);
  ad.run();
}

void OCVDemo::maj_bts()
{
  int ho = has_output();
  b_save.set_sensitive(ho);
}

void OCVDemo::on_b_open()
{
  img_selecteur.on_b_open();
}

void OCVDemo::on_b_save()
{
  infos("Save.");
  if(!has_output())
  {
    avertissement("Pas de sortie dispo.");
    return;
  }
  //auto s = utils::mmi::dialogs::save_dialog(langue.get_item("title-save"),
  //"*.jpg,*.jp2,*.png,*.bmp", "Image");

  std::string s, title = langue.get_item("title-save");

  Gtk::FileChooserDialog dialog(title, Gtk::FILE_CHOOSER_ACTION_SAVE);
  dialog.set_position(Gtk::WIN_POS_CENTER_ALWAYS);
  if(utils::mmi::mainWindow != nullptr)
    dialog.set_transient_for(*utils::mmi::mainWindow);
  dialog.set_modal(true);
  //Add response buttons the the dialog:
  dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
  dialog.add_button(Gtk::Stock::SAVE, Gtk::RESPONSE_OK);

  //Add filters, so that only certain file types can be selected:



  Glib::RefPtr<Gtk::FileFilter> filter;

  filter = Gtk::FileFilter::create();
  filter->set_name("Image JPEG");
  filter->add_mime_type("*.jpg");
  dialog.add_filter(filter);

  filter = Gtk::FileFilter::create();
  filter->set_name("Image PNG");
  filter->add_mime_type("*.png");
  dialog.add_filter(filter);

  filter = Gtk::FileFilter::create();
  filter->set_name("Image JPEG 2000");
  filter->add_mime_type("*.j2");
  dialog.add_filter(filter);

  filter = Gtk::FileFilter::create();
  filter->set_name("Image BMP");
  filter->add_mime_type("*.bmp");
  dialog.add_filter(filter);

  //Show the dialog and wait for a user response:
  int result = dialog.run();

  //Handle the response:
  if(result == Gtk::RESPONSE_OK)
  {
    std::string filename = dialog.get_filename();

    std::string ext = utils::files::get_extension(filename);
    if(ext.size() == 0)
    {
      auto s = dialog.get_filter()->get_name();
      if(s == "Image JPEG")
        ext = ".jpg";
      else if(s == "Image JPEG 2000")
        ext = ".jp2";
      else if(s == "Image BMP")
        ext = ".bmp";
      else if(s == "Image PNG")
        ext = ".png";
      filename += ext;
    }

    dialog.hide();

    if(demo_en_cours->output.nout == 1)
      imwrite(filename, get_current_output());
    else
    {
      auto fnaked = utils::files::remove_extension(filename);
      auto ext = utils::files::get_extension(filename);
      for(auto i = 0; i < demo_en_cours->output.nout; i++)
        imwrite(fnaked + utils::str::int2str(i) + "." + ext,  demo_en_cours->output.images[i]);
    }
  }
}

void OCVDemo::on_dropped_file(const Glib::RefPtr<Gdk::DragContext>& context, int x, int y, const Gtk::SelectionData& selection_data, guint info, guint time)
{
 if ((selection_data.get_length() >= 0) && (selection_data.get_format() == 8))
 {

   img_selecteur.on_dropped_file(context, x, y, selection_data, info, time);

#   if 0
   std::vector<Glib::ustring> file_list;

   file_list = selection_data.get_uris();

   if(file_list.size() > 0)
   {
     Glib::ustring path = Glib::filename_from_uri(file_list[0]);
     //do something here with the 'filename'. eg open the file for reading

     std::string s = path;//file_list[0];//path;

     infos("DnD: %s.", s.c_str());

     utils::model::Node new_model = utils::model::Node::create_ram_node(modele_global.schema());
     new_model.copy_from(modele_global);
     new_model.set_attribute("sel", 1);
     new_model.set_attribute("file-schema/path", s);
     modele_global.copy_from(new_model);

     context->drag_finish(true, false, time);
     return;
   }
#   endif
 }
 else
   context->drag_finish(false, false, time);
}

void OCVDemo::on_b_exit()
{
  trace_majeure("Fin normale de l'application ocvdemo-mmi.");
  ODEvent evt;
  evt.type = ODEvent::FIN;
  event_fifo.push(evt);
  
  utils::files::delete_file(lockfile);
  wnd.hide();


  OCVDemoFinAppli odfa;
  dispatch(odfa);

  exit(0);
}



void OCVDemo::maj_langue_systeme()
{
  trace_majeure("maj_langue_systeme() ocvdemo-mmi.");

  int sel = modele_global.get_attribute_as_int("langue");
  utils::model::Localized::current_language = (Localized::LanguageEnum) (sel + Localized::LANG_FR);
}

void OCVDemo::maj_langue()
{
  trace_majeure("maj_langue() ocvdemo-mmi.");

  auto prev_lg = utils::model::Localized::current_language;
  maj_langue_systeme();

  b_save.set_label(langue.get_item("save"));
  b_save.set_tooltip_markup(langue.get_item("save-tt"));
  b_exit.set_label(langue.get_item("quitter"));
  b_exit.set_tooltip_markup(langue.get_item("quitter-tt"));
  b_infos.set_label(langue.get_item("apropos"));
  b_infos.set_tooltip_markup(langue.get_item("apropos-tt"));
  b_entree.set_label(langue.get_item("entree"));
  b_open.set_label(langue.get_item("b-ouvrir"));
  b_entree.set_tooltip_markup(langue.get_item("entree-tt"));
  b_open.set_tooltip_markup(langue.get_item("entree-tt"));
  // Apparently OpenCV windows support only ISO-8859-1
  // Seems OpenCV does the right thing now.
  // Correction. olnly works with OpenCV on Linux. 
  // Still broke on windows.
#ifdef WIN
  titre_principal = utils::str::utf8_to_latin(langue.get_item("resultats"));
#else  
  titre_principal = langue.get_item("resultats");
#endif

  //char bf[500];
  //sprintf(bf, " [version %d.%d.%d]", VMAJ, VMIN, VPATCH);

  wnd.set_title(langue.get_item("main-title"));// + std::string(bf));

  if(utils::model::Localized::current_language != prev_lg)
  {
    lock = true;
    vue_arbre.maj_langue();
    lock = false;
    utils::mmi::SelectionChangeEvent sce;
    sce.new_selection = vue_arbre.get_selection();
    this->on_event(sce);
  }
  barre_outil_dessin.maj_lang();
  img_selecteur.maj_langue();
}

