/** @file image-selecteur.hpp

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

#ifndef IMAGE_SELECTOR_HPP
#define IMAGE_SELECTOR_HPP


#include "mmi/gtkutil.hpp"
#include <gdkmm/pixbuf.h>
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "cutil.hpp"


struct ImageSelecteurRefresh{};

/** @brief Sélecteur d'image */
class ImageSelecteur:
    public Gtk::Window,
    public utils::CProvider<ImageSelecteurRefresh>
{
public:
  ImageSelecteur();
  void maj_langue();
  void maj_actif();

  void ajoute_fichier(std::string s);
  void raz();

  void get_list(std::vector<cv::Mat> &list);

private:
  void on_size_change(Gtk::Allocation &alloc);
  bool on_b_pressed(GdkEventButton *event);
  bool on_b_released(GdkEventButton *event);
  bool on_k_released(GdkEventKey *event);
  void maj_taille(); // Obsolete
  void on_b_add();
  void on_b_del();
  void on_b_del_tout();
  void on_b_maj();
  void on_dropped_file(const Glib::RefPtr<Gdk::DragContext>& context, int x, int y, const Gtk::SelectionData& selection_data, guint info, guint time);
  void maj_mosaique();
  void maj_selection();

  Gtk::Image gtk_image;
  Gtk::EventBox evt_box;
  Glib::RefPtr<Gdk::Pixbuf> pixbuf;
  Gtk::VBox vbox;
  Gtk::ToolButton b_suppr, b_ajout, b_suppr_tout, b_maj;
  Gtk::Toolbar toolbar;
  cv::Mat bigmat;
  utils::Logable journal;

  struct Image
  {
    std::string fichier, nom;
    cv::Mat mat;
    unsigned int ix, iy, px, py;
  };

  unsigned int ncols, nrows, col_width, row_height;
  unsigned int img_width, img_height;

  std::vector<Image> images;
  int csel;
};



#endif

