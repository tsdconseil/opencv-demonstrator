#include "opencv2/core.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include "cutil.hpp"

#include "../include/ocvext.hpp"
#ifdef WINDOWS
# include <windows.h>
#endif


namespace ocvext
{


GestionnaireEvenementGenerique *gestionnaire_evenement = nullptr;
static utils::model::Node noeud_en_cours;
static std::string chemin_tmp;
static unsigned int id_img = 0;
static bool log_en_cours = false;
static int niveau_indent = 0;
static std::string algo_en_cours;

void infos_demarre_log(utils::model::Node &res, const std::string &id)
{
  noeud_en_cours = res;
  chemin_tmp = utils::get_current_user_path() + PATH_SEP + "tmp";
  utils::files::creation_dossier(chemin_tmp);
  id_img = 0;
  log_en_cours = true;
  niveau_indent = 0;
}

void infos_arrete_log()
{
  noeud_en_cours = utils::model::Node();
  log_en_cours = false;
}

static std::string fais_indent()
{
  std::string idt;
  for(auto i = 0; i < niveau_indent; i++)
    idt += "  ";
  return idt;
}

void infos_entre_algo(const std::string &nom_algo)
{
  if(log_en_cours)
  {
    auto idt = fais_indent();
    algo_en_cours = nom_algo;
    trace_majeure("%sDebut algorithme [%s]", idt.c_str(), nom_algo.c_str());
    noeud_en_cours = noeud_en_cours.add_child("img-collection");
    noeud_en_cours.set_attribute("name", nom_algo);
    niveau_indent++;
  }
}

void infos_sors_algo()
{
  if(log_en_cours)
  {
    auto idt = fais_indent();
    infos("%sFin algorithme [%s]", idt.c_str(), algo_en_cours.c_str());
    noeud_en_cours = noeud_en_cours.parent();
    niveau_indent--;
    if(niveau_indent < 0)
    {
      erreur("niveau indent = %d !", niveau_indent);
      niveau_indent = 0;
    }
  }
}

void infos_progression(const std::string &infos, ...)
{
  va_list ap;
  va_start(ap, infos);
  auto idt = fais_indent();
  //infos("%s%s", idt.c_str(), infos.c_str());

  char buf[2000];
  vsnprintf(buf, 2000, infos.c_str(), ap);

  //printf(idt.c_str());
  //printf("%s\n", buf);

  infos("%s%s", idt.c_str(), buf);

  //utils::journal::gen_trace(utils::journal::AL_NORMAL,  "",
  //    utils::journal::journal_principal, std::string(buf));

  if(log_en_cours)
  {
    auto n = noeud_en_cours.add_child("log-item");
    n.set_attribute("name", infos);
  }
  if(gestionnaire_evenement != nullptr)
    gestionnaire_evenement->maj_infos(infos);
  va_end(ap);
}

void infos_progression(const std::string &infos, const cv::Mat &img, bool normaliser)
{
  if(img.cols == 0)
  {
    erreur("infos_progression (%s): image vide.", infos.c_str());
    return;
  }

  if(log_en_cours)
  {
    cv::Mat m2;
    if(normaliser)
    {
      cv::normalize(img, m2, 0, 255.0, cv::NORM_MINMAX);
    }
    else
      m2 = img;

    char bf[1000];
    cv::Mat O;
    ocvext::adapte_en_bgr(m2, O);

    sprintf(bf, "%s/%d.png", chemin_tmp.c_str(), id_img);

    cv::imwrite(bf, O);

    id_img++;

    auto n = noeud_en_cours.add_child("img-spec");
    n.set_attribute("name", infos);
    n.set_attribute("chemin", std::string(bf));
  }

  //ocvext::dbg_image(infos, img);
  if(gestionnaire_evenement != nullptr)
    gestionnaire_evenement->maj_infos(infos, img);
}

// Display intermediate image (debug mode only)
void dbg_image(const std::string &name,
               const cv::Mat &m,
               bool normalize,
               const std::string &titre,
               const std::string &description)
{
  infos_progression(name, m, normalize);

# if 0
  if(config.affiche_images_intermediaires)
  {
    cv::namedWindow(name.c_str(), CV_WINDOW_NORMAL);
    cv::imshow(name.c_str(), m);
    cv::waitKey(5);
  }

  //bool img_ess = config.enreg_images_essentielles && (titre.size() > 0) && (current_htsec != nullptr);

  if(config.enreg_images_intermediaires)// || img_ess)
  {
    char path[200];
    sprintf(path,
            "%s/%d-%s.png",
            config.dout.c_str(),
            config.img_cnt++,
            name.c_str());
    cv::Mat m2;
    if(normalize)
    {
      cv::normalize(m, m2, 0, 255.0, cv::NORM_MINMAX);
    }
    else
    {
      if(m.depth() == CV_32F)
        m2 = m * 255;
      else
        m2 = m.clone();
    }

    if(m2.channels() == 1)
      cv::cvtColor(m2, m2, CV_GRAY2BGR);
    m2.convertTo(m2, CV_8U);

    if(config.enreg_images_intermediaires)
      cv::imwrite(path, m2);
  }
# endif
}

Config config;


Config::Config()
{
  img_cnt                       = 0;
  affiche_images_intermediaires = false;
  enreg_images_intermediaires   = false;
  /*enreg_images_importantes      = false;
  enreg_images_essentielles     = false;
  ess_cnt                       = 0;*/
  //dout  = "./build/img-out";

# ifdef WINDOWS
  QueryPerformanceFrequency(&frequency);
  QueryPerformanceCounter(&base_tick);
# endif
}

void init(bool affiche_img_interm,
          bool stocke_img_interm,
          const std::string &dossier_stockage)

{
  defini_options_debogage(affiche_img_interm, stocke_img_interm);
  if(dossier_stockage == "")
    defini_dossier_stockage("./build/img-out");
  else
    defini_dossier_stockage(dossier_stockage);
}

void defini_dossier_stockage(const std::string &chemin)
{
  config.dout    = chemin;
  config.img_cnt = 0;

  if(config.enreg_images_intermediaires)
  {
    infos("set_output_folder(%s)", chemin.c_str());
    utils::files::creation_dossier(chemin);
    utils::proceed_syscmde("rm %s/*.png", chemin.c_str());
    utils::proceed_syscmde("rm %s/*.jpg", chemin.c_str());
  }
}

bool est_debogage_actif()
{
  return config.affiche_images_intermediaires || config.enreg_images_intermediaires;
}

void defini_options_debogage(bool show_debug_view,
                              bool store_debug_view)
{
  config.affiche_images_intermediaires = show_debug_view;
  config.enreg_images_intermediaires = store_debug_view;
}




cv::Mat imread(const std::string &fn)
{
  //infos("Lecture image @%s...", fn.c_str());

  cv::Mat res = cv::imread(fn);

  if(res.cols == 0)
  {
    const char *s1 = "le fichier n'existe pas";
    const char *s2 = "le fichier existe";
    const char *s = utils::files::file_exists(fn) ? s2 : s1;
    erreur("erreur lors de l'image de [%s] (%s).", fn.c_str(), s);
  }

  return res;
}

void redim_preserve_aspect(const cv::Mat &I,
    cv::Mat &O,
    const cv::Size &dim, const cv::Scalar &bgcolor)
{
  float h1 = dim.width * (I.rows / (float) I.cols);
  float w2 = dim.height * (I.cols / (float) I.rows);

  if(h1 <= dim.height)
    cv::resize(I, O, cv::Size(dim.width, h1));
  else
    cv::resize(I, O, cv::Size(w2, dim.height));
}




void affiche_dans_cadre(const cv::Mat &I, cv::Mat &cadre, cv::Size taille_vue,
                        const cv::Scalar &arriere_plan,
                        cv::Point &p0, float &ratio)
{
  auto taille_video = I.size();
  cadre.create(taille_vue, CV_8UC4);

  //trace_verbeuse("affiche_dans_cadre...");

  // Redimensionnnement de la vidÃ©o dans le cadre
  // avec prÃ©servation du ratio d'aspect

  float ratio_x = ((float) taille_vue.width) / taille_video.width;
  float ratio_y = ((float) taille_vue.height) / taille_video.height;

  //float mratio = (ratio_x < ratio_y) ? ratio_x : ratio_y;

  cv::Rect rdi;

  cadre.setTo(arriere_plan);

  if(ratio_x < ratio_y) // bandes horizontales en haut et en bas
  {
    rdi.width = taille_vue.width;
    rdi.x = 0;
    rdi.height = taille_video.height * ratio_x;
    rdi.y = (taille_vue.height - rdi.height) / 2;
    ratio = ratio_x;
  }
  else // bandes verticales en haut et en bas
  {
    rdi.width = taille_video.width * ratio_y;
    rdi.x = (taille_vue.width - rdi.width) / 2;
    rdi.height = taille_vue.height;
    rdi.y = 0;
    // si ry > 0 : pb : rdi.width > taille_vue.width !!!
    ratio = ratio_y;
  }
  p0 = cv::Point(rdi.x,rdi.y);

  if(rdi.width * rdi.height == 0)
    return;

  /*trace_verbeuse("rx = %f, ry = %f,\nresize(%d,%d,%d,%d) -> %d,%d",
      ratio_x, ratio_y, rdi.x, rdi.y, rdi.width, rdi.height,
      taille_vue.width, taille_vue.height);*/

  if(I.cols * I.rows == 0)
  {
    erreur("I.cols = %d, I.rows = %d.", I.cols, I.rows);
    return;
  }

  cv::resize(I, cadre(rdi), rdi.size());

  //trace_verbeuse("ok.");
}

void affiche_dans_cadre(const cv::Mat &I, cv::Mat &cadre, cv::Size taille_vue, const cv::Scalar &arriere_plan)
{
  cv::Point p0;
  float ratio;
  affiche_dans_cadre(I, cadre, taille_vue, arriere_plan, p0, ratio);
}

void plot_1d(cv::Mat &image, const cv::Mat &x_, cv::Scalar color)
{
  cv::Mat x = x_.clone();
  if((x.rows == 1) && (x.cols > 1))
    x = x_.t();

  uint32_t n = x.rows;
  float sx = image.cols, sy = image.rows;



  float lxi = x.at<float>(0);
  for(auto i = 1u; i < n; i++)
  {
    float xi = x.at<float>(i);

    //printf("xi = %f\n", xi);

    cv::line(image,
             cv::Point((i-1)* sx / n,sy-lxi),
             cv::Point(i * sx / n,sy-xi),
             color, 1, CV_AA);

    lxi = xi;
  }
}


MultiPlot::MultiPlot()
{
  nx = ny = sx = sy = cnt = 0;
}

MultiPlot::MultiPlot(uint16_t nx, uint16_t ny, uint16_t sx, uint16_t sy)
{
  init(nx, ny, sx, sy);
}

void MultiPlot::init(uint16_t nx, uint16_t ny, uint16_t sx, uint16_t sy)
{
  this->nx = nx;
  this->ny = ny;
  this->sx = sx;
  this->sy = sy;
  img.create(cv::Size(sx*nx,sy*ny), CV_8UC3);
  cnt = 0;
}

void adapte_en_bgr(const cv::Mat &I, cv::Mat &O)
{
  //infos("Adaptation en BGR: type entree = %d", I.type());
  cv::Mat O1;

  if(I.channels() == 1)
    cv::cvtColor(I, O1, CV_GRAY2BGR);
  else
    O1 = I.clone();

  if(O1.type() == CV_32FC3)
  {
    cv::normalize(O1, O1, 0, 255, cv::NORM_MINMAX);
    O1.convertTo(O, CV_8UC3);
  }
  else
    O = O1;
  //infos("type sortie = %d", O.type());
}

void MultiPlot::ajoute(std::string nom, const cv::Mat &I, uint16_t ncols)
{
  cv::Mat tmp;
  adapte_en_bgr(I, tmp);
  uint16_t col   = cnt % nx;
  uint16_t ligne = cnt / nx;

  cv::resize(tmp, img(cv::Rect(col*sx,ligne*sy,sx*ncols,sy)), cv::Size(sx*ncols,sy));

  int baseline = 0;
  cv::Size sz = cv::getTextSize(nom.c_str(),
      cv::FONT_HERSHEY_COMPLEX, 0.8, 1, &baseline);
  baseline += 1;

  img(cv::Rect(col*sx,ligne*sy + 2,std::min(sz.width,(int)sx*ncols),std::min(sz.height+baseline,(int)sy))) /= 3;

  putText(img, nom.c_str(), cv::Point(col*sx,ligne*sy+sz.height + 2),
      cv::FONT_HERSHEY_COMPLEX, 0.8,
      cv::Scalar(0,255,255),1,CV_AA);
  cnt += ncols;
}

void MultiPlot::creation_auto(const cv::Mat &I0, const cv::Mat &I1)
{
  uint32_t ncols, nlignes;

  if(I0.cols > I0.rows)
  {
    ncols   = 1;
    nlignes = 2;
  }
  else
  {
    ncols   = 2;
    nlignes = 1;
  }
  init(ncols,nlignes,std::max(I0.cols, I1.cols), std::max(I0.rows,I1.rows));
  ajoute("", I0);
  ajoute("", I1);
}


}

