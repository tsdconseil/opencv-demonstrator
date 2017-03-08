/** @file demos-registration.cc
 *  Registration of the different demo items.

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

#include "ocvdemo.hpp"
#include "demo-items/filtrage.hpp"
#include "demo-items/morpho-demo.hpp"
#include "demo-items/gradient-demo.hpp"
#include "demo-items/photographie.hpp"
#include "demo-items/reco-demo.hpp"
#include "demo-items/histo.hpp"
#include "demo-items/seuillage.hpp"
#include "demo-items/video-demo.hpp"
#include "demo-items/espaces-de-couleurs.hpp"
#include "demo-items/3d.hpp"
#include "demo-items/misc.hpp"
#include "demo-items/segmentation.hpp"
#include "demo-items/appauto.hpp"
#include "demo-items/demo-skeleton.hpp"
#include "demo-items/ocr.hpp"
//add a line to include your new demo header file patterned just as "demo-skeleton.hpp"

void OCVDemo::add_demos()
{
  add_demo(new DetFlouDemo());
  add_demo(new ScoreShiTomasi());
  add_demo(new DemoRedim());
  add_demo(new DemoOCR());
  add_demo(new DemoSousSpectrale());
  add_demo(new DemoSuperpixels());
  add_demo(new DemoFaceRecognizer());
  add_demo(new DemoAppAuto());
  add_demo(new DemoHog());
  add_demo(new DemoLocalisation3D());
  add_demo(new DemoBalleTennis());
  add_demo(new DemoSqueletisation());
  add_demo(new DemoMahalanobis());
  add_demo(new DemoFiltreGabor());
  add_demo(new CameraDemo());
  add_demo(new RectificationDemo());
  add_demo(new StereoCalDemo());
  add_demo(new StereoCalLiveDemo());
  add_demo(new HDRDemo());
  add_demo(new EpiDemo());
  add_demo(new DispMapDemo());
  add_demo(new MatchDemo());
  add_demo(new ContourDemo());
  add_demo(new CamCalDemo());
  add_demo(new DFTDemo());
  add_demo(new InpaintDemo());
  add_demo(new PanoDemo());
  add_demo(new WShedDemo());
  add_demo(new SousArrierePlanDemo());
  add_demo(new DemoFiltrage());
  add_demo(new HSVDemo());
  add_demo(new MorphoDemo());
  add_demo(new CannyDemo());
  add_demo(new HoughDemo());
  add_demo(new HoughCDemo());
  add_demo(new GradientDemo());
  add_demo(new LaplaceDemo());
  add_demo(new NetDemo());
  add_demo(new CornerDemo());
  add_demo(new HistoEgalisationDemo());
  add_demo(new HistoCalc());
  add_demo(new HistoBP());
  add_demo(new Seuillage());
  add_demo(new GrabCutDemo());
  add_demo(new OptFlowDemo());
  add_demo(new CamShiftDemo());
  add_demo(new RectDemo());
  add_demo(new DTransDemo());
  add_demo(new CascGenDemo("casc-visage"));
  add_demo(new CascGenDemo("casc-profile"));
  add_demo(new CascGenDemo("casc-yeux"));
  add_demo(new CascGenDemo("casc-plate"));
  add_demo(new CascGenDemo("casc-sil"));
  add_demo(new SkeletonDemo());

// Here is the spot to add a new demo.  Copy/Paste and edit to match the name of your class.

}



