#include "fourier.hpp"
#include "cutil.hpp"
#include "opencv2/imgproc.hpp"
#include <assert.h>
#include "../include/ocvext.hpp"

namespace ocvext
{


void translation_rapide(const cv::Mat &I, cv::Mat &O,
                        int decx, int decy,
                        const cv::Size &dim_sortie,
                        const cv::Scalar &bg)
{
  O.create(dim_sortie, I.type());

  // decx > 0 => image décalée à droite

  // Deux rectangle de même dimension
  cv::Rect rdst, rsrc;

  // A priori, on prend tout
  rsrc.x = 0;
  rsrc.y = 0;
  rsrc.width = I.cols;
  rsrc.height = I.rows;

  rdst.x = decx;
  rdst.y = decy;
  rdst.width  = O.cols - rdst.x;
  rdst.height = O.rows - rdst.y;

  if(rdst.x + rdst.width >= O.cols)
  {
    rdst.width = O.cols - 1 - rdst.x;
  }
  if(rdst.y + rdst.height >= O.rows)
  {
    rdst.height = O.rows - 1 - rdst.y;
  }

  // Maintenant on adapte les rectangles

  if(rdst.x < 0)
  {
    rsrc.x -= rdst.x;
    rsrc.width += rdst.x;
    rdst.x = 0;
  }

  if(rdst.y < 0)
  {
    rsrc.y -= rdst.y;
    rsrc.height += rdst.y;
    rdst.y = 0;
  }

  if(rdst.x + rdst.width >= O.cols)
  {
    rdst.width = O.cols - 1 - rdst.x;
  }
  if(rdst.y + rdst.height >= O.rows)
  {
    rdst.height = O.rows - 1 - rdst.y;
  }


  if(rdst.width > rsrc.width)
  {
    rdst.width = rsrc.width;
  }
  if(rdst.width < rsrc.width)
  {
    rsrc.width = rdst.width;
  }
  if(rdst.height > rsrc.height)
  {
    rdst.height = rsrc.height;
  }
  if(rdst.height < rsrc.height)
  {
    rsrc.height = rdst.height;
  }

  O.setTo(bg);

  //infos("src=%d,%d,%d,%d   dst=%d,%d,%d,%d", rsrc.x,rsrc.y,rsrc.width,rsrc.height)

  if(rsrc.width * rsrc.height > 0)
  {
    //infos("copyto: src = (%d,%d)[%d,%d,%d,%d], dst = (%d,%d)[%d,%d,%d,%d]...",
    //      I.cols, I.rows, rsrc.x,rsrc.y,rsrc.width,rsrc.height,
    //      O.cols, O.rows, rdst.x,rdst.y,rdst.width,rdst.height);
    I(rsrc).copyTo(O(rdst));
    //infos("ok.");
  }
    //O(rdst) = I(rsrc);
}

void translation(const cv::Mat &I, cv::Mat &O, int offsetx, int offsety,const cv::Scalar &bg)
{
  cv::Mat H = (cv::Mat_<double>(2,3) << 1, 0, offsetx, 0, 1, offsety);
  cv::warpAffine(I,O,H,I.size(),cv::INTER_CUBIC,cv::BORDER_CONSTANT, bg);
}

void dft_shift(cv::Point &p, const cv::Size &dim)
{
  if(p.x < dim.width/2)
    p.x = p.x + dim.width/2;
  else
    p.x = dim.width/2 - (dim.width - p.x);
  if(p.y < dim.height/2)
    p.y = p.y + dim.height/2;
  else
    p.y = dim.height/2 - (dim.height - p.y);
}

void dft_shift(cv::Mat &mag)
{
  // crop the spectrum, if it has an odd number of rows or columns
  mag = mag(cv::Rect(0, 0, mag.cols & -2, mag.rows & -2));

  // rearrange the quadrants of Fourier image  so that the origin is at the image center
  int cx = mag.cols/2;
  int cy = mag.rows/2;

  cv::Mat q0(mag, cv::Rect(0, 0, cx, cy));   // Top-Left - Create a ROI per quadrant
  cv::Mat q1(mag, cv::Rect(cx, 0, cx, cy));  // Top-Right
  cv::Mat q2(mag, cv::Rect(0, cy, cx, cy));  // Bottom-Left
  cv::Mat q3(mag, cv::Rect(cx, cy, cx, cy)); // Bottom-Right

  cv::Mat tmp;                           // swap quadrants (Top-Left with Bottom-Right)
  q0.copyTo(tmp);
  q3.copyTo(q0);
  tmp.copyTo(q3);

  q1.copyTo(tmp);                    // swap quadrant (Top-Right with Bottom-Left)
  q2.copyTo(q1);
  tmp.copyTo(q2);
}

void ift_shift(cv::Mat &mag)
{
  // crop the spectrum, if it has an odd number of rows or columns
  mag = mag(cv::Rect(0, 0, mag.cols & -2, mag.rows & -2));

  // rearrange the quadrants of Fourier image  so that the origin is at the image center
  int cx = mag.cols/2;
  int cy = mag.rows/2;

  cv::Mat q0(mag, cv::Rect(0, 0, cx, cy));   // Top-Left - Create a ROI per quadrant
  cv::Mat q1(mag, cv::Rect(cx, 0, cx, cy));  // Top-Right
  cv::Mat q2(mag, cv::Rect(0, cy, cx, cy));  // Bottom-Left
  cv::Mat q3(mag, cv::Rect(cx, cy, cx, cy)); // Bottom-Right

  cv::Mat tmp;                           // swap quadrants (Top-Left with Bottom-Right)
  q0.copyTo(tmp);
  q3.copyTo(q0);
  tmp.copyTo(q3);

  q1.copyTo(tmp);                    // swap quadrant (Top-Right with Bottom-Left)
  q2.copyTo(q1);
  tmp.copyTo(q2);
}


cv::Point detection_translation2(const cv::Mat &I0_, const cv::Mat &I1_)
{
  cv::Mat I0 = I0_, I1 = I1_;

  unsigned int nchn = I0.channels();

  cv::Mat accu = cv::Mat::zeros(I0.size(), CV_32F);

  dbg_image("I0", I0, true);
  dbg_image("I1", I1, true);

  cv::Mat F0, F1, M, C;
  cv::dft(I0, F0, cv::DFT_COMPLEX_OUTPUT);
  cv::dft(I1, F1, cv::DFT_COMPLEX_OUTPUT);
  cv::mulSpectrums(F0, F1, M, 0, true);

  cv::dft(M, C, cv::DFT_REAL_OUTPUT | cv::DFT_INVERSE);
  C = cv::abs(C);
  dbg_image("TC", C, true);
  accu = C;

  cv::Point max_loc;
  cv::minMaxLoc(accu, nullptr, nullptr, nullptr, &max_loc, cv::Mat());

  {
    cv::Mat spectre = accu.clone();
    cv::normalize(spectre, spectre, 0, 255, cv::NORM_MINMAX);
    spectre.convertTo(spectre, CV_8U);
    cv::cvtColor(spectre, spectre, CV_GRAY2BGR);
    dft_shift(spectre);
    cv::Point p = max_loc;
    ocvext::dft_shift(p, spectre.size());
    cv::line(spectre, cv::Point(spectre.cols/2-10, spectre.rows/2), cv::Point(spectre.cols/2+10, spectre.rows/2), cv::Scalar(0,255,0), 1, CV_AA);
    cv::line(spectre, cv::Point(spectre.cols/2, spectre.rows/2-10), cv::Point(spectre.cols/2, spectre.rows/2+10), cv::Scalar(0,255,0), 1, CV_AA);
    cv::circle(spectre, p, 10, cv::Scalar(0,0,255), 1, CV_AA);
    dbg_image("accu", spectre);
  }

  if(max_loc.x > accu.cols / 2)
    max_loc.x = -(accu.cols - max_loc.x);
  if(max_loc.y > accu.rows / 2)
    max_loc.y = -(accu.rows - max_loc.y);


  max_loc = -max_loc;
  infos("Translation detectee : %d, %d", max_loc.x, max_loc.y);
  return max_loc;
}

#if 0
int transformee_log_polaire(const cv::Mat &I, cv::Mat &O,
    unsigned int npas_angle, float rmin, unsigned int npas_rayon)
{
  uint16_t sx = I.cols, sy = I.rows;
  O.create(cv::Size(npas_rayon,npas_angle), CV_32F);
  O.setTo(cv::Scalar(0));

  cv::Mat src = I.clone();

  float rmax   = (std::min(I.cols,I.rows) * 0.5f);
  float raison = std::pow(rmax / rmin, 1.0f / npas_rayon);

  infos("npas rayon = %d, rmin = %f, rmax = %f, raison = %f", npas_rayon, rmin, rmax, raison);

  cv::Mat X(sy,sx,CV_32F), Y(sy,sx,CV_32F);
  for(auto y = 0u; y < sy; y++)
  {
    for(auto x = 0u; x < sx; x++)
    {
      X.at<float>(y,x) = ((float) x) - sx*0.5;
      Y.at<float>(y,x) = ((float) y) - sy*0.5;
    }
  }
  cv::Mat D, A;
  cv::cartToPolar(X, Y, D, A);

  float rayon = rmin;
  for(auto i = 0u; i < npas_rayon; i++)
  {
    float r;

    // On va remplir la colone "i" de la matrice (rayon)
    // On cherche dans la matrice d'entrée tous les pixels
    // situés à une distance du centre comprise entre rayon et rayon*raison

    auto masque = (D >= rayon) && (D < rayon * raison);

    // Il faut écrire un vecteur de taille "npas_angle" avec l'énergie suivant les différentes bandes angulaires
    // Comment gérer le sous-échantillonnage pour les petits rayons ????

    // ==> Idée : on accumule bêtement, et on filtre après
    // Gain d'accu : I, position : A


    /*for(auto j = 0u; j < npas_angle; j++)
    {
    }*/

    rayon *= raison;
  }


  return 0;
}
#endif


int detection_rotation_echelle(const cv::Mat &I0, const cv::Mat &I1, float &angle, float &echelle)
{
  cv::Mat I[2], maglp[2];

  I[0] = I0;
  I[1] = I1;

  ocvext::dbg_image("I0", I0);
  ocvext::dbg_image("I1", I1);

  for(auto i = 0u; i < 2; i++)
  {
    cv::Mat F, plans[2], mag;
    cv::dft(I[i], F, cv::DFT_COMPLEX_OUTPUT);
    cv::split(F, plans);
    cv::magnitude(plans[0], plans[1], mag);
    ocvext::dft_shift(mag);
    cv::Mat tmp;
    cv::log(mag + 0.1, tmp);
    ocvext::dbg_image("mag", tmp, true);
    //cv::linearPolar(mag, maglp[i], cv::Point(mag.cols/2, mag.rows/2), mag.cols/2, cv::INTER_CUBIC);
    // Log polar : le changement d'échelle devient une simple translation.
    cv::logPolar(mag, maglp[i], cv::Point(mag.cols/2, mag.rows/2), mag.cols/2, cv::INTER_CUBIC);
    cv::log(maglp[i] + 0.1, tmp);
    ocvext::dbg_image("linpol", tmp, true);

  }

  //cv::Mat bmaglp[2];

  //cv::waitKey()


  //cv::copyMakeBorder(maglp[0], bmaglp[0], 128, 128, 128, 128, cv::BORDER_CONSTANT);
  //cv::copyMakeBorder(maglp[1], bmaglp[1], 128, 128, 128, 128, cv::BORDER_CONSTANT);

  ocvext::dbg_image("maglop0-avant-pond", maglp[0], true);
  ocvext::dbg_image("maglop1-avant-pond", maglp[1], true);
  //ocvext::dbg_image("maglop-apres-bord", bmaglp[0], true);

  /*cv::log(bmaglp[0] + 0.01, bmaglp[0]);
  cv::log(bmaglp[1] + 0.01, bmaglp[1]);*/


  for(auto j = 0u; j < maglp[0].cols; j++)
  {
    cv::Rect r(j,0,1,maglp[0].rows);
    float coef = std::pow(((float) j) / maglp[0].cols, 2);
    maglp[0](r) *= coef;
    maglp[1](r) *= coef;
  }

  ocvext::dbg_image("maglop0-pond", maglp[0], true);
  ocvext::dbg_image("maglop1-pond", maglp[1], true);


  cv::Point pt = ocvext::detection_translation2(maglp[0], maglp[1]);
  infos("translation domaine polaire : %d, %d", pt.x, pt.y);

  float ntheta = maglp[0].rows;

  // pt.y == rows/2 => 180°
  // pt.y

  angle = - (CV_PI * ((float) pt.y) / (ntheta / 2));

  if(angle > CV_PI/2)
    angle -= CV_PI;

  float M = maglp[0].cols/2;
  echelle = std::exp(((float) - pt.x) / (M));

  infos("Angle = %.1f degres, echelle = %.2f.", angle * 180 / 3.1415926, echelle);
  return 0;
}


cv::Point detection_translation(const cv::Mat &I0_, const cv::Mat &I1_, bool normaliser_spectre, cv::Mat *spectre)
{
  cv::Mat I0 = I0_, I1 = I1_;
  assert(I0.size() == I1.size());
  assert(I0.type() == I1.type());

  infos("dim img = %d, %d", I0.cols, I0.rows);

  if(I0.type() != CV_32F)
  {
    I0.convertTo(I0, CV_32F);
    I1.convertTo(I1, CV_32F);
  }

  /*cv::normalize(I0, I0, 0, 1, cv::NORM_MINMAX);
  cv::normalize(I1, I1, 0, 1, cv::NORM_MINMAX);*/

  unsigned int nchn = I0.channels();

  cv::Mat accu = cv::Mat::zeros(I0.size(), CV_32F);
  cv::Mat I0s[3], I1s[3];

  dbg_image("I0", I0, true);
  dbg_image("I1", I1, true);

  //if(nchn > 1)
  {
    cv::split(I0, I0s);
    cv::split(I1, I1s);
  }

  for(auto i = 0u; i < nchn; i++)
  {
    /*dbg_image("I0s", I0s[i], true);
    dbg_image("I1s", I1s[i], true);*/

    /*cv::normalize(I0s[i], I0s[i], -1, 1, cv::NORM_MINMAX);
    cv::normalize(I1s[i], I1s[i], -1, 1, cv::NORM_MINMAX);*/

    cv::Mat F0, F1, M, C;
    cv::dft(I0s[i], F0, cv::DFT_COMPLEX_OUTPUT);
    cv::dft(I1s[i], F1, cv::DFT_COMPLEX_OUTPUT);
    cv::mulSpectrums(F0, F1, M, 0, true);

    // Est-ce vraiment nécessaire ?
    if(normaliser_spectre)
    {
      for(auto y = 0u; y < (unsigned int) M.rows; y++)
      {
        for(auto x = 0u; x < (unsigned int) M.cols; x++)
        {
          cv::Vec2f &p = M.at<cv::Vec2f>(y,x);
          float nrm = sqrt(p[0] * p[0] + p[1] * p[1]);
          if(nrm > 0)
          {
            p[0] /= nrm;
            p[1] /= nrm;
          }
        }
      }
    }

    cv::dft(M, C, cv::DFT_REAL_OUTPUT | cv::DFT_INVERSE);
    C = cv::abs(C);
    dbg_image("TC", C, true);
    accu += C;
  }





  /*cv::Mat tmp;
  if(ocvext::est_debogage_actif())
  {
    cv::GaussianBlur(accu, tmp, cv::Size(9,9),0,0);
    dbg_image("TCB", tmp, true);
  }*/

  cv::Point max_loc;
  cv::minMaxLoc(accu, nullptr, nullptr, nullptr, &max_loc, cv::Mat());

  if(spectre != nullptr)
  {
    *spectre = accu.clone();

    cv::normalize(*spectre, *spectre, 0, 255, cv::NORM_MINMAX);
    spectre->convertTo(*spectre, CV_8U);
    cv::cvtColor(*spectre, *spectre, CV_GRAY2BGR);
    dft_shift(*spectre);

    cv::Point p = max_loc;
    ocvext::dft_shift(p, spectre->size());
    cv::circle(*spectre, p, 10, cv::Scalar(0,0,255), 1, CV_AA);
  }

  if(max_loc.x > accu.cols / 2)
    max_loc.x = -(accu.cols - max_loc.x);
  if(max_loc.y > accu.rows / 2)
    max_loc.y = -(accu.rows - max_loc.y);


  max_loc = -max_loc;
  infos("Translation detectee : %d, %d", max_loc.x, max_loc.y);
  return max_loc;
}

#define DEBOGUER_ESTIM 0

// Conversion coordonnees dans l'espace frequentiel en periode spatiale
static float uv_vers_p(float u, float v, uint16_t sx, uint16_t sy)
{
  return 1.0 / sqrt((u/sx)*(u/sx)+(v/sy)*(v/sy));
}

static void p_vers_uv(float p, uint16_t sx, uint16_t sy, float &u, float &v)
{
  u = sx / p;
  v = sy / p;
}

static inline float sqr(float x)
{
  return x * x;
}

int estime_periode(cv::Mat &I,
                   float &d, float &indice_confiance,
                   float dmin, float dmax,
                   cv::Mat *dbg)
{
# if DEBOGUER_ESTIM
  infos("dmin = %.1f, dmax = %.1f.", dmin, dmax);
  ocvext::dbg_image("patch", I, true);
# endif

  I -= cv::mean(I)[0];
  cv::Mat W, F, plans[2], mag;
  cv::createHanningWindow(W, I.size(), CV_32F);
  I = I.mul(W);

  uint16_t sx = I.cols, sy = I.rows;
  cv::dft(I, F, cv::DFT_COMPLEX_OUTPUT);
  cv::split(F, plans);
  cv::magnitude(plans[0], plans[1], mag);
  mag = mag.mul(mag);
  ocvext::dft_shift(mag);

  if(dbg != nullptr)
  {
    dbg[0] = 1.0 + mag;
    cv::log(dbg[0], dbg[0]);
    cv::normalize(dbg[0], dbg[0], 0, 255, cv::NORM_MINMAX);
    dbg[0].convertTo(dbg[0], CV_8U);
    cv::cvtColor(dbg[0], dbg[0], CV_GRAY2BGR);
  }


# if DEBOGUER_ESTIM
  ocvext::dbg_image("mag", mag, true);
  ocvext::dbg_image("log-mag", dbg);
# endif

  unsigned int cx = sx/2, cy = sy/2;
# if DEBOGUER_ESTIM
  //tmp.setTo(0.0);
  //cv::Mat dessin_dmin_dmax(q0.size(), CV_8UC3, cv::Scalar(0,0,0));
# endif


  // (1) Calcul du nuage de points
  cv::Mat T, mg, itrp;
  uint32_t n = 2 * (cx - 1) * (cy - 1);
  uint32_t j, N = dmax + 1;
  T  = cv::Mat::zeros(n, 1, CV_32F);
  mg = cv::Mat::zeros(n, 1, CV_32F);
  itrp = cv::Mat::zeros(N, 1, CV_32F);
  uint32_t i = 0;
  //FILE *fo = fopen("./build/sci.sce", "wt");
  for(auto fy = 1u; fy < cx; fy++)
  {
    for(auto fx = 1u; fx < cy; fx++)
    {
      // Utilise 2 quadrants
      T.at<float>(i)  = uv_vers_p(fx,fy,sx,sy);
      mg.at<float>(i) = mag.at<float>(cy+fy,cx+fx);
      i++;
      T.at<float>(i)  = uv_vers_p(fx,fy,sx,sy);
      mg.at<float>(i) = mag.at<float>(cy-fy,cx+fx);
      i++;
    }
  }


# if DEBOGUER_ESTIM
  infos("Interpolation par noyaux (RBF: N = %d, n = %d)...", N, n);
# endif
  // (2) Interpolation par noyaux (RBF)
  for(i = dmin; i < N; i++)
  {
    float somme1 = 0;
    float *Tptr = T.ptr<float>();
    float *mgptr = mg.ptr<float>();
    for(j = 0; j < n; j++)
    {
      //float k1 = std::exp(-(sqr(T.at<float>(j)-i))/(2*h1*h1));
      float nm = sqr(*Tptr++ - i);
      float k1 = 1.0 / (1.0 + nm);//std::exp(-nm/(2*h1*h1));
      somme1 += k1 * (*mgptr++);
    }
    itrp.at<float>(i) = somme1;
  }
# if DEBOGUER_ESTIM
  infos("Fin itrp.");
  cv::normalize(itrp, itrp, 0, 255, cv::NORM_MINMAX);
  cv::Mat O = cv::Mat(255,itrp.rows,CV_8UC3,cv::Scalar(0));
  plot_1d(O, itrp, cv::Scalar(0,255,0));
  dbg_image("Itrp", O);
# endif

  if(dbg != nullptr)
  {
    cv::Mat tmp;
    cv::normalize(itrp, tmp, 0, N-1, cv::NORM_MINMAX);
    dbg[1] = cv::Mat(N-1,itrp.rows,CV_8UC3,cv::Scalar(0));
    plot_1d(dbg[1], tmp, cv::Scalar(0,255,0));
  }

  cv::Point max_loc;
  cv::minMaxLoc(itrp, nullptr, nullptr, nullptr, &max_loc);
  d = max_loc.y;
  indice_confiance  = 0;

# if DEBOGUER_ESTIM
  infos("Meilleur: d = %f (f = %f)", d, (float) max_loc.y);
# endif

  float rd = itrp.at<float>(max_loc.y);

  // Verification si maximum local
  if((d == dmin) || (d == dmax))
    return -1;
  /*if((rd <= accu.at<float>(max_loc.y-1))
      || (rd <= accu.at<float>(max_loc.y+1)))
    return -1;*/

  //int d2 = 0;
  float rd2 = 0;//accu.at<float>(0);
  // Recherche 2eme maxima local
  for(int r = 0u; r + 1 < itrp.rows; r++)
  {
    float v = itrp.at<float>(r);
    if((r != d) // diff�rent du max global
        && (v > rd2) // maxi
        && (v > itrp.at<float>(r-1)) // Maximum local
        && (v > itrp.at<float>(r+1)))
    {
      rd2 = v;
      //d2 = r;
    }
  }

  indice_confiance = rd - rd2;
  /*indice_confiance = std::min(indice_confiance,
            rd - accu.at<float>(max_loc.y-1));
  indice_confiance = std::min(indice_confiance,
            rd - accu.at<float>(max_loc.y+1));*/

  float alpha = 1.0;
  indice_confiance *= alpha / rd;

  infos("Distance detectee : %.1f pixels, indice confiance = %f.", d, indice_confiance);

  if(dbg != nullptr)
  {
    float u, v;
    p_vers_uv(d, sx, sy, u, v);
    cv::circle(dbg[0], cv::Point(cx, cy), u, cv::Scalar(0,0,255), 1, CV_AA);
  }

  return 0;
}


}

