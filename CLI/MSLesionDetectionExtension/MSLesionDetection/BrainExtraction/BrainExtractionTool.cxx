#include "BrainExtractionTool.h"

using namespace std;

/*
 * Class constructor
 */

const string fslBET = "fsl5.0-bet ";
const string fsWatershed = "/usr/local/freesurfer/bin/mri_watershed ";

BrainExtraction::BrainExtraction(const char* folderPath){
    this->f = 0.1;
    this->g = 0.0;
    this->factorB = true;
    this->folderPath<<folderPath;
}

void BrainExtraction::setSkullStripMethod(string method)
{
    this->skullStripMethod=method;
}

void BrainExtraction::setF(double valueF)
{
    this->f=valueF;
}

void BrainExtraction::setG(double valueG)
{
    this->g=valueG;
}

void BrainExtraction::useOptionB(bool choice)
{
    this->factorB=choice;
}

string BrainExtraction::getSkullStripMethod()
{
    return this->skullStripMethod;
}

double BrainExtraction::getF()
{
    return this->f;
}

double BrainExtraction::getG()
{
    return this->g;
}

bool BrainExtraction::getOptionB()
{
    return this->factorB;
}

void BrainExtraction::setPreFlooding(int preFlooding)
{
    this->preFlooding=preFlooding;
}

void BrainExtraction::runBET(ImageModality modality)
{
    switch (modality) {
    case T1:
        betT1();
        break;
    case T1GD:
        betT1GD();
        break;
    case FLAIR:
        betFLAIR();
        break;
    case FA:
        betFA();
        break;
    }
}


void BrainExtraction::betT1()
{
    //T1 - Brain extraction
    stringstream cmdBetT1;
    if(this->factorB){
        cmdBetT1<<fslBET
               <<this->folderPath.str().c_str()<<"/t1Volume.nii.gz "
              <<this->folderPath.str().c_str()<<"/t1Volume_brain.nii.gz -B -f "<<this->f<<" -g "<<this->g;
    }else{
        cmdBetT1<<fslBET
               <<this->folderPath.str().c_str()<<"/t1Volume.nii.gz "
              <<this->folderPath.str().c_str()<<"/t1Volume_brain.nii.gz -f "<<this->f<<" -g "<<this->g;
    }

//    cout<<cmdBetT1.str().c_str();
        system(cmdBetT1.str().c_str());
}

void BrainExtraction::betT1GD()
{
    //T1Gd - Brain extraction
    stringstream cmdBetT1Gd;
    if(this->factorB){
        cmdBetT1Gd<<fslBET
                 <<this->folderPath.str().c_str()<<"/t1GdVolume.nii.gz "
                <<this->folderPath.str().c_str()<<"/t1GdVolume_brain.nii.gz -B -f "<<this->f<<" -g "<<this->g;
    }else{
        cmdBetT1Gd<<fslBET
                 <<this->folderPath.str().c_str()<<"/t1GdVolume.nii.gz "
                <<this->folderPath.str().c_str()<<"/t1GdVolume_brain.nii.gz -f "<<this->f<<" -g "<<this->g;
    }

//    cout<<cmdBetT1Gd.str().c_str();
        system(cmdBetT1Gd.str().c_str());
}

void BrainExtraction::betFLAIR()
{
    //T2-FLAIR - Brain extraction
    stringstream cmdBetFLAIR;
    if(this->factorB){
        cmdBetFLAIR<<fslBET
                  <<this->folderPath.str().c_str()<<"/FLAIRVolume.nii.gz "
                 <<this->folderPath.str().c_str()<<"/FLAIRVolume_brain.nii -B -f "<<this->f<<" -g "<<this->g;
    }else{
        cmdBetFLAIR<<fslBET
                  <<this->folderPath.str().c_str()<<"/FLAIRVolume.nii.gz "
                 <<this->folderPath.str().c_str()<<"/FLAIRVolume_brain.nii -f "<<this->f<<" -g "<<this->g;
    }

//    cout<<cmdBetFLAIR.str().c_str();
        system(cmdBetFLAIR.str().c_str());
}

void BrainExtraction::betFA()
{
    //FA - Brain extraction
    stringstream cmdBetFA;
    if(this->factorB){
        cmdBetFA<<fslBET
               <<this->folderPath.str().c_str()<<"/FAVolume.nii.gz "
              <<this->folderPath.str().c_str()<<"/FAVolume_brain.nii.gz -B -f "<<this->f<<" -g "<<this->g;
    }else{
        cmdBetFA<<fslBET
               <<this->folderPath.str().c_str()<<"/FAVolume.nii.gz "
              <<this->folderPath.str().c_str()<<"/FAVolume_brain.nii.gz -f "<<this->f<<" -g "<<this->g;
    }

//    cout<<cmdBetFA.str().c_str();
    system(cmdBetFA.str().c_str());
}

void BrainExtraction::runFsWatershed(BrainExtraction::ImageModality modality)
{
    switch (modality) {
    case T1:
        fsT1();
        break;
    case T1GD:fsT1GD();
        break;
    case FLAIR:
        fsFLAIR();
        break;
    case FA:
        fsFA();
        break;
    }
}

void BrainExtraction::fsT1()
{
    //T1 - Brain extraction
    stringstream cmdFsT1;
        cmdFsT1<<fsWatershed
               <<" -h "<<this->preFlooding<<" "
               <<this->folderPath.str().c_str()<<"/t1Volume.nii.gz "
              <<this->folderPath.str().c_str()<<"/t1Volume_brain.nii.gz";

//    cout<<cmdFsT1.str().c_str();
            system(cmdFsT1.str().c_str());
}

void BrainExtraction::fsT1GD()
{
    //T1-Gd - Brain extraction
    stringstream cmdFsT1Gd;
        cmdFsT1Gd<<fsWatershed
               <<" -h "<<this->preFlooding<<" "
               <<this->folderPath.str().c_str()<<"/t1GdVolume.nii.gz "
              <<this->folderPath.str().c_str()<<"/t1GdVolume_brain.nii.gz";

//    cout<<cmdFsT1Gd.str().c_str();
            system(cmdFsT1Gd.str().c_str());
}

void BrainExtraction::fsFLAIR()
{
    //FLAIR - Brain extraction
    stringstream cmdFsFLAIR;
        cmdFsFLAIR<<fsWatershed
               <<" -h "<<this->preFlooding<<" "
               <<this->folderPath.str().c_str()<<"/FLAIRVolume.nii.gz "
              <<this->folderPath.str().c_str()<<"/FLAIRVolume_brain.nii.gz";

//    cout<<cmdFsFLAIR.str().c_str();
            system(cmdFsFLAIR.str().c_str());
}

void BrainExtraction::fsFA()
{
    //DTI-FA - Brain extraction
    stringstream cmdFsFA;
        cmdFsFA<<fsWatershed
               <<" -h "<<this->preFlooding<<" "
               <<this->folderPath.str().c_str()<<"/FAVolume.nii.gz "
              <<this->folderPath.str().c_str()<<"/FAVolume_brain.nii.gz";

//    cout<<cmdFsFA.str().c_str();
            system(cmdFsFA.str().c_str());
}
