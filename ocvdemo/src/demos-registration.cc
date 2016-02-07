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
#include "demo-items/filter-demo.hpp"
#include "demo-items/morpho-demo.hpp"
#include "demo-items/gradient-demo.hpp"
#include "demo-items/photographie.hpp"
#include "demo-items/reco-demo.hpp"
#include "demo-items/histo.hpp"
#include "demo-items/seuillage.hpp"
#include "demo-items/video-demo.hpp"
#include "demo-items/espaces-de-couleurs.hpp"
#include "demo-items/3d.hpp"
#include "demo-items/demo-skeleton.hpp"

//add a line to include your new demo header file patterned just as demo-skeleton.hpp"

void OCVDemo::add_demos()
{
  items.push_back(new RectificationDemo());
  items.push_back(new StereoCalDemo());
  items.push_back(new HDRDemo());
  items.push_back(new EpiDemo());
  items.push_back(new DispMapDemo());
  items.push_back(new MatchDemo());
  items.push_back(new ContourDemo());
  items.push_back(new CamCalDemo());
  items.push_back(new DFTDemo());
  items.push_back(new InpaintDemo());
  items.push_back(new PanoDemo());
  items.push_back(new WShedDemo());
  items.push_back(new SousArrierePlanDemo());
  items.push_back(new FilterDemo());
  items.push_back(new HSVDemo());
  items.push_back(new MorphoDemo());
  items.push_back(new CannyDemo());
  items.push_back(new HoughDemo());
  items.push_back(new HoughCDemo());
  items.push_back(new GradientDemo());
  items.push_back(new LaplaceDemo());
  items.push_back(new NetDemo());
  items.push_back(new CornerDemo());
  items.push_back(new HistoEgalisationDemo());
  items.push_back(new HistoCalc());
  items.push_back(new HistoBP());
  items.push_back(new Seuillage());
  items.push_back(new GrabCutDemo());
  items.push_back(new OptFlowDemo());
  items.push_back(new CamShiftDemo());
  items.push_back(new RectDemo());
  items.push_back(new DTransDemo());
  items.push_back(new CascGenDemo("casc-visage"));
  items.push_back(new CascGenDemo("casc-profile"));
  items.push_back(new CascGenDemo("casc-yeux"));
  items.push_back(new CascGenDemo("casc-plate"));
  items.push_back(new CascGenDemo("casc-sil"));
  items.push_back(new SkeletonDemo());

// Here is the spot to add a new demo.  Copy/Paste and edit to match the name of your class.

}



