#ifndef VUE_COLLECTION_H
#define VUE_COLLECTION_H

#include <opencv2/imgproc.hpp>
#include "vue-image.hpp"
#include "mmi/stdview.hpp"
#include "vue-image.hpp"

namespace ocvext{

/*struct CollectionItem
{
  std::string nom;
  std::string chemin;
};

struct Collection
{
  std::vector<CollectionItem> items;
};*/





class VueCollection
{
public:
  VueCollection();
  //void affiche(const Collection &col);
  void affiche(utils::model::Node &modele);
  void ferme();

  Gtk::Window fenetre;

private:

  void gere_b_ouvrir_image();
  int gere_change_sel(const utils::mmi::SelectionChangeEvent &sce);

  Gtk::Widget *vue_en_cours;
  Gtk::VBox vbox, vbox_princ;
  Gtk::TextView texte;
  Gtk::HPaned hpaned;
  utils::model::Node modele, modele_arbre;
  VueImage vue;
  utils::mmi::TreeManager arbre;
  Gtk::Toolbar barre_outil;
  Gtk::ToolButton b_ouvrir_image;
};

}

#endif
