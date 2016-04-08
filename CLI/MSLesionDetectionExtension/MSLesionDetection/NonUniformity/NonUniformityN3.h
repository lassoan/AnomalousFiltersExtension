#ifndef __NonUniformityN3_h
#define __NonUniformityN3_h

//Basic ITK Classes
#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkImageIOBase.h"

using namespace std;
using namespace itk;


/*
 * Class for Non-parametric Non-uniform intensity Normalisation (N3) correction process.
 * At the moment, all the algorithm steps are performed by FreeSurfer-MNI pipeline (mri_nu_correction)
 */
class NonUniformityN3
{
private:
    bool useNUT1;
    bool useNUT1GD;
    bool useNUFLAIR;
    bool useNUFA;
    stringstream folderPath;
public:
    NonUniformityN3(const char *folderPath);
    enum imageModality{
        T1,
        T1Gd,
        FLAIR,
        FA
    };    
    void setUseNUT1(bool useNUT1);
    void setUseNUT1GD(bool useNUT1GD);
    void setUseNUFLAIR(bool useNUFLAIR);
    void setUseNUFA(bool useNUFA);
    bool getUseNUT1();
    bool getUseNUT1GD();
    bool getUseNUFLAIR();
    bool getUseNUFA();
    void startNonUniformity(imageModality imageModality);
};

#endif
