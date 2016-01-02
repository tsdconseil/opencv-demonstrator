# Makefile pour mini-librairie de fonctions additionnelles compatibles OpenCV
# (C) J.A. / 2015 / www.tsdconseil.fr
# Licence : LGPL V 3.0

vpath %.cc   src test

SRC := gl.cc hough.cc
OBJ := $(patsubst %.cc,build/%.o,$(SRC))
LIB := ./build/ocvext.lib
COPT = -std=c++0x -g -O1 -Wall -Iinclude 
COPT += -I"/c/downloads/ocv3/release/install/include"

ifeq ($(TARGET),LINUX)
LOPT = -lopencv_core -lopencv_highgui -lopencv_contrib
else
COPT += -march=i486 
LOPT = -L"/c/downloads/ocv3/release/install/x86/mingw/lib" -march=i486 -lopencv_core300 -lopencv_imgproc300 -lopencv_highgui300 -lopencv_imgcodecs300 
endif

all: build/test.exe $(LIB)

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
