#include "erreurs.hpp"
#include "cutil.hpp"
#include "mmi/gtkutil.hpp"

namespace utils {

struct EvErreur
{
  unsigned int id;
  std::string titre, description;
};



static std::vector<Erreur> erreurs;
Erreur indef;

static int erreurs_charge(const std::string &chemin);


std::vector<EvErreur> pile_erreur;

void signale_erreur(unsigned int id, ...)
{
  va_list ap;
  va_start(ap, id);


  EvErreur ev;
  ev.id = id;

  auto e = erreur_get(id);

  char tampon[1000];
  vsnprintf(tampon, 1000, e.locale.get_description(utils::model::Localized::LANG_CURRENT).c_str(), ap);

  ev.description = std::string(tampon);
  ev.titre = e.locale.get_localized();
  pile_erreur.push_back(ev);

  auto s = "Erreur detectee : " + ev.titre + "\n" + ev.description;
  gen_trace(utils::journal::TraceLevel::AL_ANOMALY, "", s);

  va_end(ap);
}

void affiche_pile_erreurs()
{
  if(pile_erreur.size() == 0)
    return;

  //auto e = erreur_get(id);
  //auto s = e.locale.get_localized();

  int dernier = pile_erreur.size() - 1;

  Gtk::MessageDialog dial(pile_erreur[dernier].titre,
                          true,
                          Gtk::MESSAGE_ERROR,
                          Gtk::BUTTONS_OK,
                          true);

  dial.set_title("Erreur");//pile_erreur[dernier].titre);//"Erreur");

  std::string s = "", s2 = "";

  s += std::string("<b>") + pile_erreur[dernier].titre + "</b>\n";
  s += pile_erreur[dernier].description;

  //if(pile_erreur.size() > 1)
    //s += "\"

  for(auto i = 1u; i < pile_erreur.size(); i++)
  {
    auto &e = pile_erreur[dernier-i];
    //if(i > 0)
    s2 += std::string("<b>") + e.titre + "</b>\n";
    s2 += e.description;
    if(i + 1 < pile_erreur.size())
      s2 += "\n";
  }

  dial.set_message(s, true);
  if(s2.size() > 0)
    dial.set_secondary_text(s2, true);
  dial.set_position(Gtk::WIN_POS_CENTER);
  utils::mmi::DialogManager::setup_window(&dial);
  dial.run();
  pile_erreur.clear();
}



static int erreurs_charge(const utils::model::MXml &mx)
{
  indef.id = 0xffffffff;
  indef.locale.set_value(utils::model::Localized::LANG_FR, "Code d'erreur non trouvé.");

  auto lst = mx.get_children("charge");

  for(auto &c: lst)
    erreurs_charge(utils::get_fixed_data_path() + "/" + c.get_attribute("fichier").to_string());

  lst = mx.get_children("erreur");
  for(auto &e: lst)
  {
    Erreur err;
    err.locale = utils::model::Localized(e);
    err.id = e.get_attribute("id").to_int();
    erreurs.push_back(err);
  }
  return 0;
}

static int erreurs_charge(const std::string &chemin)
{
  if(!utils::files::file_exists(chemin))
  {
    erreur("Fichier d'erreur non trouve.");
    return -1;
  }

  utils::model::MXml mx;
  if(mx.from_file(chemin))
  {
    erreur("Erreur lors du chargement du fichier d'erreur.");
    return -1;
  }

  return erreurs_charge(mx);
}


int erreurs_charge()
{
  if(erreurs_charge(utils::get_fixed_data_path() + "/" + "erreurs.xml"))
    return -1;

  infos("Charge %d messages d'erreurs.", erreurs.size());

  return 0;
}


int erreur_affiche(unsigned int id)
{
  auto e = erreur_get(id);
  auto s = e.locale.get_localized();
  avertissement("Erreur detectee : [%s]", s.c_str());

  //utils::mmi::dialogs::affiche_erreur():
  Gtk::MessageDialog dial(s,
                          false,
                          Gtk::MESSAGE_ERROR,
                          Gtk::BUTTONS_CLOSE,
                          true);
  dial.set_title("Erreur");
  dial.set_secondary_text(e.locale.get_description());
  dial.set_position(Gtk::WIN_POS_CENTER);
  utils::mmi::DialogManager::setup_window(&dial);
  dial.run();
  return 0;
}




Erreur &erreur_get(unsigned int id)
{
  for(auto i = 0u; i < erreurs.size(); i++)
    if(erreurs[i].id == id)
      return erreurs[i];
  return indef;
}

}

