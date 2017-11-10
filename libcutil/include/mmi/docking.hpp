#ifndef DOCKING_H
#define DOCKING_H

#include "mmi/gtkutil.hpp"
#include "gtkmm/toggleaction.h"


namespace utils { namespace mmi {


class VueDetachable;
//class MGCWnd;
//class MainDock;

class Controle
{
public:
  utils::model::Localized titre;
  std::string id;
  Gtk::Widget *widget;
  utils::model::Node modele, modele_dyn;

  virtual void maj_langue() {}
  virtual ~Controle(){}
protected:
};



class MyPaned: public Gtk::Container
{
public:
  struct Enfant
  {
    int num;
    int valeur_affectee;
    int valeur_forcee;
    float valeur_forcee_pourcent;

    Gtk::Allocation allocation[2];
    int largeur, dim, dim_temp, dim_mini, dim_nat;
    float hauteur_pourcent;


    int valeur_min;
    Gtk::Widget *widget;
    Gtk::Separator *sep;
    Gtk::HSeparator hsep;
    Gtk::VSeparator vsep;
    Gtk::EventBox event_box;
  };

  int get_dim(Gtk::Widget *wid);
  int set_dim(Gtk::Widget *wid, int dim);


  bool a_eu_allocation;

  MyPaned(bool vertical = true);
  ~MyPaned();
  void add_child(Gtk::Widget &w, int apres = -1);
  Gtk::Widget *get_widget();
  //void remove(Gtk::Widget &w);

  std::deque<Enfant *> enfants;

protected:

  void maj_allocation();
  unsigned int total_height;
  unsigned int total_minimum_dim, total_natural_dim;

  void est_taille_min(int &largeur, int &hauteur);


  virtual Gtk::SizeRequestMode get_request_mode_vfunc() const;
  virtual void get_preferred_width_vfunc(int& minimum_width, int& natural_width) const;
  virtual void get_preferred_height_for_width_vfunc(int width, int& minimum_height, int& natural_height) const;
  virtual void get_preferred_height_vfunc(int& minimum_height, int& natural_height) const;
  virtual void get_preferred_width_for_height_vfunc(int height, int& minimum_width, int& natural_width) const;
  virtual void on_size_allocate(Gtk::Allocation& allocation);

  virtual void forall_vfunc(gboolean include_internals, GtkCallback callback, gpointer callback_data);

  virtual void on_add(Gtk::Widget* child);
  virtual void on_remove(Gtk::Widget* child);
  virtual GType child_type_vfunc() const;
private:
  unsigned int get_n_visible_children() const;


  bool gere_motion(GdkEventMotion *mot);
  bool gere_enter(GdkEventCrossing *mot, Enfant *e);
  bool gere_leave(GdkEventCrossing *mot);
  bool gere_bpress(GdkEventButton *mot, Enfant *e);
  bool gere_brelease(GdkEventButton *mot, Enfant *e);
  bool vertical, deplacement;
  float last_dep, deplacement_pos_initiale;
  int deplacement_taille_initiale[2];
  Enfant *enfant_en_cours;
  Gtk::Allocation derniere_allocation;
  int tl_min, th_min;
  friend class MainDock;
  bool force_realloc;
};



class MyHPaned: public Gtk::HPaned
{
public:
  MyHPaned();
  MyHPaned(int largeur, int position);
  void set_contrainte(int largeur, int position);
  void applique_contrainte();
  int numero;
  bool forcer_position;
  //void set_position(int position) override;
protected:
  void on_size_allocate(Gtk::Allocation &allocation) /*override*/;
private:
  int largeur, position;
};

struct DockItem
{
  VueDetachable *vd;

};

struct Dock
{
  std::vector<DockItem> items;
};

class MainDock
{
public:
  MainDock(utils::model::Node modele);
  Gtk::Widget *get_widget();
  Gtk::VBox vbox;
  void sauve();
  void charge();
  void maj_langue();
  //MyHPaned hpaned[2];

  MyPaned hpaned;


  Dock docks[2];

  VueDetachable *recherche(std::string id);


  MyPaned vboxes[2];

  std::vector<VueDetachable *> controles;

private:
  void gere_drag_data_received(
          const Glib::RefPtr<Gdk::DragContext>& context, int b, int c,
          const Gtk::SelectionData& selection_data, unsigned int a, unsigned int time, int num);
  void gere_drag_data_received_gauche(
          const Glib::RefPtr<Gdk::DragContext>& context, int b, int c,
          const Gtk::SelectionData& selection_data, unsigned int a, unsigned int time);
  void gere_drag_data_received_droit(
          const Glib::RefPtr<Gdk::DragContext>& context, int b, int c,
          const Gtk::SelectionData& selection_data, unsigned int a, unsigned int time);
  bool gere_drag_motion(const Glib::RefPtr<Gdk::DragContext>& context, int x, int y, unsigned int prm);

  utils::model::Node modele;

  friend class VueDetachable;
  friend class MGCWnd;
};


class MyFrame: public Gtk::VBox
{
public:
  MyFrame();
  void set_titre(std::string s);
  void add(Gtk::Widget &w);
  void remove();
  Gtk::Label titre;
  Gtk::EventBox evbox;
  Gtk::Button b_fermer, b_dedocquer;
  Gtk::HBox hbox, hbox2;
  Gtk::Alignment align;
  Gtk::Widget *widget_en_cours;
};


class VueDetachable
{
public:
  VueDetachable(Controle *controle, MainDock *main_dock);
  void affiche(bool visible);
  Controle *controle;
  bool visible, docquee, docquable;
  int doc_en_cours;
  Glib::RefPtr<Gtk::ToggleAction> toggle;
  void sauve_position();
  void charge_position();
  bool est_visible();
  bool on_expose_event(const Cairo::RefPtr<Cairo::Context> &cr);
  void maj_langue();
private:
  bool gere_evt_delete(GdkEventAny *evt);
  void gere_dedocquage();
  void gere_fermeture();
  MainDock *main_dock;
  void maj_vue();
  Gtk::Widget *item;
  void gere_drag_data_get(
           const Glib::RefPtr<Gdk::DragContext>& context,
           Gtk::SelectionData& selection_data, unsigned int info, unsigned int time);
  bool gere_drag_motion(
        const Glib::RefPtr<Gdk::DragContext>& context,
        int x, int y, unsigned int time);
  Gtk::Window wnd;
  bool expose;
  MyFrame drag_frame;
  friend class MainDock;
};



}}




#endif
