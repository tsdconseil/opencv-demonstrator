#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>


/////////////////////////////////////////////////////////////
// (1) Algorithme de ZHANG-SUEN                           ///
// D'après https://web.archive.org/web/20160322113207/    ///
//         http://opencv-code.com/quick-tips/implementation-of-thinning-algorithm-in-opencv/
/////////////////////////////////////////////////////////////


/**
 * Perform one thinning iteration.
 * Normally you wouldn't call this function directly from your code.
 *
 * @param  im    Binary image with range = 0-1
 * @param  iter  0=even, 1=odd
 */
void thinning_Zhang_Suen_iteration(cv::Mat& im, int iter)
{
    cv::Mat marker = cv::Mat::zeros(im.size(), CV_8UC1);

    for (int i = 1; i < im.rows-1; i++)
    {
        for (int j = 1; j < im.cols-1; j++)
        {
            uchar p2 = im.at<uchar>(i-1, j);
            uchar p3 = im.at<uchar>(i-1, j+1);
            uchar p4 = im.at<uchar>(i, j+1);
            uchar p5 = im.at<uchar>(i+1, j+1);
            uchar p6 = im.at<uchar>(i+1, j);
            uchar p7 = im.at<uchar>(i+1, j-1);
            uchar p8 = im.at<uchar>(i, j-1);
            uchar p9 = im.at<uchar>(i-1, j-1);

            int A  = (p2 == 0 && p3 == 1) + (p3 == 0 && p4 == 1) +
                     (p4 == 0 && p5 == 1) + (p5 == 0 && p6 == 1) +
                     (p6 == 0 && p7 == 1) + (p7 == 0 && p8 == 1) +
                     (p8 == 0 && p9 == 1) + (p9 == 0 && p2 == 1);
            int B  = p2 + p3 + p4 + p5 + p6 + p7 + p8 + p9;
            int m1 = iter == 0 ? (p2 * p4 * p6) : (p2 * p4 * p8);
            int m2 = iter == 0 ? (p4 * p6 * p8) : (p2 * p6 * p8);

            if (A == 1 && (B >= 2 && B <= 6) && m1 == 0 && m2 == 0)
                marker.at<uchar>(i,j) = 1;
        }
    }

    im &= ~marker;
}

/* Function for thinning the given binary image
 * @param  im  Binary image with range = 0-255 */
void thinning_Zhang_Suen(const cv::Mat& I, cv::Mat &O)
{
  O = I / 255;

  cv::Mat prev = cv::Mat::zeros(O.size(), CV_8UC1);
  cv::Mat diff;

  do
  {
    thinning_Zhang_Suen_iteration(O, 0);
    thinning_Zhang_Suen_iteration(O, 1);
    cv::absdiff(O, prev, diff);
    O.copyTo(prev);
  }
  while (cv::countNonZero(diff) > 0);

  O *= 255;
}


/////////////////////////////////////////////////////////////
// (2) Algorithme morphologique                           ///
// D'après http://felix.abecassis.me/2011/09/opencv-morphological-skeleton/
/////////////////////////////////////////////////////////////
void thinning_morpho(const cv::Mat &I, cv::Mat &O)
{
  cv::Mat A = I;

  //cv::cvtColor(I, A, CV_BGR2GRAY);

  //cv::threshold(A, A, 128, 255, cv::THRESH_OTSU);
  //A = 255 - A;

  //output.images[0] = A.clone();

  cv::Mat K = cv::getStructuringElement(cv::MORPH_CROSS, cv::Size(3, 3));

  cv::Mat squelette(I.size(), CV_8UC1, cv::Scalar(0));
  cv::Mat temp(I.size(), CV_8UC1);

  // Algorithme d'après page web
  // http://felix.abecassis.me/2011/09/opencv-morphological-skeleton/
  bool done;
  do
  {
    cv::morphologyEx(A, temp, cv::MORPH_OPEN, K);
    cv::bitwise_not(temp, temp);
    cv::bitwise_and(A, temp, temp);
    cv::bitwise_or(squelette, temp, squelette);
    cv::erode(A, A, K);

    double max;
    cv::minMaxLoc(A, 0, &max);
    done = (max == 0);
  } while (!done);

  O = squelette;
}



/////////////////////////////////////////////////////////////
// (3) Algorithme Guo - Hall                           ///
// D'après https://web.archive.org/web/20160314104646/http://opencv-code.com/quick-tips/implementation-of-guo-hall-thinning-algorithm/
/////////////////////////////////////////////////////////////


/**
 * Perform one thinning iteration.
 * Normally you wouldn't call this function directly from your code.
 *
 * @param  im    Binary image with range = 0-1
 * @param  iter  0=even, 1=odd
 */
void thinningGuoHallIteration(cv::Mat& im, int iter)
{
    cv::Mat marker = cv::Mat::zeros(im.size(), CV_8UC1);

    for (int i = 1; i < im.rows; i++)
    {
        for (int j = 1; j < im.cols; j++)
        {
            uchar p2 = im.at<uchar>(i-1, j);
            uchar p3 = im.at<uchar>(i-1, j+1);
            uchar p4 = im.at<uchar>(i, j+1);
            uchar p5 = im.at<uchar>(i+1, j+1);
            uchar p6 = im.at<uchar>(i+1, j);
            uchar p7 = im.at<uchar>(i+1, j-1);
            uchar p8 = im.at<uchar>(i, j-1);
            uchar p9 = im.at<uchar>(i-1, j-1);

            int C  = (!p2 & (p3 | p4)) + (!p4 & (p5 | p6)) +
                     (!p6 & (p7 | p8)) + (!p8 & (p9 | p2));
            int N1 = (p9 | p2) + (p3 | p4) + (p5 | p6) + (p7 | p8);
            int N2 = (p2 | p3) + (p4 | p5) + (p6 | p7) + (p8 | p9);
            int N  = N1 < N2 ? N1 : N2;
            int m  = iter == 0 ? ((p6 | p7 | !p9) & p8) : ((p2 | p3 | !p5) & p4);

            if ((C == 1) && ((N >= 2) && (N <= 3)) & (m == 0))
                marker.at<uchar>(i,j) = 1;
        }
    }

    im &= ~marker;
}

/**  Function for thinning the given binary image
 * @param  im  Binary image with range = 0-255 */
void thinning_Guo_Hall(const cv::Mat& I, cv::Mat &O)
{
  O = I / 255;

  cv::Mat prev = cv::Mat::zeros(I.size(), CV_8UC1);
  cv::Mat diff;

  do {
      thinningGuoHallIteration(O, 0);
      thinningGuoHallIteration(O, 1);
      cv::absdiff(O, prev, diff);
      O.copyTo(prev);
  }
  while (cv::countNonZero(diff) > 0);

  O *= 255;
}

