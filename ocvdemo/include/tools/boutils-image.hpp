
/**
 *  Copyright 2015 J.A. / http://www.tsdconseil.fr

    Project web page: http://www.tsdconseil.fr/log/opencv/demo/index-en.html
 */

#ifndef BOUTILS_IMAGE_HPP
#define BOUTILS_IMAGE_HPP

#include "mmi/stdview.hpp"

////////////////////////////////////////////////////////////////////////////
/** @brief Barre d'outil pour le choix d'un outil (gomme, pinceau, ...) */
class MasqueBOutils
{
public:
  /** Constructeur */
  MasqueBOutils();

  /** Montre la barre d'outils */
  void montre();

  /** Cache la barre d'outils */
  void cache();

  /** Mise à jour de la langue */
  void maj_lang();
private:
  Gtk::VBox vbox;
  Gtk::Window wnd;
  Gtk::Toolbar tools;
  Gtk::ToolButton b_raz;
  Gtk::ToggleToolButton b_gomme, b_remplissage;
  friend class OCVDemo;
};





#endif


