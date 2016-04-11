#include "itkImageFileWriter.h"
#include "itkImage.h"

#include "itkPluginUtilities.h"
#include "itkCastImageFilter.h"
#include "itkImageDuplicator.h"
#include "itkRescaleIntensityImageFilter.h"
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
    const unsigned int                   Dimension=3;

    typedef itk::Image<InputPixelType,  Dimension> InputImageType;
    typedef itk::Image<OutputPixelType, Dimension> OutputImageType;

    typedef itk::ImageFileReader<InputImageType>    ReaderType;

    typename ReaderType::Pointer reader = ReaderType::New();
    reader->SetFileName( inputVolume.c_str() );
    reader->Update();

    //            Apply segmentation procedure
    if (segMethod == "KMeans") {
        //K-Means Segmentation Approach
        typedef itk::ScalarImageKmeansImageFilter< InputImageType > KMeansFilterType;
        KMeansFilterType::Pointer kmeansFilter = KMeansFilterType::New();
        kmeansFilter->SetInput( reader->GetOutput() );
        const unsigned int numberOfInitialClasses = numClass;

        for( unsigned k=0; k < numberOfInitialClasses; k++ )
        {
            //        const double userProvidedInitialMean = atof( argv[k+argoffset] );
            //        kmeansFilter->AddClassWithInitialMean( userProvidedInitialMean );
            kmeansFilter->AddClassWithInitialMean(meanGuess[k]);
        }

        if (oneTissue) {
            typename OutputImageType::Pointer outLabel = OutputImageType::New();
            outLabel->SetBufferedRegion( reader->GetOutput()->GetBufferedRegion() );
            outLabel->SetRequestedRegion( reader->GetOutput()->GetRequestedRegion() );
            outLabel->Allocate();

            typedef itk::ImageRegionConstIterator<KMeansFilterType::OutputImageType> ConstIteratorType;
            typedef itk::ImageRegionIterator<OutputImageType>       IteratorType;
            ConstIteratorType allTissues(kmeansFilter->GetOutput(), kmeansFilter->GetOutput()->GetRequestedRegion());
            IteratorType oneTissue(outLabel, outLabel->GetRequestedRegion());
            unsigned char tissueValue;
            if (typeTissue == "White Matter") {
                tissueValue = 3;
            }else if (typeTissue == "Gray Matter") {
                tissueValue = 2;
            }else if (typeTissue == "CSF") {
                tissueValue = 1;
            }

            allTissues.GoToBegin();
            oneTissue.GoToBegin();
            while (!allTissues.IsAtEnd()) {
                if (allTissues.Get()==tissueValue) {
                    oneTissue.Set(allTissues.Get());
                }else{
                    oneTissue.Set(0);
                }
                ++allTissues;
                ++oneTissue;
            }

            typedef itk::ImageFileWriter<OutputImageType> WriterType;
            typename WriterType::Pointer writer = WriterType::New();
            writer->SetFileName( outputLabel.c_str() );
            writer->SetInput( outLabel );
            writer->SetUseCompression(1);
            writer->Update();
            return EXIT_SUCCESS;
        }else{
            //        Take all the segmented tissues
            typedef KMeansFilterType::OutputImageType  OutputImageType;
            typedef itk::ImageFileWriter< OutputImageType > WriterType;
            WriterType::Pointer writer = WriterType::New();
            writer->SetInput( kmeansFilter->GetOutput() );
            writer->SetUseCompression(1);
            writer->SetFileName( outputLabel );
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
            typename OutputImageType::Pointer outLabel = OutputImageType::New();
            outLabel->SetBufferedRegion( reader->GetOutput()->GetBufferedRegion() );
            outLabel->SetRequestedRegion( reader->GetOutput()->GetRequestedRegion() );
            outLabel->Allocate();

            typedef itk::ImageRegionConstIterator<ClassifierFilterType::OutputImageType> ConstIteratorType;
            typedef itk::ImageRegionIterator<OutputImageType>       IteratorType;
            ConstIteratorType allTissues(bayesClassifier->GetOutput(), bayesClassifier->GetOutput()->GetBufferedRegion());
            IteratorType oneTissue(outLabel, outLabel->GetBufferedRegion());
            unsigned char tissueValue;
            if (typeTissue == "White Matter") {
                tissueValue = 3;
            }else if (typeTissue == "Gray Matter") {
                tissueValue = 2;
            }else if (typeTissue == "CSF") {
                tissueValue = 1;
            }

            allTissues.GoToBegin();
            oneTissue.GoToBegin();
            while (!allTissues.IsAtEnd()) {
                if (allTissues.Get()==tissueValue) {
                    oneTissue.Set(allTissues.Get());
                }else{
                    oneTissue.Set(0);
                }
                ++allTissues;
                ++oneTissue;
            }

            typedef itk::ImageFileWriter<OutputImageType> WriterType;
            typename WriterType::Pointer writer = WriterType::New();
            writer->SetFileName( outputLabel.c_str() );
            writer->SetInput( outLabel );
            writer->SetUseCompression(1);
            writer->Update();
            return EXIT_SUCCESS;









            //            typedef ClassifierFilterType::OutputImageType            TissueType;
            //            typedef itk::Image<TissueType::InternalPixelType, Dimension> ExtractImage;
            //            typename ExtractImage::Pointer tissue = ExtractImage::New();
            //                    tissue->CopyInformation(bayesClassifier->GetOutput());
            //            tissue->SetBufferedRegion(bayesClassifier->GetOutput()->GetBufferedRegion());
            //            tissue->SetRequestedRegion(bayesClassifier->GetOutput()->GetRequestedRegion());
            //            tissue->Allocate();
            //                    typedef itk::ImageRegionConstIterator<TissueType> ConstIteratorType;
            //                    typedef itk::ImageRegionIterator<ExtractImage>       IteratorType;
            //                    ConstIteratorType allTissues(bayesClassifier->GetOutput(), bayesClassifier->GetOutput()->GetBufferedRegion());
            //                    IteratorType oneTissue(tissue, tissue->GetLargestPossibleRegion());
            //                    unsigned int tissueType;
            //                    if (typeTissue == "White Matter") {
            //                        tissueType = 3;
            //                    }else if (typeTissue == "Gray Matter") {
            //                        tissueType = 2;
            //                    }else if (typeTissue == "CSF") {
            //                        tissueType = 1;
            //                    }

            //                    allTissues.GoToBegin();
            //                    oneTissue.GoToBegin();
            //                    while (!allTissues.IsAtEnd()) {
            //                        if (allTissues.Get()==tissueType) {
            //                            oneTissue.Set(allTissues.Get());
            //                        }else{
            //                        oneTissue.Set(0);
            //                        }
            //                        ++allTissues;
            //                        ++oneTissue;
            //                    }

            //                    typedef itk::Image< unsigned char, Dimension > OutputImageType;
            //                    typedef itk::RescaleIntensityImageFilter<
            //                      ExtractImage, OutputImageType > RescalerType;
            //                    RescalerType::Pointer rescaler = RescalerType::New();
            //                    rescaler->SetInput( tissue );
            //                    rescaler->SetOutputMinimum( 0 );
            //                    rescaler->SetOutputMaximum( 255 );
            //                    typedef itk::ImageFileWriter<  OutputImageType
            //                                        >  ExtractedComponentWriterType;
            //                    ExtractedComponentWriterType::Pointer
            //                               rescaledImageWriter = ExtractedComponentWriterType::New();
            //                    rescaledImageWriter->SetInput( rescaler->GetOutput() );
            //                    rescaledImageWriter->SetFileName( outputLabel.c_str() );
            //                    rescaledImageWriter->Update();





            //                        typedef itk::ImageFileWriter<ClassifierFilterType::OutputImageType> WriterType;
            //                        typename WriterType::Pointer writer = WriterType::New();
            //                        writer->SetFileName( outputLabel.c_str() );
            //                        writer->SetInput( bayesClassifier->GetOutput() );
            //                        writer->SetUseCompression(1);
            //                        writer->Update();
//            return EXIT_SUCCESS;
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

        //        typedef itk::ImageFileWriter<ClassifierFilterType::OutputImageType> WriterType;
        //        typename WriterType::Pointer writer = WriterType::New();
        //        writer->SetFileName( outputLabel.c_str() );
        //        writer->SetInput( bayesClassifier->GetOutput() );
        //        writer->SetUseCompression(1);
        //        writer->Update();

        //        return EXIT_SUCCESS;
    }else if (segMethod == "MRF") {
        //Markov Random Field Segmentation Method
    }

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
