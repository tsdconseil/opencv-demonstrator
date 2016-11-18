/** @file reco-demo.cc

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

#include "demo-items/reco-demo.hpp"

#include "opencv2/face.hpp"

#ifdef OCV240
# include "opencv2/stitching/stitcher.hpp"
#else
# include "opencv2/stitching.hpp"
#endif
#include "opencv2/calib3d/calib3d.hpp"



MatchDemo::MatchDemo()
{
  props.id = "corner-match";
  props.input_min = 2;
  props.input_max = 2;
  lock = false;
}


int MatchDemo::proceed(OCVDemoItemInput &input, OCVDemoItemOutput &output)
{
  //if(!lock)
  {
    lock = true;


    imgs = input.images;

    /*if(imgs[0].size() != imgs[1].size())
    {
      journal.warning("imgs[0].size() != imgs[1].size(): %d*%d, %d*%d",
          imgs[0].cols, imgs[0].rows,
          imgs[1].cols, imgs[1].rows);
      output.nout = 0;
      return 0;
    }*/

    output.nout = 1;

    Ptr<cv::Feature2D> detecteur;

    int sel = input.model.get_attribute_as_int("sel");
    if(sel == 0)
      detecteur = cv::ORB::create(input.model.get_attribute_as_int("npts")); // OCV 3.0
    else
      detecteur = cv::BRISK::create();

    std::vector<cv::KeyPoint> kpts[2];
    Mat desc[2];


    // OCV 3.0
    detecteur->detectAndCompute(imgs[0], Mat(), kpts[0], desc[0]);
    detecteur->detectAndCompute(imgs[1], Mat(), kpts[1], desc[1]);

    if((kpts[0].size() == 0) || (kpts[1].size() == 0))
    {
      output.nout = 1;
      output.images[0] = input.images[0].clone();
      return 0;
    }

    bool cross_check = input.model.get_attribute_as_boolean("cross-check");
    cv::BFMatcher matcher(NORM_HAMMING, cross_check);
    std::vector<std::vector<DMatch>> matches;
    matcher.knnMatch(desc[0], desc[1], matches, 2);
    std::vector<DMatch> good_matches;
    auto SEUIL_RATIO = input.model.get_attribute_as_float("seuil-ratio");
//#    define SEUIL_RATIO .5f
    //0.65f

    for(auto &match: matches)
    {
      // Si moins de 2 voisins, on ignore
      if(match.size() >= 2)
      {
        float ratio = match[0].distance / match[1].distance;
        if(ratio < SEUIL_RATIO)
          good_matches.push_back(match[0]);
      }
    }


#   if 0
    double max_dist = 0; double min_dist = 100;

    //-- Quick calculation of max and min distances between keypoints
    for( int i = 0; i < matches.size(); i++ )
    {
      double dist = matches[i].distance;
      if( dist < min_dist ) min_dist = dist;
      if( dist > max_dist ) max_dist = dist;
    }

    printf("-- Max dist : %f \n", max_dist );
    printf("-- Min dist : %f \n", min_dist );

    //-- Draw only "good" matches (i.e. whose distance is less than 3*min_dist )
    std::vector<DMatch> good_matches;

    int seuil = model.get_attribute_as_float("seuil");

    for(int i = 0; i < matches.size(); i++)
    {
      if(matches[i].distance < seuil/**min_dist*/)
        good_matches.push_back(matches[i]);
    }
#   endif


    output.images[0] = Mat::zeros(Size(640*2,480*2), CV_8UC3);

    if(good_matches.size() > 0)
      cv::drawMatches(imgs[0], kpts[0], imgs[1], kpts[1], good_matches, output.images[0]);

    journal.trace("draw match ok: %d assoc, %d ok.",
        matches.size(), good_matches.size());

#   if 0
    if(good_matches.size() > 8)
    {
    //-- Localize the object
      std::vector<Point2f> obj;
      std::vector<Point2f> scene;

      for(unsigned int i = 0u; i < good_matches.size(); i++ )
      {
        //-- Get the keypoints from the good matches
        obj.push_back( kpts[0][ good_matches[i].queryIdx ].pt );
        scene.push_back( kpts[1][ good_matches[i].trainIdx ].pt );
      }

      Mat H = findHomography(obj, scene, CV_RANSAC);

      //-- Get the corners from the image_1 ( the object to be "detected" )
      std::vector<Point2f> obj_corners(4);

      obj_corners[0] = cvPoint(0,0);
      obj_corners[1] = cvPoint(imgs[0].cols, 0 );
      obj_corners[2] = cvPoint(imgs[0].cols, imgs[0].rows );
      obj_corners[3] = cvPoint( 0, imgs[0].rows );

      std::vector<Point2f> scene_corners(4);
      cv::perspectiveTransform(obj_corners, scene_corners, H);

      Point2f dec(imgs[0].cols, 0);

#if 0
      //-- Draw lines between the corners (the mapped object in the scene - image_2 )
      line(I, scene_corners[0] + dec,
                        scene_corners[1] + dec,
                        Scalar(0, 255, 0), 4 );
      line(I, scene_corners[1] + dec,
                        scene_corners[2] + dec,
                        Scalar( 0, 255, 0), 4 );
      line(I, scene_corners[2] + dec,
                        scene_corners[3] + dec,
                        Scalar( 0, 255, 0), 4 );
      line(I, scene_corners[3] + dec,
                        scene_corners[0] + dec,
                        Scalar( 0, 255, 0), 4 );
#   endif
    }
#   endif

    output.names[0] = "Correspondances";
    lock = false;
  }
  return 0;
}


// Panorama:
// http://study.marearts.com/2013/11/opencv-stitching-example-stitcher-class.html
// http://ramsrigoutham.com/2012/11/22/panorama-image-stitching-in-opencv/
//

PanoDemo::PanoDemo()
{
  props.id = "pano";
  lock = false;
  props.input_max = -1;
}

int PanoDemo::proceed(OCVDemoItemInput &input, OCVDemoItemOutput &output)
{
  if(!lock)
  {
    lock = true;
    Stitcher stitcher = Stitcher::createDefault();

    Mat pano;

    auto t0 = getTickCount();
    auto status = stitcher.stitch(input.images, pano);
    t0 = getTickCount() - t0;

    output.images[0] = pano;
//    output.vrai_sortie = pano; // to deprecate
    output.names[0] = "Panorama";

    journal.verbose("%.2lf sec\n",  t0 / getTickFrequency());

    if (status != Stitcher::OK)
    {
     journal.warning("échec pano.");
     return -1;
    }
    lock = false;
  }
  return 0;
}



CornerDemo::CornerDemo()
{
  props.id = "corner-det";
}

int CornerDemo::proceed(OCVDemoItemInput &input, OCVDemoItemOutput &output)
{
  int sel = input.model.get_attribute_as_int("sel");
  int max_pts = input.model.get_attribute_as_int("max-pts");
  Ptr<FeatureDetector> detector;

  // Shi-Tomasi
  if(sel == 0)
  {
#   ifdef OCV240
    detector = new GoodFeaturesToTrackDetector(1000, 0.01, 10, 3, false);
#   else
    detector = GFTTDetector::create(max_pts, 0.01, 10, 3, false);
#   endif
  }
  // Harris
  else if(sel == 1)
  {
#   ifdef OCV240
    detector = new GoodFeaturesToTrackDetector(1000, 0.01, 10, 3, true);
#   else
    detector = GFTTDetector::create(max_pts, 0.01, 10, 3, true);
#   endif
  }
  // FAST
  else if(sel == 2)
  {
#   ifdef OCV240
    detector = FeatureDetector::create("FAST");
#   else
    detector = FastFeatureDetector::create();
#   endif
  }
  // SURF
  else if(sel == 3)
  {
#   ifdef OCV240
    detector = FeatureDetector::create("ORB");
#   else
    detector = ORB::create(max_pts);
#   endif
  }

  Mat gris;
  //GoodFeaturesToTrackDetector harris_detector(1000, 0.01, 10, 3, true );
  vector<KeyPoint> keypoints;
  cvtColor(input.images[0], gris, CV_BGR2GRAY);

  journal.trace("detection...");
  detector->detect(gris, keypoints);

  //if(keypoints.size() > max_pts)
    //keypoints.resize(max_pts);
  output.images[0] = input.images[0].clone();
  journal.trace("drawK");
  drawKeypoints(input.images[0], keypoints, output.images[0], Scalar(0, 0, 255));
  journal.trace("ok");
  return 0;
}


VisageDemo::VisageDemo(): rng(12345)
{
  String face_cascade_name = "./data/cascades/haarcascade_frontalface_alt.xml";
  String eyes_cascade_name = "./data/cascades/haarcascade_eye_tree_eyeglasses.xml";
  props.id = "casc-visage";
  //-- 1. Load the cascades
  if(!face_cascade.load(face_cascade_name))
  {
    journal.anomaly("--(!)Error loading\n");
    return;
  }
  if(!eyes_cascade.load(eyes_cascade_name))
  {
    journal.anomaly("--(!)Error loading\n");
    return;
  }
  output.names[0] = " ";
}


int VisageDemo::proceed(OCVDemoItemInput &input, OCVDemoItemOutput &output)
{
  std::vector<Rect> faces;
  Mat frame_gray;
  auto I = input.images[0];
  cvtColor(I, frame_gray, CV_BGR2GRAY);
  equalizeHist(frame_gray, frame_gray);

  output.images[0] = I.clone();

  int minsizex = input.model.get_attribute_as_int("minsizex");
  int minsizey = input.model.get_attribute_as_int("minsizey");
  //int maxsizex = model.get_attribute_as_int("maxsizex");
  //int maxsizey = model.get_attribute_as_int("maxsizey");

  //-- Detect faces
  journal.verbose("Détection visages...");
  face_cascade.detectMultiScale(frame_gray, faces,
                                1.1, // scale factor
                                2,   // min neighbors
                                0 | CV_HAAR_SCALE_IMAGE,
                                Size(minsizex,minsizey),  // Minimum size
                                Size()); // Maximum size


  journal.trace("Détecté %d visages.", faces.size());
  for(size_t i = 0; i < faces.size(); i++ )
  {
    Point center( faces[i].x + faces[i].width * 0.5, faces[i].y + faces[i].height * 0.5 );
    cv::rectangle(output.images[0], Point(faces[i].x, faces[i].y), Point(faces[i].x + faces[i].width, faces[i].y + faces[i].height), Scalar(0,255,0), 3);
    Mat faceROI = frame_gray(faces[i]);
    std::vector<Rect> eyes;
    //-- In each face, detect eyes
    journal.verbose("Détection yeux...");
    eyes_cascade.detectMultiScale(faceROI, eyes, 1.05, 2,
                                  CV_HAAR_SCALE_IMAGE);//, Size(5, 5) );
    journal.verbose("%d trouvés.\n", eyes.size());
    for(size_t j = 0; j < eyes.size(); j++)
    {
      Point center( faces[i].x + eyes[j].x + eyes[j].width * 0.5, faces[i].y + eyes[j].y + eyes[j].height * 0.5 );
      int radius = cvRound( (eyes[j].width + eyes[j].height) * 0.25 );
      circle(output.images[0], center, radius, Scalar( 255, 0, 0 ), 4, CV_AA, 0);
    }
  }

  return 0;
}


CascGenDemo::CascGenDemo(std::string id): rng(12345)
{
  output.names[0] = " ";
  cascade_ok = false;
  props.id = id;
  if(id == "casc-yeux")
    cnames.push_back("./data/cascades/haarcascade_eye_tree_eyeglasses.xml");
  else if(id == "casc-sil")
  {
    cnames.push_back("./data/cascades/haarcascade_fullbody.xml");
    // Cascade HOG plus supportée à partir d'OpenCV 3.0
    //cnames.push_back("./data/cascades/hogcascade_pedestrians.xml");
  }
  else if(id == "casc-profile")
  {
    cnames.push_back("./data/cascades/haarcascade_profileface.xml");
    cnames.push_back("./data/cascades/lbpcascade_profileface.xml");
  }
  else if(id == "casc-visage")
  {
    cnames.push_back("./data/cascades/haarcascade_frontalface_alt.xml");
    cnames.push_back("./data/cascades/lbpcascade_frontalface.xml");
  }
  else if(id == "casc-plate")
    cnames.push_back("./data/cascades/haarcascade_russian_plate_number.xml");
  else
  {
    journal.warning("Cascade inconnue: %s.", id.c_str());
    return;
  }


  // I do not know if this is correct but setting nout to 1 seems to do the right thing.
  //output.nout = 0;
  output.nout = 1;

  unsigned int i = 0;
  for(auto cname: cnames)
  {
  try
  {
    if(!cascade[i++].load(cname))
    {
      journal.warning("Erreur chargement cascade: %s.", cname.c_str());
      return;
    }
  }
  catch(...)
  {
    journal.warning("Exception loading cascade");
    return;
  }
  }
  cascade_ok = true;
}


int CascGenDemo::proceed(OCVDemoItemInput &input, OCVDemoItemOutput &output)
{
  if(!cascade_ok)
    return -1;

  auto I = input.images[0];

  std::vector<Rect> rdi;
  Mat frame_gray;
  cvtColor(I, frame_gray, CV_BGR2GRAY);
  equalizeHist(frame_gray, frame_gray);

  int sel = 0;

  if(input.model.has_attribute("sel"))
    sel = input.model.get_attribute_as_int("sel");

  if(sel >= (int) cnames.size())
    sel = 0;

  int minsizex = input.model.get_attribute_as_int("minsizex");
  int minsizey = input.model.get_attribute_as_int("minsizey");

  //-- Detect faces
  cascade[sel].detectMultiScale(frame_gray, rdi,
                                1.1, /* facteur d'échelle */
                                2, /* min voisins ? */
                                CV_HAAR_SCALE_IMAGE, /* ? */
                                Size(minsizex,minsizey),
                                Size(/*maxsizex,maxsizey*/));

  output.images[0] = I.clone();

  journal.trace("Détecté %d objets.", rdi.size());
  for(size_t i = 0; i < rdi.size(); i++ )
  {
    Point center( rdi[i].x + rdi[i].width * 0.5, rdi[i].y + rdi[i].height * 0.5 );
    cv::rectangle(output.images[0], rdi[i],
                  Scalar(0,255,0), 3);
  }
  return 0;
}

DemoHog::DemoHog()
{
  props.id = "hog";
}

int DemoHog::proceed(OCVDemoItemInput &input, OCVDemoItemOutput &output)
{
  //HOGDescriptor::getDefaultPeopleDetector();

  HOGDescriptor hog;
  std::vector<float> ders;
  std::vector<Point>locs;

  cv::Mat img;
  cvtColor(input.images[0], img, CV_BGR2GRAY);

  journal.trace("img.size = %d * %d.", img.cols, img.rows);

  hog.nbins             = input.model.get_attribute_as_int("nbins");
  hog.nlevels           = 64;
  hog.signedGradient    = true;
  hog.derivAperture     = 0;
  hog.winSigma          = -1;
  hog.histogramNormType = 0;
  hog.L2HysThreshold    = 0.2;
  hog.gammaCorrection   = false;
  uint16_t dim_cellule  = input.model.get_attribute_as_int("dim-cellule");
  uint16_t dim_bloc     = input.model.get_attribute_as_int("dim-bloc");
  hog.cellSize          = Size(dim_cellule,dim_cellule);
  hog.winSize           = Size(dim_bloc*dim_cellule,dim_bloc*dim_cellule);
  hog.blockStride       = Size(dim_bloc*dim_cellule,dim_bloc*dim_cellule);
  hog.blockSize         = Size(dim_bloc*dim_cellule,dim_bloc*dim_cellule);
  hog.compute(img, ders);

  // Théoriquement, 512 * 512 / (16 * 16) = 1024 cellules
  // Chaque cellule = 16 bins => 16 k elements
  journal.trace("ders.size = %d.", ders.size());

  // Dessin HOG
  uint16_t sx = img.cols, sy = img.rows;
  uint16_t ncellsx = sx / dim_cellule, ncellsy = sy / dim_cellule;
  uint16_t res = 16;
  Mat O = Mat::zeros(Size(ncellsx * res, ncellsy * res), CV_32F);

  Mat tmp = Mat::zeros(Size(res,res), CV_32F);
  for(auto b = 0; b < hog.nbins; b++)
  {
    Mat masque2 = Mat::zeros(Size(res, res), CV_32F);
    Point p1(res/2,res/2), p2;
    float angle = (b * 2 * 3.1415926) / hog.nbins;
    p2.x = res/2 + 2 * res * cos(angle);
    p2.y = res/2 + 2 * res * sin(angle);
    cv::line(masque2, p1, p2, Scalar(1), 1, CV_AA);
    p2.x = res/2 - 2 * res * cos(angle);
    p2.y = res/2 - 2 * res * sin(angle);
    cv::line(masque2, p1, p2, Scalar(1), 1, CV_AA);
    tmp = tmp + masque2;
  }

  for(auto y = 0; y < ncellsy; y++)
  {
    for(auto x = 0; x < ncellsx; x++)
    {
      Mat masque = Mat::zeros(Size(res, res), CV_32F);
      for(auto b = 0; b < hog.nbins; b++)
      {
        float val = ders.at(y * ncellsx * hog.nbins + x * hog.nbins + b);
        Mat masque2 = Mat::zeros(Size(res, res), CV_32F);
        Point p1(res/2,res/2), p2;
        float angle = (b * 2 * 3.1415926) / hog.nbins;
        p2.x = res/2 + 2 * res * cos(angle);
        p2.y = res/2 + 2 * res * sin(angle);
        cv::line(masque2, p1, p2, Scalar(val), 1, CV_AA);
        p2.x = res/2 - 2 * res * cos(angle);
        p2.y = res/2 - 2 * res * sin(angle);
        cv::line(masque2, p1, p2, Scalar(val), 1, CV_AA);
        masque = masque + masque2;
      }
      masque = masque / tmp;
      Mat rdi(O, Rect(x * res, y * res, res, res));
      masque.copyTo(rdi);
    }
  }

  cv::normalize(O, O, 0, 255, NORM_MINMAX);
  O.convertTo(O, CV_8U);
  cvtColor(O, O, CV_GRAY2BGR);
  output.images[0] = O;

  output.nout = 1;

  if(input.model.get_attribute_as_boolean("detecte-personnes"))
  {
    auto img = O.clone();
    auto coefs = HOGDescriptor::getDefaultPeopleDetector();
    hog.setSVMDetector(coefs);
    std::vector<cv::Rect> locs;
    hog.detectMultiScale(img, locs);//, double hit_thres, Size winStride,
    //    Size padding, double scale, double fthreshold, false);
    for(auto &r: locs)
      cv::rectangle(img, r, Scalar(0,0,255), 1, CV_AA);
    output.nout = 2;
    output.images[1] = img;
  }

  return 0;
}

DemoFaceRecognizer::DemoFaceRecognizer()
{
  props.id = "face-recognizer";
  props.input_min = 0;
  props.input_max = 0;
}



int DemoFaceRecognizer::proceed(OCVDemoItemInput &input, OCVDemoItemOutput &output)
{
  uint16_t sx = 0, sy = 0;
  unsigned int nclasses = 40;
  unsigned int nex = 10;

  output.nout = 0;

  std::vector<cv::Mat> images[2];
  std::vector<int> labels[2];

  for(auto i = 0u; i < nclasses; i++)
  {
    for(auto j = 0u; j < nex; j++)
    {
      char buf[50];
      sprintf(buf, "/img/att_faces/s%d/%d.pgm", i + 1, j + 1);
      auto chemin = utils::get_fixed_data_path() + buf;
      auto img = cv::imread(chemin, CV_LOAD_IMAGE_GRAYSCALE);
      if(img.data == nullptr)
      {
        output.errmsg = "Image non trouvée : " + chemin;
        return -1;
      }
      images[j / (nex / 2)].push_back(img);
      labels[j / (nex / 2)].push_back(i);
      sx = img.cols;
      sy = img.rows;
    }
  }

  journal.verbose("Sx = %d, sy = %d.", sx, sy);

  int algo = input.model.get_attribute_as_int("algo");

  if(algo <= 2)
  {
    unsigned int ncompos = 0;
    Ptr<cv::face::FaceRecognizer> model;

    if(algo == 0)
    {
      ncompos = input.model.get_attribute_as_int("eigenface/numcompo");
      model = cv::face::createEigenFaceRecognizer(ncompos);
    }
    else if(algo == 1)
    {
      ncompos = input.model.get_attribute_as_int("fisherface/numcompo");
      model = cv::face::createFisherFaceRecognizer(ncompos);
    }
    else if(algo == 2)
    {
      model = cv::face::createLBPHFaceRecognizer();//radius, neighbors, gx, gy);
    }
    model->train(images[0], labels[0]);

    uint32_t ntests = images[1].size();

    int nfaux = 0;

    for(auto i = 0u; i < ntests; i++)
    {
      int label;
      double confiance;
      model->predict(images[1][i], label, confiance);
      if(label != labels[1][i])
        nfaux++;
    }
    float taux = ((float) (ntests - nfaux)) / ntests;
    journal.trace("Ntests = %d, nfaux = %d, taux de réussite = %.2f %%.", ntests, nfaux, taux * 100);

    output.nout = 0;

    if(algo <= 1)
    {
      cv::face::BasicFaceRecognizer *model2 = (cv::face::BasicFaceRecognizer *) model.get();
      Mat ev = model2->getEigenVectors();
      journal.trace("Eigen vectors = %d * %d.", ev.cols, ev.rows);
      // 80 * 10304 = numcompos * (nbdims total)

      output.nout = ev.cols;//ncompos;
      if(ncompos > DEMO_MAX_IMG_OUT)
        output.nout = DEMO_MAX_IMG_OUT;
      for(auto i = 0; i < output.nout; i++)
      {
        cv::Mat mat = ev.col(i).t();
        mat = mat.reshape(1, sy);

        //journal.verbose("nv taille = %d * %d.", mat.cols, mat.rows);
        mat.convertTo(mat, CV_32F);
        cv::normalize(mat, mat, 0, 1.0, NORM_MINMAX);
        mat.convertTo(mat, CV_8U, 255);
        output.images[i] = mat;
      }
    }
  }
  else
  {
    output.errmsg = "Algorithme non supporté.";
    return -1;
  }

  return 0;
}


