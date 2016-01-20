/** @file ocv-demo-item.hpp
 *  @brief Classe de base pour toutes les démonstrations
 *
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

#ifndef OCVDEMO_ITEM_HPP
#define OCVDEMO_ITEM_HPP

#include "cutil.hpp"
#include "modele.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <stdlib.h>
#include <stdio.h>

using namespace cv;
using namespace utils::model;

////////////////////////////////////////////////////////////////////////////
/** @bref Classe de base pour toutes les démonstrations OpenCV */
/** @brief Base class for all OpenCV demonstrations */
class OCVDemoItem
{
public:

  /** @bref Propriétés d'une démonstration OpenCV */
  /** @brief Properties of an OpenCV demonstration */
  struct OCVDemoItemProperties
  {
    /** Chaine d'identification (pour retrouver le schéma XML) */
    std::string id;

    /** Nécessite un masque ? (exemple : inpainting) */
    bool requiert_masque;

    /** Nécessite la sélection d'une région d'intérêt ? */
    bool requiert_roi;

    /** Minimum number of input images for this demo.
     *  Default is 1 (needs at least one image). */
    int input_min;

    /** Maximum number of input images for this demo.
     *  -1 means "not specified" (e.g. accept arbitrary number of images). */
    int input_max;
  };

  /** Réglages d'une démonstration OpenCV */
  struct OCVDemoItemInput
  {
    /** Modèle (configuration du traitement) */
    Node model;

    /** Si oui, rectangle définissant la ROI */
    cv::Rect roi;

    /** Si oui, valeur du masque */
    cv::Mat mask;

    /** Liste des images d'entrée */
    /** List of input images */
    std::vector<Mat> images;
  };

  /** Sortie */
  /** @brief Output */
  struct OCVDemoItemOutput
  {
    /** Nombre d'images produites */
    /** @brief Number of output images */
    int nout;

    /** Images de sortie */
    /** @brief Output images */
    cv::Mat images[5];

    /** Nom des différentes images de sortie */
    /** @brief Names of the output images (UTF-8) */
    std::string names[5];

    /** Message d'erreur si échec */
    /** Error message in case of failure (UTF-8) */
    std::string errmsg;

    /** TO DEPRECATE? Pointeur vers la "vraie" image de sortie ???? to remove !!!! */
    Mat vrai_sortie;
  };

  /** Propriétés de la démo */
  OCVDemoItemProperties props;

  /** Réglages de la démo */
  OCVDemoItemInput input;

  /** Résultat de l'exécution de la démo */
  OCVDemoItemOutput output;

  /** Calcul effectif */
  /** @brief Overload this method to define the specific processing to be done for this demo */
  virtual int proceed(OCVDemoItemInput &input, OCVDemoItemOutput &output) = 0;

  /** Constructeur par défaut */
  OCVDemoItem();

  /** Destructeur */
  virtual ~OCVDemoItem(){}

  /** To deprecate? Appelé dès lors que la ROI a changé */
  virtual void set_roi(const cv::Mat &I, const cv::Rect &new_roi){input.roi = new_roi;}

  /** To deprecate? */
  virtual int configure_ui(){return 0;}

  /** To deprecate? */
  virtual void setup_model(Node &model) {}

protected:
  utils::Logable journal;
};





#endif /* OCVDEMO_ITEM_HPP_ */
