/* Include ITK libraries for Reading DICOM image series */
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkImage.h"
#include "itkGDCMImageIO.h"

#include "itksys/SystemTools.hxx"

// Include ITK Libraries for image preprocessing
#include "itkRescaleIntensityImageFilter.h"
#include "itkCastImageFilter.h"
#include "itkVersorRigid3DTransform.h"
#include "itkCenteredTransformInitializer.h"
#include "itkResampleImageFilter.h"
#include "itkLinearInterpolateImageFunction.h"
#include "itkShiftScaleImageFilter.h"

// library needed to easily convert int to std::string
#include <sstream>
#include <iostream>
#include <vector>
#include <fstream>
#include <string>