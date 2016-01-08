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

static void calc_histo(const Mat &img, MatND &hist, int channel, float maxval, double &maxprob)
{
  // Quantize the hue to 30 levels
  // and the saturation to 32 levels
  int hbins = 100;//30;//, sbins = 32;
  int histSize[] = {hbins};//, sbins};
  // hue varies from 0 to 179, see cvtColor
  float hranges[] = { 0, maxval};//180 };
  // saturation varies from 0 (black-gray-white) to
  // 255 (pure spectrum color)
  //float sranges[] = { 0, 256 };
  const float * ranges[] = { hranges};//, sranges };
  // we compute the histogram from the 0-th and 1-st channels
  int channels[] = {channel};//0, 1};
  calcHist(&img, 1, channels, Mat(), // do not use mask
      hist, 1, histSize, ranges,
      true, // the histogram is uniform
      false );
  normalize(hist, hist, 0, 255, NORM_MINMAX);
  maxprob = 0;
  minMaxLoc(hist, 0, &maxprob, 0, 0);
}

HistoBP::HistoBP()
{
  props.id = "hist-bp";
  props.requiert_roi = true;
  params.roi = Rect(116,77,134-116,96-77);//Rect(225,289,50,50);
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

int HistoBP::calcul(Node &model, cv::Mat &I)
{
  MatND backproj;
  sortie.nout = 2;
  calc_bp(I, params.roi, backproj);
  cvtColor(backproj, sortie.O[1], CV_GRAY2BGR);
  return 0;
}


HistoCalc::HistoCalc()
{
  props.id = "hist-calc";
}


#if 0
cv::Mat HistoCalc::get_2d_histogram_image(const cv::Mat &image)
{
  // Pour un histogramme 2d
  /*Mat histImg = Mat::zeros(sbins*scale, hbins*10, CV_8UC3);
  for( int h = 0; h < hbins; h++ )
    for( int s = 0; s < sbins; s++ )
    {
      float binVal = hist.at<float>(h, s);
      int intensity = cvRound(binVal*255/maxVal);
      rectangle( histImg, Point(h*scale, s*scale),
          Point( (h+1)*scale - 1, (s+1)*scale - 1),
          Scalar::all(intensity),
          CV_FILLED );
    }*/
  return Mat();
}
#endif


static void plot_curve(const MatND &x, Mat &image, Scalar color,
                       float yscale)
{
  uint32_t n = x.rows;

  //if(image.data == nullptr)
    //image = Mat(Size(n,256), CV_8UC3, cv::Scalar(255,255,255));

  float sx = image.cols, sy = image.rows;

  float lxi = x.at<float>(0)  * yscale;
  for(auto i = 1u; i < n; i++)
  {
    float xi = x.at<float>(i) * yscale;

    line(image,
         Point((i-1)* sx / n,sy-lxi),
         Point(i* sx / n,sy-xi),
         color);

    lxi = xi;
  }
}





#if 0
// Computes the 1D histogram and returns an image of it.
// D'aprÃ¨s "OpenCV 2 Computer Vision Application programming cookbook", Robert LaganiÃ¨re
// (Mise Ã  jour : d'aprÃ¨s le manuel utilisateur)
cv::Mat HistoCalc::get_1d_histogram_image(const cv::Mat &image,
					  int channel,
					  float maxval)
{
  // Quantize the hue to 30 levels
  // and the saturation to 32 levels
  int hbins = 100;//30;//, sbins = 32;
  int histSize[] = {hbins};//, sbins};
  // hue varies from 0 to 179, see cvtColor
  float hranges[] = { 0, maxval};//180 };
  // saturation varies from 0 (black-gray-white) to
  // 255 (pure spectrum color)
  //float sranges[] = { 0, 256 };
  const float * ranges[] = { hranges};//, sranges };
  MatND hist;
  // we compute the histogram from the 0-th and 1-st channels
  int channels[] = {channel};//0, 1};
  calcHist(&image, 1, channels, Mat(), // do not use mask
	    hist, 1, histSize, ranges,
	    true, // the histogram is uniform
	    false );
  double maxVal=0;
  minMaxLoc(hist, 0, &maxVal, 0, 0);
  //int scale = 10;



  // Image on which to display histogram
  cv::Mat histImg(histSize[0], histSize[0],
		  CV_8U,cv::Scalar(255));

  // set highest point at 90% of nbins
  int hpt = static_cast<int>(0.9*histSize[0]);
  // Draw a vertical line for each bin
  for( int h = 0; h < histSize[0]; h++ ) 
  {
    float binVal = hist.at<float>(h);
    int intensity = static_cast<int>(binVal*hpt/maxVal);
    // This function draws a line between 2 points
    cv::line(histImg,cv::Point(h,histSize[0]),
	     cv::Point(h,histSize[0]-intensity),
	     cv::Scalar::all(0));
  }



  cvtColor(histImg, histImg, CV_GRAY2BGR);
  return histImg;

# if 0
  // Compute histogram first
  //cv::MatND hist;

  //int histSize = 1;

  // Compute histogram
  cv::calcHist(&image,
	       1,
	       // histogram from 1 image only
	       channels, // the channel used
	       cv::Mat(), // no mask is used
	       hist,
	       // the resulting histogram
	       1,
	       // it is a 1D histogram
	       histSize, // number of bins
	       ranges
	       // pixel value range
	       );

  // Get min and max bin values
  double maxVal=0;
  double minVal=0;
  cv::minMaxLoc(hist, &minVal, &maxVal, 0, 0);
  // Image on which to display histogram
  cv::Mat histImg(histSize[0], histSize[0],
		  CV_8U,cv::Scalar(255));
  // set highest point at 90% of nbins
  int hpt = static_cast<int>(0.9*histSize[0]);
  // Draw a vertical line for each bin
  for( int h = 0; h < histSize[0]; h++ ) {
    float binVal = hist.at<float>(h);
    int intensity = static_cast<int>(binVal*hpt/maxVal);
    // This function draws a line between 2 points
    cv::line(histImg,cv::Point(h,histSize[0]),
	     cv::Point(h,histSize[0]-intensity),
	     cv::Scalar::all(0));
  }
  return histImg;
# endif
}
#endif

int HistoCalc::calcul(Node &model, cv::Mat &I)
{
  int sel = model.get_attribute_as_int("sel");
  // 0 : histogrammes sÃ©parÃ©s RGB
  // 1 : histogramme niveaux de gris
  // 2 : histogrammes sÃ©parÃ©s HSV
  // 3 : histogramme 2D HS

  if(sel == 0)
  {
    MatND hist[3];
    double maxprobs[3];
    for(auto i = 0u; i < 3; i++)
      calc_histo(I, hist[i], i, 255, maxprobs[i]);

    sortie.O[1] = Mat(Size(512,512), CV_8UC3, cv::Scalar(255,255,255));
    plot_curve(hist[0], sortie.O[1], Scalar(255,0,0), 512.0 / maxprobs[0]);
    plot_curve(hist[1], sortie.O[1], Scalar(0,255,0), 512.0 / maxprobs[1]);
    plot_curve(hist[2], sortie.O[1], Scalar(0,0,255), 512.0 / maxprobs[2]);
    //this->outname[0] = langue.get_item("histo-bvr");//;
    sortie.nout = 2;
    sortie.outname[0] = "";
    sortie.outname[1] = langue.get_item("histo-bvr");
  }
  else if(sel == 1)
  {
    cv::Mat Ig;
    cv::cvtColor(I, Ig, CV_BGR2GRAY);

    MatND hist;
    double maxprob;
    calc_histo(Ig, hist, 0, 255, maxprob);

    printf("SX = %d, SY = %d.\n", 100, (int) ceil(maxprob));
    sortie.nout = 2;
    sortie.O[1] = Mat(Size(512,512), CV_8UC3, cv::Scalar(255,255,255));
    plot_curve(hist, sortie.O[1], Scalar(0,0,0), 512.0 / maxprob);
    sortie.nout = 2;
    sortie.outname[0] = "";
    sortie.outname[1] = "Histogramme luminance";
  }
  else if(sel == 2)
  {
    Mat I2;
    cvtColor(I, I2, CV_BGR2HSV);
    MatND hist[3];
    double maxprobs[3];
    //for(auto i = 0u; i < 3; i++)
    calc_histo(I2, hist[0], 0, 179, maxprobs[0]);
    calc_histo(I2, hist[1], 1, 255, maxprobs[1]);
    calc_histo(I2, hist[2], 2, 255, maxprobs[2]);

    sortie.O[1] = Mat(Size(512,512), CV_8UC3, cv::Scalar(255,255,255));
    plot_curve(hist[0], sortie.O[1], Scalar(0,0,0), 512.0 / maxprobs[0]);
    sortie.O[2] = Mat(Size(512,512), CV_8UC3, cv::Scalar(255,255,255));
    plot_curve(hist[1], sortie.O[2], Scalar(0,0,0), 512.0 / maxprobs[1]);
    sortie.O[3] = Mat(Size(512,512), CV_8UC3, cv::Scalar(255,255,255));
    plot_curve(hist[2], sortie.O[3], Scalar(0,0,0), 512.0 / maxprobs[2]);
    sortie.nout = 4;
    sortie.outname[0] = "";
    sortie.outname[1] = "Teinte / Hue";
    sortie.outname[2] = "Saturation";
    sortie.outname[3] = "Valeur";
    /*Mat I2;
    cvtColor(I, I2, CV_BGR2HSV);
    cv::Mat h[3];
    h[0] = get_1d_histogram_image(I2, 0, 179);
    h[1] = get_1d_histogram_image(I2, 1, 255);
    h[2] = get_1d_histogram_image(I2, 2, 255);
    O = h[0];
    O2 = h[1];
    O3 = h[2];
    nb_outputs = 3;*/
  }
  else if(sel == 3)
  {
    sortie.nout = 0;
    /*Mat I2;
    cvtColor(I, I2, CV_BGR2HSV);
    O = get_2d_histogram_image(I2);
    nb_outputs = 1;*/
  }
  return 0;
}


HistoDemo::HistoDemo()
{
  props.id = "histeq";
}


int HistoDemo::calcul(Node &model, cv::Mat &I)
{
  int sel = model.get_attribute_as_int("sel");

  // Egalisation luminance
  if(sel == 0)
  {
    Mat tmp;
    cvtColor(I, tmp, CV_BGR2YUV);
    Mat chns[3];
    split(tmp, chns);
    equalizeHist(chns[0], chns[0]);
    merge(chns, 3, tmp);
    sortie.nout = 2;
    cvtColor(tmp, sortie.O[1], CV_YUV2BGR);
  }
  // Egalisation 3 canaux RGB (pour voir les artefacts couleurs)
  else if(sel == 1)
  {
    Mat chns[3];
    split(I, chns);

    for(auto i = 0u; i < 3; i++)
      equalizeHist(chns[i], chns[i]);
    sortie.nout = 2;
    merge(chns, 3, sortie.O[1]);
  }

  return 0;
}


