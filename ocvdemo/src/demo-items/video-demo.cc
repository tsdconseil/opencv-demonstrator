/** @file video-demo.cc

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

#include "demo-items/video-demo.hpp"
#include "demo-items/histo.hpp"
#include "opencv2/video/video.hpp"


CamShiftDemo::CamShiftDemo()
{
  props.id                = "camshift";
  props.requiert_roi         = true;
  bp_init_ok        = false;
  sortie.nout  = 2;
}

void CamShiftDemo::set_roi(const cv::Mat &I, const cv::Rect &new_roi)
{
  bp_init_ok = false;

  params.roi = new_roi;
  this->trackwindow = params.roi;

  if(params.roi.width * params.roi.height < 4)
  {
    journal.warning("set_roi: aire trop petite.");
    return;
  }

  /* Calcul de l'histogramme et backprojection */
  journal.trace_major("set roi(%d,%d,%d,%d): calc bp...",
      params.roi.x, params.roi.y, params.roi.width, params.roi.height);
  journal.verbose("img dim = %d * %d.", I.cols, I.rows);
  calc_hist(I, params.roi, hist);
  journal.verbose("fait.");
  bp_init_ok = true;
}

int CamShiftDemo::calcul(Node &model, cv::Mat &I)
{
  sortie.outname[1] = "back-projection histo.";
  if(!bp_init_ok)
  {
    auto sx = I.cols, sy = I.rows;
    journal.verbose("SX %d SY %d.", sx, sy);
    set_roi(I, Rect(sx/2-50,sy/3,20,20));//Rect(100,300,50,50));
  }
  if(bp_init_ok)
  {
    cv::MatND backproj;
    calc_bp(I, hist, backproj);

    sortie.O[0] = I;
    cvtColor(backproj, sortie.O[1], CV_GRAY2BGR);

    journal.verbose("camshift, %d...", trackwindow.width);
    cv::Rect tw3;
    tw3.width = trackwindow.width;
    tw3.height = trackwindow.height;
    RotatedRect trackbox = CamShift(backproj, trackwindow,
        TermCriteria( TermCriteria::EPS | TermCriteria::COUNT, 10, 1 ));
    journal.verbose("camshift ok, %d.", trackwindow.width);
    auto tw2 = trackbox.boundingRect();

    // Meme centre que tw2, mais meme largeur que tw1
    tw3.x = tw2.x + tw2.width / 2 - tw3.width / 2;
    tw3.y = tw2.y + tw2.height / 2 - tw3.height / 2;
    trackwindow = tw3;
    if(trackwindow.width * trackwindow.height > 0)
    {
      cv::rectangle(I, trackwindow, Scalar(255,255,0), 3);
      journal.verbose("trackwindow: %d, %d, %d, %d.",
		      trackwindow.x, trackwindow.y,
		      trackwindow.width, trackwindow.height);
    }
    else
      journal.warning("tracwindow: aire nulle.");
  }
  return 0;
}

SousArrierePlanDemo::SousArrierePlanDemo()
{
  props.id  = "sous-arriere-plan";
  nframes = 0;
  sortie.outname[1] = "masque";
  sortie.nout = 2;
  osel = -1;
  //this->mog2 = createBackgroundSubtractorMOG2();
}

void SousArrierePlanDemo::update_sel(int nsel)
{
  if(nsel != osel)
  {
    if(nsel == 0)
      algo = createBackgroundSubtractorMOG2();
    else if(nsel == 1)
      algo = createBackgroundSubtractorKNN();
    osel = nsel;
  }
}

int SousArrierePlanDemo::calcul(Node &model, cv::Mat &I)
{
  int sel = model.get_attribute_as_int("sel");
  update_sel(sel);

  
  Mat tmp, mask;

  /*if(I.empty())
  {
    journal.warning("%s: I is empty.", __func__);
    return -1;
  }*/

  resize(I,tmp,Size(0,0),0.25,0.25);

  //if(sel == 0)
    //mog(tmp, mask);
  //else
  //  mog2(tmp, mask);
  journal.trace("MOG...");
  algo->apply(tmp, mask);
  journal.trace("ok.");
  resize(mask,sortie.O[1],Size(0,0),4,4);

  nframes++;

  if(nframes < 5)
  {
    sortie.O[1] = I.clone();
    return 0;
  }

# ifdef OCV240

  if(mhi.data == nullptr)
    mhi = Mat::zeros(mask.size(), CV_32F);

  int maxhist = model.get_attribute_as_int("max-hist");
  journal.trace("updateMotionHistory...");
  updateMotionHistory(mask, mhi, nframes - 5, maxhist);

  Mat segmask = Mat::zeros(mask.size(), CV_32F);

  int seuil = model.get_attribute_as_int("seuil");

  std::vector<Rect> brects;
  journal.trace("segmentMotion...");
  segmentMotion(mhi, segmask, brects, nframes - 5, seuil);

  journal.trace("rects...");
  O[1] = tmp.clone();
  for(auto r: brects)
    cv::rectangle(O[1], r, Scalar(0,255,0));
  resize(O[1], O[1], Size(0,0), 4, 4);

  this->outname[1] = langue.get_item("mask-arr");
  this->outname[2] = "Segmentation";
# endif

  journal.trace("fin.");
  return 0;
}

OptFlowDemo::OptFlowDemo()
{
  props.id = "flux-optique";
  reset = true;
  algo = createOptFlow_DualTVL1();
  sortie.outname[1] = "Flux optique";
  sortie.nout = 2;
}

int OptFlowDemo::calcul(Node &model, cv::Mat &I)
{
  sortie.O[0] = I;

  //  computed flow image that has the same size as prev and type CV_32FC2
  Mat flow;

  Mat I1;
  Mat _hsv[3], hsv;
  Mat xy[2], mag, nmag, angle;

  if(reset)
  {
    reset = false;
    cv::cvtColor(I, Iprec, CV_BGR2GRAY);
    sortie.O[1] = Mat::zeros(I.size(), CV_8UC3);
    return 0;
  }

  cv::cvtColor(I, I1, CV_BGR2GRAY);
  calcOpticalFlowFarneback(Iprec, I1, flow, 0.5, 3, 15, 3, 5, 1.2, 0);
  Iprec = I1.clone();
  split(flow, xy);
  cartToPolar(xy[0],xy[1], mag, angle);
  double maxVal;
  minMaxLoc(mag, 0, &maxVal);
  normalize(mag,nmag,0,1.0,NORM_MINMAX);
  _hsv[0] = angle * ((360.0 / (2 * 3.141592))); // Teinte (couleur) = angle
  _hsv[1] = 1.0 * Mat::ones(angle.size(), CV_32F); // Chromaticité = 1
  _hsv[2] = nmag; // Luminance
  merge(_hsv, 3, hsv);
  sortie.O[1] = Mat(hsv.size(), CV_8UC3);
  cvtColor(hsv, sortie.O[1], cv::COLOR_HSV2BGR);
  sortie.O[1].convertTo(sortie.O[1], CV_8UC3, 255);
  journal.verbose("fait: %d * %d, depth = %d.", sortie.O[1].cols, sortie.O[1].rows, sortie.O[1].depth());
  return 0;
}

