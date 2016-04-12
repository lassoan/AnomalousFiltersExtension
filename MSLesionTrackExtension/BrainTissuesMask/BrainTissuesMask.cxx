#include "itkImageFileWriter.h"
#include "itkImage.h"
#include "itkVector.h"
#include "itkListSample.h"
#include "itkGaussianMixtureModelComponent.h"
#include "itkExpectationMaximizationMixtureModelEstimator.h"
#include "itkNormalVariateGenerator.h"


#include "itkPluginUtilities.h"
#include "itkImageRegionConstIterator.h"
#include "itkImageRegionIterator.h"
#include "itkThresholdImageFilter.h"
#include "itkComposeImageFilter.h"

//Segmentation Methods
#include "itkBayesianClassifierImageFilter.h"
#include "itkBayesianClassifierInitializationImageFilter.h"
#include "itkBayesianClassifierImageFilter.h"
#include "itkScalarImageKmeansImageFilter.h"

#include "itkMRFImageFilter.h"
#include "itkDistanceToCentroidMembershipFunction.h"
#include "itkMinimumDecisionRule.h"

#include "BrainTissuesMaskCLP.h"

// Use an anonymous namespace to keep class types and function names
// from colliding when module is used as shared object module.  Every
// thing should be in an anonymous namespace except for the module
// entry point, e.g. main()
//
namespace
{

template <class T>
int DoIt( int argc, char * argv[], T )
{
    PARSE_ARGS;

    typedef    float                    InputPixelType;
    typedef    unsigned char            OutputPixelType;
    const unsigned int                  Dimension=3;

    typedef itk::Image<InputPixelType,  Dimension> InputImageType;
    typedef itk::Image<OutputPixelType, Dimension> OutputImageType;

    typedef itk::ImageFileReader<InputImageType>    ReaderType;

    typename ReaderType::Pointer reader = ReaderType::New();
    reader->SetFileName( inputVolume.c_str() );
    reader->Update();

//TODO Study the use of Gaussian Mxiture Model to estimate the mean values in the image
    double meanValues[numClass];
//    = meanGuess;
    if (guessMeans) {
        // Estimating the initial mean values
        typedef itk::ScalarImageKmeansImageFilter< InputImageType > KMeansFilterType;
        KMeansFilterType::Pointer kmeansFilter = KMeansFilterType::New();
        kmeansFilter->SetInput( reader->GetOutput() );
        const unsigned int numberOfInitialClasses = numClass;

        for( unsigned k=0; k < numberOfInitialClasses; k++ )
        {
            kmeansFilter->AddClassWithInitialMean(meanGuess[k]);
        }

        kmeansFilter->Update();

        KMeansFilterType::ParametersType estimatedMeans = kmeansFilter->GetFinalMeans();
        const unsigned int numberOfClasses = estimatedMeans.Size();
        for ( unsigned int i = 0; i < numberOfClasses; ++i )
        {
            meanValues[i] = estimatedMeans[i];
            std::cout<<"Estimated mean["<<i<<"] = "<<meanValues[i]<<std::endl;
        }
    }else{
        for (int n = 0; n < numClass; ++n) {
            meanValues[n]=meanGuess[n];
        }
    }

    //            Apply segmentation procedure
    if (segMethod == "KMeans") {
        //K-Means Segmentation Approach
        typedef itk::ScalarImageKmeansImageFilter< InputImageType > KMeansFilterType;
        KMeansFilterType::Pointer kmeansFilter = KMeansFilterType::New();
        kmeansFilter->SetInput( reader->GetOutput() );
        const unsigned int numberOfInitialClasses = numClass;

        for( unsigned k=0; k < numberOfInitialClasses; k++ )
        {
            kmeansFilter->AddClassWithInitialMean(meanValues[k]);
        }

        kmeansFilter->Update();

        if (oneTissue) {
            KMeansFilterType::OutputImagePixelType tissueValue;
            if (typeTissue == "White Matter") {
                tissueValue = 3;
            }else if (typeTissue == "Gray Matter") {
                tissueValue = 2;
            }else if (typeTissue == "CSF") {
                tissueValue = 1;
            }

            typedef itk::ThresholdImageFilter <OutputImageType>     ThresholdImageFilterType;
            typename  ThresholdImageFilterType::Pointer thresholdFilter  = ThresholdImageFilterType::New();
            thresholdFilter->SetInput(kmeansFilter->GetOutput());
            thresholdFilter->ThresholdOutside(tissueValue, tissueValue);
            thresholdFilter->SetOutsideValue(0);

            KMeansFilterType::ParametersType estimatedMeans =
                    kmeansFilter->GetFinalMeans();
            const unsigned int numberOfClasses = estimatedMeans.Size();
            for ( unsigned int i = 0; i < numberOfClasses; ++i )
            {
                std::cout << "cluster[" << i << "] ";
                std::cout << "    estimated mean : " << estimatedMeans[i] << std::endl;
            }

            typedef itk::ImageFileWriter<OutputImageType> WriterType;
            typename WriterType::Pointer writer = WriterType::New();
            writer->SetFileName( outputLabel.c_str() );
            writer->SetInput( thresholdFilter->GetOutput() );
            writer->SetUseCompression(1);
            writer->Update();
            return EXIT_SUCCESS;
        }else{
            //        Take all the segmented tissues
            KMeansFilterType::ParametersType estimatedMeans =
                    kmeansFilter->GetFinalMeans();
            const unsigned int numberOfClasses = estimatedMeans.Size();
            for ( unsigned int i = 0; i < numberOfClasses; ++i )
            {
                std::cout << "cluster[" << i << "] ";
                std::cout << "    estimated mean : " << estimatedMeans[i] << std::endl;
            }

            typedef KMeansFilterType::OutputImageType  OutputImageType;
            typedef itk::ImageFileWriter< OutputImageType > WriterType;
            WriterType::Pointer writer = WriterType::New();
            writer->SetInput( kmeansFilter->GetOutput() );
            writer->SetUseCompression(1);
            writer->SetFileName( outputLabel.c_str() );
            writer->Update();
            return EXIT_SUCCESS;
        }

    }else if (segMethod == "Bayesian") {
        //Bayesian Segmentation Approach
        typedef itk::BayesianClassifierInitializationImageFilter< InputImageType >         BayesianInitializerType;
        typename BayesianInitializerType::Pointer bayesianInitializer = BayesianInitializerType::New();

        bayesianInitializer->SetInput( reader->GetOutput() );
        bayesianInitializer->SetNumberOfClasses( numClass );// Background, WM, GM and CSF
        bayesianInitializer->Update();

        typedef unsigned char  LabelType;
        typedef float          PriorType;
        typedef float          PosteriorType;

        typedef itk::VectorImage< InputPixelType, Dimension > VectorInputImageType;
        typedef itk::BayesianClassifierImageFilter< VectorInputImageType,LabelType, PosteriorType,PriorType >   ClassifierFilterType;
        typename ClassifierFilterType::Pointer bayesClassifier = ClassifierFilterType::New();

        bayesClassifier->SetInput( bayesianInitializer->GetOutput() );

        if (oneTissue) {
            unsigned char tissueValue;
            if (typeTissue == "White Matter") {
                tissueValue = 3;
            }else if (typeTissue == "Gray Matter") {
                tissueValue = 2;
            }else if (typeTissue == "CSF") {
                tissueValue = 1;
            }

            typedef itk::ThresholdImageFilter<OutputImageType> ThresholdType;
            typename ThresholdType::Pointer thresholder = ThresholdType::New();
            thresholder->SetInput(bayesClassifier->GetOutput());
            thresholder->ThresholdOutside(tissueValue, tissueValue);
            thresholder->SetOutsideValue(0);

            typedef itk::ImageFileWriter<OutputImageType> WriterType;
            typename WriterType::Pointer writer = WriterType::New();
            writer->SetFileName( outputLabel.c_str() );
            writer->SetInput( thresholder->GetOutput() );
            writer->SetUseCompression(1);
            writer->Update();
            return EXIT_SUCCESS;
        }else{
            //        Take all the tissues segmented
            typedef itk::ImageFileWriter<ClassifierFilterType::OutputImageType> WriterType;
            typename WriterType::Pointer writer = WriterType::New();
            writer->SetFileName( outputLabel.c_str() );
            writer->SetInput( bayesClassifier->GetOutput() );
            writer->SetUseCompression(1);
            writer->Update();
            return EXIT_SUCCESS;
        }
    }else if (segMethod == "MRF") {
        //Markov Random Field Segmentation Method
        typedef itk::FixedArray<InputPixelType,1>  ArrayPixelType;
        typedef itk::Image< ArrayPixelType, Dimension > ArrayImageType;
        typedef itk::ComposeImageFilter< InputImageType, ArrayImageType > ScalarToArrayFilterType;

        ScalarToArrayFilterType::Pointer  scalarToArrayFilter = ScalarToArrayFilterType::New();
        scalarToArrayFilter->SetInput( reader->GetOutput() );

        typedef itk::MRFImageFilter< ArrayImageType, OutputImageType > MRFFilterType;

        MRFFilterType::Pointer mrfFilter = MRFFilterType::New();

        mrfFilter->SetInput( scalarToArrayFilter->GetOutput() );
        mrfFilter->SetNumberOfClasses( numClass );
        mrfFilter->SetMaximumNumberOfIterations( 30 );
        mrfFilter->SetErrorTolerance( 1e-7 );

        mrfFilter->SetSmoothingFactor( 1 );

        typedef itk::ImageClassifierBase< ArrayImageType, OutputImageType >   SupervisedClassifierType;
        SupervisedClassifierType::Pointer classifier = SupervisedClassifierType::New();

        typedef itk::Statistics::MinimumDecisionRule DecisionRuleType;
        DecisionRuleType::Pointer  classifierDecisionRule = DecisionRuleType::New();
        classifier->SetDecisionRule( classifierDecisionRule.GetPointer() );

        typedef itk::Statistics::DistanceToCentroidMembershipFunction< ArrayPixelType >  MembershipFunctionType;
        typedef MembershipFunctionType::Pointer MembershipFunctionPointer;

        double meanDistance = 0;
        MembershipFunctionType::CentroidType centroid(1);
        for( unsigned int i=0; i < static_cast<unsigned int>(numClass); i++ )
        {
            MembershipFunctionPointer membershipFunction = MembershipFunctionType::New();
            centroid[0] = meanValues[i];

            membershipFunction->SetCentroid( centroid );

            classifier->AddMembershipFunction( membershipFunction );
            meanDistance += static_cast< double > (centroid[0]);
        }

        meanDistance /= numClass;

        mrfFilter->SetSmoothingFactor( 1 );
        mrfFilter->SetNeighborhoodRadius( 1 );

        //TODO Review how is the matrix neighborhood is build!
        std::vector< double > weights;
        weights.push_back(1.0);
        weights.push_back(1.5);
        weights.push_back(1.0);
        weights.push_back(1.5);
        weights.push_back(1.0); // This is the central pixel
        weights.push_back(1.5);
        weights.push_back(1.0);
        weights.push_back(1.5);
        weights.push_back(1.0);

        weights.push_back(1.5);
        weights.push_back(2.0);
        weights.push_back(1.5);
        weights.push_back(2.0);
        weights.push_back(0.0); // This is the central pixel
        weights.push_back(2.0);
        weights.push_back(1.5);
        weights.push_back(2.0);
        weights.push_back(1.5);

        weights.push_back(1.0);
        weights.push_back(1.5);
        weights.push_back(1.0);
        weights.push_back(1.5);
        weights.push_back(1.0); // This is the central pixel
        weights.push_back(1.5);
        weights.push_back(1.0);
        weights.push_back(1.5);
        weights.push_back(1.0);

        double totalWeight = 0;
        for(std::vector< double >::const_iterator wcIt = weights.begin();
            wcIt != weights.end(); ++wcIt )
        {
            totalWeight += *wcIt;
        }
        for(std::vector< double >::iterator wIt = weights.begin();
            wIt != weights.end(); ++wIt )
        {
            *wIt = static_cast< double > ( (*wIt) * meanDistance / (2 * totalWeight));
        }

        mrfFilter->SetMRFNeighborhoodWeight( weights );
        mrfFilter->SetClassifier( classifier );

        typedef MRFFilterType::OutputImageType  OutputImageType;

        if (oneTissue) {
            typedef itk::ThresholdImageFilter<OutputImageType> ThresoldType;
            typename ThresoldType::Pointer thresolder = ThresoldType::New();
            thresolder->SetInput(mrfFilter->GetOutput());

            MRFFilterType::OutputImagePixelType tissueValue;
            if (typeTissue == "White Matter") {
                tissueValue = 3;
            }else if (typeTissue == "Gray Matter") {
                tissueValue = 2;
            }else if (typeTissue == "CSF") {
                tissueValue = 1;
            }
            thresolder->ThresholdOutside(tissueValue, tissueValue);
            thresolder->SetOutsideValue(0);

            typedef itk::ImageFileWriter< OutputImageType > WriterType;
            WriterType::Pointer writer = WriterType::New();
            writer->SetInput( thresolder->GetOutput() );
            writer->SetFileName( outputLabel.c_str() );
            writer->Update();

            //General Information after segmentation
            std::cout << "Number of Iterations : ";
            std::cout << mrfFilter->GetNumberOfIterations() << std::endl;
            std::cout << "Stop condition: " << std::endl;
            std::cout << "  (1) Maximum number of iterations " << std::endl;
            std::cout << "  (2) Error tolerance:  "  << std::endl;
            std::cout << mrfFilter->GetStopCondition() << std::endl;

            return EXIT_SUCCESS;
        }else{
            typedef itk::ImageFileWriter< OutputImageType > WriterType;
            WriterType::Pointer writer = WriterType::New();
            writer->SetInput( mrfFilter->GetOutput() );
            writer->SetFileName( outputLabel.c_str() );
            writer->Update();

            // General Information after segmentation
            std::cout << "Number of Iterations : ";
            std::cout << mrfFilter->GetNumberOfIterations() << std::endl;
            std::cout << "Stop condition: " << std::endl;
            std::cout << "  (1) Maximum number of iterations " << std::endl;
            std::cout << "  (2) Error tolerance:  "  << std::endl;
            std::cout << mrfFilter->GetStopCondition() << std::endl;

            return EXIT_SUCCESS;
        }
    }
    return EXIT_SUCCESS;
}

} // end of anonymous namespace

int main( int argc, char * argv[] )
{
    PARSE_ARGS;

    itk::ImageIOBase::IOPixelType     pixelType;
    itk::ImageIOBase::IOComponentType componentType;

    try
    {
        itk::GetImageType(inputVolume, pixelType, componentType);

        // This filter handles all types on input, but only produces
        // signed types
        switch( componentType )
        {
        case itk::ImageIOBase::UCHAR:
            return DoIt( argc, argv, static_cast<unsigned char>(0) );
            break;
        case itk::ImageIOBase::CHAR:
            return DoIt( argc, argv, static_cast<char>(0) );
            break;
        case itk::ImageIOBase::USHORT:
            return DoIt( argc, argv, static_cast<unsigned short>(0) );
            break;
        case itk::ImageIOBase::SHORT:
            return DoIt( argc, argv, static_cast<short>(0) );
            break;
        case itk::ImageIOBase::UINT:
            return DoIt( argc, argv, static_cast<unsigned int>(0) );
            break;
        case itk::ImageIOBase::INT:
            return DoIt( argc, argv, static_cast<int>(0) );
            break;
        case itk::ImageIOBase::ULONG:
            return DoIt( argc, argv, static_cast<unsigned long>(0) );
            break;
        case itk::ImageIOBase::LONG:
            return DoIt( argc, argv, static_cast<long>(0) );
            break;
        case itk::ImageIOBase::FLOAT:
            return DoIt( argc, argv, static_cast<float>(0) );
            break;
        case itk::ImageIOBase::DOUBLE:
            return DoIt( argc, argv, static_cast<double>(0) );
            break;
        case itk::ImageIOBase::UNKNOWNCOMPONENTTYPE:
        default:
            std::cout << "unknown component type" << std::endl;
            break;
        }
    }

    catch( itk::ExceptionObject & excep )
    {
        std::cerr << argv[0] << ": exception caught !" << std::endl;
        std::cerr << excep << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
