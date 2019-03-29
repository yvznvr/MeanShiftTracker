#ifndef MEANSHIFTTRACKER_H
#define MEANSHIFTTRACKER_H

#include <stdint.h>
#include <math.h>


class MeanShiftTracker
{
    double *originalPdf, *pdfOfTarget;
    double gradiant[2]{0};
    uint8_t *frame;
    int x1, y1, x2, y2, height, width, rectHeight, rectWidth;
    int radiusOfBin = 16;
    int pdfLength;
    bool selected = false;
public:
    MeanShiftTracker();
    ~MeanShiftTracker();
    void setFrame(int width, int height, uint8_t* frame);
    void setArea(int x1, int y1, int x2, int y2);
    void calcOriginalPdf();
    void calcPdfOfTarget(int x, int y, uint8_t* frame, double *pdf);
    void calcGradient(uint8_t *t1);
    double bhattacharyyaCoefficient(double *p, double *q);
    double gaussianKernel(double value);
    double derivativeOfGaussianKernel(double value);
    int kroneckerDelta(int value);
    void tracking(uint8_t *t1, double *points);
    void setSelected(bool value);
};

#endif // MEANSHIFTTRACKER_H
