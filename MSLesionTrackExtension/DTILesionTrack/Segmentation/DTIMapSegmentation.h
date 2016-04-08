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
    void BayesSegmentation();
    void MRFSegmentation();
    void KMeansSegmentation();
public:
    enum segMethod{
        MRF,
        BAYES,
        KMEANS
    };
    DTIMapSegmentation(const char *inputVolume, const char *outputLabels);
    int getNumClasses() const;
    void setNumClasses(int value);
    void runSegmentation(enum segMethod);
};

#endif
