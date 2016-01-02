*** OCVDEMO README.TXT ***
*** 18/12/2015, J.A., http://www.tsdconseil.fr ***
*** Project web page: http://www.tsdconseil.fr/log/opencv/demo/index-en.html ***

Source file are organized in 3 main folders:

 - ocvdemo: OpenCV demonstrator project
 - ocvext:  Some additionnal image processing routine (Deriche gradient and Hough transform)
 - libcutil: Helper classes to generate a MMI from a XML file


*** BUILD INSTRUCTIONS FOR WINDOWS ***
(1) Ensure that the following packages are already installed: 
  - MINGW (with GCC version >= 4.6.2)
  - MSYS
  - bison
  - flex
  - libgtkmm-3.0-dev
  - OpenCV 3.0

Important note: the difficulty of installing this various libraries on Windows should not be underestimated...

(2) Open a MSYS terminal, go at the folder where you extracted the sources, and follow Linux build instructions (from point (3)).

*** BUILD INSTRUCTIONS FOR LINUX (UBUNTU) ***
*** Note: not yet working due to an incompatibility between OpenCV and GTKMM 3.0 ***

(1) Ensure that the following packages are already installed: 
  - bison
  - flex
  - libgtkmm-3.0-dev
  - OpenCV 3.0

To install the first 3 ones can simply type on the command line shell:

  sudo apt-get install bison flex libgtkmm-3.0-dev -y -qq

To install / build OpenCV 3.0 (developpment version), please refer to the OpenCV website for instructions. Note: if possible, when building OpenCV binaries, disable GTK support in the OpenCV CMAKE configuration options.

 (2) Define the following environment variable (on the command line):
  export TARGET=LINUX

Note: This is used by the Makefile to detect that we are under Linux and not under Windows/MINGW. In the future, we could try to detect it automatically (without having to define an environment variable).

 (3) Always with the terminal, go inside libcutil folder, and build:
  cd libcutil; make

 (4) Now do the same in the ocvext and ocvdemo folders:
  cd ../ocvext; make
  cd ../ocvdemo; make

 (5) While you are still in the ocvdemo folder, you can now execute the demo:
  ./build/debug/ocvdemo.exe 

Notes: 
  - While calling ocvdemo.exe, you can add the "-vv" option to see the debug traces on the terminal,
  - The ".exe" extension may seem useless on Linux, but it's because the same Makefile is used on Windows and Linux. In the future, it should be removed under Linux!
  - You must be in the ocvdemo folder when calling ocvdemo.exe, because it will look for constant data files (xml, images) relative to this folder.
  - Only when this folder does not exist, then the application will look in the "/usr/share/ocvdemo/data/" folder for the constant data files. But the data files should have been copied there through the installer / .deb package (which is not written).
  - There is another data folder that will be used by the application, where user specific files will be stored (application journal, application configuration, language selection). This folder may be:
    - ~/.ocvdemo on Linux, where ~ is the current user home folder
    - c:/appdata/.../current user/.../roaming/ocvdemo on Windows (the exact path is dependant on Windows version).  


 










