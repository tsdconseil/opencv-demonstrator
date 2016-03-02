#include "demo-items/segmentation.hpp"
#include <iostream>

DemoMahalanobis::DemoMahalanobis()
{
  props.id = "mahalanobis";
  props.requiert_roi = true;
}


int DemoMahalanobis::proceed(OCVDemoItemInput &input, OCVDemoItemOutput &output)
{
  Mat I = input.images[0];

  output.nout = 2;
  output.images[0] = I.clone();

  if(input.roi.area() == 0)
  {
    output.images[1] = I.clone();
    journal.trace("RDI invalide.");
    return 0;
  }



  journal.trace("Compute covar samples...");
  auto M = I(input.roi);
  Mat samples(M.rows*M.cols,2,CV_32F);
  for(auto y = 0u; y < (unsigned int) M.rows; y++)
  {
    for(auto x = 0u; x < (unsigned int) M.cols; x++)
    {
      const Vec3b &bgr = M.at<Vec3b>(y,x);
      float sum = bgr[0] + bgr[1] + bgr[2];
      // R / (B + G +R)
      samples.at<float>(x+y*M.cols, 0) = bgr[2] / sum;
      // B / (B + G +R)
      samples.at<float>(x+y*M.cols, 1) = bgr[0] / sum;
    }
  }

  journal.trace("Compute covar matrix...");
  Mat covar, covari, mean;
  cv::calcCovarMatrix(samples, covar, mean,
        CV_COVAR_NORMAL | CV_COVAR_SCALE | CV_COVAR_ROWS);

  cv::invert(covar, covari, cv::DECOMP_SVD);

  std::cout << "mean: " << mean << std::endl;
  std::cout << "covar: " << covar << std::endl;
  std::cout << "covari: " << covari << std::endl;

  mean.convertTo(mean, CV_32F);
  covari.convertTo(covari, CV_32F);

  journal.trace("  Mahalanobis...");
  const Vec3b *iptr = I.ptr<Vec3b>();
  Mat v(1,2,CV_32F);
  Mat fmask(I.size(), CV_32F);
  float *optr = fmask.ptr<float>();

  float *pv = v.ptr<float>(0);

  for(auto i = 0u; i < input.images[0].total(); i++)
  {
    const Vec3b &bgr = *iptr++;
    float sum = bgr[0] + bgr[1] + bgr[2];
    float cr = bgr[2] / sum;
    float cb = bgr[0] / sum;
    //v.at<float>(0,0) = cr;
    //v.at<float>(0,1) = cb;
    pv[0] = cr;
    pv[1] = cb;
    float dst = cv::Mahalanobis(mean, v, covari);
    *optr++ = dst;
  }

  journal.trace(" normalize...");

  cv::normalize(fmask, fmask, 0, 255, cv::NORM_MINMAX);
  //fmask = 255.0 - fmask;
  fmask.convertTo(fmask, CV_8UC1);

  output.images[1] = fmask;

  return 0;
}


