#ifndef __DWIEddyCorrect_h
#define __DWIEddyCorrect_h

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

class DWIEddyCorrect
{
private:


public:


};

#endif

