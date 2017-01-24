
#include "itkPluginUtilities.h"

#include <itkMetaDataObject.h>
#include "itkComposeImageFilter.h"
#include <itkMinimumMaximumImageCalculator.h>
#include <itkRescaleIntensityImageFilter.h>
#include "itkCastImageFilter.h"
#include "itkAnisotropicAnomalousDiffusionImageFilter.h"
#include "itkVectorIndexSelectionCastImageFilter.h"

#include "AADDiffusionWeightedDataCLP.h"

// CLI includes
#include "itkPluginUtilities.h"

// ITK includes
#include <itkCastImageFilter.h>

#include <iostream>

#include "itkDiffusionTensor3DReconstructionImageFilter.h"
#include "itkNrrdImageIO.h"
#include "itkImageSeriesReader.h"
#include "itkImageFileWriter.h"
#include "itkImageRegionIterator.h"
#include <iostream>

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

    const unsigned int Dimension = 3;
    unsigned int numberOfImages = 0;
    unsigned int numberOfGradientImages = 0;
    bool readb0 = false;
    double b0 = 0;
    typedef float                      PixelType;
    typedef itk::VectorImage<PixelType, Dimension> ImageType;
    itk::ImageFileReader<ImageType>::Pointer reader
            = itk::ImageFileReader<ImageType>::New();
    ImageType::Pointer img;

    reader->SetFileName(inputVolume.c_str());
    try
    {
        reader->Update();
        img = reader->GetOutput();
    }
    catch (itk::ExceptionObject &ex)
    {
        std::cout << ex << std::endl;
        return EXIT_FAILURE;
    }

    typedef itk::DiffusionTensor3DReconstructionImageFilter<
            PixelType, PixelType, double > TensorReconstructionImageFilterType;
    // -------------------------------------------------------------------------
    // Parse the Nrrd headers to get the B value and the gradient directions used
    // for diffusion weighting.
    //
    // The Nrrd headers should look like :
    // The tags specify the B value and the gradient directions. If gradient
    // directions are (0,0,0), it indicates that it is a reference image.
    //
    // DWMRI_b-value:=800
    // DWMRI_gradient_0000:= 0 0 0
    // DWMRI_gradient_0001:=-1.000000       0.000000        0.000000
    // DWMRI_gradient_0002:=-0.166000       0.986000        0.000000
    // DWMRI_gradient_0003:=0.110000        0.664000        0.740000
    // ...
    //
    itk::MetaDataDictionary imgMetaDictionary = img->GetMetaDataDictionary();
    std::vector<std::string> imgMetaKeys = imgMetaDictionary.GetKeys();
    std::vector<std::string>::const_iterator itKey = imgMetaKeys.begin();
    std::string metaString;
    TensorReconstructionImageFilterType::GradientDirectionType vect3d;
    TensorReconstructionImageFilterType::GradientDirectionContainerType::Pointer
            DiffusionVectors =
            TensorReconstructionImageFilterType::GradientDirectionContainerType::New();
    for (; itKey != imgMetaKeys.end(); ++itKey)
    {
        double x,y,z;
        itk::ExposeMetaData<std::string> (imgMetaDictionary, *itKey, metaString);
        if (itKey->find("DWMRI_gradient") != std::string::npos)
        {
            std::cout << *itKey << " ---> " << metaString << std::endl;
            sscanf(metaString.c_str(), "%lf %lf %lf\n", &x, &y, &z);
            vect3d[0] = x; vect3d[1] = y; vect3d[2] = z;
            DiffusionVectors->InsertElement( numberOfImages, vect3d );
            ++numberOfImages;
            // If the direction is 0.0, this is a reference image
            if (vect3d[0] == 0.0 &&
                    vect3d[1] == 0.0 &&
                    vect3d[2] == 0.0)
            {
                continue;
            }
            ++numberOfGradientImages;
        }
        else if (itKey->find("DWMRI_b-value") != std::string::npos)
        {
            std::cout << *itKey << " ---> " << metaString << std::endl;
            readb0 = true;
            b0 = atof(metaString.c_str());
        }
    }
    std::cout << "Number of gradient images: "
              << numberOfGradientImages
              << " and Number of reference images: "
              << numberOfImages - numberOfGradientImages
              << std::endl;
    if(!readb0)
    {
        std::cerr << "BValue not specified in header file" << std::endl;
        return EXIT_FAILURE;
    }

    typedef itk::Image< PixelType, Dimension > ReferenceImageType;
    typedef ReferenceImageType                 GradientImageType;

    std::vector< GradientImageType::Pointer > imageContainer;

    typedef itk::ImageRegionConstIterator< ImageType >         DWIIteratorType;
    DWIIteratorType dwiit( img, img->GetBufferedRegion() );
    typedef itk::ImageRegionIterator< GradientImageType > IteratorType;

    for( unsigned int i = 0; i<numberOfImages; i++ )
    {
        GradientImageType::Pointer image = GradientImageType::New();
        image->CopyInformation( img );
        image->SetBufferedRegion( img->GetBufferedRegion() );
        image->SetRequestedRegion( img->GetRequestedRegion() );
        image->Allocate();
        IteratorType it( image, image->GetBufferedRegion() );
        dwiit.GoToBegin();
        it.GoToBegin();
        while (!it.IsAtEnd())
        {
            it.Set(dwiit.Get()[i]);
            ++it;
            ++dwiit;
        }
        imageContainer.push_back( image );
    }

    typedef itk::ComposeImageFilter<GradientImageType,ImageType> ScalarToVectorFilterType;
    typename ScalarToVectorFilterType::Pointer scalar2vector = ScalarToVectorFilterType::New();

    typedef itk::AnisotropicAnomalousDiffusionImageFilter<GradientImageType,GradientImageType> FilterType;

    typedef    T OutputPixelType;

//    typedef itk::Image<float,  3> InputImageType;
//    typedef itk::Image<OutputPixelType, 3> OutputImageType;

//    typedef itk::CastImageFilter<InputImageType, OutputImageType>       CastInput2OutputType;
    typedef itk::RescaleIntensityImageFilter<GradientImageType> RescalerType;
    typedef itk::MinimumMaximumImageCalculator<GradientImageType> MinMaxCalcType;
    typename MinMaxCalcType::Pointer imgValues = MinMaxCalcType::New();

    for (int vol = 0; vol < numberOfImages; ++vol) {
        typename RescalerType::Pointer rescaler = RescalerType::New();
        typename FilterType::Pointer filter = FilterType::New();
        imgValues->SetImage(imageContainer[vol]);
        imgValues->Compute();
        rescaler->SetInput(imageContainer[vol]);
        rescaler->SetOutputMaximum(static_cast<PixelType>(255));
        rescaler->SetOutputMinimum(static_cast<PixelType>(0));
        rescaler->Update();

        std::cout<<"filtering volume "<<vol<<std::endl;
        filter->SetInput(rescaler->GetOutput());
        filter->SetIterations(iterations);
        filter->SetQ(q);
        filter->SetCondutance(condutance);
        filter->SetTimeStep(timeStep);
        filter->Update();

        rescaler->SetInput(filter->GetOutput());
        rescaler->SetOutputMaximum(imgValues->GetMaximum());
        rescaler->SetOutputMinimum(imgValues->GetMinimum());
        rescaler->Update();

        scalar2vector->SetInput(vol, rescaler->GetOutput());
    }

    scalar2vector->Update();
    typename itk::NrrdImageIO::Pointer io = itk::NrrdImageIO::New();

    io->SetFileTypeToBinary();
    io->SetMetaDataDictionary( imgMetaDictionary );

    typedef itk::ImageFileWriter<ImageType> WriterType;
    typename WriterType::Pointer nrrdWriter = WriterType::New();
    nrrdWriter->UseInputMetaDataDictionaryOff();
    nrrdWriter->SetImageIO(io);
    nrrdWriter->SetInput( scalar2vector->GetOutput() );
    nrrdWriter->SetFileName( outputVolume.c_str() );

    nrrdWriter->Update();

    std::cout << "success = " << EXIT_SUCCESS << std::endl;

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
