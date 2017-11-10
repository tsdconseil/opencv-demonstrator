#include "calib.hpp"
#include <opencv2/calib3d.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/videoio.hpp>
#include <iostream>

namespace ocvext {





DialogueCalibration::DialogueCalibration()
{
  infos("Init dialogue cal...");
  prende_photo = false;

  fs.from_file(utils::get_fixed_data_path() + "/ocvext-schema.xml");

  prm = utils::model::Node(fs.get_schema("cal"));
  vue_prm.init(prm);

  paned.add1(vue_video);
  paned.add2(*(vue_prm.get_gtk_widget()));
  paned.set_position(200);
  dialogue.add(vbox);
  vbox.pack_start(paned, Gtk::PACK_EXPAND_WIDGET);
  hbox.set_layout(Gtk::BUTTONBOX_END);

  auto img = new Gtk::Image(Gtk::StockID(Gtk::Stock::CANCEL),
      Gtk::IconSize(Gtk::ICON_SIZE_BUTTON));
  b_fermer.set_image(*img);
  b_fermer.set_border_width(4);
  hbox.pack_end(b_fermer, Gtk::PACK_SHRINK);

  b_fermer.signal_clicked().connect(sigc::mem_fun(this, &DialogueCalibration::gere_b_fermer));

  //img = new Gtk::Image(Gtk::StockID(Gtk::Stock::EXECUTE),
  //    Gtk::IconSize(Gtk::ICON_SIZE_BUTTON));
  //b_photo.set_image(*img);
  b_photo.set_label(utils::langue.get_item("cap"));
  b_photo.set_border_width(4);
  hbox.pack_end(b_photo, Gtk::PACK_SHRINK);
  b_photo.signal_clicked().connect(sigc::mem_fun(this, &DialogueCalibration::gere_b_photo));


  b_cal.set_label(utils::langue.get_item("cal"));
  b_cal.set_border_width(4);
  hbox.pack_end(b_cal, Gtk::PACK_SHRINK);
  b_cal.signal_clicked().connect(sigc::mem_fun(this, &DialogueCalibration::gere_b_cal));



  vbox.pack_start(hbox, Gtk::PACK_SHRINK);
  dialogue.set_size_request(800, 600);
  dispatcheur.add_listener(this, &DialogueCalibration::gere_evt_gtk);
  utils::hal::thread_start(this, &DialogueCalibration::thread_video);
  infos("Init dialogue cal ok.");




}


int DialogueCalibration::gere_evt_gtk(const EvtGtk &eg)
{
  infos("ev gtk.");

  assert(photos.size() > 0);
  img_list.ajoute_photo(photos[photos.size() - 1]);

  return 0;
}

void DialogueCalibration::gere_b_fermer()
{
  dialogue.hide();
  exit(0);
}

void DialogueCalibration::gere_b_photo()
{
  infos("Photo...");
  prende_photo = true;
}

void DialogueCalibration::gere_b_cal()
{
  infos("Calibration...");

  cv::Mat matrice_camera, prm_dist;


  std::vector<cv::Mat> imgs;
  img_list.get_list(imgs);

  EtalonnageCameraConfig config;

  config.ncolonnes = prm.get_attribute_as_int("nx");
  config.nlignes = prm.get_attribute_as_int("ny");
  config.largeur_case_cm = prm.get_attribute_as_int("dim");
  config.type = (EtalonnageCameraConfig::Type) prm.get_attribute_as_int("sel");

  if(etalonner_camera(imgs, config, matrice_camera, prm_dist))
  {
    avertissement("Echec etalonnage camera.");
    utils::mmi::dialogs::affiche_erreur_localisee("echec-etalonnage");
    return;
  }

  auto s = utils::mmi::dialogs::enregistrer_fichier_loc("enreg-cal", "*.yml", "fichier-cal");
  if(s.size() == 0)
    return;

  infos("Svg cal vers [%s]...", s.c_str());

  cv::FileStorage fs(s, cv::FileStorage::WRITE);
  if(!fs.isOpened())
  {
    erreur("Echec ouverture fichier.");
    return;
  }
  cv::write(fs, "camera_matrix", matrice_camera);
  cv::write(fs, "distortion_coefficients", prm_dist);
  infos("Ok.");
}

void DialogueCalibration::thread_video()
{
  cv::VideoCapture camera(1);
  cv::Mat I;
  static unsigned int num_photo = 0;
  for(;;)
  {
    camera >> I;

    if(prende_photo)
    {
      prende_photo = false;
      photos.push_back(I);

      char buf[400];
      sprintf(buf, "c:/dbi/photo-%d.png", num_photo);
      cv::imwrite(buf, I);

      num_photo++;

      EvtGtk eg;
      dispatcheur.on_event(eg);
    }

    vue_video.maj(I);
    cv::waitKey(20);

  }
}

void DialogueCalibration::affiche()
{
  dialogue.set_title("Etalonage caméra");
  dialogue.set_position(Gtk::WIN_POS_CENTER);
  b_fermer.set_label(utils::langue.get_item("fermer"));

  img_list.fenetre.show();

  dialogue.show_all_children(true);
  //vview.maj(I);
  //dialogue.run();


  img_list.ajoute_fichier("c:/dbi/bic/etal-logitec-bis/photo-0.png");
  img_list.ajoute_fichier("c:/dbi/bic/etal-logitec-bis/photo-1.png");
  img_list.ajoute_fichier("c:/dbi/bic/etal-logitec-bis/photo-2.png");
  img_list.ajoute_fichier("c:/dbi/bic/etal-logitec-bis/photo-3.png");
  img_list.ajoute_fichier("c:/dbi/bic/etal-logitec-bis/photo-4.png");
  img_list.ajoute_fichier("c:/dbi/bic/etal-logitec-bis/photo-5.png");
  img_list.ajoute_fichier("c:/dbi/bic/etal-logitec-bis/photo-6.png");
  img_list.ajoute_fichier("c:/dbi/bic/etal-logitec-bis/photo-7.png");
  img_list.ajoute_fichier("c:/dbi/bic/etal-logitec-bis/photo-8.png");

  Gtk::Main::run(dialogue);
}






int etalonner_camera(const std::vector<cv::Mat> &imgs,
                     const EtalonnageCameraConfig &config,
                     cv::Mat &matrice_camera, cv::Mat &prm_dist)
{
  //int sel = input.model.get_attribute_as_int("sel");

  int bw = config.ncolonnes, bh = config.nlignes;

  infos("Etalonnage, ncolonnes = %d, nlignes = %d.");

  cv::Size board_size(bw,bh);
  std::vector<std::vector<cv::Point2f>> points_2d;
  std::vector<std::vector<cv::Point3f>> points_3d;
  std::vector<cv::Point2f> pointbuf;


  unsigned int nb_imgs = imgs.size();

  if(nb_imgs == 0)
    return -1;

  cv::Size resolution = imgs[0].size();

  for(auto i = 0u; i < nb_imgs; i++)
  {
    infos("Analyse image de cal %d / %d...", i + 1, nb_imgs);

    //cv::cvtColor(input.images[0], Ig, CV_BGR2GRAY);

    //output.images[2] = cv::Mat(/*Ig.size()*/cv::Size(480,640), CV_8UC3);

    auto Ig = imgs[i];

    cv::Mat Ig2;
    cv::cvtColor(Ig, Ig2, CV_BGR2GRAY);

    infos("Resolution = %d * %d", Ig.cols, Ig.rows);

    bool trouve;
    if(config.type == EtalonnageCameraConfig::DAMIER)
    {
      infos("Recherche damier...");
      trouve = cv::findChessboardCorners(Ig2, board_size, pointbuf,
        cv::CALIB_CB_ADAPTIVE_THRESH
      | cv::CALIB_CB_FAST_CHECK
      | cv::CALIB_CB_NORMALIZE_IMAGE);
      // improve the found corners' coordinate accuracy
       if(trouve)
       {
         infos("Coins trouve, amelioration...");
         cv::cornerSubPix(Ig2, pointbuf, cv::Size(11,11),
             cv::Size(-1,-1), cv::TermCriteria(cv::TermCriteria::EPS+cv::TermCriteria::COUNT, 30, 0.1 ));
         infos("ok.");
       }
    }
    else
      trouve = cv::findCirclesGrid(Ig, board_size, pointbuf );


  //cvtColor(I, O[0], CV_GRAY2BGR);

  //Mat Ior = input.images[0].clone();
  //output.images[0] = Ior.clone();
  //if(trouve)
   //cv::drawChessboardCorners(output.images[0], board_size, Mat(pointbuf), trouve);

  trace_majeure("Trouvé %d coins (found = %d).",
      pointbuf.size(), (int) trouve);

    if(trouve)
    {
      float square_size = 1;

      std::vector<cv::Point3f> corners;

      for(int i = 0; i < board_size.height; i++)
            for( int j = 0; j < board_size.width; j++ )
                corners.push_back(cv::Point3f(float(j*square_size),
                                              float(i*square_size),
                                              0));

      points_3d.push_back(corners);
      points_2d.push_back(pointbuf);
    } // si trouve
  } // for i

  if(points_3d.size() == 0)
  {
    avertissement("Aucune mire trouvee.");
    return -1;
  }


  matrice_camera = cv::Mat::eye(3, 3, CV_64F);
  //if( flags & CALIB_FIX_ASPECT_RATIO )
    //  cameraMatrix.at<double>(0,0) = aspectRatio;
  prm_dist = cv::Mat::zeros(8, 1, CV_64F);

  std::vector<cv::Mat> rvecs, tvecs;
  double rms = cv::calibrateCamera(points_3d,
                  points_2d, resolution,
                  matrice_camera, prm_dist, rvecs, tvecs,
                  cv::CALIB_FIX_K4 | cv::CALIB_FIX_K5);
  infos("RMS error reported by calibrateCamera: %g\n", rms);


    //cv::undistort(Ior, output.images[1], cameraMatrix, distCoeffs);

    /*Size sz = Ior.size();
    sz.height = sz.width = max(sz.width, sz.height);
    sz.height = sz.width = max(sz.width, 500);

    output.images[2] = cv::Mat::zeros(sz, CV_8UC3);*/

    double fovx, fovy, focal, ar;
    cv::Point2d ppoint;
    cv::calibrationMatrixValues(matrice_camera, resolution, 1, 1, fovx, fovy, focal, ppoint, ar);

    infos("Matrice camera: ");
    std::cout << matrice_camera << "\n";
    infos("Focal : %.2f, %.2f, %.2f\n", (float) fovx, (float) fovy, (float) focal);
    infos("Point principal: %.2f, %.2f\n", (float) ppoint.x, (float) ppoint.y);


    /*Mat tmp = output.images[2].clone();
    cv::namedWindow("essai", CV_WINDOW_NORMAL);
    cv::imshow("essai", tmp);
    cv::waitKey();*/
    //bool ok = checkRange(cameraMatrix) && checkRange(distCoeffs);
    //totalAvgErr = computeReprojectionErrors(objectPoints, imagePoints,
    //            rvecs, tvecs, cameraMatrix, distCoeffs, reprojErrs);


  return 0;
}

}


