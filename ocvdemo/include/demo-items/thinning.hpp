#ifndef THINNING_H
#define THINNING_H

#include <opencv2/core.hpp>

extern void thinning_Zhang_Suen(const cv::Mat& I, cv::Mat &O);

extern void thinning_Guo_Hall(const cv::Mat& I, cv::Mat &O);

extern void thinning_morpho(const cv::Mat &I, cv::Mat &O);


#endif
