# Global Makefile for ocvdemo 
# (this just calls the libcutil, ocvext, and ocvdemo Makefiles)
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

UNAME := $(shell uname)


# Path to repository root directy


ifeq ($(UNAME), MINGW32_NT-6.2)
	T := WIN
else
	T := LINUX
endif

all:
	cd libcutil; export TARGET=$(T); make
	cd ocvext; export TARGET=$(T); make
	cd ocvdemo; export TARGET=$(T); make

clean:
	rm -rf libcutil/build
	rm -rf ocvext/build
	rm -rf ocvdemo/build
