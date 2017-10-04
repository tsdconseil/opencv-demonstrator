# LIBCUTIL MAKEFILE

# This file is part of LIBCUTIL.
# 
# LIBCUTIL is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
# 
# LIBCUTIL is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
# 
# You should have received a copy of the GNU Lesser General Public License
# along with LIBCUTIL.  If not, see <http://www.gnu.org/licenses/>.
# 
# Copyright 2007-2012 ja

# Project name
PROJECT= libcutil

# Path to repository root directy
RPATH = ..

# Definition of individual files to compiled
SOURCES_BASE =  cutil  crc journal bytearray serial fdserial \
         serial-enumeration serial-session socket iostreams \
         hal pugixml mxml modele modele-wrapper erreurs misc theme

SOURCES_EXT = 

# (Optionnal) Sources requiring a graphic toolkit -> will not be included in the reduced library
SOURCES_GTK= gtkutil stdview ColorCellEditable2 ColorCellRenderer2 renderers field-view mmi-gen docking

SOURCES_GTK += 

# (Optionnal) Executable targets to be build (each one must have an associated source file)
EXECS:=   model-editor rcgene schema2doc preprocess

# (Optionnal) Special directories to locate the source files 
# (not counting src and include) 
ADDITIONNAL_SRC_DIRS = src/utilities src/mmi src/comm src/img

ADDITIONNAL_INC_DIRS= 

# External libraries
EXTERNAL_LIBS = 

# (Optionnal) Additionnal targets to be build after library and executables
FIRST_TARGETS = 

ADDITIONNAL_OBJS = 

ifeq ($(OCVIPATH),)
 # OCVIPATH not defined, assume that OpenCV is installed in default directories (e.g. /usr/include/, etc.)
else
 # OCVIPATH is defined: it should be the path where opencv was builded
ADDITIONNAL_INC_DIRS += -I$(OCVIPATH)
ADDITIONNAL_LOPT     += -L$(OCVLPATH) -lopencv_core310 -lopencv_imgproc310 -lopencv_highgui310 -lopencv_videoio310 -lopencv_imgcodecs310 -march=i486
endif

# Common definitions for Makefiles and build rules
include $(RPATH)/libcutil/data/std-makefile-defs

