#include "itkImageFileWriter.h"

//Brain extraction pipeline
#include "BrainExtraction/BrainExtractionTool.h"

//Filtering Methods
#include "AnisotropicAnomalousDiffusionImageFilter.h"

//Registering Methods
#include "itkAffineTransform.h"

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

  typedef    float InputPixelType;
  typedef    T OutputPixelType;

  typedef itk::Image<InputPixelType,  3> InputImageType;
  typedef itk::Image<OutputPixelType, 3> OutputImageType;
  typedef itk::CastImageFilter<InputImageType, OutputImageType> CastType;

  typedef itk::ImageFileReader<InputImageType>  ReaderType;
  typedef itk::ImageFileWriter<OutputImageType> WriterType;

  //Create the temporary directory for the intermediated images
  std::stringstream cmdMkdir;
  cmdMkdir<<"mkdir "<<outputFolder.c_str();
  system(cmdMkdir.str().c_str());

  //Read the input volumes
  typename ReaderType::Pointer readerT1 = ReaderType::New();
  typename ReaderType::Pointer readerT1Gd = ReaderType::New();
  typename ReaderType::Pointer readerFLAIR = ReaderType::New();
  typename ReaderType::Pointer readerFA = ReaderType::New();
  itk::PluginFilterWatcher watchReader(readerT1, "Read T1 Volume", CLPProcessInformation, .5, 0.0);
  readerT1->SetFileName( t1Volume.c_str() );
  readerT1Gd->SetFileName( t1GdVolume.c_str() );
  readerFLAIR->SetFileName(FLAIRVolume.c_str());
  readerFA->SetFileName(FAVolume.c_str());

  typename CastType::Pointer caster = CastType::New();
  typename WriterType::Pointer writer = WriterType::New();
  //T1
  std::stringstream saveTmpPathT1;
  saveTmpPathT1<<outputFolder.c_str()<<"/t1Volume.nii";
  writer->SetFileName(saveTmpPathT1.str());
  caster->SetInput(readerT1->GetOutput());
  writer->SetInput(caster->GetOutput());
  writer->Update();
  //T1-Gd
  std::stringstream saveTmpPathT1Gd;
  saveTmpPathT1Gd<<outputFolder.c_str()<<"/t1GdVolume.nii";
  writer->SetFileName(saveTmpPathT1Gd.str());
  caster->SetInput(readerT1Gd->GetOutput());
  writer->SetInput(caster->GetOutput());
  writer->Update();
  //T2-FLAIR
  std::stringstream saveTmpPathFLAIR;
  saveTmpPathFLAIR<<outputFolder.c_str()<<"/FLAIRVolume.nii";
  writer->SetFileName(saveTmpPathFLAIR.str());
  caster->SetInput(readerFLAIR->GetOutput());
  writer->SetInput(caster->GetOutput());
  writer->Update();
  //DTI-FA
  std::stringstream saveTmpPathFA;
  saveTmpPathFA<<outputFolder.c_str()<<"/FAVolume.nii";
  writer->SetFileName(saveTmpPathFA.str());
  caster->SetInput(readerFA->GetOutput());
  writer->SetInput(caster->GetOutput());
  writer->Update();

  //Brain extraction - FSL
    BrainExtraction bet(outputFolder.c_str());
    bet.setF(valueF);
    bet.setG(valueG);
    bet.isOptionB(optionB);

    bet.runBET(BrainExtraction::T1);
    bet.runBET(BrainExtraction::T1GD);
    bet.runBET(BrainExtraction::FLAIR);
    bet.runBET(BrainExtraction::FA);

 //Multimodal image registering process


  //Image filtering process for all the images
//  typedef itk::AnisotropicAnomalousDiffusionImageFilter<InputImageType, InputImageType>  FilterType;
//  typename FilterType::Pointer filter = FilterType::New();
//        itk::PluginFilterWatcher watchFilter(filter, "AAD filtering on T1 Volume", CLPProcessInformation);
  //TODO: Put status here - filtering T1 image
//  filter->SetInput( readerT1->GetOutput() );
  //filter->SetQ(q);
  //filter->SetIterations(iterations);
  //filter->SetCondutance(condutance);
  //filter->SetTimeStep(timeStep);



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
