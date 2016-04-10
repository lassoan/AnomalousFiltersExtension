#include "itkImageFileWriter.h"
#include "itkImage.h"

#include "itkPluginUtilities.h"
#include "itkCastImageFilter.h"
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

    typedef    float              InputPixelType;
    typedef    unsigned char  OutputPixelType;
    const unsigned int                   Dimension=3;

    typedef itk::Image<InputPixelType,  Dimension> InputImageType;
    typedef itk::Image<OutputPixelType, Dimension> OutputImageType;

    typedef itk::ImageFileReader<InputImageType>    ReaderType;
    //    typedef itk::CastImageFilter<InputPixelType, float>     CastInputType;



    typename ReaderType::Pointer reader = ReaderType::New();
    reader->SetFileName( inputVolume.c_str() );
    reader->Update();

    //            Apply segmentation procedure
    if (segMethod == "KMeans") {


        //K-Means Segmentation Approach

        std::cerr << " inputScalarImage outputLabeledImage nonContiguousLabels";
        std::cerr << " numberOfClasses mean1 mean2... meanN " << std::endl;

        typedef itk::ScalarImageKmeansImageFilter< InputImageType > KMeansFilterType;
        KMeansFilterType::Pointer kmeansFilter = KMeansFilterType::New();
        kmeansFilter->SetInput( reader->GetOutput() );
        const unsigned int numberOfInitialClasses = numClass;


        //      const unsigned int useNonContiguousLabels = atoi( argv[3] );
        //      kmeansFilter->SetUseNonContiguousLabels( useNonContiguousLabels );

        //      const unsigned int argoffset = 5;
        //      if( static_cast<unsigned int>(argc) <
        //          numberOfInitialClasses + argoffset )
        //        {
        //        std::cerr << "Error: " << std::endl;
        //        std::cerr << numberOfInitialClasses << " classes has been specified ";
        //        std::cerr << "but no enough means have been provided in the command ";
        //        std::cerr << "line arguments " << std::endl;
        //        return EXIT_FAILURE;
        //        }

        //      std::vector<unsigned int> means;
        double means [numClass];
        for( unsigned k=0; k < numberOfInitialClasses; k++ )
        {
            //        const double userProvidedInitialMean = atof( argv[k+argoffset] );
            //        means[k]=
            //        kmeansFilter->AddClassWithInitialMean( userProvidedInitialMean );
            kmeansFilter->AddClassWithInitialMean(meanGuess[k]);
        }

        if (oneTissue) {
            typedef KMeansFilterType::OutputImageType            TissueType;
//            typename TissueType::Pointer tissue = TissueType::New();
            typename TissueType::Pointer tissue = kmeansFilter->GetOutput();

            typedef itk::ImageDuplicator<TissueType> DuplicatorType;
            DuplicatorType::Pointer clone = DuplicatorType::New();
            clone->SetInputImage(tissue);
            clone->Update();
            typename TissueType::Pointer outLabel = clone->GetModifiableOutput();

            //        tissue->CopyInformation(kmeansFilter->GetOutput());
//            tissue->SetBufferedRegion(kmeansFilter->GetOutput()->GetBufferedRegion());
//            tissue->SetRequestedRegion(kmeansFilter->GetOutput()->GetRequestedRegion());
//            tissue->Allocate();
            typedef itk::ImageRegionConstIterator<TissueType> ConstIteratorType;
            typedef itk::ImageRegionIterator<TissueType>       IteratorType;
            ConstIteratorType allTissues(kmeansFilter->GetOutput(), kmeansFilter->GetOutput()->GetBufferedRegion());
            IteratorType oneTissue(outLabel, outLabel->GetBufferedRegion());
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

            typedef itk::ImageFileWriter<KMeansFilterType::OutputImageType> WriterType;
            typename WriterType::Pointer writer = WriterType::New();
            writer->SetFileName( outputLabel.c_str() );
            writer->SetInput( outLabel);
            writer->SetUseCompression(1);
            writer->Update();
            return EXIT_SUCCESS;
        }else{
            //        Take all the tissues segmented
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

        typedef itk::VectorImage< InputPixelType, 3 > VectorInputImageType;
        typedef itk::BayesianClassifierImageFilter< VectorInputImageType,LabelType, PosteriorType,PriorType >   ClassifierFilterType;
        typename ClassifierFilterType::Pointer bayesClassifier = ClassifierFilterType::New();

        bayesClassifier->SetInput( bayesianInitializer->GetOutput() );

        if (oneTissue) {
            typedef ClassifierFilterType::OutputImageType            TissueType;
            typename TissueType::Pointer tissue = TissueType::New();
            //        tissue->CopyInformation(bayesClassifier->GetOutput());
            tissue->SetBufferedRegion(bayesClassifier->GetOutput()->GetBufferedRegion());
            tissue->SetRequestedRegion(bayesClassifier->GetOutput()->GetRequestedRegion());
            tissue->Allocate();
            //        typedef itk::ImageRegionConstIterator<TissueType> ConstIteratorType;
            //        typedef itk::ImageRegionIterator<TissueType>       IteratorType;
            //        ConstIteratorType allTissues(bayesClassifier->GetOutput(), bayesClassifier->GetOutput()->GetBufferedRegion());
            //        IteratorType oneTissue(tissue, tissue->GetBufferedRegion());
            //        unsigned int tissueType;
            //        if (typeTissue == "White Matter") {
            //            tissueType = 3;
            //        }else if (typeTissue == "Gray Matter") {
            //            tissueType = 2;
            //        }else if (typeTissue == "CSF") {
            //            tissueType = 1;
            //        }

            //        allTissues.GoToBegin();
            //        oneTissue.GoToBegin();
            //        while (!allTissues.IsAtEnd()) {
            //            if (allTissues.Get()==tissueType) {
            //                oneTissue.Set(allTissues.Get());
            //            }else{
            //            oneTissue.Set(0);
            //            }
            //            ++allTissues;
            //            ++oneTissue;
            //        }

            //            typedef itk::ImageFileWriter<ClassifierFilterType::OutputImageType> WriterType;
            //            typename WriterType::Pointer writer = WriterType::New();
            //            writer->SetFileName( outputLabel.c_str() );
            //            writer->SetInput( bayesClassifier->GetOutput() );
            //            writer->SetUseCompression(1);
            //            writer->Update();
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
