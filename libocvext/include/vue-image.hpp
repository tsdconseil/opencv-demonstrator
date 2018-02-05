#ifndef VUE_IMAGE_H
#define VUE_IMAGE_H

#include "mmi/gtkutil.hpp"
#include <opencv2/imgproc.hpp>


namespace ocvext
{

class VueImage: public Gtk::DrawingArea
{
public:
  VueImage(uint16_t dx = 150, uint16_t dy = 100, bool dim_from_parent = true);
  void get_dim(uint16_t &sx, uint16_t &sy);
  void maj(const cv::Mat &img);
  void maj(const std::string &chemin_fichier);
  void change_dim(uint16_t sx, uint16_t sy);
  void coor_vue_vers_image(int xv, int yv, cv::Point &xi);

  bool on_draw(const Cairo::RefPtr<Cairo::Context> &cr);

  cv::Scalar arriere_plan;

protected:
  //void on_size_allocate(Gtk::Allocation &allocation);

  //void get_preferred_width(int& minimum_width, int& natural_width) const;
  //void get_preferred_height(int& minimum_height, int& natural_height) const;

private:
  float ratio;
  cv::Point p0;
  bool cr_alloue;

  void change_dim_interne(uint16_t sx, uint16_t sy);

  int alloc_x, alloc_y;

  struct Trame
  {
    cv::Mat img;
  };

  utils::mmi::GtkDispatcher<Trame> dispatcher;
  Trame en_attente;

  ///////////////////////////////////
  // Data protected by a mutex.
  cv::Mat new_img;
  ///////////////////////////////////

  bool dim_from_parent;
  bool realise;
  uint16_t csx, csy;
  Cairo::RefPtr<Cairo::ImageSurface> image_surface, old;
  Cairo::RefPtr<Cairo::Context> cr;
  Glib::Dispatcher signal_video_update;
  utils::hal::Mutex mutex_video_update;

  static std::vector<Cairo::RefPtr<Cairo::Context>> *old_srf;


  void maj_surface(const cv::Mat &I);
  void on_event(const Trame &t);

  void do_update_view();

  void on_the_realisation();
  void draw_all();
  //bool on_expose_event(GdkEventExpose* event);
  bool gere_expose_event(GdkEventExpose* event);
};


class DialogueImage
{
public:
  DialogueImage();
  void affiche(cv::Mat &I, const std::string &titre, const std::string &infos);

  Gtk::Dialog dialogue;
private:
  void gere_b_fermer();
  ocvext::VueImage vview;
  Gtk::HButtonBox hbox;
  Gtk::Button b_close;
};



}

#endif
