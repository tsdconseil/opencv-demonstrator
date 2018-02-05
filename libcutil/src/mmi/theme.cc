#include "cutil.hpp"
#include "mmi/theme.hpp"
#include "mmi/gtkutil.hpp"


#ifndef LINUX
#ifndef MSYS1
# define MSYS2
#endif
#endif

//#define DISABLE_THEME 1

// Ajouter settings.ini dans le dossier
// "C:\msys32\mingw32\share\gtk-3.0"
// Contenant :
// [Settings]
// gtk-theme-name=win32

namespace utils{ namespace mmi{

std::vector<Theme> themes;

static Theme *theme_en_cours = nullptr;

class ThemeCB
{
public:
#if 0
void on_parsing_error(const Glib::RefPtr<const Gtk::CssSection>& section, const Glib::Error& error)
{
  infos("on_parsing_error(): %s",error.what().c_str());
  /*
  if (section)
  {
    const auto file = section->get_file();
    if (file)
    {
      std::cerr << "  URI = " << file->get_uri() << std::endl;
    }

    std::cerr << "  start_line = " << section->get_start_line()+1
              << ", end_line = " << section->get_end_line()+1 << std::endl;
    std::cerr << "  start_position = " << section->get_start_position()
              << ", end_position = " << section->get_end_position() << std::endl;
  }
  */
}
#endif
};


int charge_themes()
{

# ifndef MSYS2
  return -1;
# else

  // Chargement des CSS provider...

  std::string chms[2] =
  {
      "/arc-theme-master/common/gtk-3.0/3.20/gtk-dark.css",
      "/arc-theme-master/common/gtk-3.0/3.20/gtk-solid-dark.css"
  };

  std::string noms[2] = {"dark", "solid-dark"};

  /*Gtk::Window *wnd_tmp = new Gtk::Window();
  wnd_tmp->show();
  wnd_tmp->present();*/

  for(auto i = 0u; i < 2; i++)
  {



  Theme theme;
  theme.id = noms[i];//th.get_attribute_as_string("name");
  theme.chemin = "...";//th.get_attribute_as_string("chemin");
  theme.provider = Gtk::CssProvider::create();
  theme.desc = "...";//th.get_localized_name();
  //ThemeCB cb;
  //theme.provider->signal_parsing_error().connect(sigc::mem_fun(cb, &ThemeCB::on_parsing_error));
  std::string fn = //utils::get_fixed_data_path() + "/themes/" + theme.chemin;

      utils::get_fixed_data_path() + chms[i];

  // darker : aucun effet
  // dark : pb boutons et barre de titre

//  + "/themes/arc-theme-master/common/gtk-3.0/3.20/gtk-dark.css";
  //+ "/elem/elementaryDark/gtk-3.0/gtk.css";

  if(!utils::files::file_exists(fn))
  {
    erreur("Fichier css non trouv� (%s).", fn.c_str());
    return -1;
  }
  try
  {
     infos("Chargement du theme [%s]...", fn.c_str());
     theme.provider->load_from_path(fn);
     infos("Ok.");

     //auto refStyleContext = wnd_tmp->get_style_context();
     //refStyleContext->add_provider_for_screen(wnd_tmp->get_screen(), theme.provider,
       //   GTK_STYLE_PROVIDER_PRIORITY_USER+10);

  }
  catch(const Gtk::CssProviderError& ex)
  {
    erreur("CssProviderError, Gtk::CssProvider::load_from_path() failed: %s",
              ex.what().c_str());
  }
  catch(const Glib::Error& ex)
  {
    erreur("Error, Gtk::CssProvider::load_from_path() failed: %s",
              ex.what().c_str());
  }
  themes.push_back(theme);
  }

# if 0
  for(auto &th: mgc::app.modele_statique.children("theme"))
  {
    Theme theme;
    theme.id = th.get_attribute_as_string("name");
    theme.chemin = th.get_attribute_as_string("chemin");
    theme.provider = Gtk::CssProvider::create();
    theme.desc = th.get_localized_name();
    ThemeCB cb;
    theme.provider->signal_parsing_error().connect(sigc::mem_fun(cb, &ThemeCB::on_parsing_error));
    std::string fn = utils::get_fixed_data_path() + "/themes/" + theme.chemin;

    if(!utils::files::file_exists(fn))
    {
      erreur("Fichier css non trouv� (%s).", fn.c_str());
      return -1;
    }
    try
    {
       infos("Chargement du theme [%s]...", fn.c_str());
       theme.provider->load_from_path(fn);
       infos("Ok.");
    }
    catch(const Gtk::CssProviderError& ex)
    {
      erreur("CssProviderError, Gtk::CssProvider::load_from_path() failed: %s",
                ex.what().c_str());
    }
    catch(const Glib::Error& ex)
    {
      erreur("Error, Gtk::CssProvider::load_from_path() failed: %s",
                ex.what().c_str());
    }
    themes.push_back(theme);
  }
# endif

  //wnd_tmp->hide();


  return 0;
# endif
}

int installe_theme(std::string th, bool tactile)
{
  infos("installe theme (%s)...", th.c_str());

# ifndef MSYS2
  return -1;
  //avertissement("Themage desactivé !");
  //return 0;
# else
  if((th == "aucun") || (th.size() == 0))
    return 0;

  for(auto &theme: themes)
  {
    if(theme.id == th)
    {
      infos("Theme [%s] trouve.", th.c_str());
      //assert(ihm::Mmi::get_instance() != nullptr);
      //auto wnd = ihm::Mmi::get_instance()->engine;
      //assert(wnd != nullptr);

      /*auto refStyleContext = wnd->get_style_context();

      if(theme_en_cours != nullptr)
      {
        infos("Remove provider...");
        //if(refStyleContext->get_p)
        //refStyleContext->remove_provider_for_screen(wnd->get_screen(), theme_en_cours->provider);
        refStyleContext->remove_provider(theme_en_cours->provider);
        infos("Ok.");
      }*/

    //  refStyleContext->add_provider(css_prov,
    //      GTK_STYLE_PROVIDER_PRIORITY_USER+10);


      Gtk::Window wnd_tmp;// = new Gtk::Window();
      wnd_tmp.show();
      wnd_tmp.present();
      auto refStyleContext = wnd_tmp.get_style_context();

      //auto refStyleContext = Gtk::StyleContext::create();

      infos("Add provider...");
      refStyleContext->add_provider_for_screen(wnd_tmp.get_screen(), theme.provider,
           GTK_STYLE_PROVIDER_PRIORITY_USER+10);
      infos("ok.");
      wnd_tmp.hide();
      theme_en_cours = &theme;
      return 0;
    }
  }
  return -1;
# endif
}



#if 0
int installe_theme(int th, bool tactile)
{
  // A FAIRE
  // Supporter au moins :
  // - THEME_STD_WINDOWS + !tactile
  // - THEME_FOND_NOIR   + tactile



  infos("%s(%d,%s)...", __func__, (int) th, tactile ? "tactile" : "non tactile");

# if 0
  auto css_prov = Gtk::CssProvider::create();

  ThemeCB cb;
  css_prov->signal_parsing_error().connect(sigc::mem_fun(cb, &ThemeCB::on_parsing_error));


  std::string pts[NB_THEMES] =
  {
      "arc-theme-3.20/gtk.css",
      "arc-theme-3.20/gtk-dark.css",
      "arc-theme-3.20/gtk-darker.css",
      "arc-theme-3.20/gtk-solid.css",
      "arc-theme-3.20/gtk-solid-dark.css",
      "arc-theme-3.20/gtk-solid-darker.css",
      "gtk-theme-ubuntustudio-legacy-master/UbuntuStudio_Legacy/gtk-3.20/gtk.css",
      //"essai/gtk.css"
  };

  if(((int) th) >= NB_THEMES)
  {
    erreur("Num�ro de theme invalide : %d.", (int) th);
    return -1;
  }

  std::string chemin_theme = pts[(int) th];


   try
   {
      infos("Chargement du theme [%s]...", fn.c_str());
      css_prov->load_from_path(fn);
      infos("Ok.");
   }
   catch(const Gtk::CssProviderError& ex)
   {
     erreur("CssProviderError, Gtk::CssProvider::load_from_path() failed: %s",
               ex.what().c_str());
   }
   catch(const Glib::Error& ex)
   {
     erreur("Error, Gtk::CssProvider::load_from_path() failed: %s",
               ex.what().c_str());
   }

  auto wnd = mgc::vue::MGCWnd::get_instance();
  auto refStyleContext = wnd->get_style_context();

//  refStyleContext->add_provider(css_prov,
//      GTK_STYLE_PROVIDER_PRIORITY_USER+10);
  refStyleContext->add_provider_for_screen(wnd->get_screen(), css_prov,
       GTK_STYLE_PROVIDER_PRIORITY_USER+10);

  for(auto &ctr: mgc::vue::MGCWnd::get_instance()->controles)
  {
    auto wnd2 = &(ctr->wnd);
    refStyleContext = wnd2->get_style_context();
//    refStyleContext->add_provider(css_prov,
//        GTK_STYLE_PROVIDER_PRIORITY_USER+10);
    refStyleContext->add_provider_for_screen(wnd2->get_screen(), css_prov,
       GTK_STYLE_PROVIDER_PRIORITY_USER+10);
  }


  //wnd->show();
# endif

  return 0;
}
#endif

}}



