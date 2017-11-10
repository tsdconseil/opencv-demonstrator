#!/bin/sh
cd ~/
sudo apt-get install libasound-dev
wget http://www.portaudio.com/archives/pa_stable_v190600_20161030.tgz
tar -zxvf pa_stable_v190600_20161030.tgz
rm -f pa_stable_v190600_20161030.tgz
cd portaudio
./configure && make
cp lib/.libs/libportaudio.a /usr/lib
cd ..
