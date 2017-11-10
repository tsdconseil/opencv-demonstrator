#ifndef CALIB_H
#define CALIB_H

#include "ocvext.hpp"
#include "vue-image.hpp"
#include "images-selection.hpp"

namespace ocvext {

struct EtalonnageCameraConfig
{
  enum Type
  {
    DAMIER = 0,
    CIRC   = 1
  };
  Type type;
  int nlignes, ncolonnes;
  float largeur_case_cm; // largeur d'une case, en cm
};

extern int etalonner_camera(const std::vector<cv::Mat> &imgs,
    const EtalonnageCameraConfig &config,
    cv::Mat &matrice_camera, cv::Mat &prm_dist);


class DialogueCalibration
{
public:

  DialogueCalibration();

  void affiche();

  Gtk::Window dialogue;

private:
  void thread_video();
  void gere_b_cal();
  void gere_b_fermer();
  void gere_b_photo();

  ocvext::VueImage vue_video;
  ocvext::ImagesSelection img_list;
  Gtk::HButtonBox hbox;
  Gtk::VBox vbox;
  Gtk::Button b_fermer, b_photo, b_cal;
  Gtk::HPaned paned;

  utils::model::Node prm;
  utils::mmi::NodeView vue_prm;
  utils::model::FileSchema fs;

  bool prende_photo;
  std::vector<cv::Mat> photos;
  struct EvtGtk
  {

  };
  utils::mmi::GtkDispatcher<EvtGtk> dispatcheur;
  int gere_evt_gtk(const EvtGtk &eg);
};





}

#endif

