#ifndef __NonUniformityN3_h
#define __NonUniformityN3_h

//Basic ITK Classes
#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkImageIOBase.h"


const unsigned int Dimension = 3;

using namespace std;
using namespace itk;


/*
 * Class for multimodality registration pipeline.
 *
 */
class NonUniformityN3
{
public:
    NonUniformityN3();
    enum imageModality{
        T1,
        T1Gd,
        FLAIR,
        FA
    };    
};

#endif
