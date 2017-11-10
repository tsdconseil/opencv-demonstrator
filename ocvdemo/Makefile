# Makefile pour digital fingerprint enhancing 
#   Copyright 2017 J.A. / http://www.tsdconseil.fr

# Nom du projet
NOM_PROJET = ocvdemo

# Version de l'application
VMAJ   = 1
VMIN   = 3
VPATCH = 0

# Chemin vers libcutil, libdsp, libocvext
RPATH = ..

# Fichiers sources
SOURCES = ocr image-selecteur photographie image-mosaic filtrage morpho-demo gradient-demo reco-demo histo seuillage video-demo espaces-de-couleurs 3d boutils-image demos-registration gestion-souris ocvdemo-mmi ocvdemo-export demo-skeleton misc segmentation appauto ocvdemo-misc demo-fourier

# Exécutables
EXECS = ocvdemo test-webcam

# Dossiers où trouver les fichiers sources (autres que ./src and ./include)
AUTRES_DOSSIERS_SOURCES = src/tools src/demo-items

# Libraries externes
LIBS_EXTERNES = $(RPATH)/libcutil $(RPATH)/libocvext 
#$(RPATH)/site

# Autres options de link
OPTIONS_LINK = 

# Pour pouvoir compiler les fichiers d'en-tête de libarf
OPTIONS_COMPIL = 

# Dépendances OpenCV
include $(RPATH)/libcutil/data/makefile-opencv320

# Makefile standard
include $(RPATH)/libcutil/data/std-makefile-defs




