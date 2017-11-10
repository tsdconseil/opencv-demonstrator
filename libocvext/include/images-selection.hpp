/** @file image-selecteur.hpp

    Copyright 2015 J.A. / http://www.tsdconseil.fr

    Project web page: http://www.tsdconseil.fr/log/opencv/demo/index-en.html

    This file is part of libocvext.

    libocvext is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    libocvext is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with OCVDemo.  If not, see <http://www.gnu.org/licenses/>.
 **/

#ifndef IMAGE_SELECTEUR_HPP
#define IMAGE_SELECTEUR_HPP

#include "mmi/gtkutil.hpp"
#include <gdkmm/pixbuf.h>
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "cutil.hpp"
#include "mmi/stdview.hpp"
#include "vue-image.hpp"

namespace ocvext {




struct ImagesSelectionRefresh{};

/** @brief Sélecteur d'image */
class ImagesSelection:
    public utils::CProvider<ImagesSelectionRefresh>
{
public:
  ImagesSelection();
  void maj_langue();
  void maj_actif();

  void ajoute_photo(const cv::Mat &I);
  void ajoute_fichier(std::string s);

  void raz();

  void get_list(std::vector<cv::Mat> &list);
  unsigned int get_nb_images() const;

  bool has_video();
  void get_video_list(std::vector<std::string> &list);


  struct SpecEntree
  {
    enum
    {
      TYPE_IMG = 0,
      TYPE_VIDEO,
      TYPE_WEBCAM,
      TYPE_IMG_RAM
    } type;
    bool is_video(){return (type == TYPE_WEBCAM) || (type == TYPE_VIDEO);}
    std::string chemin;
    unsigned int id_webcam;
    cv::Mat img;
  };

  void get_entrees(std::vector<SpecEntree> &liste);

  int nmin, nmax;

  Gtk::Window fenetre;

private:

  utils::model::FileSchema fs;

  void set_fichier(int idx, std::string s);
  std::string media_open_dialog(utils::model::Node mod);

  void on_size_change(Gtk::Allocation &alloc);
  bool on_b_pressed(GdkEventButton *event);
  bool on_b_released(GdkEventButton *event);
  bool on_k_released(GdkEventKey *event);

  void on_b_open();
  void on_b_add();
  void on_b_del();
  void on_b_del_tout();
  void on_b_maj();
  void on_dropped_file(const Glib::RefPtr<Gdk::DragContext>& context, int x, int y, const Gtk::SelectionData& selection_data, guint info, guint time);
  void maj_mosaique();
  void maj_selection();
  void maj_has_video();

  ///Gtk::Image gtk_image;
  Gtk::EventBox evt_box;
  ocvext::VueImage vue;
  //Glib::RefPtr<Gdk::Pixbuf> pixbuf;
  Gtk::VBox vbox;
  Gtk::ToolButton b_suppr, b_open, b_suppr_tout, b_maj, b_ajout;
  Gtk::Toolbar toolbar;
  bool toolbar_est_pleine; // En ce moment, a les boutons b_suppr, b_suppr_tout et b_add ?
  cv::Mat bigmat;

  struct Image
  {
    SpecEntree spec;
    std::string /*fichier,*/ nom;
    //cv::Mat mat;
    unsigned int ix, iy, px, py;
    utils::model::Node modele; // Modèle de source
  };

  unsigned int ncols, nrows, col_width, row_height;
  unsigned int img_width, img_height;

  std::deque<Image> images;
  int csel;
  bool has_a_video;
};

}

#endif

