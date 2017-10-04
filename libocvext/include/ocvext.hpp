#ifndef MISC_H
#define MISC_H

#include "opencv2/core.hpp"
#include <string>
#include "modele.hpp"

namespace ocvext
{

class GestionnaireEvenementGenerique
{
public:
  virtual ~GestionnaireEvenementGenerique(){}
  virtual void maj_infos(const std::string &infos) = 0;
  virtual void maj_infos(const std::string &infos, const cv::Mat &img) = 0;
};

extern GestionnaireEvenementGenerique *gestionnaire_evenement;


extern void infos_demarre_log(utils::model::Node &res, const std::string &id);
extern void infos_arrete_log();

//extern utils::model::Node infos_get_log();


extern void infos_entre_algo(const std::string &nom_algo);
extern void infos_sors_algo();
extern void infos_progression(const std::string &infos, ...);
extern void infos_progression(const std::string &infos, const cv::Mat &img, bool normaliser = false);

struct Config
{
  std::string dout;
  unsigned int img_cnt;
  bool affiche_images_intermediaires;
  bool enreg_images_intermediaires;

# ifdef WINDOWS
  LARGE_INTEGER base_tick, frequency;
# endif
  Config();
};

extern Config config;

extern void init(bool affiche_img_interm,
                 bool stocke_img_interm,
                 const std::string &dossier_stockage = "");

extern void defini_dossier_stockage(const std::string &chemin);
extern void defini_options_debogage(bool affiche_img_interm,
                                     bool stocke_img_interm);
extern bool est_debogage_actif();

extern void dbg_image(const std::string &name,
                      const cv::Mat &m,
                      bool normalize = false,
                      const std::string &titre = "",
                      const std::string &description = "");

extern cv::Mat imread(const std::string &fn);

extern void redim_preserve_aspect(const cv::Mat &I,
                                  cv::Mat &O,
                                  const cv::Size &dim,
                                  const cv::Scalar &bgcolor = cv::Scalar(0));

extern void plot_1d(cv::Mat &image, const cv::Mat &x_, cv::Scalar color);

struct MultiPlot
{
  MultiPlot();
  // sx,sy = taille par case
  MultiPlot(uint16_t nx, uint16_t ny, uint16_t sx, uint16_t sy);
  void init(uint16_t nx, uint16_t ny, uint16_t sx, uint16_t sy);
  void ajoute(std::string nom, const cv::Mat &I, uint16_t ncols = 1);
  void creation_auto(const cv::Mat &I0, const cv::Mat &I1);
  uint16_t nx, ny, sx, sy;
  uint16_t cnt;
  cv::Mat img;
};

extern void adapte_en_bgr(const cv::Mat &I, cv::Mat &O);

extern void affiche_dans_cadre(const cv::Mat &I, cv::Mat &cadre, cv::Size taille_vue, const cv::Scalar &arriere_plan);

}

#endif
