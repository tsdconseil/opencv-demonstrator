#ifndef RECO_H
#define RECO_H

#include <opencv2/features2d.hpp>


namespace ocvext {



struct AssociateurConfig
{
  AssociateurConfig();
  bool verifie_ratio;
  bool verifie_symetrie;
  float seuil_ratio;
};

class Associateur
{
public:
  Associateur(const AssociateurConfig &config = AssociateurConfig());


  void calcule(const cv::Mat &desc0, const cv::Mat &desc1,
               std::vector<cv::DMatch> &res);


private:
  AssociateurConfig config;
};




}


#endif
