#ifndef __BrainExtraction_h
#define __BrainExtraction_h

#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkImage.h"

using namespace std;

/*
 * Class for brain extraction.
 *
 */

class BrainExtraction{
private:
    double f; //fractional level
    double g; //gradient intensity
    bool factorB; //use the -B option in FSL-BET
    void betT1();
    void betT1GD();
    void betFLAIR();
    void betFA();
    stringstream folderPath;

public:
    BrainExtraction(const char *folderPath);
    void setF(double valueF);
    void setG(double valueG);
    void isOptionB(bool choice);
    double getF();
    double getG();
    bool getOptionB();
    enum ImageModality{
        T1,
        T1GD,
        FLAIR,
        FA
    };
    void runBET(ImageModality modality);

};

#endif

