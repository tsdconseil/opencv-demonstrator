# Makefile pour mini-librairie de fonctions additionnelles compatibles OpenCV
# (C) J.A. / 2015 / www.tsdconseil.fr
# Licence : LGPL V 3.0

vpath %.cc   src test

SRC := gl.cc hough.cc
OBJ := $(patsubst %.cc,build/%.o,$(SRC))
LIB := ./build/ocvext.lib
COPT = -std=c++0x -g -O1 -Wall -Iinclude

ifeq ($(TARGET),LINUX)
LOPT = -lopencv_core -lopencv_highgui -lopencv_imgproc -lopencv_imgcodecs
else
COPT += -march=i486
LOPT =  -march=i486 -lopencv_core300 -lopencv_imgproc300 -lopencv_highgui300 -lopencv_imgcodecs300
endif

ifeq ($(OCVIPATH),)
 # OCVIPATH not defined, assume that OpenCV is installed in default directories (e.g. /usr/include/, etc.)
else
 # OCVIPATH is defined: it should be the path where opencv was builded
COPT += -I$(OCVIPATH)
LOPT += -L$(OCVLPATH)
endif

all: check-dirs build/test.exe $(LIB)

check-dirs: 
	@test -d ./build || mkdir ./build

build/%.o:  %.cc
	@echo "Construction $@..."
	@g++ $(COPT) -c $< -o $@

build/%.exe:  ./build/tests.o $(LIB)
	@echo "Construction $@..."
	g++ -g $^ -o $@ $(LOPT)

$(LIB): $(OBJ)
	ld -r $(OBJ) -o $(LIB)

clean:
	rm build/*
