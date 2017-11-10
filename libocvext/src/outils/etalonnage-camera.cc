#include "calib.hpp"
#include "mmi/theme.hpp"


int main(int argc, const char **argv)
{
  utils::init(argc, argv, "ocvext");
  ocvext::init(false, false);

  Gtk::Main kit(0, nullptr);
  utils::langue.load(utils::get_fixed_data_path() + "/std-lang.xml");
  utils::langue.load(utils::get_fixed_data_path() + "/ocvext-lang.xml");
  utils::mmi::charge_themes();
  utils::mmi::installe_theme("dark");

  cv::setBreakOnError(true);

  ocvext::DialogueCalibration dc;
  dc.affiche();
  return 0;
}

