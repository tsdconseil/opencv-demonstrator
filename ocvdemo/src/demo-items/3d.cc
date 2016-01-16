/** @file 3d.cc
 *  @brief Démonstratation relatives à la 3D

    Copyright 2015 J.A. / http://www.tsdconseil.fr

    Project web page: http://www.tsdconseil.fr/log/opencv/demo/index-en.html

    This file is part of OCVDemo.

    OCVDemo is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    OCVDemo is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with OCVDemo.  If not, see <http://www.gnu.org/licenses/>.
 **/

#include "demo-items/3d.hpp"
#include "opencv2/calib3d/calib3d.hpp"


//////////////////////////////////////////////////////////
/// STEREO CALIBRATION DEMO                       ////////
/// (TODO !)                                      ////////
//////////////////////////////////////////////////////////
StereoCalDemo::StereoCalDemo()
{
  props.id = "stereo-cal";
}

int StereoCalDemo::calcul(Node &model, cv::Mat &I)
{
  return 0;
}

//////////////////////////////////////////////////////////
/// EPIPOLAR LINE DEMO                            ////////
/// (TODO !)                                      ////////
//////////////////////////////////////////////////////////
EpiDemo::EpiDemo()
{
  props.id = "epi";
}

int EpiDemo::calcul(Node &model, cv::Mat &I)
{

  return 0;
}


//////////////////////////////////////////////////////////
/// DISPARITY MAP DEMO                            ////////
//////////////////////////////////////////////////////////
DispMapDemo::DispMapDemo()
{
  props.id = "disp-map";
  props.requiert_mosaique = 1;
  props.mosaique.min = 2;
  props.mosaique.max = 2;
  sortie.nout = 2;
}


int DispMapDemo::calcul(Node &model, cv::Mat &I)
{
  if(params.mosaique.size() != 2)
  {
    sortie.errmsg = "Disparity map demo: needs 2 input images.";
    return -1;
  }

# ifdef OCV240
  StereoBM sbm;
# else
  auto sbm = StereoBM::create();
# endif
  Mat disp, disp8;
  Mat imgs[2];
  cvtColor(params.mosaique[0], imgs[0], CV_BGR2GRAY);
  cvtColor(params.mosaique[1], imgs[1], CV_BGR2GRAY);
  //sbm(imgs[0], imgs[1], disp);
  sbm->compute(imgs[0], imgs[1], disp);
  normalize(disp, disp8, 0, 255, CV_MINMAX, CV_8U);

  sortie.O[0]  = imgs[0];
  sortie.O[1] = imgs[1];
  sortie.O[2] = disp8;
  sortie.nout = 3;
  sortie.outname[0] = "Image 1";
  sortie.outname[1] = "Image 2";
  sortie.outname[2] = langue.get_item("disp-map");
  return 0;
}



//////////////////////////////////////////////////////////
/// CAMERA CALIBRATION DEMO                       ////////
//////////////////////////////////////////////////////////

CamCalDemo::CamCalDemo()
{
  props.id = "cam-cal";
  sortie.nout = 2;
  sortie.outname[0] = "Detection des coins";
  sortie.outname[1] = "Distortion corrigee";
  //sortie.outname[2] = "Infos";
}



class MatText
{
public:
  MatText(Mat I)
  {
    this->I = I;
    x = 0; y = 0;
    last_dy = 20;
  }

  inline void print(std::string s, ...)
  {
    va_list ap;
    va_start(ap, s);
    printv(s, ap);
    va_end(ap);
  }
private:

  inline void printv(std::string s, va_list ap)
  {
    char tmp_buffer[1000];
    if(vsnprintf(tmp_buffer, 500, s.c_str(), ap) > 0)
      printi1(std::string(tmp_buffer));
  }

  void printi1(const std::string &s)
  {
    std::string tamp;
    for(auto i = 0u; i < s.length(); i++)
    {
      if(s[i] == '\n')
      {
        printi(tamp);
        x = 0;
        y += last_dy;
      }
      else
      {
        char c[2];
        c[0] = s[i];
        c[1] = 0;
        tamp += std::string(c);
      }
    }
    printi(tamp);
  }

  void printi(const std::string &s)
  {
    if(s.size() == 0)
      return;
    int baseLine;
    double tscale = 1.0;
    auto font = FONT_HERSHEY_COMPLEX_SMALL;
    Size si =  getTextSize(s, font, tscale, 1.2, &baseLine);
    putText(I, s,
            Point(x, y),
            font,
            tscale,
            Scalar(255,255,255),
            1.2,
            CV_AA);
    last_dy = si.height + 2;
    x += si.width;
    if(x >= (unsigned int) I.cols)
    {
      x = 0;
      y += si.height + 2;
    }
  }
  Mat I;
  uint32_t x, y;
  int last_dy;
};


int CamCalDemo::calcul(Node &model, cv::Mat &I)
{
  Mat Ig;
  int sel = model.get_attribute_as_int("sel");
  int bw = model.get_attribute_as_int("bw");
  int bh = model.get_attribute_as_int("bh");

  Size board_size(bw,bh);
  vector<vector<Point2f>> imagePoints;
  vector<Point2f> pointbuf;

  cvtColor(I, Ig, CV_BGR2GRAY);
  bool found;
  if(sel == 0)
  {
    found = cv::findChessboardCorners(Ig, board_size, pointbuf,
      CALIB_CB_ADAPTIVE_THRESH | CALIB_CB_FAST_CHECK | CALIB_CB_NORMALIZE_IMAGE);
    // improve the found corners' coordinate accuracy
     if(found)
       cv::cornerSubPix(Ig, pointbuf, Size(11,11),
         Size(-1,-1), TermCriteria( TermCriteria::EPS+TermCriteria::COUNT, 30, 0.1 ));
  }
  else if(sel == 1)
    found = cv::findCirclesGrid(Ig, board_size, pointbuf );
  else
    found = cv::findCirclesGrid(Ig, board_size, pointbuf, CALIB_CB_ASYMMETRIC_GRID );


  //cvtColor(I, O[0], CV_GRAY2BGR);

  Mat Ior = I.clone();
  sortie.O[0] = I;
  if(found)
   cv::drawChessboardCorners(sortie.O[0], board_size, Mat(pointbuf), found);

  journal.trace_major("Trouvé %d coins (found = %d).",
      pointbuf.size(), (int) found);

  if(found)
  {
    Mat distCoeffs;
    Mat cameraMatrix;
    cameraMatrix = Mat::eye(3, 3, CV_64F);
    //if( flags & CALIB_FIX_ASPECT_RATIO )
      //  cameraMatrix.at<double>(0,0) = aspectRatio;
    distCoeffs = Mat::zeros(8, 1, CV_64F);

    float square_size = 1;

    std::vector<std::vector<Point3f> > objectPoints(1);
    std::vector<Point3f> &corners = objectPoints[0];
    if(sel <= 1)
    {
      for( int i = 0; i < board_size.height; i++ )
            for( int j = 0; j < board_size.width; j++ )
                corners.push_back(Point3f(float(j*square_size),
                                          float(i*square_size), 0));
    }
    else
    {
      for( int i = 0; i < board_size.height; i++ )
          for( int j = 0; j < board_size.width; j++ )
              corners.push_back(Point3f(float((2*j + i % 2)*square_size),
                                        float(i*square_size), 0));
    }

    imagePoints.push_back(pointbuf);
    objectPoints.resize(imagePoints.size(),objectPoints[0]);

    vector<Mat> rvecs, tvecs;
    // Fonction obsoléte ?
    double rms = cv::calibrateCamera(objectPoints,
                                     imagePoints, I.size(),
                    cameraMatrix, distCoeffs, rvecs, tvecs,
                    CALIB_FIX_K4 | CALIB_FIX_K5);
    journal.trace("RMS error reported by calibrateCamera: %g\n", rms);


    cv::undistort(Ior, sortie.O[1], cameraMatrix, distCoeffs);

    Size sz = I.size();
    sz.height = sz.width = max(sz.width, sz.height);
    sz.height = sz.width = max(sz.width, 500);

    sortie.O[2] = cv::Mat::zeros(sz, CV_8UC3);

    double fovx, fovy, focal, ar;
    Point2d ppoint;
    cv::calibrationMatrixValues(cameraMatrix, I.size(), 1, 1, fovx, fovy, focal, ppoint, ar);


    MatText mt(sortie.O[2]);

    std::stringstream str;

    mt.print("camera matrix: \n");

    str << cameraMatrix << "\n";

    mt.print(str.str());


    //bool ok = checkRange(cameraMatrix) && checkRange(distCoeffs);
    //totalAvgErr = computeReprojectionErrors(objectPoints, imagePoints,
    //            rvecs, tvecs, cameraMatrix, distCoeffs, reprojErrs);
  }

  return 0;
}




