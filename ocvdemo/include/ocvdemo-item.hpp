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
/** @brief Classe de base pour toutes les démonstrations OpenCV */
class OCVDemoItem
{
public:

  /** Propriétés d'une démonstration OpenCV */
  struct OCVDemoItemProprietes
  {
    /** Chaine d'identification (pour retrouver le schéma XML) */
    std::string id;

    /** Nécessite un masque ? (exemple : inpainting) */
    bool requiert_masque;

    /** Nécessite la sélection d'une région d'intérêt ? */
    bool requiert_roi;

    /** Nécessite la sélection d'une suite d'image ? */
    int requiert_mosaique;

    struct MosaiqueProps
    {
      int min, max; // Nombre min / max d'images nécessaires
      MosaiqueProps(){min = -1; max = -1;}
    } mosaique;
  };

  /** Réglages d'une démonstration OpenCV */
  struct OCVDemoItemParams
  {
    /** Modèle (configuration du traitement) */
    Node modele;

    /** Si oui, rectangle définissant la ROI */
    cv::Rect roi;

    /** Si oui, valeur du masque */
    cv::Mat masque;

    /** Si oui, liste des images d'entrée */
    std::vector<Mat> mosaique;
  };

  /** Sortie */
  struct OCVDemoItemSortie
  {
    /** Nombre d'images produites */
    int nb_sorties;

    /** Images de sortie */
    cv::Mat O[5];

    /** Nom des différentes images de sortie */
    std::string outname[5];

    /** Message d'erreur si échec */
    std::string errmsg;

    /** TO DEPRECATE? Pointeur vers la "vraie" image de sortie ???? to remove !!!! */
    Mat vrai_sortie;
  };

  /** Propriétés de la démo */
  OCVDemoItemProprietes props;

  /** Réglages de la démo */
  OCVDemoItemParams params;

  /** Résultat de l'exécution de la démo */
  OCVDemoItemSortie sortie;

  /** Calcul effectif */
  virtual int calcul(Node &model, cv::Mat &I) = 0;

  /** Constructeur */
  OCVDemoItem()
  {
    sortie.nb_sorties = -1;
    props.requiert_roi = false;
    props.requiert_masque = false;
    props.requiert_mosaique = false;
    journal.setup("ocvdemo-item","");
  }

  /** Destructeur */
  virtual ~OCVDemoItem(){}

  /** To deprecate? Appelé dès lors que la ROI a changé */
  virtual void set_roi(const cv::Mat &I, const cv::Rect &new_roi){params.roi = new_roi;}

  /** To deprecate? */
  virtual int configure_ui(){return 0;}

  /** To deprecate? */
  virtual void setup_model(Node &model) {}

protected:
  utils::Logable journal;
};





#endif /* OCVDEMO_ITEM_HPP_ */
