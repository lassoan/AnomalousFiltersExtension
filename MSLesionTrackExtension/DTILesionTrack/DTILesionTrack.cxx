#include "itkImageFileWriter.h"

#include "Segmentation/DTIMapSegmentation.h"

#include "itkPluginUtilities.h"

#include "DTILesionTrackCLP.h"

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

//          typedef    T                  InputPixelType;
////          typedef    T                  OutputPixelType;
//          typedef itk::Image<InputPixelType,  3> InputImageType;
//    //      typedef itk::Image<OutputPixelType, 3> OutputImageType;

//          typedef itk::ImageFileReader<InputImageType>  ReaderType;
//      typedef itk::ImageFileWriter<InputImageType> WriterType;

////    Reading input files
//    typename ReaderType::Pointer readerT1 = ReaderType::New();
//    typename ReaderType::Pointer readerFA = ReaderType::New();
//    readerT1->SetFileName( T1InputVolume.c_str() );
//    readerFA->SetFileName(FAInputVolume.c_str());

////    Inserting input files in output folder
//    typename WriterType::Pointer writer = WriterType::New();
//    //T1
//        std::stringstream savePathT1;
//        savePathT1<<outputFolder.c_str()<<"/t1_volume.nii.gz";
//        writer->SetFileName(savePathT1.str());
//        writer->SetInput(readerT1->GetOutput());
//        writer->Update();

//    //    White Matter Mask
//    DTIMapSegmentation segmentationWM(T1InputVolume.c_str(), outputFolder.c_str());
//    if (doT1Filtering) {
//        if (filterT1Method == "Classic AD") {
//            segmentationWM.runFiltering(DTIMapSegmentation::ClassicAD);
//            segmentationWM.runSegmentation(DTIMapSegmentation::BAYES);
//        }
//    }else{
//        if (segWMMethod == "Bayesian") {
//            segmentationWM.runSegmentation(DTIMapSegmentation::BAYES);
//        }else if (segWMMethod == "MRF") {
//            segmentationWM.runSegmentation(DTIMapSegmentation::MRF);
//        }else if (segWMMethod == "KMeans") {
//            segmentationWM.runSegmentation(DTIMapSegmentation::KMEANS);
//        }
//    }

    //    DTIMapSegmentation segmentationWM(T1InputVolume.c_str());
    //    segmentationWM.setNumIter(nT1Iter);
    //    if (segWMMethod == "Bayesian") {
    //        segmentationWM.runSegmentation(DTIMapSegmentation::BAYES);
    //    }else if (segWMMethod == "MRF") {
    //        segmentationWM.runSegmentation(DTIMapSegmentation::MRF);
    //    }else if (segWMMethod == "KMeans") {
    //        segmentationWM.runSegmentation(DTIMapSegmentation::KMEANS);
    //    }


        DTIMapSegmentation segmentation(FAInputVolume.c_str(),outputLabels.c_str());
        segmentation.setNumClasses(numClasses);
        if (segFAMethod == "Bayesian") {
            segmentation.runSegmentation(DTIMapSegmentation::BAYES);
        }else if (segFAMethod == "MRF") {
            segmentation.runSegmentation(DTIMapSegmentation::MRF);
        }else if (segFAMethod == "KMeans") {
            segmentation.runSegmentation(DTIMapSegmentation::KMEANS);
        }



    //  typedef itk::SmoothingRecursiveGaussianImageFilter<
    //    InputImageType, OutputImageType>  FilterType;

    //  typename ReaderType::Pointer reader = ReaderType::New();

    //  reader->SetFileName( FAInputVolume.c_str() );

    //  typename FilterType::Pointer filter = FilterType::New();
    //  filter->SetInput( reader->GetOutput() );
    //  filter->SetSigma( sigma );

    //  typename WriterType::Pointer writer = WriterType::New();
    //  writer->SetFileName( outputLabelVolume.c_str() );
    //  writer->SetInput( filter->GetOutput() );
    //  writer->SetUseCompression(1);
    //  writer->Update();

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
        itk::GetImageType(FAInputVolume, pixelType, componentType);

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
