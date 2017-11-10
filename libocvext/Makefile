# Makefile pour LIBOCVEXT
#   Copyright 2017 J.A. / http://www.tsdconseil.fr

# Nom du projet
NOM_PROJET = libocvext

# Version de l'application
VMAJ   = 1
VMIN   = 0
VPATCH = 0

# Chemin vers libcutil
RPATH = ..

# Fichiers sources
SOURCES = gl hough fourier ocvext thinning vue-image vue-collection calib images-selection reco

# Exécutables
EXECS = etalonnage-camera
#test-fourier

# Dossiers où trouver les fichiers sources (autres que ./src and ./include)
AUTRES_DOSSIERS_SOURCES = src/outils src/test

# Libraries externes
LIBS_EXTERNES = $(RPATH)/libcutil 

# Autres options de compilation
OPTIONS_COMPIL = 

# Dépendances OpenCV
include $(RPATH)/libcutil/data/makefile-opencv320

# Makefile standard
include $(RPATH)/libcutil/data/std-makefile-defs




