#include "meanshifttracker.h"



void MeanShiftTracker::setSelected(bool value)
{
    selected = value;
}

MeanShiftTracker::MeanShiftTracker()
{
    pdfLength = 3*256/radiusOfBin;
    originalPdf = new double[pdfLength]{0};
    pdfOfTarget = new double[pdfLength]{0};
}

MeanShiftTracker::~MeanShiftTracker()
{
    delete[] originalPdf;
    delete[] pdfOfTarget;
}

void MeanShiftTracker::setArea(int x1, int y1, int x2, int y2)
{
    this->x1 = x1;
    this->y1 = y1;
    this->x2 = x2;
    this->y2 = y2;
    rectWidth = abs(x1-x2);
    rectHeight = abs(y1-y2);
    if(!selected)
    {
        selected = true;
        calcOriginalPdf();
    }
}

void MeanShiftTracker::setFrame(int width, int height, uint8_t* frame)
{
    this->width = width;
    this->height = height;
    this->frame = frame;
}

void MeanShiftTracker::calcOriginalPdf()
{
    int centerX = abs(x1+x2)/2;
    int centerY = abs(y1+y2)/2;
    double normalizaationConstant = 0;

    for(int i=x1; i<x2; i++)
    {
        for(int j=y1; j<y2; j++)
        {
            int x = i - centerX;
            int y = j - centerY;
            double gaussian = gaussianKernel(4*x*x/((double)rectWidth*rectWidth) + 4*y*y/((double)rectHeight*rectHeight));
            uint8_t r = frame[3*j*width+3*i];
            uint8_t g = frame[3*j*width+3*i+1];
            uint8_t b = frame[3*j*width+3*i+2];
            originalPdf[frame[3*j*width+3*i]/radiusOfBin] += gaussian;
            originalPdf[frame[3*j*width+3*i+1]/radiusOfBin+pdfLength/3] += gaussian;
            originalPdf[frame[3*j*width+3*i+2]/radiusOfBin+2*pdfLength/3] += gaussian;
            normalizaationConstant += gaussian;
        }
    }

    for(int u=0; u<pdfLength; u++)
    {
        originalPdf[u] *= (1/(3*normalizaationConstant));
    }

}

void MeanShiftTracker::calcPdfOfTarget(int centerX, int centerY, uint8_t* frame, double *pdf)
{
    double normalizaationConstant = 0;
    for(int i=centerX-(rectWidth/2); i<centerX+(rectWidth/2); i++)
    {
        for(int j=centerY-(rectHeight/2); j<centerY+(rectHeight/2); j++)
        {
            int x = i - centerX;
            int y = j - centerY;
            double gaussian = gaussianKernel(4*x*x/((double)rectWidth*rectWidth) + 4*y*y/((double)rectHeight*rectHeight));
            pdf[frame[3*j*width+3*i]/radiusOfBin] += gaussian;
            pdf[frame[3*j*width+3*i+1]/radiusOfBin+pdfLength/3] += gaussian;
            pdf[frame[3*j*width+3*i+2]/radiusOfBin+2*pdfLength/3] += gaussian;
            normalizaationConstant += gaussian;
        }
    }

    for(int u=0; u<pdfLength; u++)
    {
        pdf[u] *= (1/(3*normalizaationConstant));
    }
}


void MeanShiftTracker::calcGradient(uint8_t *t1)
{
    int centerX = abs(x1+x2)/2;
    int centerY = abs(y1+y2)/2;

    double divisor = 0;
    for(int i=x1; i<x2; i++)
    {
        for(int j=y1; j<y2; j++)
        {
            double weight = 0;
            uint8_t valueR = t1[3*j*width+3*i]/radiusOfBin;
            uint8_t valueG = t1[3*j*width+3*i+1]/radiusOfBin;
            uint8_t valueB = t1[3*j*width+3*i+2]/radiusOfBin;
            int x = i - centerX;
            int y = j - centerY;
            double gaussian = derivativeOfGaussianKernel(4*x*x/((double)rectWidth*rectWidth) + 4*y*y/((double)rectHeight*rectHeight));
            weight += sqrt(originalPdf[valueR] / pdfOfTarget[valueR]);
            weight += sqrt(originalPdf[valueG+pdfLength/3] / pdfOfTarget[valueG+pdfLength/3]);
            weight += sqrt(originalPdf[valueB+2*pdfLength/3] / pdfOfTarget[valueB+2*pdfLength/3]);
            divisor += weight*gaussian;
            gradiant[0] += weight*gaussian*x;
            gradiant[1] += weight*gaussian*y;
        }
    }
    gradiant[0] /= divisor;
    gradiant[1] /= divisor;
}

double MeanShiftTracker::bhattacharyyaCoefficient(double *p, double *q)
{
    double temp = 0;
    for(int i=0;i<pdfLength;i++)
    {
        temp += sqrt(p[i]*q[i]);
    }
    return temp;
}

double MeanShiftTracker::gaussianKernel(double value)
{
    return (1/sqrt(2*M_PI*2*M_PI))*exp(-0.5*value);
}

double MeanShiftTracker::derivativeOfGaussianKernel(double value)
{
    return (1/(2*sqrt(2*M_PI*2*M_PI)))*exp(-0.5*value);

}

int MeanShiftTracker::kroneckerDelta(int value)
{
    return value == 0;
}

void MeanShiftTracker::tracking(uint8_t *t1, double *points)
{
    int centerX = abs(x1+x2)/2;
    int centerY = abs(y1+y2)/2;
    for(int t=0; t<5; t++){
        calcPdfOfTarget(centerX, centerY, t1, pdfOfTarget);
        double bhattacharyyaValue = bhattacharyyaCoefficient(pdfOfTarget, originalPdf);
        calcGradient(t1);
        gradiant[0] += centerX;
        gradiant[1] += centerY;
        double pdfOfEstimatedArea[pdfLength];
        do
        {
            for(int i=0;i<pdfLength;i++) pdfOfEstimatedArea[i] = 0;
            calcPdfOfTarget(gradiant[0], gradiant[1], t1, pdfOfEstimatedArea);
            gradiant[0] = (centerX + gradiant[0]) / 2;
            gradiant[1] = (centerY + gradiant[1]) / 2;
            if( ((gradiant[0]-centerX)*(gradiant[0]-centerX) + (gradiant[1]-centerY)*(gradiant[1]-centerY)) < 1 ) break;
        }while(bhattacharyyaCoefficient(pdfOfEstimatedArea,originalPdf) <
              bhattacharyyaValue);

        points[0] = gradiant[0];
        points[1] = gradiant[1];
        centerX = gradiant[0];
        centerY = gradiant[1];
    }
    if(points[1]-(rectHeight/2)<0) points[1]=(rectHeight/2);
    else if(points[0]-(rectWidth/2)<0) points[0]=(rectWidth/2);
    else if(points[0]+(rectWidth/2)>width) points[0]=width-(rectWidth/2);
    else if(points[1]+(rectHeight/2)>height) points[1]=height-(rectHeight/2);
}
