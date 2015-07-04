#ifndef __BrainExtraction_h
#define __BrainExtraction_h

#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkImage.h"

using namespace std;

/*
 * Class for brain extraction.
 * It is used here both FSL-BET and Freesurfer brain extraction tools.
 * Choose the more appropriated method to perform the brain extraction in your data.
 * TIPS: Usually, Freesurfer have a more robustic response over T1-Gd weighted images and for non-axial aquisiction orientation.
 */

class BrainExtraction
{
private:
    string skullStripMethod; //Use FSL or Freesurfer
    double f; //fractional level
    double g; //gradient intensity
    int preFlooding; // FS parameter: Preflooding parameters indicates how strong is the watershed method (0-100%). Default is 25%.
    bool factorB; //use the -B option in FSL-BET
    void betT1();
    void betT1GD();
    void betFLAIR();
    void betFA();
    void fsT1();
    void fsT1GD();
    void fsFLAIR();
    void fsFA();
    stringstream folderPath;

public:
    BrainExtraction(const char *folderPath);
    void setSkullStripMethod(string method);
    void setF(double valueF);
    void setG(double valueG);
    void useOptionB(bool choice);
    void setPreFlooding(int preFlooding);
    string getSkullStripMethod();
    double getF();
    double getG();
    bool getOptionB();
    int getPreFlooding();
    enum ImageModality{
        T1,
        T1GD,
        FLAIR,
        FA
    };
    void runBET(ImageModality modality);
    void runFsWatershed(ImageModality modality);

};

#endif

