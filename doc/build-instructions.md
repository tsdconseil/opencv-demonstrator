# Build instructions


## Project directories

Source file are organized in 3 main folders:

 - ocvdemo: OpenCV demonstrator project
 - ocvext:  Some additional image processing routine (Deriche gradient and Hough transform)
 - libcutil: Helper classes to generate a MMI from a XML file


## BUILD INSTRUCTIONS FOR WINDOWS
(1) Ensure that the following packages are already installed:
  - MINGW (with GCC version >= 4.6.2)
  - MSYS
  - bison
  - flex
  - libgtkmm-3.0-dev
  - OpenCV 3.0

Important note: the difficulty of installing this various libraries on Windows should not be underestimated...

(2) Open a MSYS terminal, go at the folder where you extracted the sources, and follow Linux build instructions.


## BUILD INSTRUCTIONS FOR LINUX (UBUNTU)

These instructions will download **opencv-3.0.0**, **opencv_contrib-3.0.0**, and **OpenCV demonstrator (GUI)** into the
folder **`~/build_from_source`**. Feel free to use another folder, if you wish.

```
$ cd ~/
$ sudo aptitude update
$ sudo aptitude upgrade
$ sudo aptitude install build-essential cmake git pkg-config libjpeg8-dev libtiff4-dev \
    libjasper-dev libpng12-dev libavcodec-dev libavformat-dev libswscale-dev libv4l-dev \
    libgtk2.0-dev libatlas-base-dev gfortran libgtkmm-3.0-1 libgtkmm-3.0-dev \
    libgtkmm-3.0-doc bison flex
$ mkdir build_from_source
$ cd build_from_source/
$ wget https://github.com/Itseez/opencv/archive/3.0.0.tar.gz
$ tar -zxvf 3.0.0.tar.gz
$ rm -rf 3.0.0.tar.gz
$ wget https://github.com/Itseez/opencv_contrib/archive/3.0.0.tar.gz
$ tar -zxvf 3.0.0.tar.gz
$ rm -rf 3.0.0.tar.gz
$ cd opencv-3.0.0
$ mkdir build
$ cd build/
$ cmake \
    -DCMAKE_BUILD_TYPE=RELEASE \
    -DCMAKE_INSTALL_PREFIX=/usr/local \
    -DINSTALL_C_EXAMPLES=ON  \
    -DOPENCV_EXTRA_MODULES_PATH=~/build_from_source/opencv_contrib-3.0.0/modules \
    -DBUILD_EXAMPLES=ON \
    -DBUILD_SHARED_LIBS=ON \
    -DWITH_GTK=ON \
    ..
$ make --jobs=4
$ sudo make install
$ sudo ldconfig
$ cd ~/build_from_source
$ git clone https://github.com/tsdconseil/opencv-demonstrator.git
$ cd opencv-demonstrator/libcutil
$ export TARGET=LINUX
$ make
$ cd ../ocvext
$ make
$ cd ../ocvdemo
$ make
```

Notes:

- These instructions were tested on:
  *  **`Debian GNU/Linux 8 (jessie)`**
  * **`Ubuntu 14.04.3`**
  * **`Linux Mint 17.1 KDE`**
- While calling ocvdemo.exe, you can add the "-vv" option to see the debug traces on the terminal.
- The ".exe" extension may seem useless on Linux, but it's because the same Makefile is used on Windows and Linux. In
the future, it should be removed under Linux!
- You must be in the ocvdemo folder when calling ocvdemo.exe, because it will look for constant data files (xml, images)
relative to this folder.
- Only when this folder does not exist, then the application will look in the "/usr/share/ocvdemo/data/" folder for the
constant data files. But the data files should have been copied there through the installer / .deb package (which is not
written).
- There is another data folder that will be used by the application, where user specific files will be stored
(application journal, application configuration, language selection). This folder may be:
  * ~/.ocvdemo on Linux, where ~/ is the current user home folder.
  * c:/appdata/.../current user/.../roaming/ocvdemo on Windows (the exact path is dependent on Windows version).
