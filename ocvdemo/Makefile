# Makefile for ocvdemo
#
#   Copyright 2015 J.A. / http://www.tsdconseil.fr
#
#   Project web page: http://www.tsdconseil.fr/log/opencv/demo/index-en.html
#
#   This file is part of OCVDemo.
#
#   OCVDemo is free software: you can redistribute it and/or modify
#   it under the terms of the GNU Lesser General Public License as published by
#   the Free Software Foundation, either version 3 of the License, or
#   (at your option) any later version.
#
#   OCVDemo is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU Lesser General Public License for more details.
#
#   You should have received a copy of the GNU Lesser General Public License
#   along with OCVDemo.  If not, see <http://www.gnu.org/licenses/>.

# Project name
PROJECT= ocvdemo

# Path to repository root directy
RPATH = ..

# Definition of individual files to compiled
SOURCES_BASE= image-selecteur photographie image-mosaic filter-demo morpho-demo gradient-demo reco-demo histo seuillage video-demo espaces-de-couleurs 3d boutils-image

# (Optionnal) Sources requiring a graphic toolkit -> will not be included in the reduced library
SOURCES_IHM= 

# (Optionnal) Executable targets to be build (each one must have an associated source file)
EXECS:= ocvdemo test-webcam

# (Optionnal) Special directories to locate the source files 
# (not counting ./src and ./include) 
ADDITIONNAL_SRC_DIRS = src/tools src/demo-items

OCV_VER = 300
# 240

ADDITIONNAL_INC_DIRS= -I ../ocvext/include -I$(RPATH)/libcutil/include -I$(RPATH)/libdsp/include 

ifeq ($(OCV_VER), 300)
ifeq ($(TARGET),LINUX)
else
ADDITIONNAL_INC_DIRS += -I"/c/downloads/ocv3/release/install/include" -march=i486 -DOCV300
endif
else
ADDITIONNAL_INC_DIRS += -I"/c/downloads/ocv/opencv/build/include" -DOCV240
endif

# External libraries
EXTERNAL_LIBS = $(RPATH)/libcutil

# (Optionnal) Additionnal targets to be build after library and executables
ADDITIONNAL_TARGETS = 

FIRST_TARGETS =

ifeq ($(TARGET),LINUX)
ADDITIONNAL_LOPT = ../ocvext/build/ocvext.lib -lopencv_core -lopencv_highgui -lopencv_objdetect -lopencv_features2d -lopencv_stitching
else
ADDITIONNAL_LOPT = ../ocvext/build/ocvext.lib -mwindows
ifeq ($(OCV_VER), 300)
ADDITIONNAL_LOPT += -L"/c/downloads/ocv3/release/install/x86/mingw/lib" -lopencv_core300 -lopencv_imgproc300 -lopencv_highgui300 -lopencv_video300 -lopencv_videoio300 -lopencv_features2d300 -lopencv_objdetect300 -lopencv_imgcodecs300 -lopencv_calib3d300 -lopencv_stitching300 -lopencv_photo300 -lfftw3-3 -lfftw3f-3 -march=i486
else
ADDITIONNAL_LOPT += -L"/c/downloads/ocv/opencv/build/x86/mingw/lib" -lopencv_core246 -lopencv_imgproc246 -lopencv_highgui246 -lopencv_video246 -lopencv_features2d246 -lopencv_objdetect246 -lopencv_contrib246 -lopencv_video246 -lopencv_calib3d246 -lfftw3-3 -lfftw3f-3 
endif
endif

ADDITIONNAL_CLEAN = 

# Common definitions for Makefiles and build rules
include $(RPATH)/libcutil/data/std-makefile-defs
