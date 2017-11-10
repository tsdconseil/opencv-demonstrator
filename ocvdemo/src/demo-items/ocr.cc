#include "demo-items/ocr.hpp"
#ifdef USE_CONTRIB
#include "opencv2/text.hpp"
#endif
#include <cmath>
#include <iostream>

DemoOCR::DemoOCR()
{
  props.id = "ocr";
}


int DemoOCR::proceed(OCVDemoItemInput &input, OCVDemoItemOutput &output)
{
#ifdef USE_CONTRIB
  cv::Mat I = input.images[0];

  cv::Ptr<cv::text::BaseOCR> algo;

  Ptr<cv::text::OCRHMMDecoder::ClassifierCallback> cls
  = cv::text::loadOCRHMMClassifierNM("OCRHMM_knn_model_data.xml");


  cv::Mat trans_prob = cv::Mat::ones(6,6,CV_32F),
          em_prob    = cv::Mat::eye(6,6,CV_32F);
  trans_prob /= 6.0;
  std::string voc = "OpenCV";


  //algo = cv::text::OCRHMMDecoder::create(cls, voc, trans_prob, em_prob);

  //algo = cv::text::OCRTesseract::create();

  std::string res;

  std::vector<Rect> component_rects;
  std::vector<std::string> component_texts;



  algo->run(I, res, &component_rects, &component_texts);

  cv::Mat O = I.clone();
  cv::putText(O, res, Point(0,0), FONT_HERSHEY_SIMPLEX, 1.0, Scalar(0,255,0), 1);

  infos("Nb rect : %d.", component_rects.size());
  infos("Texte detecte : [%s]", res.c_str());

  output.images[0] = O;
  //auto algo = cv::text::OCRHMMDecoder::create()
# else
  output.images[0] = input.images[0];
# endif
  return 0;
}


