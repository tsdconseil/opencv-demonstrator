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



// Conversion latin -> utf8:
// iconv -f latin1 -t utf-8 data/lang8859.xml > data/lang.xml

using utils::model::Localized;

static OCVDemo *instance = nullptr;


static void prepare_image(cv::Mat &I)
{
  if(I.channels() == 1)
    cv::cvtColor(I, I, CV_GRAY2BGR);

  if(I.depth() == CV_32F)
    I.convertTo(I, CV_8UC3);
}

void OCVDemo::thread_calcul()
{
  for(;;)
  {
    auto evt = event_fifo.pop();

    switch(evt.type)
    {
      ////////////////////////////	
      // Fin de l'application
      ////////////////////////////	
      case ODEvent::FIN:
	journal.trace_major("Fin du thread de calcul.");
	signal_thread_calcul_fin.raise();
	return;
      ////////////////////////////	
      // Calcul sur une image
      ////////////////////////////
      case ODEvent::CALCUL:
	try
	{
	  calcul_status = evt.demo->calcul(evt.modele, evt.img);
	}
	catch(...)
        {
	  // TODO -> transmettre msg erreur
	  //utils::mmi::dialogs::show_error(langue.get_item("err-opencv-title"),
	  //				  langue.get_item("err-opencv-msg"), langue.get_item("err-opencv-msg2"));
	  break;
	}
	signal_calcul_termine.raise();
	break;
      default:
	journal.anomaly("%s: invalid event.", __func__);
	return;
    }
  }
}

void OCVDemo::thread_video()
{
  Mat tmp;
  for(;;)
  {
    if(video_stop)
    {
      video_stop = false;
      video_en_cours = false;
      signal_video_suspendue.raise();
      signal_video_demarre.wait();
    }
    
    while(!entree_video || (!video_capture.isOpened()))
    {
      video_en_cours = false;
      signal_video_suspendue.raise();
      signal_video_demarre.wait();
    }
    video_en_cours = true;

    mutex_video.lock();
    journal.verbose("[tvideo] lecture trame...");
    video_capture >> tmp;
    journal.verbose("[tvideo] ok.");
    if(tmp.empty())
    {
      journal.trace("[tvideo] Fin de vidéo : redémarrage.");
      if(video_fp.size() > 0)
      {
	video_capture.release();
	video_capture.open(video_fp);
      }
      mutex_video.unlock();
      continue;
    }
    mutex_video.unlock();

    // Prévention deadlock
    if(video_stop)
    {
      video_stop = false;
      video_en_cours = false;
      signal_video_suspendue.raise();
      signal_video_demarre.wait();
      continue;
    }
      
    gtk_dispatcher.on_event(tmp);
    signal_image_video_traitee.wait();
  }
}

int OCVDemo::on_video_image(const cv::Mat &tmp)
{
  // Récupération d'une trame vidéo (mais ici on est dans le thread GTK)
  I0 = tmp;
  journal.trace("recalcul, I0: %d * %d...", I0.cols, I0.rows);
  update();
  signal_image_video_traitee.raise();
  return 0;
}


void OCVDemo::on_b_masque_raz()
{
  masque_raz();
  update_Ia();
  update();
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
        demo_en_cours->params.masque.at<unsigned char>(y,x) = 255;
      }
    }
  }
  else
  {
    // Floodfill
    cv::Mat mymask = Mat::zeros(Ia.rows+2,Ia.cols+2,CV_8U);
    cv::floodFill(Ia, mymask, Point(x0,y0), Scalar(255,255,255));
    Mat roi(mymask, Rect(1,1,Ia.cols,Ia.rows));
    this->demo_en_cours->params.masque |= roi;
    update();
  }
}

// Calcul d'une sortie à partir de l'image I0
// avec mise à jour de la mosaique de sortie.
void OCVDemo::update()
{
  if(demo_en_cours == nullptr)
    return;

  // Première fois que la démo est appelée avec des entrées (I0) valides ?
  if(first_processing)
  {
    first_processing = false;
    if(demo_en_cours->props.requiert_masque)
      demo_en_cours->params.masque = cv::Mat::zeros(I0.size(), CV_8U);
  }
  
  journal.verbose("Acquisition mutex_update...");
  mutex_update.lock();
  journal.verbose("mutex_update ok.");
  
  this->sortie_en_cours = Mat();


  if(modele_demo.is_nullptr())
  {
    mutex_update.unlock();
    return;
  }

  I1 = I0.clone();
  auto s = modele.to_xml();

  journal.trace("Calcul [%s], img: %d*%d, %d chn, model =\n%s",
		demo_en_cours->props.id.c_str(),
		I1.cols, I1.rows, I1.channels(),
		s.c_str());

    
  if(demo_en_cours->props.requiert_mosaique)
    this->img_selecteur.get_list(demo_en_cours->params.mosaique);


  // Appel au thread de calcul
  ODEvent evt;
  evt.type = ODEvent::CALCUL;
  evt.img  = I1;
  evt.demo = demo_en_cours;
  evt.modele = modele;
  event_fifo.push(evt);
  // Attente fin de calcul
  signal_calcul_termine.wait();

  if(calcul_status)
  {
    journal.warning("ocvdemo: Echec calcul.");
    auto s = demo_en_cours->sortie.errmsg;
    if(langue.has_item(s))
      s = langue.get_item(s);
    utils::mmi::dialogs::show_warning("Erreur de traitement",
				      langue.get_item("echec-calcul"), s);
    mutex_update.unlock();
    return;
  }
  else
  {
    journal.trace("Calcul [%s] ok.", demo_en_cours->props.id.c_str());
    
    if(demo_en_cours->sortie.vrai_sortie.data != nullptr)
      sortie_en_cours = demo_en_cours->sortie.vrai_sortie;
    else if(demo_en_cours->sortie.nb_sorties > 0)
      sortie_en_cours = demo_en_cours->sortie.O[demo_en_cours->sortie.nb_sorties - 1];
    else
      sortie_en_cours = I1;
  }

  std::vector<cv::Mat> lst;
  compute_Ia();
  prepare_image(Ia);
  lst.push_back(Ia);
  unsigned int img_count = modele_demo.get_attribute_as_int("img-count");

  if(demo_en_cours->sortie.nb_sorties >= 0)
    img_count = 1 + demo_en_cours->sortie.nb_sorties;

  if((img_count > 1) && (demo_en_cours->sortie.O[0].data == nullptr))
  {
    img_count = 1;
    journal.warning("Img count = 2, et image O non initialisée.");
  }

  std::vector<std::string> titres;


  for(auto j = 0u; j < img_count; j++)
  {
    if(j > 0)
    {
      prepare_image(demo_en_cours->sortie.O[j-1]);
      lst.push_back(demo_en_cours->sortie.O[j-1]);
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
    if((j < 4) && (demo_en_cours->sortie.outname[j].size() > 0))
    {
      s = demo_en_cours->sortie.outname[j];
      if(langue.has_item(s))
	s = langue.get_item(s);
    }
    titres.push_back(utils::str::utf8_to_latin(s));;
  }
  
  mosaique.show_multiple_images(titre_principal, lst, titres);

  maj_bts();
  journal.verbose("Liberation mutex_update...");
  mutex_update.unlock();

  cv::waitKey(10); // Encore nécessaire à cause de la fenêtre OpenCV
}




///////////////////////////////////////////////////////////////
// Détection d'un changement sur le modèle
//  - global
//    ==> maj_entree
//  - ou de la démo en cours
//    ==> update_demo, update 
///////////////////////////////////////////////////////////////
void OCVDemo::on_event(const ChangeEvent &ce)
{
  journal.verbose("change-event: %d / %s",
		  (int) ce.type, ce.path[0].name.c_str());

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




///////////////////////////////////
// Mise à jour du flux / image d'entrée
// Requiert : demo_en_cours bien défini
///////////////////////////////////
void OCVDemo::maj_entree()
{
  if(demo_en_cours == nullptr)
    return;

  first_processing = true;
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

  journal.verbose("lock...");
  mutex_video.lock();
  journal.verbose("lock ok.");
  
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
      mutex_video.unlock();
      return;
    }
    video_fp    = "";
    video_idx   = idx;
    entree_video = true;
    video_capture >> I0; // TODO: à supprimer
    mutex_video.unlock();
    signal_video_demarre.raise();
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
      mutex_video.unlock();
      return;
    }
    video_fp = fp;
    entree_video = true;
    video_capture >> I0; // TODO: à supprimer
    mutex_video.unlock();
    signal_video_demarre.raise();
  }
  else
  {
    mutex_video.unlock();
    journal.trace("Ouverture fichier image [%s]...", fp.c_str());
    I0 = cv::imread(fp.c_str());
    if(I0.data == nullptr)
    {
      utils::mmi::dialogs::show_error("Erreur",
          "Impossible de charger l'image", std::string("Chemin : ") + fp);
      destroyWindow(titre_principal);
      mosaique.callback_init_ok = false;
    }
    update();
  }
  maj_bts();
}


void OCVDemo::setup_demo(const Node &sel)
{
  auto id = sel.get_attribute_as_string("name");
  auto s = sel.get_localized_name();
  journal.trace_major("Selection changed: %s (%s).", id.c_str(), s.c_str());

  while(video_en_cours)
  {
    signal_video_demarre.clear();
    signal_video_suspendue.clear();
    video_stop = true;
    journal.trace("interruption flux video...");
    signal_video_suspendue.wait();
    journal.trace("Flux video interrompu.");
  }
  
  mutex_update.lock();
  journal.verbose("Debut setup...");

  cadre_proprietes.remove();
  if (rp != nullptr)
  {
    delete rp;
    rp = nullptr;
  }

  modele_demo = sel;

  if((sel.schema()->name.get_id() != "demo")
     || (fs_racine->get_schema(id) == nullptr))
  {
    destroyWindow(titre_principal);
    mosaique.callback_init_ok = false;
    mutex_update.unlock();
    maj_bts();
    return;
  }

  auto schema = fs_racine->get_schema(id);
  modele = utils::model::Node::create_ram_node(schema);
      
  modele.add_listener(this);
  {
    utils::mmi::NodeViewConfiguration vconfig;
    vconfig.show_desc = true;
    vconfig.show_main_desc = true;
    rp = new utils::mmi::NodeView(&wnd, modele, vconfig);
    cadre_proprietes.set_label(s);
    cadre_proprietes.add(*(rp->get_widget()));
    cadre_proprietes.show();
    cadre_proprietes.show_all_children(true);
  }
  journal.verbose("setup demo...");

  ///////////////////////////////////////////
  // - Localise la demo parmi les démos enregistrées
  // - Initialise les zones d'intérêt
  // - Appel maj_entree()
  // - Affiche la barre d'outil si nécessaire
  ///////////////////////////////////////////
      
  journal.verbose("update_demo()...");
  namedWindow(titre_principal, CV_WINDOW_NORMAL);

  demo_en_cours = nullptr;
  for(auto demo: items)
  {
    if(demo->props.id == id)
    {
      demo_en_cours = demo;
      
      
      rdi0.x = demo->params.roi.x;
      rdi0.y = demo->params.roi.y;
      rdi1.x = demo->params.roi.x + demo->params.roi.width;
      rdi1.y = demo->params.roi.y + demo->params.roi.height;
	  
      demo->params.modele = modele;
      demo->setup_model(modele);
	  
      if(demo->configure_ui())
	break;
	  
      if(demo_en_cours->props.requiert_masque)
	barre_outil_dessin.montre();
      else
	barre_outil_dessin.cache();

      if(demo_en_cours->props.requiert_mosaique)
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
  }
  if(demo_en_cours == nullptr)
    journal.warning("Demo not found: %s", id.c_str());

  
  this->wnd.present();

  if((demo_en_cours != nullptr) && demo_en_cours->props.requiert_mosaique)
    img_selecteur.present();

  if(demo_en_cours && (demo_en_cours->props.requiert_masque))
    barre_outil_dessin.montre();

  maj_bts();
  mutex_update.unlock();
  maj_entree();
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

  setup_demo(e.new_selection);
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
  if((demo_en_cours != nullptr) && (demo_en_cours->props.requiert_roi))
  {
    Ia = I1.clone();
    cv::rectangle(Ia, rdi0, rdi1, Scalar(0,255,0), 3);
  }
  else if((demo_en_cours != nullptr) && (demo_en_cours->props.requiert_masque))
  {
    if((Ia.data == nullptr) || (Ia.size() != I1.size()))
      Ia = I1.clone();
  }
  else
  {
    Ia = I1;
  }
}

void OCVDemo::masque_raz()
{
  Ia = I0.clone();
  demo_en_cours->params.masque = cv::Mat::zeros(I0.size(), CV_8U);
}

void OCVDemo::update_Ia()
{
  mosaique.update_image(0, Ia);
}

OCVDemo *OCVDemo::get_instance()
{
  return instance;
}


OCVDemo::OCVDemo(utils::CmdeLine &cmdeline)
{
  journal.setup("", "ocvdemo");
  utils::current_language = Localized::LANG_EN;
  langue.current_language = "en";
  video_en_cours = false;
  video_stop = false;
  outil_dessin_en_cours = 0;
  entree_video = false;
  demo_en_cours = nullptr;
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

  hpaned.pack1(vue_arbre, true, true);
  hpaned.pack2(cadre_proprietes, true, true);
  vue_arbre.set_size_request(300, 300);
  hpaned.set_border_width(5);
  hpaned.set_position(300);

  wnd.show_all_children(true);
  wnd.set_size_request(730,500);

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

  this->img_selecteur.CProvider<ImageSelecteurRefresh>::add_listener(this);
  wnd.signal_delete_event().connect(sigc::mem_fun(*this,&OCVDemo::on_delete_event));

  utils::hal::thread_start(this, &OCVDemo::thread_calcul);
  utils::hal::thread_start(this, &OCVDemo::thread_video);

  //gtk_dispatcher = new utils::mmi::GtkDispatcher<cv::Mat>;
  gtk_dispatcher.add_listener(this, &OCVDemo::on_video_image);

  Gtk::Main::run(wnd);
}


bool OCVDemo::has_output()
{
  return sortie_en_cours.data != nullptr;
}

Mat OCVDemo::get_current_output()
{
  return sortie_en_cours;
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

