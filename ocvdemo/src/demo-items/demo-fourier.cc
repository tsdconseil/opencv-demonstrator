#include "demo-items/fourier-demo.hpp"
#include "fourier.hpp"
#include "ocvext.hpp"

IFTDemo::IFTDemo()
{
  props.id = "ift";
  props.requiert_masque = true;
}

int IFTDemo::proceed(OCVDemoItemInput &input, OCVDemoItemOutput &output)
{
  cv::Mat I, O, mag;
  input.mask.convertTo(I, CV_32F);
  //cv::cvtColor(I, I, CV_BGR2GRAY);
  ocvext::dft_shift(I);
  cv::dft(I, O, cv::DFT_COMPLEX_OUTPUT);

  //cv::putText()

  cv::Mat plans[2];
  cv::split(O, plans);

  infos("plan0 = %d,%d, plan1 = %d,%d", plans[0].cols, plans[0].rows, plans[1].cols, plans[1].rows);

  cv::magnitude(plans[0], plans[1], mag);
  ocvext::dft_shift(mag);



  cv::normalize(mag, mag, 0, 255, cv::NORM_MINMAX);
  //cv::normalize(plans[0], mag, 0, 255, cv::NORM_MINMAX);
  ocvext::dft_shift(mag);
  output.images[0] = I.clone();
  output.images[1] = mag;
  output.nout = 2;
  output.names[0] = "Espace de Fourier";
  output.names[1] = "Trf. inverse";
  return 0;
}

DFTDemo::DFTDemo()
{
  props.id = "dft";
  output.nout = 2;
  output.names[0] = "Entree";
  output.names[1] = "DFT";
}

int DFTDemo::proceed(OCVDemoItemInput &input, OCVDemoItemOutput &output)
{
  Mat Ig, padded;                            //expand input image to optimal size

  float angle = input.model.get_attribute_as_float("angle");
  bool fenetrage = input.model.get_attribute_as_boolean("fenetrage");
  bool vue_log = input.model.get_attribute_as_boolean("log");
  bool decalage_dc = input.model.get_attribute_as_boolean("decalage-dc");

  int source = input.model.get_attribute_as_int("source");
  int periode = input.model.get_attribute_as_int("dft-sin/periode");

  cvtColor(input.images[0], Ig, CV_BGR2GRAY);

  if(source == 1)
  {
    Ig = cv::Mat::zeros(512, 512, CV_32F);
    for(auto y = 0u; y < 512; y++)
    {
      float *ptr = Ig.ptr<float>(y);
      for(auto x = 0u; x < 512; x++)
      {
        float xf = x;
        //*ptr++ = 128.0 + 128 * std::sin(2 * CV_PI * xf / (512 * (periode / 512.0)));
        *ptr++ = 128 * std::sin(2 * CV_PI * xf / (512 * (periode / 512.0)));
      }
    }
  }



  Ig.convertTo(Ig, CV_32F);

  if(fenetrage)
  {
    cv::Mat W;
    cv::createHanningWindow(W, Ig.size(), CV_32F);
    Ig = Ig.mul(W);

    /*cv::imshow("W", W);
    cv::imshow("F", Ig/255.0);
    cv::waitKey(0);*/
  }



  //uint16_t idx = 1;

  if(angle != 0)
  {
    cv::Size sz = Ig.size() / 2;
    cv::Point centre;
    centre.x = sz.width;
    centre.y = sz.height;
    uint16_t sx = sz.width, sy = sz.height;
    cv::Mat R = cv::getRotationMatrix2D(centre, angle, 1.0);
    cv::warpAffine(Ig, Ig, R, Ig.size());
    Ig = Ig(Rect(sx/4,sy/4,sx/2,sy/2));
    //output.images[idx++] = Ig;
  }

  output.images[0] = Ig;

  /*int m = getOptimalDFTSize( Ig.rows );
  int n = getOptimalDFTSize( Ig.cols ); // on the border add zero values
  copyMakeBorder(Ig, padded, 0, m - Ig.rows, 0, n - Ig.cols, BORDER_CONSTANT, Scalar::all(0));

  Mat planes[] = {Mat_<float>(padded), Mat::zeros(padded.size(), CV_32F)};

  merge(planes, 2, complexI);         // Add to the expanded another plane with zeros
  */

  Mat plans[2], complexI, mag;
  cv::dft(Ig, complexI, cv::DFT_COMPLEX_OUTPUT);

  cv::split(complexI, plans);
  magnitude(plans[0], plans[1], mag);

  if(vue_log)
    cv::log(mag + 1.0, mag);

  if(decalage_dc)
    ocvext::dft_shift(mag);

  cv::normalize(mag, mag, 0, 255, NORM_MINMAX);
  output.images[1] = mag;
  //output.nout = idx;

  return 0;
}


DemoDetectionRotation::DemoDetectionRotation()
{
  props.id = "detection-rotation";
}

DemoDetectionTranslation::DemoDetectionTranslation()
{
  props.id = "detection-translation";
}


int DemoDetectionTranslation::proceed(OCVDemoItemInput &input, OCVDemoItemOutput &output)
{
  cv::Mat I, O;
  uint16_t idx = 0;

  //cv::cvtColor(input.images[0], I, CV_BGR2GRAY);
  I = input.images[0].clone();

  output.names[idx] = "Image";
  output.images[idx++] = I.clone();


  I.convertTo(I, CV_32F);

  cv::Mat T, S;

  int dx = input.model.get_attribute_as_int("tx");
  int dy = input.model.get_attribute_as_int("ty");
  bool norm_spectre = input.model.get_attribute_as_boolean("norm-spectre");

  T = I.clone();
  ocvext::translation_rapide(I, T, dx, dy, I.size(), cv::Scalar(0));

  cv::Mat tmp;
  T.convertTo(tmp, CV_8U);

  output.names[idx] = "Translation";
  output.images[idx++] = tmp;

  ocvext::detection_translation(I, T, norm_spectre, &S);

  cv::normalize(S, S, 0, 255, cv::NORM_MINMAX);
  //S.convertTo(S, CV_8U);
  output.names[idx] = "Correlation";
  output.images[idx++] = S;
  output.nout = idx;
  return 0;
}


int DemoDetectionRotation::proceed(OCVDemoItemInput &input, OCVDemoItemOutput &output)
{
  cv::Mat I, mag, O;
  cv::cvtColor(input.images[0], I, CV_BGR2GRAY);
  I.convertTo(I, CV_32F);

  float echelle = input.model.get_attribute_as_float("echelle");
  float angle = input.model.get_attribute_as_float("angle");
  int dx = input.model.get_attribute_as_int("tx");
  int dy = input.model.get_attribute_as_int("ty");
  bool mode_log = input.model.get_attribute_as_boolean("mode-log");
  uint16_t idx = 0;

  //if(angle != 0)
  {
    cv::Size sz = I.size() / 2;
    cv::Point centre;
    centre.x = sz.width;
    centre.y = sz.height;
    cv::Mat R = cv::getRotationMatrix2D(centre, angle, echelle);
    // R = Matrice 2x3
    R.at<double>(0,2) = dx;
    R.at<double>(1,2) = dy;
    cv::warpAffine(I, I, R, I.size());
    //I = I(Rect(sx/4,sy/4,sx/2,sy/2));
    output.names[idx] = "Rotation";
    output.images[idx++] = I;
  }

  cv::Mat F, plans[2];
  cv::dft(I, F, DFT_COMPLEX_OUTPUT, I.rows);
  cv::split(F, plans);
  cv::magnitude(plans[0], plans[1], mag);

  mag += Scalar::all(1);                    // switch to logarithmic scale
  cv::log(mag, mag);

  ocvext::dft_shift(mag);

  if(mode_log)
    cv::logPolar(mag, O, cv::Point(mag.cols/2, mag.rows/2), mag.cols/2, cv::INTER_CUBIC);
  else
    cv::linearPolar(mag, O, cv::Point(mag.cols/2, mag.rows/2), mag.cols/2, cv::INTER_CUBIC);

  cv::normalize(mag, mag, 0, 255, cv::NORM_MINMAX);
  cv::normalize(O, O, 0, 255, cv::NORM_MINMAX);
  output.names[idx] = "Mag TFR";
  output.images[idx++] = mag;
  output.names[idx] = "Trf pol.";
  output.images[idx++] = O;
  output.nout = idx;
  return 0;
}

DemoSousSpectrale::DemoSousSpectrale()
{
  props.id = "sous-spect";
}

static void sousstraction_spectrale_gs(cv::Mat &I, cv::Mat &O, cv::Mat &mag, cv::Mat &masque, float seuil)
{
  cv::Mat F, plans[2];
  cv::dft(I, F, DFT_COMPLEX_OUTPUT, I.rows);

  //infos("F: cols=%d, rows=%d, type=%d, nchn=%d", F.cols, F.rows, F.type(), F.channels());

  cv::split(F, plans);
  cv::magnitude(plans[0], plans[1], mag);

  float moy = cv::mean(mag)[0];
  masque = mag > (seuil * moy);

  F.setTo(0.0, masque);

  cv::dft(F, O, cv::DFT_INVERSE + cv::DFT_SCALE + DFT_REAL_OUTPUT, I.rows);// + K_spatial[i].rows / 2);

  cv::normalize(O,O,0,255,cv::NORM_MINMAX);
}

static void sousstraction_spectrale(cv::Mat &I, cv::Mat &O, cv::Mat &mag, float seuil)
{
  cv::Mat compos[3], compos_sortie[3];
  cv::split(I, compos);
  cv::Mat masque;
  for(auto i = 0u; i < 3; i++)
  {

    sousstraction_spectrale_gs(compos[i], compos_sortie[i], mag, masque, seuil);
  }
  cv::merge(compos_sortie, 3, O);
}

int DemoSousSpectrale::proceed(OCVDemoItemInput &input, OCVDemoItemOutput &output)
{
  cv::Mat mag, O, I = input.images[0], If;
  I.convertTo(If, CV_32F);

  sousstraction_spectrale(If, O, mag, input.model.get_attribute_as_float("seuil"));

  O.convertTo(O, CV_8U);

  output.nout = 3;
  output.images[0] = If;
  output.names[0] = "Entree";
  cv::log(mag + 0.1, mag);
  cv::normalize(mag, mag, 0, 255, cv::NORM_MINMAX);
  ocvext::dft_shift(mag);
  output.images[1] = mag;
  output.names[1] = "Magnitude DFT";
  output.images[2] = O;
  output.names[2] = "Soustraction spectrale";
  return 0;
}


DemoDetectionPeriode::DemoDetectionPeriode()
{
  props.id = "detection-periode";
}


int DemoDetectionPeriode::proceed(OCVDemoItemInput &input, OCVDemoItemOutput &output)
{
  float zoom = input.model.get_attribute_as_int("zoom") / 100.0;

  cv::Mat Ig;
  cvtColor(input.images[0], Ig, CV_BGR2GRAY);

  Ig.convertTo(Ig, CV_32F);

  unsigned int sx = Ig.cols, sy = Ig.rows;

  cv::resize(Ig, Ig, cv::Size(0,0), zoom, zoom);

  cv::Rect r(Ig.cols/2-sx/2,Ig.rows/2-sy/2,sx,sy);

  infos("Ig: %d,%d, r: %d,%d,%d,%d", Ig.cols, Ig.rows, r.x, r.y, r.width, r.height);

  cv::Mat I = Ig(r).clone();

  /*auto px = Ig.cols * (zoom - 1);
  auto py = Ig.rows * (zoom - 1);

  cv::Mat I = Ig(cv::Rect())*/

  output.images[0] = I.clone();
  output.names[0] = "Entree";


  float d, indice_confiance;
  float dmin = 1, dmax = 50;

 // ocvext::defini_options_debogage(false, true);
//  ocvext::defini_dossier_stockage("./build/ocvext");

  cv::Mat dbg[2];

  ocvext::estime_periode(I, d, indice_confiance, dmin, dmax, dbg);

  output.nout = 3;
  output.images[1] = dbg[0];
  output.names[1] = "TFD";
  output.images[2] = dbg[1];
  output.names[2] = "Energie radiale max.";

# if 0
  Ig -= cv::mean(Ig)[0];

  cv::resize(Ig, Ig, cv::Size(256, 256));
  // Fenêtrage

  cv::Mat W;
  cv::createHanningWindow(W, Ig.size(), CV_32F);
  Ig = Ig.mul(W);


  //cv::copyMakeBorder(Ig, Ig, 128, 128, 128, 128, cv::BORDER_CONSTANT, cv::Scalar(0));

  dft(Ig, F, cv::DFT_COMPLEX_OUTPUT);

  cv::split(F, plans);
  magnitude(plans[0], plans[1], mag);

  mag += Scalar::all(0.00001);
  cv::log(mag, mag);



  ocvext::dft_shift(mag);

  cv::normalize(mag, mag, 0, 255, NORM_MINMAX);
  output.images[0] = mag;


  // TODO: accumuler suivant le rayon



# if 0
  // TODO: accumuler suivant le rayon
  cv::Mat pol, red;
  cv::linearPolar(mag, pol, cv::Point(mag.size())/2, mag.cols, cv::INTER_LINEAR);

  cv::normalize(pol, pol, 0, 255, cv::NORM_MINMAX);
  output.images[1] = pol;

  cv::reduce(pol, red, 0, REDUCE_SUM);

  // TODO : plot

  {
    cv::Mat tmp(255, 640, CV_8UC3, cv::Scalar(0));
    cv::Mat pc2;
    cv::normalize(red, pc2, 0, 255, cv::NORM_MINMAX);

    //for(auto i = 0; i < ntheta; i++)
      //printf("pc[%d] = %f\n", i, pc2.at<float>(i));

    ocvext::plot_1d(tmp, pc2, cv::Scalar(0,255,0));
    output.images[2] = tmp;
  }

  output.nout = 3;
# endif
# endif

  return 0;
}


