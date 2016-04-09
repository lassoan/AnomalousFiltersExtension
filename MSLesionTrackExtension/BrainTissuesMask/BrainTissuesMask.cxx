#include "itkImageFileWriter.h"

#include "itkPluginUtilities.h"
#include "itkImageDuplicator.h"
#include "itkImageRegionConstIterator.h"
#include "itkImageRegionIterator.h"

//Segmentation Methods
#include "itkBayesianClassifierImageFilter.h"
#include "itkBayesianClassifierInitializationImageFilter.h"
#include "itkBayesianClassifierImageFilter.h"
#include "itkScalarImageKmeansImageFilter.h"

#include "itkMRFImageFilter.h"
#include "itkDistanceToCentroidMembershipFunction.h"
#include "itkMinimumDecisionRule.h"

//Spatial Filters
#include "itkGradientAnisotropicDiffusionImageFilter.h"
#include "itkDiscreteGaussianImageFilter.h"

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

    typedef    T              InputPixelType;
    typedef    unsigned char  OutputPixelType;

    typedef itk::Image<InputPixelType,  3> InputImageType;
    typedef itk::Image<OutputPixelType, 3> OutputImageType;

    typedef itk::ImageFileReader<InputImageType>    ReaderType;
    typedef itk::ImageDuplicator<InputImageType>    InputDuplicatorType;
    typedef itk::ImageDuplicator<OutputImageType>   OutputDuplicatorType;


    typename ReaderType::Pointer reader = ReaderType::New();
    reader->SetFileName( inputVolume.c_str() );
    reader->Update();


    if (useBrainExtraction) {
        //TODO Build code to extract the brain in the input volume
    }

//    InputImageType::Pointer dup_reader = reader->GetOutput();
//    InputDuplicatorType::Pointer dup_reader = InputDuplicatorType::New();
//    dup_reader->SetInputImage(reader->GetOutput());


    typedef itk::DiscreteGaussianImageFilter<InputImageType, InputImageType> FilterType;
    typename FilterType::Pointer filter = FilterType::New();
    filter->SetInput(reader->GetOutput());
    filter->SetVariance(static_cast<double>(sqrt(2.0*nIter)));
    filter->Update();

//      if (doSmooth) {
//          if (filterMethod == "PM") {
//              typedef itk::GradientAnisotropicDiffusionImageFilter<InputImageType, InputImageType> FilterType;
//              typename FilterType::Pointer filter = FilterType::New();
//              filter->SetInput(reader->GetOutput());
//              filter->SetNumberOfIterations(nIter);
//              filter->SetTimeStep(0.125);
//              filter->Update();
//              dup_reader->SetInputImage(filter->GetOutput());
//          }else if (filterMethod == "Gaussian") {
//              typedef itk::RecursiveGaussianImageFilter<InputImageType, InputImageType> FilterType;
//              typename FilterType::Pointer gauss = FilterType::New();
//              gauss->SetInput(reader->GetOutput());
//              gauss->SetSigma(static_cast<double>(sqrt(2.0*nIter)));
//              gauss->Update();
//              dup_reader->SetInputImage(gauss->GetOutput());
//          }
//      }



    //            Apply segmentation procedure
    typedef itk::BayesianClassifierInitializationImageFilter< InputImageType >         BayesianInitializerType;
    typename BayesianInitializerType::Pointer bayesianInitializer = BayesianInitializerType::New();

    bayesianInitializer->SetInput( filter->GetOutput() );
//      InputImageType::Pointer reader_step2 = dup_reader->GetModifiableOutput();
//      bayesianInitializer->SetInput( reader_step2 );
    bayesianInitializer->SetNumberOfClasses( 4 );// Background, WM, GM and CSF
    bayesianInitializer->Update();

    typedef unsigned char  LabelType;
    typedef float          PriorType;
    typedef float          PosteriorType;

    typedef itk::VectorImage< float, 3 > VectorInputImageType;
    typedef itk::BayesianClassifierImageFilter< VectorInputImageType,LabelType, PosteriorType,PriorType >   ClassifierFilterType;
    typename ClassifierFilterType::Pointer bayesClassifier = ClassifierFilterType::New();

    bayesClassifier->SetInput( bayesianInitializer->GetOutput() );



    //    Take only the choosen tissue
    typename OutputImageType::Pointer tissue = OutputImageType::New();
    tissue->SetRegions(bayesClassifier->GetOutput()->GetLargestPossibleRegion());
    tissue->Allocate();

    typedef itk::ImageRegionConstIterator<OutputImageType> ConstIteratorType;
    typedef itk::ImageRegionIterator<OutputImageType>       IteratorType;
    ConstIteratorType allTissues(bayesClassifier->GetOutput(), bayesClassifier->GetOutput()->GetLargestPossibleRegion());
    IteratorType oneTissue(tissue, bayesClassifier->GetOutput()->GetLargestPossibleRegion());
    unsigned int tissueType;
    if (typeTissue == "White Matter") {
        tissueType = 3;
    }else if (typeTissue == "Gray Matter") {
        tissueType = 2;
    }else if (typeTissue == "CSF") {
        tissueType = 1;
    }

    allTissues.GoToBegin();
    oneTissue.GoToBegin();
    while (!allTissues.IsAtEnd()) {
        if (allTissues.Get()==tissueType) {
            oneTissue.Set(allTissues.Get());
        }else{
        oneTissue.Set(0);
        }
        ++allTissues;
        ++oneTissue;
    }

    typedef itk::ImageFileWriter<ClassifierFilterType::OutputImageType> WriterType;
    typename WriterType::Pointer writer = WriterType::New();
    writer->SetFileName( outputLabel.c_str() );
//    writer->SetInput( tissue );
    writer->SetInput( bayesClassifier->GetOutput() );
    writer->SetUseCompression(1);
    writer->Update();

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
