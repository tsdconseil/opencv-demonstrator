#!/bin/sh

# Automatic installation script of standard development tools and libraries: 
# opencv, fftw, valgrind, bison, flex, gtk, emacs, ...
# This script must run with admistrative privilege, e.g.:
# $ sudo ./install-libs.sh

IOPT = -y -qq
INST = apt-get install 

echo -e "Updating database for apt-get..."
apt-get update

echo -e "Installing external libraries..."

echo " - valgrind (outil de profiling C/C++).."
$INST valgrind $IOPT

echo " - Open CV (image processing toolkit).."
$INST libcv-dev libcvaux-dev libhighgui-dev  $IOPT

echo " - FFTW.."
$INST libfftw3-dev  $IOPT

echo " - Bison / Flex (generation parser / lexer).."
$INST bison $IOPT
$INST flex $IOPT

echo " - GTKMM (toolkit graphique).."
$INST libgtkmm-3.0-dev $IOPT

echo " - emacs (editeur fichiers sources).."
$INST emacs23 $IOPT


echo " - doxygen.."
$INST doxygen $IOPT
$INST doxygen-gui $IOPT
echo " - graphviz (outil de generation auto. de diagrammes).."
$INST graphviz $IOPT

# Ceci permet d'utiliser des espaces à la place des caractères de tabulation.
# (dans emacs)
echo "\nConfiguration emacs.. "
touch ~/.emacs
echo "(setq-default indent-tabs-mode nil) (setq inhibit-startup-screen t) (add-hook 'emacs-startup-hook 'delete-other-windows) (setq inhibit-startup-buffer-menu t)" >> ~/.emacs
# ;; open with single window
printf "ok.\n"

# Ceci permet de configurer la cible par défaut des fichiers Makefile
echo "\nConfiguration des cibles Makefile..  "
echo "export TARGET=LINUX" >> ~/.bashrc
printf "ok.\n"


echo "\n/////////////////////////////////"
echo "Installation des librairies terminee."


