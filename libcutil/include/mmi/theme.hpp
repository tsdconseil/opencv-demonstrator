#ifndef THEME_H
#define THEME_H

#ifdef MSYS2
#include <gtkmm/cssprovider.h>
#include <gtkmm/styleproperty.h>
#endif

namespace utils{ namespace mmi{

struct Theme
{
  std::string id;
  std::string desc;
  std::string chemin;
# ifdef MSYS2
  Glib::RefPtr<Gtk::CssProvider> provider;
# endif
};

extern std::vector<Theme> themes;

extern int charge_themes();

extern int installe_theme(std::string th, bool tactile = false);



}}

#endif

