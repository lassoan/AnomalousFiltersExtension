#ifndef __MultimodalityRegistration_h
#define __MultimodalityRegistration_h

//Basic ITK Classes
#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkImageIOBase.h"

//Registration ITK Classes
#include "itkImageRegistrationMethod.h"
#include "itkAffineTransform.h"
#include "itkMutualInformationImageToImageMetric.h"
#include "itkGradientDescentOptimizer.h"


const unsigned int Dimension = 3;

using namespace std;
using namespace itk;


/*
 * Class for multimodality registration pipeline.
 *
 */
class MultimodalityRegistration
{
private:
    stringstream folderPath;
    double nSamples;
    double learnRate;
    int nInteration;
    double metricFixedStd;
    double metricMovingStd;
public:
    MultimodalityRegistration(const char *folderPath);
    enum atlasType{
        MNI_T1_2mm,
        MNI_T1_1mm,
        MNI_T1_05mm,
        MNI_T2_1mm
    };
    enum imageModality{
        T1,
        T1Gd,
        FLAIR,
        FA
    };
    void setNSamples(double nSamples);
    void setLearnRate(double learnRate);
    void setNumInteration(int nInteration);
    void setMetricFixedStd(double metricFixedStd);
    void setMetricMovingStd(double metricMovingStd);
    atlasType getAtlas();
    double getNSamples();
    double getLearnRate();
    int getNumInteration();
    double getMetricFixedStd();
    double getMetricMovingStd();
//    void startITKRegistration(imageModality movingImage);
    void startCLIRegistration(imageModality movingImage);
};

#endif
