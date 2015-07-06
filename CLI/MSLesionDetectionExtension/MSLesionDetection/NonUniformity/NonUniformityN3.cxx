#include "NonUniformityN3.h"


NonUniformityN3::NonUniformityN3(const char *folderPath)
{
    this->folderPath<<folderPath;
}

void NonUniformityN3::setUseNUT1(bool useNUT1)
{
    this->useNUT1=useNUT1;
}

void NonUniformityN3::setUseNUT1GD(bool useNUT1GD)
{
    this->useNUT1GD=useNUT1GD;
}

void NonUniformityN3::setUseNUFLAIR(bool useNUFLAIR)
{
    this->useNUFLAIR=useNUFLAIR;
}

void NonUniformityN3::setUseNUFA(bool useNUFA)
{
    this->useNUFA=useNUFA;
}

bool NonUniformityN3::getUseNUT1()
{
    return this->useNUT1;
}

bool NonUniformityN3::getUseNUT1GD()
{
    return this->useNUT1GD;
}

bool NonUniformityN3::getUseNUFLAIR()
{
    return this->useNUFLAIR;
}

bool NonUniformityN3::getUseNUFA()
{
    return this->useNUFA;
}


void NonUniformityN3::startNonUniformity(NonUniformityN3::imageModality imageModality)
{
    stringstream imagePath, outputFileName, FsNU;

    FsNU<<"mri_nu_correct.mni ";

    switch(imageModality){
    case T1:
        imagePath<<folderPath.str().c_str()<<"/t1Volume_brain_corr.nii.gz";
        outputFileName<<folderPath.str().c_str()<<"/t1Volume_brain_corr_nu.nii.gz";


        FsNU<<" --i "<<imagePath.str().c_str()
           <<" "
          <<" --o "<<outputFileName.str().c_str();

//        cout<<FsNU.str().c_str();
                system(FsNU.str().c_str());
        break;
    case T1Gd:
        imagePath<<folderPath.str().c_str()<<"/t1GdVolume_brain_corr.nii.gz";
        outputFileName<<folderPath.str().c_str()<<"/t1GdVolume_brain_corr_nu.nii.gz";


        FsNU<<" --i "<<imagePath.str().c_str()
           <<" "
          <<" --o "<<outputFileName.str().c_str();

//        cout<<FsNU.str().c_str();
                system(FsNU.str().c_str());
        break;
    case FLAIR:
        imagePath<<folderPath.str().c_str()<<"/FLAIRVolume_brain_corr.nii.gz";
        outputFileName<<folderPath.str().c_str()<<"/FLAIRVolume_brain_corr_nu.nii.gz";


        FsNU<<" --i "<<imagePath.str().c_str()
           <<" "
          <<" --o "<<outputFileName.str().c_str();

//        cout<<FsNU.str().c_str();
                system(FsNU.str().c_str());
        break;
    case FA:
        imagePath<<folderPath.str().c_str()<<"/FAVolume_brain_corr.nii.gz";
        outputFileName<<folderPath.str().c_str()<<"/FAVolume_brain_corr_nu.nii.gz";


        FsNU<<" --i "<<imagePath.str().c_str()
           <<" "
          <<" --o "<<outputFileName.str().c_str();

//        cout<<FsNU.str().c_str();
                system(FsNU.str().c_str());
        break;
    }
}
