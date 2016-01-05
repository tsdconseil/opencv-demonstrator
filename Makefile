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

all:
	@echo "Please specify a build target. Try one of:"
	@echo "    make windows"
	@echo "    make linux"
	@echo "    make clean"

windows:
	cd libcutil && make && cd ../ocvext && make && cd ../ocvdemo && make

linux:
	cd libcutil; export TARGET=LINUX; make
	cd ocvext; export TARGET=LINUX; make
	cd ocvdemo; export TARGET=LINUX; make

clean:
	rm -rf libcutil/build
	rm -rf ocvext/build
	rm -rf ocvdemo/build
