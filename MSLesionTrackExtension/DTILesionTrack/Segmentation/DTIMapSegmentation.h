#ifndef __DTIMapSegmentation_h
#define __DTIMapSegmentation_h

#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkImage.h"

//Segmentation Methods - ITK
#include "itkBayesianClassifierImageFilter.h"
#include "itkBayesianClassifierInitializationImageFilter.h"
#include "itkBayesianClassifierImageFilter.h"
#include "itkScalarImageKmeansImageFilter.h"

#include "itkMRFImageFilter.h"
#include "itkDistanceToCentroidMembershipFunction.h"
#include "itkMinimumDecisionRule.h"


//Support Classes
#include "itkRescaleIntensityImageFilter.h"
#include "itkComposeImageFilter.h"

//Image filters
#include "itkGradientAnisotropicDiffusionImageFilter.h"
#include "itkDiscreteGaussianImageFilter.h"
using namespace std;

/*
 * Class for brain extraction.
 * It is used here both FSL-BET and Freesurfer brain extraction tools.
 * Choose the more appropriated method to perform the brain extraction in your data.
 * TIPS: Usually, Freesurfer have a more robustic response over T1-Gd weighted images and for non-axial aquisiction orientation.
 */

class DTIMapSegmentation
{
private:
    const char* inputVolume;
    const char* outputLabels;
    int numClasses;
    int numIter;
    bool doFiltering;
    void BayesSegmentation();
    void MRFSegmentation();
    void KMeansSegmentation();
    void ClassicAnisotropicFiltering();
    void AnomalousAnisotropicFiltering();
    void GaussianFiltering();
    void NonLocalMeansFiltering();
public:
    enum filterMethod{
        ClassicAD,
        AnomalousAD,
        NLM,
        Gaussian
    };
    enum segMethod{
        MRF,
        BAYES,
        KMEANS
    };
    DTIMapSegmentation(const char *inputVolume, const char *outputLabels);
    DTIMapSegmentation(const char *inputVolume);
    int getNumClasses() const;
    void setNumClasses(int value);
    void runSegmentation(enum segMethod);
    void runFiltering(enum filterMethod);
    int getNumIter() const;
    void setNumIter(int value);
    bool getDoFiltering() const;
    void setDoFiltering(bool value);
};

#endif
