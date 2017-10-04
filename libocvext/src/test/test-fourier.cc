#include "fourier.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"
#include "cutil.hpp"
#include "../../include/ocvext.hpp"




void test_rotation(cv::Mat &I0, float angle, float echelle)
{
  trace_majeure("Test rotation (%.1f, %.3f)...", angle, echelle);

  cv::Mat I1;

  cv::Size sz = I0.size();
  cv::Point centre;
  centre.x = sz.width / 2;
  centre.y = sz.height / 2;
  uint16_t sx = sz.width, sy = sz.height;
  cv::Mat R = cv::getRotationMatrix2D(centre, angle, echelle);
  cv::warpAffine(I0, I1, R, I0.size());
  //Ig = Ig(Rect(sx/4,sy/4,sx/2,sy/2));
  //output.images[idx++] = Ig;
  float ad, ed;
  ocvext::detection_rotation_echelle(I0, I1, ad, ed);
  angle *= 3.1415926 / 180.0;
  float ea = std::abs(ad - angle);
  float ee = std::abs(ed - echelle);
  infos("Erreur détection : angle -> %.1f degrés, echelle = %.3f", ea * 180.0 / 3.1415926, ee);
}

int main(int argc, const char **argv)
{
  utils::init(argc, argv, "libocvext");

  ocvext::init(false, true);

  cv::Mat I0 = cv::imread("./data/img/5pts.png");

# if 0
  cv::Mat I1 = cv::Mat::zeros(I0.size(), CV_8UC3);


  cv::Point pt;
  // Applique une translation
  ocvext::translation(I0, I1, 130, -50, cv::Scalar(255,255,255));
  pt = ocvext::detection_translation(I0, I1);
  ocvext::translation(I0, I1, 130, 50, cv::Scalar(255,255,255));
  pt = ocvext::detection_translation(I0, I1);
  ocvext::translation(I0, I1, -130, 50, cv::Scalar(255,255,255));
  pt = ocvext::detection_translation(I0, I1);
  ocvext::translation(I0, I1, -130, -50, cv::Scalar(255,255,255));
  pt = ocvext::detection_translation(I0, I1);
# endif


  I0.convertTo(I0, CV_32F);
  cv::cvtColor(I0, I0, CV_BGR2GRAY);
  //test_rotation(I0, 0.0, 1.0);
  //test_rotation(I0, 10.0, 1.0);
  //test_rotation(I0, 25.0, 1.0);
  //test_rotation(I0, 0.0, 1.2);
  test_rotation(I0, 0.0, 1.5);
  return 0;
}

