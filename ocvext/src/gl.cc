
/** @file   gl.cc
 *  @brief  Garcia-Lorca exponential filter and optimized Deriche gradient
 *  @author J.A. / 2015 / www.tsdconseil.fr
 *  @license LGPL V 3.0 */

#include "gl.hpp"
#include <stdio.h>
#include "opencv2/imgproc/imgproc.hpp" // for cv::Sobel function


template<typename T>
  static void lexp_forward(T *x, uint16_t n, float gamma);
template<typename T>
  static void lexp_reverse(T *x, uint16_t n, float gamma);
template<typename T>
  static void lexp2_forward(T *x, uint16_t n, float gamma);
template<typename T>
  static void lexp2_reverse(T *x, uint16_t n, float gamma);
/* lissage garcia - lorca (1d) */
template<typename T>
  static void lgl_1d(T *x, uint16_t n, float gamma);

template<typename T>
  static void lexp_forward(const T *x, T *y, uint16_t n, float gamma, uint16_t stride)
{
  uint16_t i;
  float accu = x[0];
  float gam  = gamma;
  float ugam = 1.0 - gamma;
  // Accu:  représentation XXX.X
  // Gamma:                   .X
  for(i = 0; i < n; i++)
  {
    accu = ugam * x[i*stride]  + gam * accu;
    y[i*stride] = (T) accu;
  }
}

template<typename T>
  static void lexp_reverse(const T *x, T *y, uint16_t n, float gamma, uint16_t stride)
{
  signed short i;
  float accu = x[(n-1)*stride];
  float gam  = gamma;
  float ugam = 1.0 - gamma;
  // Accu:  représentation XXX.X
  // Gamma:                   .X
  for(i = n-1; i >= 0; i--)
  {
    accu = ugam * x[i*stride]  + gam * accu;
    y[i*stride] = (T) accu;
  }
}

template<typename T>
  void lexp2_forward(const T *x, T *y, uint16_t n, float gamma, uint16_t stride)
{
  lexp_forward<T>(x, y, n, gamma, stride);
  lexp_forward<T>(x, y, n, gamma, stride);
}

template<typename T>
  void lexp2_reverse(const T *x, T *y, uint16_t n, float gamma, uint16_t stride)
{
  lexp_reverse<T>(x, y, n, gamma, stride);
  lexp_reverse<T>(x, y, n, gamma, stride);
}

template<typename T>
void lgl_1d(const T *x, T *y, uint16_t n, float gamma, uint16_t stride)
{
  lexp2_forward<T>(x, y, n, gamma, stride);
  lexp2_reverse<T>(x, y, n, gamma, stride);
}



template<typename T>
  void garciaLorcaBlur_template(const cv::Mat &I, cv::Mat &O, float gamma)
{
  uint16_t sx   = I.cols,
           sy   = I.rows,
           nchn = I.channels();

  cv::Mat t2 = I.t(); // transposition

  // Filtrage vertical
  for(auto c = 0; c < nchn; c++)
  {
    for(uint16_t x = 0u; x < sx; x++)
      lgl_1d<T>(t2.ptr<T>(x)+c, t2.ptr<T>(x)+c, sy, gamma, nchn); // ligne x
  }

  O = t2.t();

  // Filtrage horizontal
  for(auto c = 0; c < nchn; c++)
  {
    for(uint16_t y = 0u; y < sy; y++)
      lgl_1d<T>(O.ptr<T>(y)+c, O.ptr<T>(y)+c, sx, gamma, nchn);
  }
}

int Deriche_blur(const cv::Mat &I, cv::Mat &O, float gamma)
{
  switch(I.depth())
  {
    case CV_32F:
      garciaLorcaBlur_template<float>(I, O, gamma);
      break;
    case CV_8U:
      garciaLorcaBlur_template<uint8_t>(I, O, gamma);
      break;
    case CV_16S:
      garciaLorcaBlur_template<int16_t>(I, O, gamma);
      break;
    default:
      fprintf(stderr, "garciaLorcaBlur: type de données non supporté (%d).\n", I.depth());
      return -1;
  }
  return 0;
}

int Deriche_gradient(const cv::Mat &I,
                    cv::Mat &gx,
                    cv::Mat &gy,
                    float gamma)
{
  cv::Mat lissee;  
  if(Deriche_blur(I, lissee, gamma))
    return -1;

  //cv::GaussianBlur(I, lissee, cv::Size(7,7), 2, 2);

  // Noyau simple, sans lissage : [-1 0 1]
  cv::Sobel(lissee, gx, CV_32F, 1, 0, 1);
  cv::Sobel(lissee, gy, CV_32F, 0, 1, 1);
  return 0;
}

int Deriche_gradient(const cv::Mat &I,
                    cv::Mat &O,
                    float gamma)
{
  cv::Mat gx, gy;
  if(Deriche_gradient(I, gx, gy, gamma))
    return -1;
  cv::Mat agx, agy;
  convertScaleAbs(gx,agx); // Conversion 8 bits non signé
  convertScaleAbs(gy,agy); // Conversion 8 bits non signé
  addWeighted(agx, .5, agy, .5, 0, O);
  cv::normalize(O, O, 0, 255, cv::NORM_MINMAX);
  return 0;
}


#if 0

#define trace(...) printf(__VA_ARGS__); fflush(0);

class GLRowFilter: public cv::BaseRowFilter
{
public:
  GLRowFilter(float gamma, int type)
  {
    this->gamma = gamma;
    this->type = type;
    anchor = 0;
    ksize = 2;
  }
  virtual void operator()(const uchar *src, uchar *dst, int width, int cn)
  {
    trace("row filter: width = %d, cn = %d, src = %x, dst = %x\n", width, cn, (uint32_t) src, (uint32_t) dst);
    //trace("anchor = %d, ksize = %d.\n", anchor, ksize);
    switch(type)
    {
    //case CV_8U:
      //for(auto c = 0; c < cn; c++)
        //lgl_1d<uint8_t>(&(src[c]),&(dst[c]),width,gamma,cn);
      //break;
    case CV_32F:
    {
      const float *fsrc = (const float *) src;
      float *fdst = (float *) dst;
      for(auto c = 0; c < cn; c++)
        for(auto x = 0; x < width; x++)
          fdst[x*cn+c] = fsrc[x*cn+c];
        //lgl_1d<float>(&(fsrc[c]), &(fdst[c]), width, gamma, cn);
      break;
    }
    default:
      printf("Type non supporté : %d.\n", type); fflush(0);
      abort();
      return;
    }
  }
  float gamma;
  int type;
};

class GLColFilter: public cv::BaseColumnFilter
{
public:
  GLColFilter(float gamma, int type)
  {
    this->gamma = gamma;
    this->type = type;
    anchor = 0;
    ksize = 2;
  }
  virtual void operator()(const uchar **src, uchar *dst,
                          int dststep, int dstcount, int width)
  {
    trace("col filter: dstep = %d, dcount = %d, width = %d\n", dststep, dstcount, width);
    trace("src = %x, src[0] = %x, src[1] = %x, dst = %x\n", (int) src, (int) src[0], (int) src[1], (int) dst);

    /*int prev = (int) src[0];
    for(auto i = 1; i < dstcount; i++)
    {
      int s = (int) src[i];
      trace("ecart %d: %d.\n", i, s - prev);
      prev = s;
    }*/

    switch(type)
    {
    //case CV_8U:
      //for(auto c = 0; c < cn; c++)
      //  lgl_1d<uint8_t>(&(src[c]),&(dst[c]),width,gamma,cn);
      //break;
    case CV_32F:
    {
      const float **fsrc = (const float **) src;
      float *fdst = (float *) dst;
      //for(auto c = 0; c < cn; c++)
        //lgl_1d<float>(&(fsrc[c]), &(fdst[c]), width, gamma, cn);*/
      /*for(auto y = 0; y < dstcount; y++)
      {
        for(auto x = 0; x < width; x++)
          fdst[y*dststep/4+x] = (fsrc[y])[x];
          //fdst[y*dststep/4+x] = fsrc[y*dststep/4+x];
      }*/

      // PB 1 : les échantillons de sortie ne sont pas du tout alignés en mémoire
      // PB 2 : on ne fait qu'une partie à chaque fois !

      for(auto x = 0; x < width; x++)
      {
        lgl_1d<float>(&((fsrc[0])[x]), &(fdst[x]), dstcount, gamma, dststep/4);
      }
      break;
    }
    default:
      printf("Type non supporté : %d.\n", type); fflush(0);
      abort();
      return;
    }
  }
  float gamma;
  int type;
};

cv::Ptr<cv::FilterEngine> createGarciaLorcaFilter(float gamma, int type)
{
  cv::Ptr<cv::BaseRowFilter> row_filter(new GLRowFilter(gamma, type));
  cv::Ptr<cv::BaseColumnFilter> col_filter(new GLColFilter(gamma, type));

  trace("init filter engine...\n");
  cv::FilterEngine *fe = new cv::FilterEngine(nullptr, row_filter, col_filter,
                      type, type, type);
  trace("ok.\n");

  return cv::Ptr<cv::FilterEngine>(fe);
}

void garciaLorcaBlur(const cv::Mat &I,
                     cv::Mat &O,
                     float gamma)
{
  trace("creation filtre, type = %d...\n", I.type());
  auto f = createGarciaLorcaFilter(gamma, I.type());
  O = cv::Mat(I.rows,I.cols,I.type());
  trace("apply...\n");
  f->apply(I, O);//, cv::Rect(0,0,I.cols,I.rows));
}
#endif


