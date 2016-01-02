Description of the folder ocvdemo (OpenCV demonstrator source code):

The code is organized this way:
 
 - Makefile: for project compilation (needs GCC >= 4.7, OpenCV 3.0, GTKMM 3.0, and works only on Windows for now ; on linux, the last time I tried, I had compatibility problems between OpenCV and GTK, yet it was with OpenCV 2.4 at this time).
 - include:  the header files (the main file is ocvdemo.hpp)
 - src:      the source files
 - data:     the XML data files (contains the MMI specification)
 - install:  NSIS installer script (for windows)

18/12/2015, J.A., http://www.tsdconseil.fr
Lastest version available on the following page: http://www.tsdconseil.fr/log/opencv/demo/index-en.html
