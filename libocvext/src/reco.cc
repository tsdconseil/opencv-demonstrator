#include "reco.hpp"
#include "cutil.hpp"

namespace ocvext
{

AssociateurConfig::AssociateurConfig()
{
  verifie_ratio     = true;
  verifie_symetrie  = true;
  seuil_ratio       = 0.65;
}

Associateur::Associateur(const AssociateurConfig &config)
{
  this->config = config;
}

void Associateur::calcule(const cv::Mat &desc0,
                          const cv::Mat &desc1,
                          std::vector<cv::DMatch> &res)
{
  auto matcheur = cv::DescriptorMatcher::create("BruteForce-Hamming(2)");

  //auto matcheur = new cv::FlannBasedMatcher();

  infos("Calcule association...");
  std::vector<std::vector<cv::DMatch>> matches;
  infos("knn match (%d * %d, %d cols)...", desc0.rows, desc1.rows, desc0.cols);
  matcheur->knnMatch(desc0, desc1, matches, 3);
  infos("ok.");

  for(auto &match: matches)
  {
    if(config.verifie_ratio)
    {
      if(match.size() < 3)
        continue;
      // On autorise 2 pts d'intérêts similaires
      float ratio = match[0].distance / match[2].distance;
      if(ratio > config.seuil_ratio)
        continue;
    }
    res.push_back(match[0]);
  }
  infos("ok, nb assos = %d.", res.size());

  if(config.verifie_symetrie)
  {
    std::vector<cv::DMatch> matches10, res2;
    matcheur->match(desc1, desc0, matches10);
    for(auto &m: res)
    {
      for(auto &m2: matches10)
      {
        if(m.queryIdx == m2.trainIdx)
        {
          if(m2.queryIdx == m.trainIdx)
          {
            res2.push_back(m);
          }
          break;
        }
      }
    }
    res = res2;
    infos("Apres test symetrie : %d.", res.size());
  }
}



}


