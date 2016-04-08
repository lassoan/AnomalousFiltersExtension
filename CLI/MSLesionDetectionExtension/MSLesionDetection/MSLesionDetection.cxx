#include "itkImageFileWriter.h"

//Brain extraction pipeline
#include "BrainExtraction/BrainExtractionTool.h"

//Filtering Methods
#include "AnisotropicAnomalousDiffusionImageFilter.h"

//Registering Methods
#include "Registration/MultimodalityRegistration.h"

//Non Uniformity Methods
#include "NonUniformity/NonUniformityN3.h"

//Segmentation Methods


//Utils
#include "itkCastImageFilter.h"

#include "itkPluginUtilities.h"

#include "MSLesionDetectionCLP.h"

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

    typedef float InputPixelType;
    typedef T OutputPixelType;
    const unsigned int Dimension = 3;

    typedef itk::Image<InputPixelType,  Dimension> InputImageType;
    typedef itk::Image<OutputPixelType, Dimension> OutputImageType;
    typedef itk::CastImageFilter<InputImageType, OutputImageType> CastType;

    typedef itk::ImageFileReader<InputImageType>  ReaderType;
    typedef itk::ImageFileWriter<OutputImageType> WriterType;

    //Read the input volumes
    typename ReaderType::Pointer readerT1 = ReaderType::New();
    typename ReaderType::Pointer readerT1Gd = ReaderType::New();
    typename ReaderType::Pointer readerFLAIR = ReaderType::New();
    typename ReaderType::Pointer readerFA = ReaderType::New();
    typename ReaderType::Pointer readerAtlas = ReaderType::New();
    itk::PluginFilterWatcher watchReader(readerT1, "Read T1 Volume", CLPProcessInformation);
    readerT1->SetFileName( t1Volume.c_str() );
    readerT1Gd->SetFileName( t1GdVolume.c_str() );
    readerFLAIR->SetFileName(FLAIRVolume.c_str());
    readerFA->SetFileName(FAVolume.c_str());
    readerAtlas->SetFileName(fixedVolume.c_str());

    typename CastType::Pointer caster = CastType::New();
    typename WriterType::Pointer writer = WriterType::New();
    //T1
    std::stringstream saveTmpPathT1;
    saveTmpPathT1<<outputFolder.c_str()<<"/t1Volume.nii.gz";
    writer->SetFileName(saveTmpPathT1.str());
    caster->SetInput(readerT1->GetOutput());
    writer->SetInput(caster->GetOutput());
    writer->Update();
    //T1-Gd
    std::stringstream saveTmpPathT1Gd;
    saveTmpPathT1Gd<<outputFolder.c_str()<<"/t1GdVolume.nii.gz";
    writer->SetFileName(saveTmpPathT1Gd.str());
    caster->SetInput(readerT1Gd->GetOutput());
    writer->SetInput(caster->GetOutput());
    writer->Update();
    //T2-FLAIR
    std::stringstream saveTmpPathFLAIR;
    saveTmpPathFLAIR<<outputFolder.c_str()<<"/FLAIRVolume.nii.gz";
    writer->SetFileName(saveTmpPathFLAIR.str());
    caster->SetInput(readerFLAIR->GetOutput());
    writer->SetInput(caster->GetOutput());
    writer->Update();
    //DTI-FA
    std::stringstream saveTmpPathFA;
    saveTmpPathFA<<outputFolder.c_str()<<"/FAVolume.nii.gz";
    writer->SetFileName(saveTmpPathFA.str());
    caster->SetInput(readerFA->GetOutput());
    writer->SetInput(caster->GetOutput());
    writer->Update();
    //Atlas
    std::stringstream saveTmpPathAtlas;
    saveTmpPathAtlas<<outputFolder.c_str()<<"/AtlasVolume.nii.gz";
    writer->SetFileName(saveTmpPathAtlas.str());
    caster->SetInput(readerAtlas->GetOutput());
    writer->SetInput(caster->GetOutput());
    writer->Update();

    //Brain extraction - Freesurfer and FSL
    BrainExtraction brainExtraction(outputFolder.c_str());
    brainExtraction.setF(valueF);
    brainExtraction.setG(valueG);
    brainExtraction.useOptionB(optionB);
    brainExtraction.setPreFlooding(preFlooding);

    //Brain extraction for T1 weighted image
    brainExtraction.setSkullStripMethod(skullStripMethodT1);
    if(brainExtraction.getSkullStripMethod()==string("Fs-watershed")){
        brainExtraction.runFsWatershed(BrainExtraction::T1);
    }else{
        brainExtraction.runBET(BrainExtraction::T1);
    }

    //Brain extraction for T1-Gd weighted image
    brainExtraction.setSkullStripMethod(skullStripMethodT1Gd);
    if(brainExtraction.getSkullStripMethod()==string("Fs-watershed")){
        brainExtraction.runFsWatershed(BrainExtraction::T1GD);
    }else{
        brainExtraction.runBET(BrainExtraction::T1GD);
    }

    //Brain extraction for FLAIR weighted image
    brainExtraction.setSkullStripMethod(skullStripMethodFLAIR);
    if(brainExtraction.getSkullStripMethod()==string("Fs-watershed")){
        brainExtraction.runFsWatershed(BrainExtraction::FLAIR);
    }else{
        brainExtraction.runBET(BrainExtraction::FLAIR);
    }

    //Brain extraction for DTI-FA weighted image
    brainExtraction.setSkullStripMethod(skullStripMethodFA);
    if(brainExtraction.getSkullStripMethod()==string("Fs-watershed")){
        brainExtraction.runFsWatershed(BrainExtraction::FA);
    }else{
        brainExtraction.runBET(BrainExtraction::FA);
    }

    //TODO: mri_nu_correct (bias correction) depois do coregistro!!!
    //Multimodal image registering process
    MultimodalityRegistration registration(outputFolder.c_str());
    registration.setNSamples(nSamples);
    //    registration.setLearnRate(learnRate);
    registration.setNumInteration(nInteration);
    //    registration.setMetricFixedStd(metricFixedStd);
    //    registration.setMetricMovingStd(metricMovingStd);
    registration.startCLIRegistration(MultimodalityRegistration::T1);
    registration.startCLIRegistration(MultimodalityRegistration::T1Gd);
    registration.startCLIRegistration(MultimodalityRegistration::FLAIR);
    registration.startCLIRegistration(MultimodalityRegistration::FA);

    //Non Uniformity process
    NonUniformityN3 nu(outputFolder.c_str());
    nu.setUseNUT1(useNUT1);
    nu.setUseNUT1GD(useNUT1Gd);
    nu.setUseNUFLAIR(useNUFLAIR);
    nu.setUseNUFA(useNUFA);

    if(nu.getUseNUT1()){
        nu.startNonUniformity(NonUniformityN3::T1);
    }
    if(nu.getUseNUT1GD()){
        nu.startNonUniformity(NonUniformityN3::T1Gd);
    }
    if(nu.getUseNUFLAIR()){
        nu.startNonUniformity(NonUniformityN3::FLAIR);
    }
    if(nu.getUseNUFA()){
        nu.startNonUniformity(NonUniformityN3::FA);
    }

    //Image filtering process for all the images
    //  typedef itk::AnisotropicAnomalousDiffusionImageFilter<InputImageType, InputImageType>  FilterType;
    //  typename FilterType::Pointer filter = FilterType::New();
    //        itk::PluginFilterWatcher watchFilter(filter, "AAD filtering on T1 Volume", CLPProcessInformation);
    //TODO: Put status here - filtering T1 image
    //  filter->SetInput( readerT1->GetOutput() );
    //filter->SetQ(q);
    //filter->SetIterations(iterations);
    //filter->SetCondutance(condutance);
    //filter->SetTimeStep(timeStep);s



    //Converting the pixel data to output type
    //typename CastType::Pointer cast = CastType::New();
    //cast->SetInput( filter->GetOutput() );

    //typename WriterType::Pointer writer = WriterType::New();
    //writer->SetFileName( outputVolume.c_str() );
    //writer->SetInput( cast->GetOutput() );
    //writer->SetUseCompression(1);
    //writer->Update();

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
        itk::GetImageType(t1Volume, pixelType, componentType);

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
