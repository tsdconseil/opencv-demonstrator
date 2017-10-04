/** @file   ocvext/include/gl.hpp
 *  @brief  Garcia-Lorca exponential filter and optimized Deriche gradient
 *  @author J.A. 2015 / www.tsdconseil.fr
 *  @license LGPL V 3.0 */

#ifndef GL_HPP
#define GL_HPP

#include "opencv2/core.hpp"

namespace ocvext
{

/** @brief Garcia-Lorca filtering
 *  @param I input image
 *  @param O output image (filtered)
 *  @param gamma Filtering coefficient between 0 and 1.
 *  0 means no filtering, 1 means maximum filtering. */
extern int Deriche_blur(const cv::Mat &I,
                       cv::Mat &O,
                       float gamma);


/** @brief Gradient de Deriche
 *  @param I input image
 *  @param gx, gy Output gradient (along x and y axis)
 *  @param gamma Filtering coefficient between 0 and 1.
 *  0 means no filtering, 1 means maximum filtering. */
extern int Deriche_gradient(const cv::Mat &I,
                           cv::Mat &gx,
                           cv::Mat &gy,
                           float gamma);

/** @brief Gradient de Deriche (norme du gradient)
 *  @param I input image
 *  @param O Output gradient norm (normalized between 0 and 255)
 *  @param gamma Filtering coefficient between 0 and 1.
 *  0 means no filtering, 1 means maximum filtering. */
extern int Deriche_gradient(const cv::Mat &I,
                           cv::Mat &O,
                           float gamma);

}

#endif
