#include "mmi/gtkutil.hpp"

namespace utils{ namespace mmi{

static std::string chemin_fichier_lock;

int verifie_dernier_demarrage()
{
  chemin_fichier_lock = utils::get_current_user_path() + PATH_SEP + "lock.dat";
  if(utils::files::file_exists(chemin_fichier_lock))
  {
#   if MODE_RELEASE
    auto &sec = utils::langue.get_section("svg-log");
    if(utils::mmi::dialogs::check_dialog(
        sec.get_item("check-lock-1"),
        sec.get_item("check-lock-2"),
        sec.get_item("check-lock-3")))
    {
      auto s = utils::mmi::dialogs::enregistrer_fichier(sec.get_item("svg-log-titre"),
          ".txt", "Log file");
      if(s.size() > 0)
      {
        if(utils::files::get_extension(s).size() == 0)
          s += ".txt";
        //std::string src = utils::get_current_user_path() + PATH_SEP + appdata.nom_appli + "-log.txt";
        //std::string src = utils::get_current_user_path() + PATH_SEP + "-log.txt.old";
        //trace_majeure("Copie [%s] <- [%s]...", s.c_str(), src.c_str());
        //utils::files::copy_file(s, src);
      }
    }
#   endif
    return -1;
  }
  else
  {
    utils::files::save_txt_file(chemin_fichier_lock, "En cours d'execution.");
    return 0;
  }
}


int termine_appli()
{
  utils::files::delete_file(chemin_fichier_lock);
  return 0;
}


}}

