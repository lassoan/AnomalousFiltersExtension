#include "itkImage.h"
#include "itkExtractImageFilter.h"
#include "itkTileImageFilter.h"

#include "itkComposeImageFilter.h"
#include "itkRescaleIntensityImageFilter.h"
#include "itkMinimumMaximumImageCalculator.h"
#include "itkAnisotropicAnomalousDiffusionImageFilter.h"
#include "itkDiffusionEdgeOptimizationImageCalculator.h"

// ITK includes
#include "itkImageFileWriter.h"
#include "itkImageRegionIterator.h"

#include "itkPluginUtilities.h"

#include "AADEPIDataCLP.h"

// Use an anonymous namespace to keep class types and function names
// from colliding when module is used as shared object module.  Every
// thing should be in an anonymous namespace except for the module
// entry point, e.g. main()
//
using namespace std;
namespace
{

template <typename TPixel>
int DoIt( int argc, char * argv[], TPixel )
{
    PARSE_ARGS;

    typedef float InputPixelType;
    const unsigned int Dimension = 4;

    typedef itk::Image<InputPixelType,  Dimension> EPIImageType;
    typename EPIImageType::Pointer img;

    typedef itk::ImageFileReader<EPIImageType>  ReaderType;

    typename ReaderType::Pointer reader = ReaderType::New();

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

    EPIImageType::SizeType size = reader->GetOutput()->GetRequestedRegion().GetSize();
    EPIImageType::RegionType ExtractedRegion;

    unsigned int numberOfImages = size[3];
    cout<<"Input - Number of components: "<<numberOfImages<<endl;

    typedef itk::Image< InputPixelType, Dimension - 1 > ScalarImageType;

    typedef itk::ExtractImageFilter< EPIImageType, ScalarImageType > ExtractorType;
    ExtractorType::Pointer extractVolume = ExtractorType::New();

    extractVolume->SetInput(img);
    ExtractedRegion.SetSize(0, size[0]);
    ExtractedRegion.SetSize(1, size[1]);
    ExtractedRegion.SetSize(2, size[2]);
    ExtractedRegion.SetSize(3, 0);

    ExtractedRegion.SetIndex(0, 0);
    ExtractedRegion.SetIndex(1, 0);
    ExtractedRegion.SetIndex(2, 0);


    typedef itk::TileImageFilter< ScalarImageType, EPIImageType > TilerType;
    TilerType::Pointer tiler = TilerType::New();

    itk::FixedArray< unsigned int, Dimension > layout;

    layout[0] = 1;
    layout[1] = 1;
    layout[2] = 1;
    layout[3] = 0;

    tiler->SetLayout( layout );

    //Apply the Anisotropic Anomalous Filter on each volume
    typedef itk::AnisotropicAnomalousDiffusionImageFilter<ScalarImageType,ScalarImageType> FilterType;

    typedef itk::RescaleIntensityImageFilter<ScalarImageType> RescalerType;
    typedef itk::MinimumMaximumImageCalculator<ScalarImageType> MinMaxCalcType;
    typename MinMaxCalcType::Pointer imgValues = MinMaxCalcType::New();

    ScalarImageType::Pointer aux;
    for (int vol = 0; vol < numberOfImages; ++vol) {
        ExtractedRegion.SetIndex(3, vol);
        extractVolume->SetExtractionRegion(ExtractedRegion);
        extractVolume->SetDirectionCollapseToIdentity();
        extractVolume->Update();

        typename RescalerType::Pointer rescaler = RescalerType::New();
        typename FilterType::Pointer filter = FilterType::New();
        imgValues->SetImage(extractVolume->GetOutput());
        imgValues->Compute();

        rescaler->SetInput(extractVolume->GetOutput());
        rescaler->SetOutputMaximum(static_cast<InputPixelType>(255));
        rescaler->SetOutputMinimum(static_cast<InputPixelType>(0));
        rescaler->Update();

        std::cout<<"AAD filtering: Volume "<<vol+1<<std::endl;
        filter->SetInput(rescaler->GetOutput());
        filter->SetIterations(iterations);
        filter->SetQ(q);
        if (useAutoConductance) {
            typedef itk::DiffusionEdgeOptimizationImageCalculator<ScalarImageType>   ConductanceOptimizationCalculator;
            typename ConductanceOptimizationCalculator::Pointer optKappa = ConductanceOptimizationCalculator::New();
            optKappa->SetImage(rescaler->GetOutput());
            if (optFunction=="Canny") {
                optKappa->SetOptimizationMethod(ConductanceOptimizationCalculator::CANNY);
            }else if (optFunction=="MAD") {
                optKappa->SetOptimizationMethod(ConductanceOptimizationCalculator::MAD);
            }else if (optFunction=="Morphological") {
                optKappa->SetOptimizationMethod(ConductanceOptimizationCalculator::MORPHOLOGICAL);
            }
            optKappa->Compute();
            if (q!=1.0) {
                filter->SetConductance(optKappa->GetKappa()*(std::abs(1.0-q))*10.0);
                std::cout<<"Optimum kappa value estimate: "<<optKappa->GetKappa()*(std::abs(1.0-q))*10.0<<std::endl;
            }else {
                filter->SetConductance(optKappa->GetKappa());
                std::cout<<"Optimum kappa value estimate: "<<optKappa->GetKappa()<<std::endl;
            }
        }else{
            filter->SetConductance(conductance);
        }
        filter->SetTimeStep(timeStep);
        filter->Update();

        rescaler->SetInput(filter->GetOutput());
        rescaler->SetOutputMaximum(imgValues->GetMaximum());
        rescaler->SetOutputMinimum(imgValues->GetMinimum());
        rescaler->Update();

        aux = rescaler->GetOutput();
        aux->DisconnectPipeline();
        tiler->SetInput( vol, aux );
    }

    tiler->Update();

    typedef itk::ImageFileWriter<EPIImageType> WriterType;
    typename WriterType::Pointer writer = WriterType::New();
    writer->SetFileName( outputVolume.c_str() );
    writer->SetInput( tiler->GetOutput() );
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
            return DoIt( argc, argv, static_cast<signed char>(0) );
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
            std::cerr << "Unknown input image pixel component type: ";
            std::cerr << itk::ImageIOBase::GetComponentTypeAsString( componentType );
            std::cerr << std::endl;
            return EXIT_FAILURE;
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
