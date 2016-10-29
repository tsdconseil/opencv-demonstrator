/** @file   hough.hpp
 *  @brief  Hough transform / using gradient
 *  @author J.A. / 2015 / www.tsdconseil.fr
 *  @license LGPL V 3.0 */



#ifndef HOUGH_E_HPP
#define HOUGH_E_HPP

#include "opencv2/core/core.hpp"
#include <vector>
#include <cmath>

//////////////////////////////////////////////////////
// Generic function to compute the Hough transform  //
// (independant of the type of objects searched)    //
//////////////////////////////////////////////////////

/** @brief Gradient direction-based Hough transform
 *  @param img     Input image (can be BGR, graylevels, etc.)
 *  @param res     Resulting accumulation space, with coordinates (rho,theta)
 *  @param theta   Angular resolution (in radians)
 *  @param rho     Distance resolution (in pixels)
 *  @param gamma   Filtering factor between 0 (no filtering) and 1 (maximum filtering) for the Deriche gradient.
 *  @note Rho dimension is automatically computed from input image dimension and
 *        set such that the resolution is one pixel. */
extern void Hough_with_gradient_dir(const cv::Mat &img,
                                 cv::Mat &res,
                                 float rho   = 1.0, // 1 pixel
                                 float theta = 2 * 3.1415926 / 360, // 1 degree
                                 float gamma = 0.6);

/** @brief Gradient magnitude only based Hough transform
 *  @param img     Input image (can be BGR, graylevels, etc.)
 *  @param res     Resulting accumulation space, with coordinates (rho,theta)
 *  @param theta   Angular resolution (in radians)
 *  @param rho     Distance resolution (in pixels)
 *  @param gamma   Filtering factor between 0 (no filtering) and 1 (maximum filtering) for the Deriche gradient.
 *  @note Rho dimension is automatically computed from input image dimension and
 *        set such that the resolution is one pixel. */
extern void Hough_without_gradient_dir(const cv::Mat &img,
                                    cv::Mat &res,
                                    float rho   = 1.0, // 1 pixel
                                    float theta = 2 * 3.1415926 / 360, // 1 degree
                                    float gamma = 0.6);

//////////////////////////////////////////////////////
// Hough transform to detect specific objects       //
//////////////////////////////////////////////////////

/** @brief Lines detection
 *  This function has the same first 4 parameters as
 *  the OpenCV native function "HoughLines".
 *  But it does not needs any threshold to be set. */
extern void Hough_lines_with_gradient_dir(const cv::Mat &img,
                                      std::vector<cv::Vec2f> &lines,
                                      cv::Mat &debug,
                                      float rho = 1.0,
                                      float theta = 2 * 3.1415926 / 360,
                                      float gamma = 0.6,
                                      float seuil = 0.4);


#endif
