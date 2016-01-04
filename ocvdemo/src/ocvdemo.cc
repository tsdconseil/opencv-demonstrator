/** @file ocvdemo.cc

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
#include "demo-items/filter-demo.hpp"
#include "demo-items/morpho-demo.hpp"
#include "demo-items/gradient-demo.hpp"
#include "demo-items/photographie.hpp"
#include "demo-items/reco-demo.hpp"
#include "demo-items/histo.hpp"
#include "demo-items/seuillage.hpp"
#include "demo-items/video-demo.hpp"
#include "demo-items/espaces-de-couleurs.hpp"
#include "demo-items/3d.hpp"
#include <glibmm.h>

#define VMAJ 1
#define VMIN 3

// Conversion latin -> utf8:
// iconv -f latin1 -t utf-8 data/lang8859.xml > data/lang.xml

using utils::model::Localized;

static OCVDemo *instance = nullptr;

void OCVDemo::add_demos()
{
  items.push_back(new HDRDemo());
  items.push_back(new EpiDemo());
  items.push_back(new DispMapDemo());
  items.push_back(new MatchDemo());
  items.push_back(new ContourDemo());
  items.push_back(new CamCalDemo());
  items.push_back(new DFTDemo());
  items.push_back(new InpaintDemo());
  items.push_back(new PanoDemo());
  items.push_back(new WShedDemo());
  items.push_back(new SousArrierePlanDemo());
  items.push_back(new FilterDemo());
  items.push_back(new HSVDemo());
  items.push_back(new MorphoDemo());
  items.push_back(new CannyDemo());
  items.push_back(new HoughDemo());
  items.push_back(new HoughCDemo());
  items.push_back(new GradientDemo());
  items.push_back(new LaplaceDemo());
  items.push_back(new NetDemo());
  items.push_back(new CornerDemo());
  items.push_back(new HistoDemo());
  items.push_back(new HistoCalc());
  items.push_back(new HistoBP());
  items.push_back(new Seuillage());
  items.push_back(new GrabCutDemo());
  items.push_back(new OptFlowDemo());
  items.push_back(new CamShiftDemo());
  items.push_back(new RectDemo());
  items.push_back(new DTransDemo());
  items.push_back(new CascGenDemo("casc-visage"));
  items.push_back(new CascGenDemo("casc-profile"));
  items.push_back(new CascGenDemo("casc-yeux"));
  items.push_back(new CascGenDemo("casc-plate"));
  items.push_back(new CascGenDemo("casc-sil"));

}

void OCVDemo::on_b_masque_raz()
{
  masque_raz();
  update_Ia();
  update();
}

void OCVDemo::on_b_masque_gomme()
{
  barre_outil_dessin.b_remplissage.set_active(false);
  barre_outil_dessin.b_gomme.set_active(true);
  outil_dessin_en_cours = 0;
}

void OCVDemo::on_b_masque_remplissage()
{
  barre_outil_dessin.b_gomme.set_active(false);
  barre_outil_dessin.b_remplissage.set_active(true);
  outil_dessin_en_cours = 1;
}






void OCVDemo::compute_Ia()
{
  if((item_en_cours != nullptr) && (item_en_cours->props.requiert_roi))
  {
    Ia = I1.clone();
    cv::rectangle(Ia, rdi0, rdi1, Scalar(0,255,0), 3);
  }
  else if((item_en_cours != nullptr) && (item_en_cours->props.requiert_masque))
  {
    if((Ia.data == nullptr) || (Ia.size() != I1.size()))
      Ia = I1.clone();
  }
  else
  {
    Ia = I1;
  }
}

void OCVDemo::masque_clic(int x0, int y0)
{
  if(this->outil_dessin_en_cours == 0)
  {
    for(auto i = -2; i <= 2; i++)
    {
      for(auto j = -2; j <= 2; j++)
      {
        int x = x0 + i;
        int y = y0 + j;
        Ia.at<Vec3b>(y,x).val[0] = 255;
        Ia.at<Vec3b>(y,x).val[1] = 255;
        Ia.at<Vec3b>(y,x).val[2] = 255;
        item_en_cours->params.masque.at<unsigned char>(y,x) = 255;
      }
    }
  }
  else
  {
    // Floodfill
    cv::Mat mymask = Mat::zeros(Ia.rows+2,Ia.cols+2,CV_8U);
    cv::floodFill(Ia, mymask, Point(x0,y0), Scalar(255,255,255));
    Mat roi(mymask, Rect(1,1,Ia.cols,Ia.rows));
    this->item_en_cours->params.masque |= roi;
    update();
  }

}

void OCVDemo::masque_raz()
{
  Ia = I0.clone();
  item_en_cours->params.masque = cv::Mat::zeros(I0.size(), CV_8U);
}

void OCVDemo::mouse_callback(int image, int event, int x, int y, int flags)
{
  mutex.lock();
  journal.trace("ocvdemo mouse callback img = %d, x = %d, y = %d.", image, x, y);
  switch (event)
  {
    case CV_EVENT_LBUTTONDOWN:
    {
      journal.trace("LB DOWN x = %d, y = %d.", x, y);
      //Point seedPoint = cvPoint(x,y); //setting mouse clicked location as seed point

      rdi0.x = x;
      rdi0.y = y;
      rdi1 = rdi0;
      etat_souris = 1;

      if((item_en_cours != nullptr) && (item_en_cours->props.requiert_masque))
      {
        masque_clic(x,y);
        compute_Ia();
        update_Ia();
      }

      //floodFill(I0, seedPoint, Scalar(0,255,0));
      //update();
      break;
    }
    case CV_EVENT_MOUSEMOVE:
    {
      if(etat_souris == 1)
      {
        //Ia = I1.clone();
        rdi1.x = x;
        rdi1.y = y;

        if((item_en_cours != nullptr) && (item_en_cours->props.requiert_masque))
          masque_clic(x,y);

        //cv::rectangle(Ia, roi0, roi1, Scalar(0,255,0), 3);
        compute_Ia();
        update_Ia();
        journal.trace("updated ia.");
      }
      break;
    }
    case CV_EVENT_LBUTTONUP:
    {
      journal.trace("LB UP x = %d, y = %d.", x, y);
      if(etat_souris == 1)
      {
        //Ia = I1.clone();
        rdi1.x = x;
        rdi1.y = y;
        compute_Ia();
        //cv::rectangle(Ia, roi0, roi1, Scalar(0,255,0), 3);
        etat_souris = 0;
        if(item_en_cours != nullptr)
        {
          int minx = min(rdi0.x, rdi1.x);
          int miny = min(rdi0.y, rdi1.y);
          int maxx = max(rdi0.x, rdi1.x);
          int maxy = max(rdi0.y, rdi1.y);
          Rect roi(minx, miny, maxx - minx, maxy - miny);
          journal.trace_major("Set roi(%d,%d,%d,%d).",
                              roi.x, roi.y, roi.width, roi.height);
          item_en_cours->set_roi(I0, roi);
        }
        update();
      }
      break;
    }
  }
  mutex.unlock();
}

void OCVDemo::update_Ia()
{
  mosaique.update_image(0, Ia);
}

static void prepare_image(cv::Mat &I)
{
  if(I.channels() == 1)
    cv::cvtColor(I, I, CV_GRAY2BGR);

  if(I.depth() == CV_32F)
    I.convertTo(I, CV_8UC3);
}

void OCVDemo::update()
{
  this->sortie_en_cours = Mat();


  if(modele_demo.is_nullptr())
    return;

  journal.trace("full update.");
  auto id = modele_demo.get_attribute_as_string("name");
  unsigned int i, j;
  namedWindow(titre_principal, CV_WINDOW_NORMAL);
  for(i = 0u; i < items.size(); i++)
  {
    auto demo = items[i];
    if(demo->props.id == id)
    {
      item_en_cours = demo;

      if(first_entry)
      {
        first_entry = false;
        rdi0.x = demo->params.roi.x;
        rdi0.y = demo->params.roi.y;
        rdi1.x = demo->params.roi.x + demo->params.roi.width;
        rdi1.y = demo->params.roi.y + demo->params.roi.height;



        demo->params.modele = modele;
        demo->setup_model(modele);

        if(demo->configure_ui())
          break;

        maj();

        if(item_en_cours->props.requiert_masque)
        {
          item_en_cours->params.masque = cv::Mat::zeros(I0.size(), CV_8U);
          barre_outil_dessin.montre();
        }
        else
          barre_outil_dessin.cache();

        if(item_en_cours->props.requiert_mosaique)
        {
          img_selecteur.show();
          img_selecteur.present();
          img_selecteur.raz();
          for(const auto &img: modele_demo.children("img"))
            img_selecteur.ajoute_fichier(img.get_attribute_as_string("path"));

        }
        else
          img_selecteur.hide();


      }

      try
      {
        I1 = I0.clone();

        auto s = modele.to_xml();

        journal.trace("Calcul [%s], img: %d*%d, %d chn, model =\n%s",
                      demo->props.id.c_str(),
                      I1.cols, I1.rows, I1.channels(),
                      s.c_str());


        if(demo->props.requiert_mosaique)
          this->img_selecteur.get_list(demo->params.mosaique);

        int retcode = demo->calcul(modele, I1);

        if(retcode)
        {
          journal.warning("ocvdemo: Echec calcul.");
          auto s = demo->sortie.errmsg;
          if(langue.has_item(s))
            s = langue.get_item(s);
          utils::mmi::dialogs::show_warning("Erreur de traitement",
                      langue.get_item("echec-calcul"), s);
          break;
        }
        else
        {
          journal.verbose("Calcul [%s] ok.", demo->props.id.c_str());

          if(demo->sortie.vrai_sortie.data != nullptr)
            sortie_en_cours = demo->sortie.vrai_sortie;
          else if(demo->sortie.nb_sorties > 0)
            sortie_en_cours = demo->sortie.O[demo->sortie.nb_sorties - 1];
          else
            sortie_en_cours = I1;//demo->O[0];
        }

      }
      catch(...)
      {
        utils::mmi::dialogs::show_error(langue.get_item("err-opencv-title"),
            langue.get_item("err-opencv-msg"), langue.get_item("err-opencv-msg2"));
        break;
      }


      std::vector<cv::Mat> lst;
      compute_Ia();
      prepare_image(Ia);
      lst.push_back(Ia);
      unsigned int img_count = modele_demo.get_attribute_as_int("img-count");

      if(demo->sortie.nb_sorties >= 0)
        img_count = 1 + demo->sortie.nb_sorties;

      if((img_count > 1) && (demo->sortie.O[0].data == nullptr))
      {
        img_count = 1;
        journal.warning("Img count = 2, et image O non initialisée.");
      }

      std::vector<std::string> titres;


      for(j = 0; j < img_count; j++)
      {
        if(j > 0)
        {
          prepare_image(demo->sortie.O[j-1]);
          lst.push_back(demo->sortie.O[j-1]);
        }

        char buf[50];
        sprintf(buf, "o[id=%d]", j);
        std::string s = "";
        if((j + 1 == img_count) && (img_count > 1))
          s = langue.get_item("img-sortie");
        if((j == 0) && (img_count > 1))
          s = langue.get_item("img-entree");
        if(modele_demo.has_child(std::string(buf)))
        {
          auto n = modele_demo.get_child(std::string(buf));
          s = n.get_localized().get_localized();
        }
        if((j < 4) && (item_en_cours->sortie.outname[j].size() > 0))
        {
          s = item_en_cours->sortie.outname[j];
          if(langue.has_item(s))
            s = langue.get_item(s);
        }
        titres.push_back(utils::str::utf8_to_latin(s));;
      }

      mosaique.show_multiple_images(titre_principal, lst, titres);
      break;
    }
  }
  maj_bts();
  if(i == items.size())
    journal.warning("Demo not found: %s", id.c_str());
  cv::waitKey(10);
}

void OCVDemo::maj_entree()
{
  maj();
  this->first_entry = true;
  update();
  maj_bts();
}

void OCVDemo::on_event(const ChangeEvent &ce)
{
  journal.verbose("change-event: %d / %s", (int) ce.type, ce.path[0].name.c_str());

  if(ce.type != ChangeEvent::GROUP_CHANGE)
    return;

  if(lock)
    return;

  lock = true;

  if(ce.path[0].name == "global-schema")
  {
    journal.verbose("Changement sur configuration globale");
    lock = false;
    maj_langue();
    maj_entree();
    modele_global.save(chemin_fichier_config);
    maj_bts();
    return;
  }

  journal.verbose("Change event detected.");

  update();
  lock = false;

  maj_bts();
}

void OCVDemo::on_event(const ImageSelecteurRefresh &e)
{
  if(lock)
    return;
  lock = true;
  update();
  lock = false;
  maj_bts();
}

void OCVDemo::setup_demo(std::string id)
{

  ChangeEvent ce;
  ce.type = ChangeEvent::GROUP_CHANGE;
  on_event(ce);
  this->wnd.present();

  if(item_en_cours->props.requiert_mosaique)
    img_selecteur.present();


  if(item_en_cours && (item_en_cours->props.requiert_masque))
    barre_outil_dessin.montre();



  maj_bts();
}


void OCVDemo::maj()
{
  /*if(lock)
    return;*/
  if(item_en_cours == nullptr)
    return;


  //lock = true;
  entree_video = false;

  std::string fp = "";
  int idx = 0;
  int sel = modele_global.get_attribute_as_int("sel");
  // Image / vidéo par défaut
  if(sel == 0)
  {
    fp = modele_demo.get_attribute_as_string("default-img");
  }
  // Fichier
  else if(sel == 1)
  {
    fp = modele_global.get_attribute_as_string("file-schema/path");
  }
  // Caméra USB
  else
  {
    idx = 0;//global_model.get_attribute_as_int("cam-schema/idx");
  }



  std::string ext;
  if(sel != 2)
    ext = utils::files::get_extension(fp);

  if(video_capture.isOpened())
    video_capture.release();

  if(sel == 2)
  {
    journal.trace_major("Ouverture camera usb #%d...", idx);
    if(!video_capture.open(idx))
    {
      journal.warning("Camera non détectée: reset sel = 0...");
      modele_global.set_attribute("sel", (int) 0);
      journal.trace_major("Reset effectué.");
      utils::mmi::dialogs::show_error(langue.get_item("ech-cam-tit"),
          langue.get_item("ech-cam-sd"),
          langue.get_item("ech-cam-d"));
      //lock = false;
      maj();
      return;
    }
    video_fp    = "";
    video_idx   = idx;
    entree_video = true;
    video_capture >> I0;
  }
  else if((ext == "mpg") || (ext == "avi") || (ext == "mp4") || (ext == "wmv"))
  {
    journal.trace("Ouverture fichier video [%s]...", fp.c_str());
    if(!video_capture.open(fp))
    {
      utils::mmi::dialogs::show_error(langue.get_item("ech-vid-tit"),
          langue.get_item("ech-vid-sd"),
          langue.get_item("ech-vid-d") + "\n" + fp);
      modele_global.set_attribute("sel", (int) 0);
      //lock = false;
      maj();
      return;
    }
    video_fp = fp;
    entree_video = true;
    video_capture >> I0;
  }
  else
  {
    journal.trace("Ouverture fichier image [%s]...", fp.c_str());
    I0 = cv::imread(fp.c_str());
    if(I0.data == nullptr)
    {
      utils::mmi::dialogs::show_error("Erreur",
          "Impossible de charger l'image", std::string("Chemin : ") + fp);
      destroyWindow(titre_principal);
      mosaique.callback_init_ok = false;
    }
  }
  I1 = I0.clone();
  Ia = I0.clone();
  maj_bts();
  //lock = false;
}


void OCVDemo::setup_demo_p(const Node &sel)
{
  auto id = sel.get_attribute_as_string("name");
  auto s = sel.get_localized_name();
  journal.verbose("Selection changed: %s (%s).", id.c_str(), s.c_str());

  first_entry = true;

  if (rp != nullptr)
  {
    //log.trace("Removing prop. contents...");
    cadre_proprietes.remove();
    delete rp;
    rp = nullptr;
  }
  else
  {
    cadre_proprietes.remove();
  }

  modele_demo = sel;//tree_view.get_selection();

  if(sel.schema()->name.get_id() == "demo")
  {

    auto schema = fs_racine->get_schema(id);
    maj();

    if(schema != nullptr)
    {
      modele = utils::model::Node::create_ram_node(schema);
      modele.add_listener(this);
      utils::mmi::NodeViewConfiguration vconfig;
      vconfig.show_desc = true;
      vconfig.show_main_desc = true;
      rp = new utils::mmi::NodeView(&wnd, modele, vconfig);
      cadre_proprietes.set_label(s);
      cadre_proprietes.add(*(rp->get_widget()));
      cadre_proprietes.show();
      cadre_proprietes.show_all_children(true);
      journal.trace("setup demo...");
      setup_demo(id);
    }
  }
  else
  {
    destroyWindow(titre_principal);
    mosaique.callback_init_ok = false;
  }
  maj_bts();
}




void OCVDemo::on_event(const utils::mmi::SelectionChangeEvent &e)
{
  if(lock)
    return;

  this->sortie_en_cours = Mat();

  if(e.new_selection.is_nullptr())
  {
    maj_bts();
    return;
  }

  setup_demo_p(e.new_selection);
}

OCVDemo *OCVDemo::get_instance()
{
  return instance;
}

namespace utils
{
  extern Localized::Language current_language;
}

void OCVDemo::maj_langue_systeme()
{
  int sel = modele_global.get_attribute_as_int("langue");


  // TODO: remove this stupid redondance
  utils::current_language = (Localized::LanguageEnum) (sel + Localized::LANG_FR);
  langue.current_language = Localized::language_id(utils::current_language);
}

void OCVDemo::maj_langue()
{
  auto prev_lg = langue.current_language;
  maj_langue_systeme();

  b_save.set_label(langue.get_item("save"));
  b_save.set_tooltip_markup(langue.get_item("save-tt"));
  b_exit.set_label(langue.get_item("quitter"));
  b_exit.set_tooltip_markup(langue.get_item("quitter-tt"));
  b_infos.set_label(langue.get_item("apropos"));
  b_infos.set_tooltip_markup(langue.get_item("apropos-tt"));
  b_entree.set_label(langue.get_item("entree"));
  b_entree.set_tooltip_markup(langue.get_item("entree-tt"));
  titre_principal = utils::str::utf8_to_latin(langue.get_item("resultats"));
  wnd.set_title(langue.get_item("main-title"));

  if(langue.current_language != prev_lg)
  {
    lock = true;
    vue_arbre.maj_langue();
    lock = false;
    utils::mmi::SelectionChangeEvent sce;
    sce.new_selection = vue_arbre.get_selection();
    this->on_event(sce);
  }
  barre_outil_dessin.maj_lang();
}

OCVDemo::OCVDemo(utils::CmdeLine &cmdeline)
{
  utils::current_language = Localized::LANG_EN;
  langue.current_language = "en";
  outil_dessin_en_cours = 0;
  entree_video = false;
  first_entry = true;
  item_en_cours = nullptr;
  etat_souris = 0;
  instance = this;
  lock = false;
  rp = nullptr;
  fs_racine = new utils::model::FileSchema("./data/schema.xml");
  utils::mmi::NodeViewConfiguration vconfig;


  chemin_fichier_config = utils::get_current_user_path() + PATH_SEP + "cfg.xml";
  if(!files::file_exists(chemin_fichier_config))
  {
    modele_global = utils::model::Node::create_ram_node(fs_racine->get_schema("global-schema"));
    modele_global.save(chemin_fichier_config);
    this->on_menu_entree();
  }
  else
  {
    modele_global = Node::create_ram_node(fs_racine->get_schema("global-schema"), chemin_fichier_config);
    if(modele_global.is_nullptr())
    {
      modele_global = utils::model::Node::create_ram_node(fs_racine->get_schema("global-schema"));
      modele_global.save(chemin_fichier_config);
    }
  }

  journal.trace("Application configuration:\n%s\n", modele_global.to_xml().c_str());

  maj_langue_systeme();

  lockfile = utils::get_current_user_path() + PATH_SEP + "lock.dat";
  if(utils::files::file_exists(lockfile))
  {
    if(utils::mmi::dialogs::check_dialog(
        langue.get_item("check-lock-1"),
        langue.get_item("check-lock-2"),
        langue.get_item("check-lock-3")))
    {
      auto s = utils::mmi::dialogs::save_dialog(langue.get_item("save-log-title"),
          ".txt", "Log file");
      if(s.size() > 0)
      {
        if(utils::files::get_extension(s).size() == 0)
          s += ".txt";
        utils::files::copy_file(s,
            utils::get_current_user_path() + PATH_SEP + "ocvdemo-log.txt.old");
      }
    }
  }
  else
    utils::files::save_txt_file(lockfile, "OCV demo est en cours.");


  modele_global.add_listener(this);

  wnd.add(vbox);

  barre_outils.set_icon_size(Gtk::ICON_SIZE_SMALL_TOOLBAR);
  barre_outils.set_has_tooltip(false);

  vbox.pack_start(barre_outils, Gtk::PACK_SHRINK);
  barre_outils.add(b_entree);
  barre_outils.add(b_save);
  barre_outils.add(b_infos);
  barre_outils.add(b_exit);


  b_save.set_stock_id(Gtk::Stock::SAVE);
  b_exit.set_stock_id(Gtk::Stock::QUIT);
  b_infos.set_stock_id(Gtk::Stock::ABOUT);
  b_entree.set_stock_id(Gtk::Stock::PREFERENCES);


  b_save.signal_clicked().connect(sigc::mem_fun(*this, &OCVDemo::on_b_save));
  b_exit.signal_clicked().connect(sigc::mem_fun(*this, &OCVDemo::on_b_exit));
  b_infos.signal_clicked().connect(sigc::mem_fun(*this, &OCVDemo::on_b_infos));
  b_entree.signal_clicked().connect(sigc::mem_fun(*this, &OCVDemo::on_menu_entree));

  vbox.pack_start(hpaned, Gtk::PACK_EXPAND_WIDGET);

  auto schema = fs_racine->get_schema("ocv-demo");
  tdm = utils::model::Node::create_ram_node(schema, "./data/model.xml");

  auto s = tdm.to_xml();
  journal.trace("TOC = \n%s\n", s.c_str());
  auto sc2 = tdm.schema();
  s = sc2->to_string();
  journal.trace("TOC SCHEMA = \n%s\n", s.c_str());


  add_demos();

  vue_arbre.set_model(tdm);

  vue_arbre.utils::CProvider<utils::mmi::SelectionChangeEvent>::add_listener(this);

  //rtitle = utils::str::utf8_to_latin(langue.get_item("resultats"));

  //hbox.pack_start(vbox, Gtk::PACK_EXPAND_WIDGET);
  //vbox.pack_start(tree_view, Gtk::PACK_EXPAND_WIDGET);



  hpaned.pack1(vue_arbre, true, true);
  hpaned.pack2(cadre_proprietes, true, true);
  vue_arbre.set_size_request(300, 300);
  hpaned.set_border_width(5);
  hpaned.set_position(300);




  //vbox.pack_start(*(view->get_widget()), Gtk::PACK_EXPAND_WIDGET);
  wnd.show_all_children(true);
  wnd.set_size_request(730,500);

  //utils::mmi::DialogManager::setup_window(&wnd);


  sigc::slot<bool> my_slot = sigc::bind(sigc::mem_fun(*this, &OCVDemo::on_timeout), 0);
  sigc::connection conn = Glib::signal_timeout().connect(my_slot, 250); // 100 ms


  if(cmdeline.has_option("-s"))
  {
    journal.trace_major("Export tableau des fonctions supportees...");

    auto s = this->export_html(Localized::Language::LANG_FR);
    utils::files::save_txt_file("../../../site/contenu/opencv/ocvdemo/table.html", s);
    s = this->export_html(Localized::Language::LANG_EN);
    utils::files::save_txt_file("../../../site/contenu/opencv/ocvdemo/table-en.html", s);
    s = this->export_html(Localized::Language::LANG_DE);
    utils::files::save_txt_file("../../../site/contenu/opencv/ocvdemo/table-de.html", s);

    if(cmdeline.has_option("-c"))
    {
      journal.trace_major("Export des captures d'écran...");
      export_captures();
    }
  }




  barre_outil_dessin.b_raz.signal_clicked().connect(sigc::mem_fun(*this,
      &OCVDemo::on_b_masque_raz));
  barre_outil_dessin.b_gomme.signal_clicked().connect(sigc::mem_fun(*this,
      &OCVDemo::on_b_masque_gomme));
  barre_outil_dessin.b_remplissage.signal_clicked().connect(sigc::mem_fun(*this,
      &OCVDemo::on_b_masque_remplissage));

  maj_langue();



  std::vector<Gtk::TargetEntry> listTargets;
  listTargets.push_back(Gtk::TargetEntry("text/uri-list"));
  wnd.drag_dest_set(listTargets, Gtk::DEST_DEFAULT_MOTION | Gtk::DEST_DEFAULT_DROP, Gdk::ACTION_COPY | Gdk::ACTION_MOVE);
  wnd.signal_drag_data_received().connect(sigc::mem_fun(*this, &OCVDemo::on_dropped_file));

  maj_bts();

  //ImageSelecteur *is = new ImageSelecteur();
  //Gtk::Main::run(*is);

  this->img_selecteur.CProvider<ImageSelecteurRefresh>::add_listener(this);

  wnd.signal_delete_event().connect(sigc::mem_fun(*this,&OCVDemo::on_delete_event));

  Gtk::Main::run(wnd);
}

void OCVDemo::on_dropped_file(const Glib::RefPtr<Gdk::DragContext>& context, int x, int y, const Gtk::SelectionData& selection_data, guint info, guint time)
{
 if ((selection_data.get_length() >= 0) && (selection_data.get_format() == 8))
 {
   std::vector<Glib::ustring> file_list;

   file_list = selection_data.get_uris();

   if(file_list.size() > 0)
   {
     Glib::ustring path = Glib::filename_from_uri(file_list[0]);
     //do something here with the 'filename'. eg open the file for reading

     std::string s = path;//file_list[0];//path;

     journal.trace("DnD: %s.", s.c_str());

     Node new_model = Node::create_ram_node(modele_global.schema());
     new_model.copy_from(modele_global);
     new_model.set_attribute("sel", 1);
     new_model.set_attribute("file-schema/path", s);
     modele_global.copy_from(new_model);

     context->drag_finish(true, false, time);
     return;
   }
 }
 context->drag_finish(false, false, time);
}



std::string OCVDemo::export_demos(utils::model::Node &cat, Localized::Language lg)
{
  std::string s;
  auto nb_demos = cat.get_children_count("demo");
  for(auto i = 0u; i < nb_demos; i++)
  {
    auto demo = cat.get_child_at("demo", i);
    s += "<tr>";
    if(i == 0)
    {
      s += "<td rowspan=\"" + utils::str::int2str(nb_demos) + "\"><b>";
      s +=  cat.get_localized().get_value(lg);
      s += "</b></td>";
    }
    s += "<td>" + demo.get_localized().get_value(lg) + "</td>";


    //s += "<td>" + demo.get_localized().get_description(lg) + "</td>";

    auto id = demo.name();
    NodeSchema *ns = fs_racine->get_schema(id);

    if(ns != nullptr)
    {
      std::string s2 = "<br/><img src=\"imgs/" + id + ".jpg\" alt=\"" + demo.get_localized().get_value(lg) + "\"/>";
      s += "<td>" + ns->name.get_description(lg) + s2 + "</td>";
    }
    else
    {
      s += "<td></td>";
      journal.warning("Pas de schema / description pour %s.", id.c_str());
    }

    s += "</tr>";
  }
  return s;
}

std::string OCVDemo::export_html(Localized::Language lg)
{
  std::string s = "<table class=\"formtable\">\n";
  if(lg == Localized::Language::LANG_FR)
    s += std::string(R"(<tr><td colspan="2"><b>Démonstration</b></td><td><b>Description</b></td></tr>)") + "\n";
  else
    s += std::string(R"(<tr><td colspan="2"><b>Demonstration</b></td><td><b>Description</b></td></tr>)") + "\n";
  for(auto cat1: tdm.children("cat"))
  {
    for(auto cat2: cat1.children("cat"))
    {
      s += export_demos(cat2, lg);
    }
    s += export_demos(cat1, lg);
  }
  s += "</table>\n";

  return s;
}

void OCVDemo::export_captures()
{
  for(auto cat1: tdm.children("cat"))
  {
    for(auto cat2: cat1.children("cat"))
    {
      export_captures(cat2);
    }
    export_captures(cat1);
  }
}

void OCVDemo::export_captures(utils::model::Node &cat)
{
  auto nb_demos = cat.get_children_count("demo");
  for(auto i = 0u; i < nb_demos; i++)
  {

    auto demo = cat.get_child_at("demo", i);
    auto id = demo.get_attribute_as_string("name");

    journal.trace_major("Export démo [%s]...", id.c_str());

    //setup_demo(id);
    setup_demo_p(demo);

    std::string s = "../../../site/data/log/opencv/demo/list/imgs/";

    s += id + ".jpg";

    Mat A = get_current_output();


    A = mosaique.get_global_img();

    if(A.data == nullptr)
    {
      journal.warning("A.data == nullptr");
      continue;
    }

    cv::pyrDown(A, A);
    cv::pyrDown(A, A);

    journal.verbose("taille finale = %d * %d.", A.cols, A.rows);
    imwrite(s, A);
  }
}

void OCVDemo::on_b_exit()
{
  journal.trace_major("Fin normale de l'application.");
  utils::files::delete_file(lockfile);
  wnd.hide();
  exit(0);
}

bool OCVDemo::has_output()
{
  return sortie_en_cours.data != nullptr;
}

Mat OCVDemo::get_current_output()
{
  return sortie_en_cours;
}

void OCVDemo::maj_bts()
{
  int ho = has_output();
  journal.trace("maj bts: has output: %d.", ho);
  b_save.set_sensitive(ho);
}

void OCVDemo::on_b_save()
{
  journal.trace("Save.");
  if(!has_output())
  {
    journal.warning("Pas de sortie dispo.");
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
    imwrite(filename, get_current_output());
  }
}

void OCVDemo::on_b_infos()
{
  Gtk::AboutDialog ad;
  ad.set_copyright("(C) 2015 TSD Conseil / J.A.");
  Glib::RefPtr<Gdk::Pixbuf>  pix = Gdk::Pixbuf::create_from_file(utils::get_img_path() + "/logo.png");
  ad.set_logo(pix);
  //ad.set_logo_icon_name("OpenCV demonstrator");
  ad.set_name(langue.get_item("main-title") + "\n");
  ad.set_program_name(langue.get_item("main-title"));

  char buf[500];
  std::string s = langue.get_item("rev");
  sprintf(buf, s.c_str(), VMAJ, VMIN);

  ad.set_version(buf);
  ad.set_copyright("(C) 2015 TSD Conseil / J.A.");
  ad.set_license("LGPL");
  ad.set_license_type(Gtk::LICENSE_LGPL_3_0);
  ad.set_website("http://www.tsdconseil.fr");
  ad.set_website_label("http://www.tsdconseil.fr");
  std::vector<Glib::ustring> v;
  v.push_back("J.A. / TSD Conseil");
  ad.set_authors(v);

  //ad.set_comments("bla bla bla bla bla bla bla bla bla bla bla bla bla bla bla bla bla bla bla bla bla bla bla bla bla bla bla bla bla bla bla bla bla bla bla bla bla bla bla bla bla bla bla bla bla bla bla bla bla bla bla bla bla bla bla bla ");

  std::string cmts;

  cmts += langue.get_item("credit-image") + "\n";
  cmts += langue.get_item("credit-image-opencv") + "\n";
  cmts += langue.get_item("credit-image-carte") + "\n";
  cmts += langue.get_item("credit-image-autre");

  ad.set_comments(cmts);

  /*std::vector<Glib::ustring> v2;
  v2.push_back(langue.get_item("credit-image-opencv"));
  v2.push_back("J.A.");
  v2.push_back(langue.get_item("credit-image-carte"));
  ad.add_credit_section(langue.get_item("credit-image"), v2);*/
  ad.set_wrap_license(true);

  ad.set_position(Gtk::WIN_POS_CENTER);
  ad.run();
}

bool OCVDemo::on_timeout(int unused)
{
  if(entree_video)
  {
    mutex.lock();
    if(video_capture.isOpened())
    {
      journal.trace("lecture trame...");
      Mat tmp;
      video_capture >> tmp;
      if(tmp.empty())
      {
        journal.trace("Fin de vidéo : redémarrage.");
        if(video_fp.size() > 0)
        {
          video_capture.release();
          video_capture.open(video_fp);

          int nf = video_capture.get(CAP_PROP_FRAME_COUNT);
          journal.trace("*** nframes = %d.", nf);

        }
        mutex.unlock();
        return true;
      }
      I0 = tmp;
      journal.trace("recalcul, I0: %d * %d...", I0.cols, I0.rows);
      update();
    }
    mutex.unlock();
  }
  return true;
}

void OCVDemo::setup_menu()
{
  agroup = Gtk::ActionGroup::create();
  agroup->add( Gtk::Action::create("MenuMain", "_Menu") );
  agroup->add( Gtk::Action::create("Input", langue.get_item("menu-entree")),
    sigc::mem_fun(*this, &OCVDemo::on_menu_entree) );
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
      "      <menuitem action='Quit'/>"
      "    </menu>"
      "  </menubar>"
      "</ui>";

  m_refUIManager->add_ui_from_string(ui_info);
  Gtk::Widget *pMenuBar = m_refUIManager->get_widget("/MenuBar");
  vbox.pack_start(*pMenuBar, Gtk::PACK_SHRINK);
}

void OCVDemo::on_menu_entree()
{
  journal.trace("menu entree.");

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

int main(int argc, char **argv)
{
  utils::CmdeLine cmdeline(argc, argv);
  utils::init(cmdeline, "ocvdemo", "ocvdemo");
  utils::TraceManager::set_global_min_level(TraceManager::TraceTarget::TRACE_TARGET_FILE, TraceLevel::AL_VERBOSE);
  utils::TraceManager::set_global_min_level(TraceManager::TraceTarget::TRACE_TARGET_STD, TraceLevel::AL_VERBOSE);
  std::string dts = utils::get_current_date_time();
  utils::TraceManager::trace(utils::TraceLevel::AL_MAJOR, 0,
      "\nFichier journal pour l'application OCVDEMO, version %d.%02d\nDate / heure lancement application : %s\n**************************************\n**************************************\n**************************************",
      VMAJ, VMIN, dts.c_str());
  langue.load("./data/lang.xml");
  Gtk::Main kit(argc, argv);
  OCVDemo demo(cmdeline);
  return 0;
}

