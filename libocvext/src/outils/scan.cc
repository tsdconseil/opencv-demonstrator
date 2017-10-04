#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include "cutil.hpp"
#include <vector>

int main(int argc, const char **argv)
{

  std::string dossier = argv[1];

  std::vector<std::string> fichiers;
  utils::files::explore_dossier(dossier, fichiers);

  auto cnt = 0;

  for(auto &f: fichiers)
  {
    auto ext = utils::files::get_extension(f);

    if(ext != "jpg")
      continue;

    cv::Mat I = cv::imread(f.c_str());
    if(I.cols == 0)
    {
      utils::erreur("Erreur ouverture fichier (%s)", f.c_str());
      return -1;
    }

    utils::trace("Traitement [%s]: %d * %d", f.c_str(), I.cols, I.rows);

    /*if(I.cols > I.rows)
    {
      I = I.t();
    }*/

    if(I.cols > 1000)
    {
      float ratio = 1000.0 / I.cols;
      cv::resize(I, I, cv::Size(0,0), ratio, ratio);
      utils::trace(" redim: %d * %d", I.cols, I.rows);
    }

    //cv::medianBlur(I, I, 3);

    //cv::equalizeHist(I, I);
    cv::cvtColor(I, I, CV_BGR2GRAY);
    //cv::threshold(I, I, 0, 255, cv::THRESH_OTSU);
    //cv::AdaptiveThresholdTypes
    cv::adaptiveThreshold(I, I, 255, cv::ADAPTIVE_THRESH_GAUSSIAN_C, cv::THRESH_BINARY, 21, 20);

    f = utils::files::remove_extension(f);

    cv::imwrite(f + ".png", I);


    cnt++;

    /*if(cnt > 10)
      break;*/

    //cv::imwrite(f + ".jp2", I);
  }

  return 0;
}



