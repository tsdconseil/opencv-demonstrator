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




OCVDemoItem::OCVDemoItem()
{
  output.nout = 1;
  props.requiert_roi = false;
  props.requiert_masque = false;

  // Default demo item requires only 1 input image / video
  props.input_min = 1;
  props.input_max = 1;
  journal.setup("ocvdemo-item","");
}

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
        calcul_status = evt.demo->proceed(evt.demo->input, evt.demo->output);
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

bool OCVDemo::are_all_video_ok()
{
  bool res = true;
  for(auto &vc: video_captures)
    if(!vc.isOpened())
      res = false;
  return res;
}

void OCVDemo::thread_video()
{
  std::vector<cv::Mat> tmp;
  for(;;)
  {
    debut:
    if(video_stop)
    {
      video_stop = false;
      video_en_cours = false;
      signal_video_suspendue.raise();
      signal_video_demarre.wait();
    }

    while(!entree_video || !are_all_video_ok() || (video_captures.size() == 0))
    {
      video_en_cours = false;
      signal_video_suspendue.raise();
      signal_video_demarre.wait();
    }
    video_en_cours = true;

    tmp.resize(video_captures.size());

    mutex_video.lock();

    for(auto i = 0u; i < video_captures.size(); i++)
    {
      journal.verbose("[tvideo] lecture trame video[%d]...", i);
      video_captures[i] >> tmp[i];
      journal.verbose("[tvideo] ok.");

      if(tmp[i].empty())
      {
        journal.trace_major("[tvideo] Fin de vidéo : redémarrage.");
        if(video_fp.size() > 0)
        {
          // TODO: redémarrage de toutes les vidéos en même temps
          video_captures[i].release();
          video_captures[i].open(video_fp);
        }
        mutex_video.unlock();
        goto debut; // Hack to fix
      }
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

    journal.verbose("gtk dispatcher...");
    gtk_dispatcher.on_event(tmp);

    // Pour éviter le deadlock, attends soit que :
    //  - fin de traitement
    //  - ou demande d'arrêt de la vidéo
    for(;;)
    {
      if(signal_image_video_traitee.wait(10) == 0)
        break;
      if(video_stop)
        break;
    }
  }
}

int OCVDemo::on_video_image(const std::vector<cv::Mat> &tmp)
{
  // Récupération d'une trame vidéo (mais ici on est dans le thread GTK)
  // (Recovery of a video frame (but here we are in the GTK thread))
  journal.verbose("on_video_image...");
  if(demo_en_cours != nullptr)
  {
    I0 = tmp[0];
    auto &v = demo_en_cours->input.images;
    for(auto i = 0u; i < tmp.size(); i++)
    {
      if(v.size() <= i)
        v.push_back(tmp[i]);
      else
        v[i] = tmp[i];
    }
    update();
  }
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
        demo_en_cours->input.mask.at<unsigned char>(y,x) = 255;
      }
    }
  }
  else
  {
    // Floodfill
    cv::Mat mymask = Mat::zeros(Ia.rows+2,Ia.cols+2,CV_8U);
    cv::floodFill(Ia, mymask, Point(x0,y0), Scalar(255,255,255));
    Mat roi(mymask, Rect(1,1,Ia.cols,Ia.rows));
    this->demo_en_cours->input.mask |= roi;
    update();
  }
}

// Calcul d'une sortie à partir de l'image I0
// avec mise à jour de la mosaique de sortie.
// (Calculating an output from the image I0 with updating the output mosaic)
void OCVDemo::update()
{
  if(demo_en_cours == nullptr)
    return;

  // Première fois que la démo est appelée avec des entrées (I0) valides ?
  // (First time the demo is called with the inputs (I0) valid?)
  if(first_processing)
  {
    first_processing = false;
    if(demo_en_cours->props.requiert_masque)
      demo_en_cours->input.mask = cv::Mat::zeros(I0.size(), CV_8U);
  }

  journal.verbose("Acquisition mutex_update...");
  mutex_update.lock();
  journal.verbose("mutex_update ok.");

  sortie_en_cours = false;

  if(modele_demo.is_nullptr())
  {
    mutex_update.unlock();
    return;
  }

  I1 = I0.clone();
  auto s = modele.to_xml();

  journal.trace("Calcul [%s], img: %d*%d, %d chn, model =\n%s",
      demo_en_cours->props.id.c_str(),
      I0.cols, I0.rows, I0.channels(),
      s.c_str());

  // RAZ des images de sorties
  for(auto i = 0u; i < DEMO_MAX_IMG_OUT; i++)
    demo_en_cours->output.images[i] = cv::Mat();

  // Appel au thread de calcul
  ODEvent evt;
  evt.type = ODEvent::CALCUL;
  evt.demo = demo_en_cours;
  evt.modele = modele;
  event_fifo.push(evt);
  // Attente fin de calcul
  signal_calcul_termine.wait();

  if(calcul_status)
  {
    journal.warning("ocvdemo: Echec calcul.");
    auto s = demo_en_cours->output.errmsg;
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
    
    if(demo_en_cours->output.nout > 0)
      sortie_en_cours = true;
    else
      sortie_en_cours = false;
  }

  std::vector<cv::Mat> lst;
  compute_Ia();
  prepare_image(Ia);
  lst.push_back(Ia);
  unsigned int img_count = demo_en_cours->output.nout;

  if(img_count && (demo_en_cours->output.images[img_count - 1 ].data == nullptr))
  {
    journal.warning("Img count = %d, et image de sortie non initialisée.", img_count);
    img_count = 1;
  }
  else if(img_count == 0)
  {
    if(Ia.data != nullptr)
      img_count = 1;
    else
      lst.clear();
  }

  std::vector<std::string> titres;
  for(auto j = 0u; j < img_count; j++)
  {
    if(j > 0)
    {
      prepare_image(demo_en_cours->output.images[j]);
      lst.push_back(demo_en_cours->output.images[j]);
    }

    char buf[50];
    sprintf(buf, "o[id=%d]", j);
    std::string s = "";
    if(modele_demo.has_child(std::string(buf)))
    {
      auto n = modele_demo.get_child(std::string(buf));
      s = n.get_localized().get_localized();
    }
    if((j < 4) && (demo_en_cours->output.names[j].size() > 0))
    {
      s = demo_en_cours->output.names[j];
      if(langue.has_item(s))
        s = langue.get_item(s);
    }
//  titres.push_back(utils::str::utf8_to_latin(s)); // à passer en utf-8 dès que fenêtre GTK fait
    titres.push_back(s); 

  }

  mosaique.show_multiple_images(titre_principal, lst, titres);

  maj_bts();
  journal.verbose("Liberation mutex_update...");
  mutex_update.unlock();

  signal_une_trame_traitee.raise();
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

void OCVDemo::add_demo(OCVDemoItem *demo)
{
  items.push_back(demo);
  demo->add_listener(this);
}

void OCVDemo::on_event(const ImageSelecteurRefresh &e)
{
  if(!ignore_refresh)
    maj_entree();
}

void OCVDemo::on_event(const OCVDemoItemRefresh &e)
{
  update();
}

void OCVDemo::release_all_videos()
{
  for(auto &vc: video_captures)
    if(vc.isOpened())
      vc.release();
  video_captures.clear();
}

///////////////////////////////////
// Mise à jour du flux / image d'entrée (update flux / input image)
// Requiert : demo_en_cours bien défini (requires: demo_en_cours clear)
///////////////////////////////////
void OCVDemo::maj_entree()
{
  if(demo_en_cours == nullptr)
    return;

  first_processing = true;
  entree_video = false;

  journal.verbose("lock...");
  mutex_video.lock();
  journal.verbose("lock ok.");

  release_all_videos();

  // Idée :
  //  - transformer video_capture en un tableau
  //  - mais on peut avoir un fichier vidéo et une image en même temps !
  //
  // si mode vidéo.

  std::vector<ImageSelecteur::SpecEntree> entrees;
  img_selecteur.get_entrees(entrees);

  unsigned int vid = 0; // Index in video list

  demo_en_cours->input.images.resize(entrees.size());

  for(auto i = 0u; i < entrees.size(); i++)
  {
    auto &se = entrees[i];

    if(se.is_video())
    {
      journal.trace("Ouverture fichier video [%s]...", se.chemin.c_str());
      int res;
      video_captures.push_back(cv::VideoCapture());
      if(se.type == ImageSelecteur::SpecEntree::TYPE_WEBCAM)
        res = video_captures[vid].open(se.id_webcam);
      else
        res = video_captures[vid].open(se.chemin);
      journal.verbose("Effectué.");
      if(!res)
      {
        utils::mmi::dialogs::show_error(langue.get_item("ech-vid-tit"),
            langue.get_item("ech-vid-sd"),
            langue.get_item("ech-vid-d") + "\n" + se.chemin);
        mutex_video.unlock();
        return;
      }
      video_fp = se.chemin; // TODO : vecteur
      entree_video = true;
      video_captures[vid] >> I0; // TODO: à supprimer
      vid++;
    }
    else
    {
      demo_en_cours->input.images[i] = se.img;
      I0 = se.img.clone();
      if(I0.data == nullptr)
      {
        utils::mmi::dialogs::show_error("Erreur",
            "Impossible de charger l'image", "");
        destroyWindow(titre_principal);
        mosaique.callback_init_ok = false;
      }
    }
  }

  mutex_video.unlock();
  if(video_captures.size() > 0)
    signal_video_demarre.raise();
  else
    update();

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
    img_selecteur.hide();
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
  //en:
  // - Locates the demo from the demos recorded
  // - Initializes areas of interest
  // - Call maj_entree() eg. major_entry() 
  // - Displays the toolbar if necessary
  //////////////////////////////////////////

  journal.verbose("update_demo()...");
  namedWindow(titre_principal, CV_WINDOW_NORMAL);
  
  //en: current demo
  demo_en_cours = nullptr;
  for(auto demo: items)
  {
    if(demo->props.id == id)
    {
      demo_en_cours = demo;


      rdi0.x = demo->input.roi.x;
      rdi0.y = demo->input.roi.y;
      rdi1.x = demo->input.roi.x + demo->input.roi.width;
      rdi1.y = demo->input.roi.y + demo->input.roi.height;

      demo->input.model = modele;
 
      /* code mort
      * dead code
      demo->setup_model(modele);
      */


      // Réinitialisation des images de sorties (au cas où elles pointaient vers les images d'entrées)
      // Resetting outputs images ( in case they pointed to the pictures of inputs )
      //for(auto i = 0u; i < DEMO_MAX_IMG_OUT; i++)
      //  demo->

      /* code mort
       * dead code 
      if(demo->configure_ui())
        break;
      */

      if(demo_en_cours->props.requiert_masque)
        barre_outil_dessin.montre();
      else
        barre_outil_dessin.cache();

      img_selecteur.show();
      //img_selecteur.present();
      ignore_refresh = true;
      img_selecteur.raz();
      img_selecteur.nmin = demo->props.input_min;
      img_selecteur.nmax = demo->props.input_max;

      for(const auto &img: modele_demo.children("img"))
        img_selecteur.ajoute_fichier(img.get_attribute_as_string("path"));

      int nmissing = demo->props.input_min - img_selecteur.get_nb_images();
      journal.verbose("nmissing is %d ", nmissing );
      if(nmissing > 0)
      {
        for(auto i = 0; i < nmissing; i++)
          img_selecteur.ajoute_fichier(utils::get_fixed_data_path() + "/img/lena.jpg");
      }
      ignore_refresh = false;
    }
  }
  if(demo_en_cours == nullptr)
  {
    journal.warning("Demo non trouvee : %s", id.c_str());
    std::string s = "";
    for(auto demo: items)
      s += demo->props.id + " ";
    journal.trace("Liste des demos disponibles :\n%s", s.c_str());
  }


  this->wnd.present();

  //if((demo_en_cours != nullptr) )//&& demo_en_cours->props.requiert_mosaique)
  //img_selecteur.present();

  if(demo_en_cours && (demo_en_cours->props.requiert_masque))
    barre_outil_dessin.montre();

  maj_bts();
  mutex_update.unlock();

  journal.verbose("** SETUP SCHEMA TERMINE, MAJ ENTREE...");
  maj_entree();
}




void OCVDemo::on_event(const utils::mmi::SelectionChangeEvent &e)
{
  if(lock)
    return;

  sortie_en_cours = false;

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
    Ia = I0.clone();
    cv::rectangle(Ia, rdi0, rdi1, Scalar(0,255,0), 3);
  }
  else if((demo_en_cours != nullptr) && (demo_en_cours->props.requiert_masque))
  {
    if((Ia.data == nullptr) || (Ia.size() != I1.size()))
      Ia = demo_en_cours->output.images[0];
  }
  else if(demo_en_cours != nullptr)
    Ia = demo_en_cours->output.images[0];
  else
    Ia = I0;
}

void OCVDemo::masque_raz()
{
  Ia = I0.clone();
  demo_en_cours->input.mask = cv::Mat::zeros(I0.size(), CV_8U);
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
  journal.trace("OCVDemo::OCVDemo() (constructeur).");
  utils::current_language = Localized::LANG_EN;
  langue.current_language = "en";
  video_en_cours = false;
  video_stop = false;
  outil_dessin_en_cours = 0;
  entree_video = false;
  ignore_refresh = false;
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
  //barre_outils.add(b_open);
  barre_outils.add(b_infos);
  barre_outils.add(b_exit);

  mosaique.add_listener(this);

  b_save.set_stock_id(Gtk::Stock::SAVE);
  b_open.set_stock_id(Gtk::Stock::OPEN);
  b_exit.set_stock_id(Gtk::Stock::QUIT);
  b_infos.set_stock_id(Gtk::Stock::ABOUT);
  b_entree.set_stock_id(Gtk::Stock::PREFERENCES);


  b_save.signal_clicked().connect(sigc::mem_fun(*this, &OCVDemo::on_b_save));
  b_open.signal_clicked().connect(sigc::mem_fun(*this, &OCVDemo::on_b_open));
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
    //these are used to generate the web site
    auto s = this->export_html(Localized::Language::LANG_FR);
    utils::files::save_txt_file("../../../site/contenu/opencv/ocvdemo/table.html", s);
    s = this->export_html(Localized::Language::LANG_EN);
    utils::files::save_txt_file("../../../site/contenu/opencv/ocvdemo/table-en.html", s);
    s = this->export_html(Localized::Language::LANG_DE);
    utils::files::save_txt_file("../../../site/contenu/opencv/ocvdemo/table-de.html", s);
    s = this->export_html(Localized::Language::LANG_RU);
    utils::files::save_txt_file("../../../site/contenu/opencv/ocvdemo/table-ru.html", s);

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

  gtk_dispatcher.add_listener(this, &OCVDemo::on_video_image);

  // Moved "-c" from above to here so it runs after thread_calcul has been started.
  if(cmdeline.has_option("-c"))
  {
    journal.trace_major("Export des captures d'écran...");
    export_captures();
    journal.trace_major("Toutes les captures ont été exportées.");
    on_b_exit();
  }
}

void OCVDemo::demarre_interface()
{
  Gtk::Main::run(wnd);
}


bool OCVDemo::has_output()
{
  return sortie_en_cours;
}

Mat OCVDemo::get_current_output()
{
  return demo_en_cours->output.images[0];
}



