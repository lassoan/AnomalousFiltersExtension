#include "MultimodalityRegistration.h"

using namespace std;


MultimodalityRegistration::MultimodalityRegistration(const char *folderPath)
{
    this->folderPath<<folderPath;
}

void MultimodalityRegistration::setNSamples(double nSamples)
{
    this->nSamples=nSamples;
}

void MultimodalityRegistration::setLearnRate(double learnRate)
{
    this->learnRate = learnRate;
}

void MultimodalityRegistration::setNumInteration(int nInteration)
{
    this->nInteration=nInteration;
}

void MultimodalityRegistration::setMetricFixedStd(double metricFixedStd)
{
    this->metricFixedStd=metricFixedStd;
}

void MultimodalityRegistration::setMetricMovingStd(double metricMovingStd)
{
    this->metricMovingStd=metricMovingStd;
}

double MultimodalityRegistration::getNSamples()
{
    return this->nSamples;
}

double MultimodalityRegistration::getLearnRate()
{
    return this->learnRate;
}

int MultimodalityRegistration::getNumInteration()
{
    return this->nInteration;
}

double MultimodalityRegistration::getMetricFixedStd()
{
    return this->metricFixedStd;
}

double MultimodalityRegistration::getMetricMovingStd()
{
    return this->metricMovingStd;
}

void MultimodalityRegistration::startCLIRegistration(MultimodalityRegistration::imageModality movingImage)
{
    stringstream atlasPath, movingPath, outputFileName, BRAINSFit;
    atlasPath<<folderPath.str().c_str()<<"/AtlasVolume.nii.gz";


    BRAINSFit<<"slicer --launch BRAINSFit";
    switch(movingImage){
    case T1:
        movingPath<<folderPath.str().c_str()<<"/t1Volume_brain.nii.gz";
        outputFileName<<folderPath.str().c_str()<<"/t1Volume_brain_corr.nii.gz";

        BRAINSFit
                <<" --fixedVolume "<<atlasPath.str().c_str()
               <<" --movingVolume "<<movingPath.str().c_str()
              <<" --outputVolume "<<outputFileName.str().c_str()
             <<" --transformType Affine"
            <<" --samplingPercentage "<<this->nSamples
           <<" --interpolationMode Linear"
          <<" --costMetric MMI"
         <<" --numberOfIterations "<<this->nInteration;
        //        cout<<BRAINSFit.str().c_str();
        system(BRAINSFit.str().c_str());
        break;
    case T1Gd:
        movingPath<<folderPath.str().c_str()<<"/t1GdVolume_brain.nii.gz";
        outputFileName<<folderPath.str().c_str()<<"/t1GdVolume_brain_corr.nii.gz";

        BRAINSFit
                <<" --fixedVolume "<<atlasPath.str().c_str()
               <<" --movingVolume "<<movingPath.str().c_str()
              <<" --outputVolume "<<outputFileName.str().c_str()
             <<" --transformType Affine"
            <<" --samplingPercentage "<<this->nSamples
           <<" --interpolationMode Linear"
          <<" --costMetric MMI"
         <<" --numberOfIterations "<<this->nInteration;
        //        cout<<BRAINSFit.str().c_str();
        system(BRAINSFit.str().c_str());
        break;
    case FLAIR:
        movingPath<<folderPath.str().c_str()<<"/FLAIRVolume_brain.nii.gz";
        outputFileName<<folderPath.str().c_str()<<"/FLAIRVolume_brain_corr.nii.gz";

        BRAINSFit
                <<" --fixedVolume "<<atlasPath.str().c_str()
               <<" --movingVolume "<<movingPath.str().c_str()
              <<" --outputVolume "<<outputFileName.str().c_str()
             <<" --transformType Affine"
            <<" --samplingPercentage "<<this->nSamples
           <<" --interpolationMode Linear"
          <<" --costMetric MMI"
         <<" --numberOfIterations "<<this->nInteration;
        //        cout<<BRAINSFit.str().c_str();
        system(BRAINSFit.str().c_str());
        break;
    case FA:
        movingPath<<folderPath.str().c_str()<<"/FAVolume_brain.nii.gz";
        outputFileName<<folderPath.str().c_str()<<"/FAVolume_brain_corr.nii.gz";

        BRAINSFit
                <<" --fixedVolume "<<atlasPath.str().c_str()
               <<" --movingVolume "<<movingPath.str().c_str()
              <<" --outputVolume "<<outputFileName.str().c_str()
             <<" --transformType Affine"
            <<" --samplingPercentage "<<this->nSamples
           <<" --interpolationMode Linear"
          <<" --costMetric MMI"
         <<" --numberOfIterations "<<this->nInteration;
        //        cout<<BRAINSFit.str().c_str();
        system(BRAINSFit.str().c_str());
    }
}

//void MultimodalityRegistration::startITKRegistration(imageModality movingImageModality)
//{
//    typedef itk::Image<float, Dimension>        ImageType;
//    typedef itk::ImageFileReader<ImageType>     ReaderType;
//    typedef itk::ImageFileWriter<ImageType>     WriterType;

//    //    Registration methods
//    typedef itk::AffineTransform<double, Dimension>                         TransformType;
//    typedef itk::GradientDescentOptimizer                                   OptimizerType;
//    typedef itk::LinearInterpolateImageFunction<ImageType,double>           InterpolatorType;
//    typedef itk::ImageRegistrationMethod<ImageType, ImageType>              RegistrationType;
//    typedef itk::MutualInformationImageToImageMetric<ImageType, ImageType>  MetricType;

//    typename ReaderType::Pointer        readerFixed     = ReaderType::New();
//    typename ReaderType::Pointer        readerMoving    = ReaderType::New();
//    typename WriterType::Pointer        writer          = WriterType::New();
//    typename TransformType::Pointer     transform       = TransformType::New();
//    typename OptimizerType::Pointer     optimizer       = OptimizerType::New();
//    typename InterpolatorType::Pointer  interpolator    = InterpolatorType::New();
//    typename MetricType::Pointer        metric          = MetricType::New();
//    typename RegistrationType::Pointer  registration    = RegistrationType::New();

//    registration->SetOptimizer(optimizer);
//    registration->SetMetric(metric);
//    registration->SetTransform(transform);
//    registration->SetInterpolator(interpolator);

//    ImageType::RegionType fixedImageRegion = atlasImage->GetOutput()->GetBufferedRegion();
//    registration->SetFixedImageRegion(fixedImageRegion);

//    typedef RegistrationType::ParametersType ParametersType;
//    ParametersType initialParameters(transform->GetNumberOfParameters());
//    // rotation matrix (identity)
//    initialParameters[0] = 1.0;  // R(0,0)
//    initialParameters[1] = 0.0;  // R(0,1)
//    initialParameters[2] = 0.0;  // R(1,0)
//    initialParameters[3] = 1.0;  // R(1,1)

//    // translation vector
//    initialParameters[4] = 0.0;
//    initialParameters[5] = 0.0;

//    registration->SetInitialTransformParameters( initialParameters );

//    const unsigned int numberOfPixels = fixedImageRegion.GetNumberOfPixels();
//    const unsigned int numberOfSamples = static_cast< unsigned int >( numberOfPixels * this->nSamples );
//    metric->SetNumberOfSpatialSamples( numberOfSamples );
//    metric->SetFixedImageStandardDeviation(metricFixedStd);
//    metric->SetMovingImageStandardDeviation(metricFixedStd);

//    optimizer->SetLearningRate( this->learnRate );
//    optimizer->SetNumberOfIterations( this->nInteration );
//    optimizer->MaximizeOn();

//    stringstream BrainPath,filePath;
//    TODO: trocar arquivos de imagens para _brain_nu.nii
//    switch (movingImageModality) {
//    case T1:
//        BrainPath<<folderPath.str().c_str()<<"/t1Volume.nii";
//        readerMoving->SetFileName(BrainPath.str().c_str());
//        readerMoving->Update();
//        readerFixed->SetFileName(fixedVolume.c_str());
//        readerFixed->Update();

//            registration->SetFixedImage(readerMoving->GetOutput());
//            registration->SetMovingImage(readerMoving->GetOutput());

//        registration->Update();

//        writer->SetInput(registration->GetOutput());
//        filePath<<folderPath.str().c_str()<<"/t1Volume_brain_nu_corr.nii";
//        writer->SetFileName(filePath.str().c_str());
//        writer->Update();
//        break;
//    case T1Gd:
//    Reading the T1-Gd volume
//        stringstream t1GdBrainPath;
//        BrainPath<<folderPath.str().c_str()<<"/t1GdVolume.nii";
//        readerMoving->SetFileName(BrainPath.str().c_str());
//        readerMoving->Update();

//    Add atlas or intrasubject image
//    registration->SetFixedImage();
//    registration->SetMovingImage();

//        registration->Update();

//        break;
//    case FLAIR:
//    Reading the T2-FLAIR volume
//        stringstream FLAIRBrainPath;
//        BrainPath<<folderPath.str().c_str()<<"/FLAIRVolume.nii";
//        readerMoving->SetFileName(BrainPath.str().c_str());
//        readerMoving->Update();

//    Add atlas or intrasubject image
//    registration->SetFixedImage();
//    registration->SetMovingImage();

//        registration->Update();
//        break;
//    case FA:
//    Reading the DTI-FA volume
//        stringstream FABrainPath;
//        BrainPath<<folderPath.str().c_str()<<"/FAVolume.nii";
//        readerMoving->SetFileName(BrainPath.str().c_str());
//        readerMoving->Update();

//    Add atlas or intrasubject image
//    registration->SetFixedImage();
//    registration->SetMovingImage();

//        registration->Update();
//        break;
//    }

//}

// /home/antonio/Slicer-Git/Slicer-SuperBuild-Debug/Slicer-build/lib/Slicer-4.3/cli-modules/BRAINSFit
//--fixedVolume /tmp/Slicer/FBJA_vtkMRMLScalarVolumeNodeG.nrrd --movingVolume /tmp/Slicer/FBJA_vtkMRMLScalarVolumeNodeE.nrrd
//--linearTransform /tmp/Slicer/FBJA_vtkMRMLLinearTransformNodeE.h5
//--outputVolume /tmp/Slicer/FBJA_vtkMRMLScalarVolumeNodeH.nrrd
//--initialTransform /tmp/Slicer/FBJA_vtkMRMLLinearTransformNodeE.h5
//--initializeTransformMode Off
//--samplingPercentage 0.02
//--useAffine
//--splineGridSize 14,10,12
//--numberOfIterations 1500
//--maskProcessingMode NOMASK
//--outputVolumePixelType float
//--backgroundFillValue 0
//--maskInferiorCutOffFromCenter 1000
//--interpolationMode Linear
//--minimumStepLength 0.001
//--translationScale 1000
//--reproportionScale 1
//--skewScale 1
//--maxBSplineDisplacement 0
//--numberOfHistogramBins 50
//--numberOfMatchPoints 10
//--numberOfSamples 0
//--fixedVolumeTimeIndex 0
//--movingVolumeTimeIndex 0
//--medianFilterSize 0,0,0
//--removeIntensityOutliers 0
//--useCachingOfBSplineWeightsMode ON
//--useExplicitPDFDerivativesMode AUTO
//--ROIAutoDilateSize 0
//--ROIAutoClosingSize 9
//--relaxationFactor 0.5
//--maximumStepLength 0.05
//--failureExitCode -1
//--numberOfThreads -1
//--forceMINumberOfThreads -1
//--debugLevel 0
//--costFunctionConvergenceFactor 2e+13
//--projectedGradientTolerance 1e-05
//--maximumNumberOfEvaluations 900
//--maximumNumberOfCorrections 25
//--metricSamplingStrategy Random
//--costMetric MMI
//--metricSeed 21474836
