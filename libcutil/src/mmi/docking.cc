/*#include <gtk/gtk.h>
#include <gtkmm.h>
#include "gdl/gdl.h"

extern "C"
{
#include <glib.h>
#include <glib/gi18n-lib.h>
#include <string.h>
#include <stdlib.h>
#include <gtk/gtk.h>
}*/

#include <gtkmm.h>
#include "mmi/docking.hpp"


namespace utils { namespace mmi {


MyPaned::MyPaned(bool vertical)
{
  force_realloc   = false;
  enfant_en_cours = nullptr;
  a_eu_allocation = false;
  total_natural_dim = total_minimum_dim = 0;
  this->total_height = 0;
  this->vertical = vertical;
  derniere_allocation.set_x(0);
  derniere_allocation.set_y(0);
  derniere_allocation.set_width(1);
  derniere_allocation.set_height(1);
  set_has_window(false);
  set_redraw_on_allocate(false);
  maj_allocation();
}

MyPaned::~MyPaned()
{
/*
  // These calls to Gtk::Widget::unparent() are necessary if MyPaned is
  // deleted before its children. But if you use a version of gtkmm where bug
  // https://bugzilla.gnome.org/show_bug.cgi?id=605728
  // has not been fixed (gtkmm 3.7.10 or earlier) and the children are deleted
  // before the container, these calls can make the program crash.
  // That's because on_remove() is not called, when the children are deleted.
  if (m_child_one)
    m_child_one->unparent();

  if (m_child_two)
    m_child_two->unparent();
*/
}

int MyPaned::get_dim(Gtk::Widget *wid)
{
  for(auto e: enfants)
    if(e->widget == wid)
      return e->dim;
  erreur("%s: non trouvé.", __func__);
  return -1;
}

int MyPaned::set_dim(Gtk::Widget *wid, int dim)
{
  for(auto e: enfants)
  {
    if(e->widget == wid)
    {
      int mini, natu;
      if(vertical)
        wid->get_preferred_height(mini, natu);
      else
        wid->get_preferred_width(mini, natu);
      if(dim >= mini)
        e->dim = dim;
      this->maj_allocation();
      return 0;
    }
  }
  erreur("%s: non trouvé.", __func__);
  return -1;
}

void MyPaned::add_child(Gtk::Widget &w, int apres)
{
  Enfant *e = new Enfant();

  int mh, nh;
  int mw, nw;
  w.get_preferred_height(mh, nh);
  w.get_preferred_width(mw, nw);

  /*if(vertical)
    e->dim = nh;
  else
    e->dim = nw;*/

  e->dim = -1;

  e->largeur = nw;
  e->hauteur_pourcent = -1;
  e->widget = &w;
  if(vertical)
    e->sep = &(e->hsep);
  else
    e->sep = &(e->vsep);

  w.set_parent(*this);
  e->event_box.set_parent(*this);
  e->event_box.add(*(e->sep));
  if(vertical)
    e->event_box.set_hexpand(true);
  else
    e->event_box.set_vexpand(true);
  e->sep->show();
  e->event_box.show();
  //e.vsep->set_margin_top(5);
  e->event_box.set_events(Gdk::POINTER_MOTION_MASK
                   | Gdk::BUTTON_MOTION_MASK
                   | Gdk::ENTER_NOTIFY_MASK
                   | Gdk::LEAVE_NOTIFY_MASK
                   | Gdk::POINTER_MOTION_HINT_MASK
                   | Gdk::BUTTON_PRESS_MASK);
  e->event_box.signal_motion_notify_event().connect(sigc::mem_fun(*this, &MyPaned::gere_motion));
  e->event_box.signal_enter_notify_event().connect(
      sigc::bind(sigc::mem_fun(*this, &MyPaned::gere_enter), e));
  e->event_box.signal_leave_notify_event().connect(sigc::mem_fun(*this, &MyPaned::gere_leave));
  e->event_box.signal_button_press_event().connect(
      sigc::bind(sigc::mem_fun(*this, &MyPaned::gere_bpress), e));
  e->event_box.signal_button_release_event().connect(
      sigc::bind(sigc::mem_fun(*this, &MyPaned::gere_brelease), e));


  if(apres == -1)
    enfants.push_back(e);
  else
    enfants.insert(enfants.begin() + apres, e);



  for(unsigned int i = 0u; i < enfants.size(); i++)
    enfants[i]->num = i;

  //allocation_initiale();
  //reallocation_complete();
  maj_allocation();
}



void MyPaned::maj_allocation()
{
  unsigned int n = enfants.size();
  est_taille_min(tl_min, th_min);

  unsigned int total_dim = 0;

  // On regarde si la taille affectée est :
  //  (1) >= à la taille totale naturelle
  //  (2) ou inférieure

  int alloc_dim;
  if(vertical)
    alloc_dim = derniere_allocation.get_height();
  else
    alloc_dim = derniere_allocation.get_width();

  //
  for(auto i = 0u; i < n; i++)
  {
    auto e = enfants[i];
    int mini, nat;
    if(vertical)
      e->widget->get_preferred_height(mini, nat);
    else
      e->widget->get_preferred_width(mini, nat);

    e->dim_mini = mini;
    e->dim_nat = nat;

    //int h = e->hauteur;

    // (1) APPEL SANS ALLOCATION, h[i] = -1
    // (2) APPEL AVEC ALLOCATION, h[i] = -1
    // (3) APPEL AVEC ALLOCATION, h[i] != -1

    e->dim_temp = e->dim;

    if(e->dim == -1)
    {
      e->dim_temp = nat; // Allocation taille naturelle
      if(a_eu_allocation && (alloc_dim > 0) && (nat > 0))
      {
        // Si dernier, alloue tout ce qui reste
        if((i == (unsigned int) (n - 1)) && (alloc_dim >= (nat + (int) total_dim)))
        {
            e->dim_temp = alloc_dim - total_dim;
            infos("%d <- %d", e->dim, e->dim_temp);
            e->dim = e->dim_temp;
        }
        else if(alloc_dim >= (int) (nat + total_dim))
        {
          infos("%d <- %d", e->dim, e->dim_temp);
          e->dim = e->dim_temp;
        }
      }
    }
    // Si déjà une hauteur programmée, mais le dernier -> tout ce qui reste quand même
    else if(i == n - 1)
    {
      if(a_eu_allocation)
      {
        if(alloc_dim > (int) total_dim)
          e->dim_temp = alloc_dim - total_dim;
      }
      else
        e->dim_temp = nat;
    }

    //if(!vertical)
      infos("%s/%s: widget[%d] -> dim_temp = %d (nat = %d, min = %d, dim = %d, a_eu_allocation = %d).",
                 __func__, vertical ? "V" : "H", i, e->dim_temp, nat, mini, e->dim, a_eu_allocation);

    total_dim += e->dim_temp;
    //e->sep->get_preferred_height(mini, nat);
    total_dim += 5;//mini;
  }

  total_natural_dim = total_dim;
  if(vertical)
    total_minimum_dim = th_min;
  else
    total_minimum_dim = tl_min;


  total_dim = 0;
  for(auto i = 0u; i < n; i++)
  {
    auto e = enfants[i];
    //if(enfants[i]->widget->get_visible())
    {
      //int h;
      int mini, nat;

      if(vertical)
        e->widget->get_preferred_height(mini, nat);
      else
        e->widget->get_preferred_width(mini, nat);

      //h = enfants[i]->dim_temp;

      if(vertical)
      {
        e->allocation[0].set_height(enfants[i]->dim_temp);
        //Make it take up the full width available:
        e->allocation[0].set_width(derniere_allocation.get_width());
        e->allocation[0].set_x(derniere_allocation.get_x());
        e->allocation[0].set_y(total_dim + derniere_allocation.get_y());
      }
      else
      {
        e->allocation[0].set_width(enfants[i]->dim_temp);
        e->allocation[0].set_height(derniere_allocation.get_height());
        e->allocation[0].set_x(total_dim + derniere_allocation.get_x());
        e->allocation[0].set_y(derniere_allocation.get_y());
        infos("placement[%d]: x = %d.", i, total_dim);
      }
      //enfants[i]->widget->size_allocate(ch_alloc);
      total_dim += enfants[i]->dim_temp;

      if(i + 1 < n)
      {
        //enfants[i]->sep->get_preferred_height(mini, nat);
        mini = nat = 5;
        //h = nat;
        //infos("Separator alloc: mini = %d, nat = %d, POS = %d.", mini, nat,total_dim);
        if(vertical)
        {
          e->allocation[1].set_height(nat);
          e->allocation[1].set_width(derniere_allocation.get_width());
          e->allocation[1].set_x(derniere_allocation.get_x());
          e->allocation[1].set_y(total_dim + derniere_allocation.get_y());
        }
        else
        {
          e->allocation[1].set_width(nat);
          e->allocation[1].set_height(derniere_allocation.get_height());
          e->allocation[1].set_x(total_dim + derniere_allocation.get_x());
          e->allocation[1].set_y(derniere_allocation.get_y());
        }
        //enfants[i]->event_box.size_allocate(ch_alloc);
        total_dim += nat;
      }
    }
  }
  total_natural_dim = total_dim;
  infos("total_dim = %d", total_dim);
}

Gtk::Widget *MyPaned::get_widget()
{
  return this;
}



//This example container is a simplified VBox with at most two children.
Gtk::SizeRequestMode MyPaned::get_request_mode_vfunc() const
{
  if(vertical)
    return Gtk::SIZE_REQUEST_HEIGHT_FOR_WIDTH;
  else
    return Gtk::SIZE_REQUEST_WIDTH_FOR_HEIGHT;
}


void MyPaned::est_taille_min(int &largeur, int &hauteur)
{
  largeur = 0;
  hauteur = 0;

  for(auto &ch: enfants)
  {
    int ml[2], pl[2], mh[2], ph[2];
    ch->widget->get_preferred_width(ml[0], pl[0]);
    ch->widget->get_preferred_height(mh[0], ph[0]);
    ch->sep->get_preferred_width(ml[1], pl[1]);
    ch->sep->get_preferred_height(mh[1], ph[1]);

    if(vertical)
      ch->valeur_min = mh[0];
    else
      ch->valeur_min = ml[0];

    for(auto i = 0u; i < 2; i++)
    {
      if(vertical)
      {
        if(ml[i] > largeur)
          largeur = ml[i];
        hauteur += mh[i];
      }
      else
      {
        if(mh[i] > hauteur)
          hauteur = mh[i];
        largeur += ml[i];
      }
    }
  }

  infos("%s: %d, %d", __func__, largeur, hauteur);
}


//Discover the total amount of minimum space and natural space needed by
//this container and its children.
void MyPaned::get_preferred_width_vfunc(int& minimum_width, int& natural_width) const
{
  if(enfants.size() == 0)
  {
    minimum_width = 0;
    natural_width = 0;
    return;
  }

  if(!vertical)
  {
    minimum_width = total_minimum_dim;
    natural_width = total_natural_dim;
    if(minimum_width > natural_width)
      erreur("%s: min = %d, nat = %d.", __func__, minimum_width, natural_width);
    return;
  }

  int min_width[2*enfants.size()];
  int nat_width[2*enfants.size()];

  unsigned int i = 0u;
  for(auto &ch: enfants)
  {
    if(ch->widget->get_visible())
    {
      ch->widget->get_preferred_width(min_width[2*i], nat_width[2*i]);
      ch->sep->get_preferred_width(min_width[2*i+1], nat_width[2*i+1]);
      //infos("child reports %d, %d", min_width[2*i], nat_width[2*i]);
    }
    else
    {
      min_width[2*i] = nat_width[2*i] = 0;
      min_width[2*i+1] = nat_width[2*i+1] = 0;
    }
    i++;
  }

  // Request a width equal to the width of the widest visible child.
  minimum_width = *std::max_element(min_width, min_width + 2*enfants.size());
  natural_width = *std::max_element(nat_width, nat_width + 2*enfants.size());
  //infos("%s: %d, %d", __func__, minimum_width, natural_width);
  if(minimum_width > natural_width)
    erreur("%s: min = %d, nat = %d.", __func__, minimum_width, natural_width);
}

void MyPaned::get_preferred_height_for_width_vfunc(int width, int& minimum_height, int& natural_height) const
{
  get_preferred_height_vfunc(minimum_height, natural_height);
# if 0
  if(enfants.size() == 0)
  {
    minimum_height = 0;
    natural_height = 0;
    return;
  }

  unsigned int n = enfants.size();
  int child_minimum_height[n];
  int child_natural_height[n];
  int nvis_children = get_n_visible_children();

  for(auto i = 0u; i < n; i++)
    if(enfants[i]->widget->get_visible())
      enfants[i]->widget->get_preferred_height_for_width(width, child_minimum_height[i], child_natural_height[i]);

  //The allocated height will be divided equally among the visible children.
  //Request a height equal to the number of visible children times the height
  //of the highest child.
  minimum_height = nvis_children *
      *std::max_element(child_minimum_height, child_minimum_height + enfants.size());
  natural_height = nvis_children *
      *std::max_element(child_natural_height, child_natural_height + enfants.size());;
  infos("%s: %d, %d", __func__, minimum_height, natural_height);
# endif
}

void MyPaned::get_preferred_height_vfunc(int& minimum_height, int& natural_height) const
{
  if(enfants.size() == 0)
  {
    minimum_height = 0;
    natural_height = 0;
    return;
  }

  if(vertical)
  {
    minimum_height = total_minimum_dim;
    natural_height = total_natural_dim;
    //verbose("%s: min = %d, nat = %d", __func__, minimum_height, natural_height);
    if(minimum_height > natural_height)
    {
      avertissement("%s: min = %d, nat = %d.", __func__, minimum_height, natural_height);
      natural_height = minimum_height;
    }
    return;
  }


  int min_height[2*enfants.size()];
  int nat_height[2*enfants.size()];

  unsigned int i = 0u;
  for(auto &ch: enfants)
  {
    if(ch->widget->get_visible())
    {
      ch->widget->get_preferred_height(min_height[2*i], nat_height[2*i]);
      ch->sep->get_preferred_height(min_height[2*i+1], nat_height[2*i+1]);
      //infos("child reports %d, %d", min_height[2*i], nat_height[2*i]);
    }
    else
    {
      min_height[2*i] = nat_height[2*i] = 0;
      min_height[2*i+1] = nat_height[2*i+1] = 0;
    }
    i++;
  }

  // Request a width equal to the width of the widest visible child.
  minimum_height = *std::max_element(min_height, min_height + 2*enfants.size());
  natural_height = *std::max_element(nat_height, nat_height + 2*enfants.size());

  //infos("%s: %d, %d", __func__, minimum_height, natural_height);
  if(minimum_height > natural_height)
    erreur("%s: min = %d, nat = %d.", __func__, minimum_height, natural_height);

# if 0
  unsigned int n = enfants.size();
  int child_minimum_height[n];
  int child_natural_height[n];
  int nvis_children = get_n_visible_children();

  for(auto i = 0u; i < n; i++)
  {
    if(enfants[i]->widget->get_visible())
    {
      enfants[i]->widget->get_preferred_height(child_minimum_height[0], child_natural_height[0]);
    }
  }

  //The allocated height will be divided equally among the visible children.
  //Request a height equal to the number of visible children times the height
  //of the highest child.
  minimum_height = nvis_children *
      *std::max_element(child_minimum_height, child_minimum_height + enfants.size());
  natural_height = nvis_children *
      *std::max_element(child_natural_height, child_natural_height + enfants.size());;
  infos("%s: %d, %d", __func__, minimum_height, natural_height);
# endif
}

void MyPaned::get_preferred_width_for_height_vfunc(int height,
   int& minimum_width, int& natural_width) const
{
  get_preferred_width_vfunc(minimum_width, natural_width);
# if 0
  if(enfants.size() == 0)
  {
    minimum_width = 0;
    natural_width = 0;
    return;
  }

  unsigned int n = enfants.size();
  int child_minimum_width[n];
  int child_natural_width[n];
  int nvis_children = get_n_visible_children();

  if(nvis_children > 0)
  {
    //Divide the height equally among the visible children.
    const int height_per_child = height / nvis_children;

    for(auto i = 0u; i < n; i++)
    {
      if(enfants[i]->widget->get_visible())
        enfants[i]->widget->get_preferred_width_for_height(height_per_child,
                   child_minimum_width[i], child_natural_width[i]);
    }
  }

  //Request a width equal to the width of the widest child.
  minimum_width = *std::max_element(child_minimum_width, child_minimum_width + n);
  natural_width = *std::max_element(child_natural_width, child_natural_width + n);
  infos("%s: %d, %d", __func__, minimum_width, natural_width);
# endif
}

unsigned int MyPaned::get_n_visible_children() const
{
  unsigned int res = 0;
  for(auto &ch: enfants)
    if(ch->widget->get_visible())
      res++;
  return res;
}

void MyPaned::on_size_allocate(Gtk::Allocation& allocation)
{
  // Do something with the space that we have actually been given:
  // (We will not be given heights or widths less than we have requested, though
  // we might get more.)

  if(!force_realloc)
  {
    if((allocation.get_x() == derniere_allocation.get_x())
        && (allocation.get_y() == derniere_allocation.get_y())
        && (allocation.get_width() == derniere_allocation.get_width())
        && (allocation.get_height() == derniere_allocation.get_height()))
    {
      for(auto e: enfants)
      {
        e->widget->size_allocate(e->allocation[0]);
        if(e != enfants[enfants.size() - 1])
          e->event_box.size_allocate(e->allocation[1]);
      }
      return;
    }
  }
  else
    force_realloc = false;

  trace_verbeuse("(%d,%d,%d,%d)",
      allocation.get_x(), allocation.get_y(),
      allocation.get_width(), allocation.get_height());

  // Use the offered allocation for this container:
  set_allocation(allocation);

# if 1
  if(a_eu_allocation && (derniere_allocation.get_width() > 0))
  {
    // Essaye de garder les mêmes proportions qu'avant

    float tot = 0;
    bool all_ok = true;
    for(auto &e: enfants)
    {
      if(e->dim <= 0)
      {
        all_ok = false;
        break;
      }
      tot += e->dim;
    }
    if(all_ok)
    {
      for(auto &e: enfants)
        e->hauteur_pourcent = ((float) e->dim) / tot;

      int dim_dispo;
      if(vertical)
        dim_dispo = allocation.get_height();
      else
        dim_dispo = allocation.get_width();

      for(auto &e: enfants)
      {
        int essai = e->hauteur_pourcent * dim_dispo;
        if(essai >= e->dim_mini)
          e->dim = essai;
      }
    }
  }
# endif

  derniere_allocation = allocation;
  a_eu_allocation = true;
  maj_allocation();

  for(auto e: enfants)
  {
    e->widget->size_allocate(e->allocation[0]);
    if(e != enfants[enfants.size() - 1])
      e->event_box.size_allocate(e->allocation[1]);
  }
}

void MyPaned::forall_vfunc(gboolean, GtkCallback callback, gpointer callback_data)
{
  for(auto ch: enfants)
  {
    callback(ch->widget->gobj(), callback_data);
    if(ch != *(enfants.end()-1))
      callback((GtkWidget *) ch->event_box.gobj(), callback_data);
  }
}

void MyPaned::on_add(Gtk::Widget* child)
{
  erreur("%s: TODO", __func__);
  //children.push_back(child);
  //child->set_parent(*this);
}

bool MyPaned::gere_motion(GdkEventMotion *mot)
{
  if(deplacement)
  {
    trace_verbeuse("déplacement: %d, %d", (int) mot->x, (int) mot->y);

    // Il faut se débrouiller pour que mot->y soit la coordonnée de départ du séparateur du bas

    if(enfant_en_cours != nullptr)
    {

      int id = enfant_en_cours->num;

      if(id >= (int) enfants.size() - 1)
      {
        avertissement("Déplacement dernier paneau -> impossible.");
        return true;
      }


      int inc;

      if(vertical)
        inc = mot->y_root - deplacement_pos_initiale;
      else
        inc = mot->x_root - deplacement_pos_initiale;

      int nvh[2];
      nvh[0] = deplacement_taille_initiale[0] + inc;
      nvh[1] = deplacement_taille_initiale[1] - inc;

      if(inc > 0)
      {
        if(nvh[1] < enfants[id+1]->valeur_min)
        {
          nvh[1] = enfants[id+1]->valeur_min;
          nvh[0] -= inc;
          nvh[0] += deplacement_taille_initiale[1] - nvh[1];
          trace_verbeuse("Blocage à cause du bas (min = %d)", enfants[id+1]->valeur_min);
        }
      }
      else
      {
        if(nvh[0] < enfants[id]->valeur_min)
        {
          nvh[0] = enfants[id]->valeur_min;
          nvh[1] += inc;
          nvh[1] -= nvh[0] - deplacement_taille_initiale[0];
          trace_verbeuse("Blocage à cause du haut");
        }
      }

      enfants[id]->dim   = nvh[0];
      enfants[id+1]->dim = nvh[1];



#     if 0
      int w0 = enfants[id]->widget->get_allocated_width();
      int h0 = enfants[id]->widget->get_allocated_height();
      int w1 = enfants[id+1]->widget->get_allocated_width();
      int h1 = enfants[id+1]->widget->get_allocated_height();
      int inc = mot->y - last_dep;
      //last_dep += inc;
      last_dep = mot->y;
      if(inc > 0)
      {
        // Si pas l'avant dernier
        //if(id != enfants.size() - 1)
        {
          if(h1 - inc < enfants[id+1]->valeur_min)
          {
            inc = h1 - enfants[id+1]->valeur_min;
            avertissement("Blocage à cause du bas (min = %d, h = %d)", enfants[id+1]->valeur_min, h1);
          }
        }

        enfants[id]->dim    = h0 + inc;
        enfants[id+1]->dim  = h1 - inc;
      }
      else
      {
        if(h0 + inc < enfants[id]->valeur_min)
        {
          inc = enfants[id]->valeur_min - h0;
          avertissement("Blocage à cause du haut");
        }

        enfants[id]->dim    = h0 + inc;

        if(id != enfants.size() - 2)
          enfants[id+1]->dim  = h1 - inc;
      }
#     endif

      //maj_allocation();

      //enfants[id]->hauteur

      force_realloc = true;

      queue_resize();
      //queue_draw();
      //queue_resize_no_redraw();
    }
  }
  return true;
}

bool MyPaned::gere_leave(GdkEventCrossing *mot)
{
  auto ref_window = this->get_window();
  ref_window->set_cursor(); // Curseur par défaut
  return true;
}

bool MyPaned::gere_enter(GdkEventCrossing *mot, Enfant *e)
{
  infos("enter notify: %d, %d", (int) mot->x, (int) mot->y);


  auto ref_window = e->sep->get_window();

  Glib::RefPtr<Gdk::Cursor> curseur;

  if(vertical)
    curseur = Gdk::Cursor::create(Gdk::DOUBLE_ARROW);
  else
    curseur = Gdk::Cursor::create(Gdk::SB_H_DOUBLE_ARROW);
  ref_window->set_cursor(curseur);

  return true;
}

bool MyPaned::gere_brelease(GdkEventButton *mot, Enfant *e)
{
  infos("brelease: %d, %d", (int) mot->x, (int) mot->y);

  enfant_en_cours = nullptr;
  deplacement = false;
  return true;
}

bool MyPaned::gere_bpress(GdkEventButton *mot, Enfant *e)
{
  infos("bpress: paneau[%d], %d, %d", e->num, mot->x, mot->y);
  enfant_en_cours = e;

  if(vertical)
    deplacement_pos_initiale = mot->y_root;
  else
    deplacement_pos_initiale = mot->x_root;
  last_dep = deplacement_pos_initiale;

  if(vertical)
  {
    deplacement_taille_initiale[0] = e->widget->get_allocated_height();
    if(e->num != ((int) enfants.size() - 1))
      deplacement_taille_initiale[1] = enfants[e->num+1]->widget->get_allocated_height();
  }
  else
  {
    deplacement_taille_initiale[0] = e->widget->get_allocated_width();
    if(e->num != ((int) enfants.size() - 1))
      deplacement_taille_initiale[1] = enfants[e->num+1]->widget->get_allocated_width();
  }

  deplacement = true;
  return true;
}

/*void MyPaned::remove(Gtk::Widget &w)
{

}*/

void MyPaned::on_remove(Gtk::Widget* child)
{
  Enfant *e = nullptr;

  if(child)
  {
    const bool visible = child->get_visible();
    for(auto i = 0u; i < enfants.size(); i++)
    {
      if(child == enfants[i]->widget)
      {
        e = enfants[i];
        child->unparent();
        e->event_box.unparent();
        enfants.erase(enfants.begin() + i);
        delete e;
        if(visible)
          queue_resize();
        return;
      }
    }
  }
}

GType MyPaned::child_type_vfunc() const
{
  //If there is still space for one widget, then report the type of widget that
  //may be added.
  //if(!m_child_one || !m_child_two)
    return Gtk::Widget::get_type();
  //else
  //{
    //No more widgets may be added.
    //return G_TYPE_NONE;
  //}
}


void MainDock::maj_langue()
{
  for(auto &ctrl: controles)
    ctrl->maj_langue();
}


void MainDock::sauve()
{
  int paned1, paned2;
  paned1 = hpaned.enfants[0]->dim;
  paned2 = hpaned.enfants[1]->dim;
  /*mgc::app.modele_global.set_attribute("pos-fenetre-princ/paned1", paned1);
  mgc::app.modele_global.set_attribute("pos-fenetre-princ/paned2", paned2);*/

  modele.set_attribute("paned1", paned1);
  modele.set_attribute("paned2", paned2);

  //auto &lst = mgc::vue::MGCWnd::get_instance()->controles;
  for(auto &vd: controles/*lst*/)
  {
    auto md = vd->controle->modele_dyn;
    md.set_attribute("id", vd->controle->id);
    md.set_attribute("visible", vd->visible);
    md.set_attribute("docquee", vd->docquee);
    md.set_attribute("dock-en-cours", vd->doc_en_cours);
    if(!vd->docquee)
    {
      if(vd->visible)
      {
        auto win = vd->wnd.get_window();
        if(win)
        {
          int x, y, l, h;
          //win->get_position(x, y);
          //l = win->get_width();
          //h = win->get_height();
          vd->wnd.get_position(x, y);
          l = vd->wnd.get_width();
          h = vd->wnd.get_height();
          md.set_attribute("x", x);
          md.set_attribute("y", x);
          md.set_attribute("largeur", l);
          md.set_attribute("hauteur", h);
        }
      }
    }
    else
    {
      // Recherche de l'endroit où est docké
      int dim = vboxes[vd->doc_en_cours].get_dim(&(vd->drag_frame));
      md.set_attribute("dim", dim);
    }
  }
}

void MainDock::charge()
{
  /*int paned1 = mgc::app.modele_global.get_attribute_as_int("pos-fenetre-princ/paned1");
  int paned2 = mgc::app.modele_global.get_attribute_as_int("pos-fenetre-princ/paned2");*/
  int paned1 = modele.get_attribute_as_int("paned1");
  int paned2 = modele.get_attribute_as_int("paned2");
  hpaned.enfants[0]->dim = paned1;
  hpaned.enfants[1]->dim = paned2;

  Glib::RefPtr<Gdk::Screen> ecran = Gdk::Screen::get_default();
  int ecran_largeur = ecran->get_width();
  int ecran_hauteur = ecran->get_height();

  //auto &lst = mgc::vue::MGCWnd::get_instance()->controles;
  for(auto &vd: controles)
  {
    int x, y, l, h;
    auto md = vd->controle->modele_dyn;
    x = md.get_attribute_as_int("x");
    y = md.get_attribute_as_int("y");
    l = md.get_attribute_as_int("largeur");
    h = md.get_attribute_as_int("hauteur");
    vd->visible = md.get_attribute_as_boolean("visible");
    vd->docquee = md.get_attribute_as_boolean("docquee");
    vd->doc_en_cours = md.get_attribute_as_int("dock-en-cours");

    if(vd->visible)
    {

      infos("Chargement ctrl [%s], docquee = %d.", vd->controle->id.c_str(), vd->docquee);

      if(vd->docquee)
      {
        vboxes[vd->doc_en_cours].add_child(vd->drag_frame);
        vd->drag_frame.show();
        vd->drag_frame.b_dedocquer.show();
        vboxes[vd->doc_en_cours].set_dim(&(vd->drag_frame), md.get_attribute_as_int("dim"));
      }
      else
      {
        if(!vd->docquable)
          vd->wnd.add(*(vd->controle->widget));
        else
          vd->wnd.add(vd->drag_frame);
        vd->wnd.show_all_children(true);
        vd->wnd.show();

        vd->drag_frame.b_dedocquer.hide();
        vd->maj_vue();
        auto win = vd->wnd.get_window();
        if(win)
        {
          infos("move_resize(%d,%d,%d,%d)...", x, y, l, h);
          if(x < 0)
          {
            avertissement("%s: x < 0 (%d)", __func__, x);
            x = 0;
          }
          if(y < 0)
          {
            avertissement("%s: y < 0 (%d)", __func__, y);
            y = 0;
          }
          if(x + l > ecran_largeur)
          {
            avertissement("%s: x + l (%d) > largeur ecran (%d)", __func__, x + l, ecran_largeur);
            x -= (x + l) - ecran_largeur;
            if(x < 0)
            {
              x = 0;
              l = ecran_largeur;
            }
          }
          if(y + h > ecran_hauteur)
          {
            avertissement("%s: y + h (%d) > hauteur ecran (%d)", __func__, y + h, ecran_hauteur);
            y -= (y + h) - ecran_hauteur;
            if(y < 0)
            {
              y = 0;
              h = ecran_hauteur;
            }
          }
          infos("move_resize reel(%d,%d,%d,%d)...", x, y, l, h);
          vd->wnd.move(x, y);
          vd->wnd.resize(l, h);
        }
      }
    }
  }
  hpaned.queue_resize();
}

void VueDetachable::gere_dedocquage()
{
  main_dock->vboxes[doc_en_cours].remove(drag_frame);
  //drag_frame.remove();

  docquee = false;

  if(!docquable)
    wnd.add(*(controle->widget));
  else
    wnd.add(drag_frame);
  wnd.show_all_children(true);
  wnd.show();
  drag_frame.b_dedocquer.hide();
}

void VueDetachable::gere_fermeture()
{
  if(docquee)
    main_dock->vboxes[doc_en_cours].remove(drag_frame);
  //drag_frame.remove();

  visible = false;
}

void VueDetachable::affiche(bool visible)
{
  this->controle->modele_dyn.set_attribute("visible", visible);
  // Affichage
  if(!this->visible && visible)
  {
    if(this->docquee)
    {
      //drag_frame.add(*(controle->widget));
      //main_dock->vboxes[doc_en_cours].pack_start(drag_frame, Gtk::PACK_EXPAND_WIDGET);
      main_dock->vboxes[doc_en_cours].add_child(drag_frame);
      drag_frame.b_dedocquer.show();
      drag_frame.show();
    }
    else
    {
      trace_verbeuse("Affichage fenetre [%s]", controle->id.c_str());
      drag_frame.b_dedocquer.hide();
      if(docquable)
        wnd.add(drag_frame);
      else
        wnd.add(*(controle->widget));
      wnd.show_all_children(true);
      wnd.show();
    }
    this->visible = visible;
  }
  // Masquage
  else if(!visible && this->visible)
  {
    if(docquee)
    {
      main_dock->vboxes[doc_en_cours].remove(drag_frame);
      //drag_frame.remove();
    }
    else
    {
      wnd.remove();
      wnd.hide();
    }
    this->visible = false;
  }


  //this->visible = visible;
  //maj_vue();
}


void MainDock::gere_drag_data_received_gauche(
        const Glib::RefPtr<Gdk::DragContext>& context, int b, int c,
        const Gtk::SelectionData& selection_data, unsigned int a, unsigned int time)
{
  gere_drag_data_received(context, b, c, selection_data, a, time, 0);
}

void MainDock::gere_drag_data_received_droit(
        const Glib::RefPtr<Gdk::DragContext>& context, int b, int c,
        const Gtk::SelectionData& selection_data, unsigned int a, unsigned int time)
{
  gere_drag_data_received(context, b, c, selection_data, a, time, 1);
}

bool VueDetachable::gere_evt_delete(GdkEventAny *evt)
{
  infos("Suppression manuelle fenêtre [%s].", controle->id.c_str());
  if(visible && !docquee)
  {
    wnd.hide();
    wnd.remove();
    visible = false;
    toggle->set_active(false);
  }
  return true;
}

void MainDock::gere_drag_data_received(
        const Glib::RefPtr<Gdk::DragContext>& context, int b, int c,
        const Gtk::SelectionData& selection_data, unsigned int a, unsigned int time, int num)
{
  const int length = selection_data.get_length();
  if((length >= 0) && (selection_data.get_format() == 8))
  {
    std::string s = selection_data.get_data_as_string();
    infos("Reçu [%s] sur paned[%d]", s.c_str(), num);


    VueDetachable *ctrl = recherche(s);
    if(ctrl == nullptr)
      return;

    if(ctrl->visible && !ctrl->docquee && ctrl->docquable)
    {
      infos("Docquage en cours...");
      ctrl->wnd.hide();
      ctrl->wnd.remove();
      //ctrl->drag_frame.add(*(ctrl->controle->widget));
      //vboxes[num].pack_start(ctrl->drag_frame, Gtk::PACK_EXPAND_WIDGET);
      vboxes[num].add_child(ctrl->drag_frame);
      ctrl->drag_frame.show();
      ctrl->docquee = true;
      ctrl->doc_en_cours = num;
      ctrl->drag_frame.b_dedocquer.show();
    }
    else if(ctrl->visible && ctrl->docquee && ctrl->docquable)
    {
      infos("Déplacement docquée %d -> %d...", ctrl->doc_en_cours, num);
      vboxes[ctrl->doc_en_cours].remove(ctrl->drag_frame);
      //vboxes[num].pack_start(ctrl->drag_frame, Gtk::PACK_EXPAND_WIDGET);
      vboxes[num].add_child(ctrl->drag_frame);
      ctrl->drag_frame.show();
      ctrl->doc_en_cours = num;
      ctrl->drag_frame.b_dedocquer.show();
    }
    //this->hpaned[1].show_all_children(true);
  }
  else
    infos("Drag sans message ?");

  context->drag_finish(false, false, time);
}

bool MainDock::gere_drag_motion(const Glib::RefPtr<Gdk::DragContext>& context, int x, int y, unsigned int prm)
{
  infos("drag_motion : %d, %d.", x, y);
  //const Glib::RefPtr< Gdk::Cursor > cursor = Gdk::Cursor::create(Gdk::HAND1);
  //this->get_window()->set_cursor(cursor);

  return true;
}

VueDetachable *MainDock::recherche(std::string id)
{
  //auto lst = vue::MGCWnd::get_instance()->controles;
  for(auto &vd: controles)
  {
    if(vd->controle->id == id)
      return vd;
  }
  erreur("Controle non trouvé : [%s].", id.c_str());
  return nullptr;
}


MyHPaned::MyHPaned(int largeur, int position)
{
  set_contrainte(largeur, position);
  //this->signal_check_resize()
}

MyHPaned::MyHPaned()
{
  set_contrainte(-1, -1);
}

void MyHPaned::applique_contrainte()
{
  infos("HPANED[%d] : Forcage position = %d (allocation = %d)", numero, position, get_allocated_width());
  this->set_position(position);
}

void MyHPaned::set_contrainte(int largeur, int position)
{
  forcer_position = true;
  this->largeur = largeur;
  this->position = position;
}


void MyHPaned::on_size_allocate(Gtk::Allocation &allocation)
{
  //int w = this->get_allocated_width();
  trace_majeure("MyHPaned[%d]::on_size_allocate(w = %d, h = %d)",
      numero,
      allocation.get_width(), allocation.get_height());
  //if(forcer_position && (position != -1))
    //this->set_position(position);
    //this->set_position(100);

    //this->set_position((position * w) / largeur);
  HPaned::on_size_allocate(allocation);
  //if(forcer_position && (position != -1))
    //this->set_position(100);
    //this->set_position(position);
    //this->set_position((position * w) / largeur);


  //set_position(3);
}

/*void set_position(int position)
{

}*/

MainDock::MainDock(utils::model::Node modele): hpaned(false)
{
  infos("Creation du MainDock...");

  this->modele = modele;

	hpaned.add_child(*(vboxes[0].get_widget()));
	hpaned.add_child(vbox);
	hpaned.add_child(*(vboxes[1].get_widget()));

  std::vector<Gtk::TargetEntry> cibles;
  cibles.push_back( Gtk::TargetEntry("STRING") );
  cibles.push_back( Gtk::TargetEntry("text/plain") );

  vboxes[0].get_widget()->drag_dest_set(cibles);
  vboxes[0].get_widget()->signal_drag_data_received().connect(sigc::mem_fun(*this,
                  &MainDock::gere_drag_data_received_gauche));
  //gauche.signal_drag_begin()


  vboxes[1].get_widget()->drag_dest_set(cibles);
  vboxes[1].get_widget()->signal_drag_data_received().connect(sigc::mem_fun(*this,
                  &MainDock::gere_drag_data_received_droit));
  //vbox.signal_drag_motion().connect(sigc::mem_fun(*this,
  //    &MGCWnd::gere_drag_motion));


  infos("Ok.");
}

Gtk::Widget *MainDock::get_widget()
{
  return &hpaned;//&hpaned[0];
}

void VueDetachable::gere_drag_data_get(
         const Glib::RefPtr<Gdk::DragContext>& context,
         Gtk::SelectionData& selection_data, unsigned int info, unsigned int time)
{
  selection_data.set(selection_data.get_target(), 8 /* 8 bits format */,
            (const unsigned char*) controle->id.c_str(),
            controle->id.size());


  //const Glib::RefPtr< Gdk::Cursor > cursor = Gdk::Cursor::create(Gdk::HAND1);
  //this->wnd.get_window()->set_cursor(cursor);
  //wnd.get_window()->set_cursor()
}


bool VueDetachable::gere_drag_motion(
         const Glib::RefPtr<Gdk::DragContext>& context,
         int x, int y, unsigned int time)
{
  infos("Drag motion (%d, %d)", x, y);
  return true;
}


bool VueDetachable::est_visible()
{
  return visible;
}



bool VueDetachable::on_expose_event(const Cairo::RefPtr<Cairo::Context> &cr)
{
  if(!expose)
  {
    expose = true;
    if(!docquee)
    {
      auto md = controle->modele_dyn;
      int x, y, l, h;
      x = md.get_attribute_as_int("x");
      y = md.get_attribute_as_int("y");
      l = md.get_attribute_as_int("largeur");
      h = md.get_attribute_as_int("hauteur");
      visible = md.get_attribute_as_boolean("visible");
      maj_vue();
      auto win = wnd.get_window();
      if(win)
      {
        wnd.move(x, y);
        wnd.resize(l, h);
      }
    }
  }
  return true;
}



void VueDetachable::maj_vue()
{
  if(docquee)
  {

  }
  else
  {
    wnd.set_title(controle->modele.get_localized_name());
    if(visible)
    {
      wnd.show();
      wnd.present();
    }
    else
    {
      wnd.hide();
    }
  }
}

MyFrame::MyFrame()
//    b_fermer(Gtk::Stock::CLOSE),
//    b_dedocquer(Gtk::Stock::QUIT)
{
  widget_en_cours = nullptr;

  std::string pt = utils::get_fixed_data_path() + PATH_SEP + "img/fermer.png";

  if(!utils::files::file_exists(pt))
  {
    erreur("Fichier non trouvé : %s", pt.c_str());
  }

  Gtk::Image *i1 = new Gtk::Image(utils::get_fixed_data_path() + PATH_SEP + "img/fermer.png");
  b_fermer.set_image(*i1);
  Gtk::Image *i2 = new Gtk::Image(utils::get_fixed_data_path() + PATH_SEP + "img/dedocquer.png");
  b_dedocquer.set_image(*i2);

  titre.show();
  b_dedocquer.show();
  b_fermer.show();
  hbox.show();
  hbox2.show();
  //b_dedocquer = Gtk::ToolButton(Gtk::Stock::QUIT);
  //b_fermer    = Gtk::ToolButton(Gtk::Stock::CLOSE);

  // 153, 180, 209

  std::string css = "* { background: #99B4D1; background-color: #99B4D1; color: #ffffff; }";
  auto p = Gtk::CssProvider::create();
  try
  {
      p->load_from_data(css);
      titre.get_style_context()->add_provider(p, GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
      b_fermer.get_style_context()->add_provider(p, GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
      b_dedocquer.get_style_context()->add_provider(p, GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
      hbox.get_style_context()->add_provider(p, GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
      evbox.get_style_context()->add_provider(p, GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
  }
  catch(const Glib::Error& ex)
  {
    erreur("Error, Gtk::CssProvider::load_from_path() failed: %s",
              ex.what().c_str());
  }
  catch(...)
  {
    erreur("Erreur add_provider.");
  }

  evbox.add(titre);
  titre.set_hexpand(true);
  hbox.pack_start(evbox, Gtk::PACK_EXPAND_WIDGET);

  hbox.pack_start(b_dedocquer, Gtk::PACK_SHRINK);
  hbox.pack_start(b_fermer, Gtk::PACK_SHRINK);
  b_dedocquer.set_halign(Gtk::ALIGN_END);
  b_fermer.set_halign(Gtk::ALIGN_END);
  hbox.show();
  hbox.set_hexpand(true);
  hbox.show_all_children(true);
  pack_start(hbox, Gtk::PACK_SHRINK);
  hbox.set_hexpand(true);
}

void MyFrame::set_titre(std::string s)
{
  titre.set_use_markup(true);
  titre.set_label("<b>" + s + "</b>");
}

void MyFrame::add(Gtk::Widget &w)
{
  pack_start(w, Gtk::PACK_EXPAND_WIDGET);
  widget_en_cours = &w;
}

void MyFrame::remove()
{
  if(widget_en_cours != nullptr)
    Gtk::VBox::remove(*widget_en_cours);
}

void VueDetachable::maj_langue()
{
  std::string ctrl_nom = controle->titre.get_localized();

  wnd.set_title(ctrl_nom);
  drag_frame.set_titre(ctrl_nom);

  controle->maj_langue();
}

VueDetachable::VueDetachable(Controle *controle, MainDock *main_dock)
{
  this->main_dock = main_dock;
  doc_en_cours = 0;
  docquee = false;
  expose = false;
  this->controle = controle;
  controle->widget->show_all();
  docquable = controle->modele.get_attribute_as_boolean("docable");
  std::string ctrl_id = controle->titre.get_id();
  std::string ctrl_nom = controle->titre.get_localized();
  infos("Création vue détachable pour controle [%s] [%s]...", ctrl_id.c_str(), ctrl_nom.c_str());

  visible = false;

  wnd.set_title(ctrl_nom);
  drag_frame.set_titre(ctrl_nom);

  //if(docquable)
    //wnd.set_decorated(false);

  drag_frame.b_dedocquer.signal_clicked().connect(sigc::mem_fun(*this, &VueDetachable::gere_dedocquage));
  drag_frame.b_fermer.signal_clicked().connect(sigc::mem_fun(*this, &VueDetachable::gere_fermeture));


  if(docquable)
    drag_frame.add(*(controle->widget));


  std::vector<Gtk::TargetEntry> cibles;
  cibles.push_back( Gtk::TargetEntry("STRING") );
  cibles.push_back( Gtk::TargetEntry("text/plain") );

  drag_frame.drag_source_set(cibles);
  drag_frame.drag_source_add_text_targets();
  drag_frame.signal_drag_data_get().connect(sigc::mem_fun(*this,
                &VueDetachable::gere_drag_data_get));

  controle->widget->drag_source_set(cibles);
  controle->widget->drag_source_add_text_targets();
  controle->widget->signal_drag_data_get().connect(sigc::mem_fun(*this,
              &VueDetachable::gere_drag_data_get));
  controle->widget->signal_drag_motion().connect(sigc::mem_fun(*this,
      &VueDetachable::gere_drag_motion));
  controle->widget->signal_draw().connect(sigc::mem_fun(*this, &VueDetachable::on_expose_event));


  wnd.signal_delete_event().connect(sigc::mem_fun(*this, &VueDetachable::gere_evt_delete));

  //controle->widget->drag_source_set_icon(Gtk::StockID("gtk-about"));
  //controle->widget->drag_highlight()
}


}}

