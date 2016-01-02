# opencv-demonstrator
OpenCV demonstrator


## Introduction

The original **OpenCV demonstrator (GUI)** was written by Julien A. (www.tsdconseil.fr), and released at
[TSD Conseil](http://www.tsdconseil.fr/log/opencv/demo/index-en.html). This GitHub repository was created
from the original source code. The purpose of this GitHub repository is to enable open source contributions
for the original **OpenCV demonstrator (GUI)**. With the help of the OpenCV community we can make a very
exciting product!


## Setup

First, make sure you get the data (images, videos) for this project. It is a separate GitHub project, located
at [opencv-demonstrator-data](https://github.com/tsdconseil/opencv-demonstrator-data). Put the content's of
the **opencv-demonstrator-data** project into the folder `ocvdemo/data`. You can do this with a simple command:

    $ cd PATH_TO_SRC/opencv-demonstrator/ocvdemo
    $ git clone https://github.com/tsdconseil/opencv-demonstrator-data.git data

Second, at this stage, the build instruction are located in the file [README.txt](README.txt). The file was written
by Julien A. (www.tsdconseil.fr). In the near future, more detailed and thorough instructions will be written.


## Road map

- clean up source code
- write detailed and thorough build instructions for Windows, Linux, and Mac OSX
- create separate build scripts (makefiles) for Windows, Linux, and Mac OSX
- provide `deb` and `rpm` installation packages for Linux distributions
- provide a `brew` installation package for Mac OSX
- better documentation
- increase coverage of OpenCV functions
- tests
- GUI improvements (multiple filters, scripting support, etc.)


## License

The software is provided according to the GNU Lesser General Public License (LGPL) version 3. Please see
[LICENSE](LICENSE).
