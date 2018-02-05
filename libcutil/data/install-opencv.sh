#!/bin/sh
cd ~/
sudo apt update
sudo apt upgrade
sudo apt install build-essential cmake pkg-config libjpeg8-dev libtiff5-dev libjasper-dev libpng12-dev \
libavcodec-dev libavformat-dev libswscale-dev \
libv4l-dev libgtk2.0-dev libatlas-base-dev gfortran
wget https://github.com/Itseez/opencv/archive/3.2.0.tar.gz
tar -zxvf 3.2.0.tar.gz
rm -rf 3.2.0.tar.gz
cd opencv-3.2.0
mkdir build
cd build/
cmake -DCMAKE_BUILD_TYPE=RELEASE -DCMAKE_INSTALL_PREFIX=/usr/local -DINSTALL_C_EXAMPLES=OFF -DBUILD_EXAMPLES=OFF -DBUILD_SHARED_LIBS=ON -DWITH_GTK=OFF ../
make --jobs=4
sudo make install
sudo ldconfig
 
