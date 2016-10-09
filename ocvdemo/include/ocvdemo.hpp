/** @file ocvdemo.hpp
    @brief Classe principale pour le démonstrateur OpenCV

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

#ifndef OCV_DEMO_H
#define OCV_DEMO_H

#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <stdlib.h>
#include <stdio.h>
#include "cutil.hpp"
#include "mmi/stdview.hpp"
#include <assert.h>
#include <vector>
#include "mmi/gtkutil.hpp"
#include "hal.hpp"
#include "ocvdemo-item.hpp"
#include "tools/image-selecteur.hpp"
#include "tools/image-mosaique.hpp"
#include "tools/boutils-image.hpp"


using namespace utils;
using namespace utils::model;
using namespace utils::mmi;
using namespace cv;

/** Current software revision */
#ifndef VMAJ
# define VMAJ    1
# define VMIN    1
# define VPATCH  0
#endif

#ifndef OCV_VPATCH
# define OCV_VMAJ     3
# define OCV_VMIN     0
# define OCV_VPATCH   0
#endif

// TODO: remove this ugly hack
namespace utils
{
  extern Localized::Language current_language;
}


////////////////////////////////////////////////////////////////////////////
/** @brief Classe principale pour le démonstrateur OpenCV */
class OCVDemo:

    // Changement de sélection dans l'arbre
    private CListener<utils::mmi::SelectionChangeEvent>,

    // Changement d'un paramètre de calcul
    private CListener<ChangeEvent>,

    // Changement dans le sélecteur d'image
    private CListener<ImageSelecteurRefresh>,

    // Evenement souris
    private CListener<OCVMouseEvent>,

    // Demande de rafraichissement de la part d'une démo
    private CListener<OCVDemoItemRefresh>
{
public:
  /** Constructeur (devrait être privé !) */
  OCVDemo(utils::CmdeLine &cmdeline);

  /** Démarrage interface graphique */
  void demarre_interface();

  /** Singleton */
  static OCVDemo *get_instance();

  /** Export vers doc HTML pour site web */
  std::string export_html(Localized::Language lg);

  /** Export des images pour la doc HTML */
  void export_captures();

  FileSchema *get_fileschema(){return this->fs_racine;}

  void add_demo(OCVDemoItem *demo);

private:
  void thread_calcul();
  void thread_video();

  /** Demande le recalcul avec la dernière image d'entrée prête (dans I0) */
  void update();

  /** Réceptacle GTK pour une trame à partir du flux vidéo */
  int  on_video_image(const std::vector<cv::Mat> &tmp);
  

  void release_all_videos();
  bool are_all_video_ok();

  void export_captures(utils::model::Node &cat);
  Mat  get_current_output();
  bool has_output();
  void setup_demo(const utils::model::Node &sel);
  void maj_bts();
  void maj_langue();
  void maj_langue_systeme();
  std::string export_demos(utils::model::Node &cat, Localized::Language lg);
  void maj_entree();
  void add_demos();
  void on_menu_entree();
  void on_menu_quitter();
  void setup_menu();
  
  // Evenement souris sur la fenêtre d'affichage
  void on_event(const OCVMouseEvent &me);

  // Changement modèle (démo en cours ou schéma global)
  void on_event(const ChangeEvent &ce);

  // Sélection d'une autre démo
  void on_event(const utils::mmi::SelectionChangeEvent &e);

  // Nouvelle image sélectionnée
  void on_event(const ImageSelecteurRefresh &e);

  // Demande de rafraichissement en provenance d'une démo
  void on_event(const OCVDemoItemRefresh &e);
  
  // Drag & drop
  void on_dropped_file(const Glib::RefPtr<Gdk::DragContext>& context,
		       int x, int y, const Gtk::SelectionData& selection_data,
		       guint info, guint time);
  bool on_delete_event(GdkEventAny *event);
  void on_b_save();
  void on_b_open();
  void on_b_exit();
  void on_b_infos();

  void update_Ia();
  void compute_Ia();
  void masque_clic(int x, int y);
  void masque_raz();
  void on_b_masque_raz();
  void on_b_masque_gomme();
  void on_b_masque_remplissage();

  // To communicate from the video thread to the main GTK thread 
  utils::mmi::GtkDispatcher<std::vector<cv::Mat>> gtk_dispatcher;

  // Lock for video stream access.
  utils::hal::Mutex mutex_video;

  // Lock for update current output.
  utils::hal::Mutex mutex_update;

  std::string lockfile; // Chemin du fichier de lock
  // Obsolete ?
  utils::hal::Mutex mutex; // Verrou pour les calculs VS video
  // Dialogue de démarrage
  utils::mmi::NodeDialog *entree_dial;
  // Pour le menu
  Glib::RefPtr<Gtk::ActionGroup> agroup;
  // Fenêtre principale
  Gtk::Window wnd;
  // Conteneur principal
  Gtk::VBox vbox;
  // Séparateur paneaux de gauche et droite
  Gtk::HPaned hpaned;
  // Modèles
  utils::model::Node modele, modele_global, tdm;
  utils::model::Node modele_demo; // Modèle dans la TOC
  // Configuration de la démo en cours
  utils::mmi::NodeView *rp;
  // Logs
  utils::Logable journal;
  // Arbre de sélection à gauche
  TreeManager vue_arbre;
  // Cadre autour de la config
  JFrame cadre_proprietes;
  // Schéma racine
  FileSchema *fs_racine;
  // Verrou
  bool lock;
  // Titre principal
  std::string titre_principal;
  // Affichage des résultats
  ImageMosaique mosaique;
  // Liste des démos supportées
  std::vector<OCVDemoItem *> items;
  // Démo en cours
  OCVDemoItem *demo_en_cours;
  // I0 : image originale
  // I1 : telle que modifiée par le processus
  // Ia : avec rectangle utilisateur
  cv::Mat I0, I1, I, Ia;
  int etat_souris;
  // Région d'intérêt
  cv::Rect rect_rdi;
  cv::Point rdi0, rdi1;
  // Vidéo en cours ?
  bool entree_video;
  // Capture vidéo
  std::vector<cv::VideoCapture> video_captures;
  // Fichier vidéo en cours
  std::string video_fp;
  // Caméra vidéo en cours
  int video_idx;
  // Barre d'outils
  Gtk::Toolbar barre_outils;
  Gtk::ToolButton b_entree, b_open, b_infos, b_exit, b_save;
  // Chemin du fichier de configuration
  std::string chemin_fichier_config;
  // Barre d'outils
  MasqueBOutils barre_outil_dessin;
  int outil_dessin_en_cours;
  bool sortie_en_cours;
  ImageSelecteur img_selecteur;
  bool video_en_cours, video_stop;
  bool first_processing;

  // Objets envoyés dans la fifo de calcul
  struct ODEvent
  {
    enum
    {
      // Requiert la fin du thread (fin de l'application)
      FIN,
      // Calcul sur une image
      CALCUL
    } type;

    OCVDemoItem *demo;
    utils::model::Node modele;
  };
  

  // FIFO pour envoyer les commandes au thread de calcul
  utils::hal::Fifo<ODEvent> event_fifo;

  // Fin du dernier calcul demandé
  utils::hal::Signal signal_calcul_termine;
  int calcul_status;
  
  // Confirmation de la terminaison du thread de calcul
  utils::hal::Signal signal_thread_calcul_fin;

  utils::hal::Signal signal_video_demarre,
    signal_image_video_traitee,
    signal_video_suspendue;

  // Levé quand au moins une trame vidéo à fini d'être traitée
  utils::hal::Signal signal_une_trame_traitee;

  bool ignore_refresh;
};

#endif
