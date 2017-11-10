#ifndef FOURIER_DEMO_H
#define FOURIER_DEMO_H


#include "ocvdemo-item.hpp"

class DFTDemo: public OCVDemoItem
{
public:
  DFTDemo();
  int proceed(OCVDemoItemInput &input, OCVDemoItemOutput &output);
};


class IFTDemo: public OCVDemoItem
{
public:
  IFTDemo();
  int proceed(OCVDemoItemInput &input, OCVDemoItemOutput &output);
};

class DemoSousSpectrale: public OCVDemoItem
{
public:
  DemoSousSpectrale();
  int proceed(OCVDemoItemInput &input, OCVDemoItemOutput &output);
};

class DemoDetectionRotation: public OCVDemoItem
{
public:
  DemoDetectionRotation();
  int proceed(OCVDemoItemInput &input, OCVDemoItemOutput &output);
};

class DemoDetectionTranslation: public OCVDemoItem
{
public:
  DemoDetectionTranslation();
  int proceed(OCVDemoItemInput &input, OCVDemoItemOutput &output);
};

class DemoDetectionPeriode: public OCVDemoItem
{
public:
  DemoDetectionPeriode();
  int proceed(OCVDemoItemInput &input, OCVDemoItemOutput &output);
};

#endif
