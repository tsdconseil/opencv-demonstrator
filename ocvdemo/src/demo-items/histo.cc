/** @file histo.cc
 *  @brief Traitements basés sur les histogrammes

    Copyright 2015 J.A. / http://www.tsdconseil.fr

    Project web page: http://www.tsdconseil.fr/log/opencv/demo/index-en.html

    This file is part of OCVDemo.

    OCVDemo is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    OCVDemo is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with OCVDemo.  If not, see <http://www.gnu.org/licenses/>.
 **/

#include "demo-items/histo.hpp"

static void calcul_histogramme_1d(const Mat &I,
                                  MatND &hist,
                                  int canal,
                                  int maxval,
                                  int nbins)
{
  float hranges[] = {0, (float) maxval};
  const float *ranges[] = {hranges};

  calcHist(&I, 1, &canal, Mat(), // (pas de masque)
           hist, 1, &nbins, ranges,
           true, // Histogram uniforme
           false);

  normalize(hist, hist, 0, 255, NORM_MINMAX);
}

static void calcul_histogramme_2d(const Mat &I,
                                  MatND &hist,
                                  int canaux[],
                                  int maxval[],
                                  int nbins[])
{
  float hranges0[] = {0, (float) maxval[0]};
  float hranges1[] = {0, (float) maxval[1]};
  const float *ranges[] = {hranges0, hranges1};

  calcHist(&I, 1, canaux, Mat(), // (pas de masque)
           hist, 2, nbins, ranges,
           true, // Histogram uniforme
           false); // Pas d'accumulation

  normalize(hist, hist, 0, 255, NORM_MINMAX);
}

HistoBP::HistoBP()
{
  props.id = "hist-bp";
  props.requiert_roi = true;
  input.roi = Rect(116,77,134-116,96-77);//Rect(225,289,50,50);
}

int calc_hist(const cv::Mat &I, cv::Rect &roi, cv::MatND &hist)
{
  Mat hsv, hue, mask;

  if(roi.width * roi.height == 0)
    return -1;

  Mat tmp = I(roi);
  cvtColor(tmp, hsv, CV_BGR2HSV);
  inRange(hsv, Scalar(0, 30, 30), Scalar(180, 256, 256), mask);

  hue.create( hsv.size(), hsv.depth());
  int ch[] = { 0, 0 };
  mixChannels(&hsv, 1, &hue, 1, ch, 1 );

  int bins = 25;
  int histSize = MAX( bins, 2 );
  float hue_range[] = { 0, 180 };
  const float* ranges = { hue_range };

  /// Get the histogram and normalize it
  calcHist( &hue, 1, 0, mask, hist, 1, &histSize, &ranges, true, false );
  normalize(hist, hist, 0, 255, NORM_MINMAX, -1, Mat());
  return 0;
}



int calc_bp(const cv::Mat &I, const cv::MatND &hist, cv::MatND &backproj)
{
  Mat Ihsv, Ihue, mask;
  cvtColor(I, Ihsv, CV_BGR2HSV);

  inRange(Ihsv, Scalar(0, 30, 30), Scalar(180, 256, 256), mask);

  Ihue.create(Ihsv.size(), Ihsv.depth());
  int ch[] = { 0, 0 };
  mixChannels(&Ihsv, 1, &Ihue, 1, ch, 1 );
  float hue_range[] = { 0, 180 };
  const float* ranges = { hue_range };
  /// Get backprojection
  calcBackProject(&Ihue, 1, 0, hist, backproj, &ranges, 1, true);
  backproj &= mask;
  return 0;
}

int calc_bp(const cv::Mat &I, cv::Rect &roi, cv::MatND &backproj)
{
  MatND hist;
  calc_hist(I, roi, hist);
  return calc_bp(I, hist, backproj);
}

int HistoBP::proceed(OCVDemoItemInput &input, OCVDemoItemOutput &output)
{
  output.nout = 2;
  output.images[0] = input.images[0].clone();
  if(input.roi.width * input.roi.height > 0)
    calc_bp(input.images[0], input.roi, output.images[1]);
  output.names[0] = utils::langue.get_item("ROI selection");
  output.names[1] = utils::langue.get_item("Backprojection");
  return 0;
}


HistoCalc::HistoCalc()
{
  props.id = "hist-calc";
}

static void dessine_courbe(const MatND &x, Mat &image, Scalar color, float yscale)
{
  uint32_t n = x.rows;
  float sx = image.cols, sy = image.rows;

  float lxi = x.at<float>(0)  * yscale;
  for(auto i = 1u; i < n; i++)
  {
    float xi = x.at<float>(i) * yscale;

    line(image,
         Point((i-1)* sx / n,sy-lxi),
         Point(i* sx / n,sy-xi),
         color, 2, CV_AA);

    lxi = xi;
  }
}




int HistoCalc::proceed(OCVDemoItemInput &input, OCVDemoItemOutput &output)
{
  // 0 : histogrammes sÃ©parÃ©s RGB
  // 1 : histogramme niveaux de gris
  // 2 : histogrammes sÃ©parÃ©s HSV
  // 3 : histogramme 2D HS
  int sel = input.model.get_attribute_as_int("sel");
  int nbins = input.model.get_attribute_as_int("nbins");

  if(nbins < 1)
    nbins = 1;

  MatND hist;
  Mat I = input.images[0];

  if(sel == 0)
  {
    output.nout = 1;
    MatND hist[3];

    output.images[0] = Mat(Size(512,512), CV_8UC3, cv::Scalar(255,255,255));

    for(auto i = 0u; i < 3; i++)
      calcul_histogramme_1d(I, hist[i], i, 255, nbins);

    dessine_courbe(hist[0], output.images[0], Scalar(255,0,0), 512.0 / 255);
    dessine_courbe(hist[1], output.images[0], Scalar(0,255,0), 512.0 / 255);
    dessine_courbe(hist[2], output.images[0], Scalar(0,0,255), 512.0 / 255);
    output.names[0] = utils::langue.get_item("histo-bvr");
  }
  else if(sel == 1)
  {
    cv::Mat Ig;
    cv::cvtColor(I, Ig, CV_BGR2GRAY);

    calcul_histogramme_1d(Ig, hist, 0, 255, nbins);
    output.images[0] = Mat(Size(512,512), CV_8UC3, cv::Scalar(255,255,255));
    dessine_courbe(hist, output.images[0], Scalar(0,0,0), 512.0 / 255);
    output.names[0] = "Histogramme luminance";
  }
  else if(sel == 2)
  {
    Mat I2;
    cvtColor(I, I2, CV_BGR2HSV);
    int vmax[3] = {179,255,255};

    for(auto i = 0u; i < 3; i++)
    {
      calcul_histogramme_1d(I2, hist, 0, vmax[i], nbins);
      output.images[i] = Mat(Size(512,512), CV_8UC3, cv::Scalar(255,255,255));
      dessine_courbe(hist, output.images[i], Scalar(0,0,0), 512.0 / 255.0);
    }

    output.nout = 3;
    output.names[0] = "Teinte / Hue";
    output.names[1] = "Saturation";
    output.names[2] = "Valeur";
  }
  else if(sel == 3)
  {
    output.nout = 1;
    Mat I2;
    cvtColor(I, I2, CV_BGR2HSV);
    int canaux[2] = {0, 1}; // Teinte & Saturation
    int vmax[2] = {180, 255};
    int vnbins[2] = {nbins, nbins};
    calcul_histogramme_2d(I2, hist, canaux, vmax, vnbins);
    while(hist.cols < 400)
      cv::pyrUp(hist, hist);
    output.images[0] = hist;
  }
  return 0;
}


HistoEgalisationDemo::HistoEgalisationDemo()
{
  props.id = "histeq";
}


int HistoEgalisationDemo::proceed(OCVDemoItemInput &input, OCVDemoItemOutput &output)
{
  int sel = input.model.get_attribute_as_int("sel");
  int esp = input.model.get_attribute_as_int("espace");

  int code[2] = {0};
  int index_L = 0;
  if(esp == 0)
  {
    code[0] = CV_BGR2YUV;
    code[1] = CV_YUV2BGR;
  }
  else if(esp == 1)
  {
    code[0] = CV_BGR2Lab;
    code[1] = CV_Lab2BGR;
  }
  else if(esp == 2)
  {
    code[0] = CV_BGR2HSV;
    code[1] = CV_HSV2BGR;
    index_L = 2;
  }
  else if(esp == 3)
  {
    code[0] = CV_BGR2HLS;
    code[1] = CV_HLS2BGR;
    index_L = 1;
  }

  Mat tmp;
  if(esp == 4)
    tmp = input.images[0].clone();
  else
    cvtColor(input.images[0], tmp, code[0]);
  Mat chns[3];
  split(tmp, chns);

  // Egalisation luminance
  if(sel == 0)
  {
    cv::equalizeHist(chns[index_L], chns[index_L]);
  }
  // Egalisation 3 canaux RGB (pour voir les artefacts couleurs)
  else if(sel == 1)
  {
    for(auto i = 0u; i < 3; i++)
      equalizeHist(chns[i], chns[i]);
  }
  merge(chns, 3, tmp);
  if(esp == 4)
    output.images[0] = tmp;
  else
    cvtColor(tmp, output.images[0], code[1]);

  return 0;
}


