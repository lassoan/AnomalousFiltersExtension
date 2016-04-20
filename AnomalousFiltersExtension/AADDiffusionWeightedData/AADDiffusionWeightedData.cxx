#include "AnisotropicAnomalousDiffusionImageFilter.h"

#include "itkPluginUtilities.h"

#include <itkMetaDataObject.h>
#include <itkImageFileWriter.h>
#include <itkNrrdImageIO.h>
#include <itkCastImageFilter.h>
#if ITK_VERSION_MAJOR < 4
#include "itkCompose3DCovariantVectorImageFilter.h"
#else
#include "itkComposeImageFilter.h"
#endif

#include <itkMinimumMaximumImageCalculator.h>
#include <itkRescaleIntensityImageFilter.h>
#include <itkDiffusionTensor3DReconstructionImageFilter.h>
#include <itkTensorFractionalAnisotropyImageFilter.h>
#include <itkTensorRelativeAnisotropyImageFilter.h>

#include "itkVectorIndexSelectionCastImageFilter.h"

#include "AADDiffusionWeightedDataCLP.h"

#define DIMENSION 3

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
    typedef float                                           PixelType;
    typedef itk::VectorImage<PixelType, Dimension>          VectorImageType;
    typedef itk::Image< PixelType, Dimension >              ScalarImageType;
    typedef itk::RescaleIntensityImageFilter<ScalarImageType>            RescalerInputFilterType;
    itk::ImageFileReader<VectorImageType>::Pointer reader = itk::ImageFileReader<VectorImageType>::New();
    VectorImageType::Pointer img;
    typedef itk::AnisotropicAnomalousDiffusionImageFilter<ScalarImageType, ScalarImageType> AADFilterType;
    // Set the properties for NrrdReader
    reader->SetFileName( inputVolume.c_str());
    // Read in the nrrd data. The file contains the reference image and the gradient
    // images.
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
    // Here we instantiate the DiffusionTensor3DReconstructionImageFilter class.
    // The class is templated over the pixel types of the reference, gradient
    // and the to be created tensor pixel's precision. (We use double here). It
    // takes as input the Reference (B0 image aquired in the absence of diffusion
    // sensitizing gradients), 'n' Gradient images and their directions and produces
    // as output an image of tensors with pixel-type DiffusionTensor3D.
    //
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

    std::vector< ScalarImageType::Pointer > imageContainer;
    // iterator to iterate over the DWI Vector image just read in.
    typedef itk::ImageRegionConstIterator< VectorImageType >         DWIIteratorType;
    DWIIteratorType dwiit( img, img->GetBufferedRegion() );
    typedef itk::ImageRegionIterator< ScalarImageType > IteratorType;



#if ITK_VERSION_MAJOR < 4
    typedef itk::Compose3DCovariantVectorImageFilter<ScalarImageType,
            VectorImageType> ScalarToVectorFilterType;
#else
    typedef itk::ComposeImageFilter<ScalarImageType,
            VectorImageType> ScalarToVectorFilterType;
#endif
    typename ScalarToVectorFilterType::Pointer scalarToVectorImageFilter = ScalarToVectorFilterType::New();
    typedef itk::VectorIndexSelectionCastImageFilter<VectorImageType, ScalarImageType> VectorToScalarType;
    typename VectorToScalarType::Pointer vector2scalar = VectorToScalarType::New();
    vector2scalar->SetInput(img);
    for( unsigned int i = 0; i<numberOfImages; i++ )
    {
//        ScalarImageType::Pointer image = ScalarImageType::New();
//        image->CopyInformation( img );
//        image->SetBufferedRegion( img->GetBufferedRegion() );
//        image->SetRequestedRegion( img->GetRequestedRegion() );
//        image->Allocate();
//        IteratorType it( image, image->GetBufferedRegion() );
//        dwiit.GoToBegin();
//        it.GoToBegin();
//        while (!it.IsAtEnd())
//        {
//            it.Set(dwiit.Get()[i]);
//            ++it;
//            ++dwiit;
//        }
//        imageContainer.push_back( image );
        vector2scalar->SetIndex(i);
        scalarToVectorImageFilter->SetInput(i, vector2scalar->GetOutput());
    }


    for(unsigned int countImage=0; countImage<numberOfImages; countImage++){
//        typename RescalerInputFilterType::Pointer input_rescaler = RescalerInputFilterType::New();
//        input_rescaler->SetInput( imageContainer[countImage] );
//        input_rescaler->SetOutputMaximum(255);
//        input_rescaler->SetOutputMinimum(0);
//        typename AADFilterType::Pointer filter = AADFilterType::New();
//        filter->SetInput( input_rescaler->GetOutput() );
//        filter->SetIterations(iterations);
//        filter->SetCondutance(condutance);
//        filter->SetTimeStep(timeStep);
//        filter->SetQ(q);
//        filter->Update();

//        typedef itk::MinimumMaximumImageCalculator<ScalarImageType> MinMaxCalcType;
//        typename MinMaxCalcType::Pointer imgValues = MinMaxCalcType::New();
//        imgValues->SetImage(imageContainer[countImage] );
//        imgValues->Compute();

//        typename RescalerInputFilterType::Pointer output_rescaler = RescalerInputFilterType::New();
//        output_rescaler->SetInput(filter->GetOutput());
//        output_rescaler->SetOutputMinimum(imgValues->GetMinimum());
//        output_rescaler->SetOutputMaximum(imgValues->GetMaximum());
//        scalarToVectorImageFilter->SetInput(countImage, imageContainer[countImage]);
    }

    scalarToVectorImageFilter->Update();
    VectorImageType::Pointer diffusionImage = scalarToVectorImageFilter->GetOutput();

    // let's write it out
    typedef itk::NrrdImageIO NrrdImageType;
    typename NrrdImageType::Pointer io = NrrdImageType::New();

    itk::MetaDataDictionary metaDataDictionary;
    metaDataDictionary = reader->GetMetaDataDictionary();

    io->SetFileTypeToBinary();
    io->SetMetaDataDictionary( metaDataDictionary );

    typedef itk::ImageFileWriter<VectorImageType> WriterType;
    typename WriterType::Pointer nrrdWriter = WriterType::New();
    nrrdWriter->UseInputMetaDataDictionaryOff();
    nrrdWriter->SetInput( diffusionImage );
    nrrdWriter->SetImageIO(io);
    nrrdWriter->SetFileName( outputVolume.c_str() );
    nrrdWriter->UseCompressionOn();
    try
    {
        nrrdWriter->Update();
    }
    catch( itk::ExceptionObject e )
    {
        std::cerr << argv[0] << ": exception caught !" << std::endl;
        std::cerr << e << std::endl;
        return EXIT_FAILURE;
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
