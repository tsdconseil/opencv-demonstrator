#ifndef FOURIER_H
#define FOURIER_H

#include "opencv2/core.hpp"


namespace ocvext
{

/** Recentrage d'une DFT 2D de manière à ce que
 *  le DC soit au centre de l'image */
extern void dft_shift(cv::Mat &mag);

/** Recentrage d'une DFT 2D de manière à ce que
 *  le DC soit au centre de l'image */
extern void ift_shift(cv::Mat &mag);


/** Détection du vecteur translation la plus probable entre deux images */
extern cv::Point detection_translation(const cv::Mat &I0, const cv::Mat &I1, bool normaliser_spectre = false, cv::Mat *spectre = nullptr);


extern int detection_rotation_echelle(const cv::Mat &I0, const cv::Mat &I1, float &angle, float &echelle);

extern void translation(const cv::Mat &I, cv::Mat &O, int offsetx, int offsety, const cv::Scalar &bg = cv::Scalar(0));


extern void translation_rapide(const cv::Mat &I, cv::Mat &O,
                               int decx, int decy,
                               const cv::Size &dim_sortie,
                               const cv::Scalar &bg = cv::Scalar(0));

extern int estime_periode(cv::Mat &I,
                          float &d, float &indice_confiance,
                          float dmin, float dmax,
                          cv::Mat *dbg = nullptr);

}

#endif
