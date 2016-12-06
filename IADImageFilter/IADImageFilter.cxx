/*
   Copyright 2016 Antonio Carlos da Silva Senra Filho

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
 */

#include "itkImageFileWriter.h"

#include "itkIsotropicAnomalousDiffusionImageFilter.h"
#include "itkCastImageFilter.h"

#include "itkPluginUtilities.h"

#include "IADImageFilterCLP.h"

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

    typedef itk::ImageFileReader<InputImageType>  ReaderType;
    typedef itk::ImageFileWriter<OutputImageType> WriterType;
    typedef itk::CastImageFilter<InputImageType, OutputImageType> CastType;

    typedef itk::IsotropicAnomalousDiffusionImageFilter<InputImageType, InputImageType>  FilterType;

    typename ReaderType::Pointer reader = ReaderType::New();
    itk::PluginFilterWatcher watchReader(reader, "Read Volume",CLPProcessInformation);

    reader->SetFileName( inputVolume.c_str() );

    typename FilterType::Pointer filter = FilterType::New();
    itk::PluginFilterWatcher watchFilter(filter, "Isotropic Anomalous Diffusion",CLPProcessInformation);

    filter->SetInput( reader->GetOutput() );
    filter->SetGeneralizedDiffusion( D );
    filter->SetIterations( iterations );
    filter->SetTimeStep( timeStep );
    filter->SetQ( q );

    typename CastType::Pointer cast = CastType::New();
    cast->SetInput( filter->GetOutput() );

    typename WriterType::Pointer writer = WriterType::New();
    itk::PluginFilterWatcher watchWriter(writer, "Write Volume",CLPProcessInformation);

    writer->SetFileName( outputVolume.c_str() );
    writer->SetInput( cast->GetOutput() );
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
