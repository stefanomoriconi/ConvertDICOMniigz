	/* This Software is developed using ITK v.4.5
	 * Please refer to official documentation at http://www.itk.org/Doxygen/html/
	 * Author: Stefano Moriconi - CNR IBFM - Milano, Italy - October 31st 2014
	 * Contacts: stefano.nicola.moriconi@gmail.com
	 * 
	 * Please Note: the following software manages the DICOM images with an intrinsic orientation to be corrected according to the orthogonal planes.
	 * This means that, the output Converted Image will have orientation equal to: [1 0 0;0 1 0;0 0 1], after a proper interpolation.
	 * Moreover in that case, a new set of re-oriented DICOM images will be created (within a new subfolder)!
	 * NB: Since an interpolation is required, the more the intrinsic angle, the less the final accuracy!
	 * 
	 * WARNING: There is a known bug with the ITK DICOM SERIES MANAGER when exporting new DICOM images:
	 * The dictionary for DICOM tags might have been changed with the standard updates, and so might have been changed also some default values.
	 * (i.e. DICOM Rescale Intercept / Rescale Slope)
	 * 
	 * To overcome this issue, a Matlab® script is attached to correct the output headers of the re-created DICOM images.
	 * Please note that this is a necessary workaround, in order to let the corrected re-created DICOM images be compatible with multi-platforms systems.
	 */


	#include "ConvertDICOM.h"

	//static void CopyDictionary (itk::MetaDataDictionary &fromDict, itk::MetaDataDictionary &toDict);

	int main( int argc, char* argv[] )
	{
	  // Controllo sul numero degli input
		if( argc < 2 )
		{
		     std::cout << std::endl << "-------------------------------------------------" << std::endl << std::endl;
		     std::cout << "ConvertDICOM is a tool for DICOM images conversion." << std::endl;
		     std::cout << "Parse the desired DICOMimage as input, and specify the name of the output." << std::endl;
		     std::cout << "Output image will have the extension nii.gz ." << std::endl<< std::endl;
		     std::cout << "Usage: " << argv[0] << " DICOMfile " << "OutputImage " << std::endl << std::endl;
		     std::cout << "Example: To convert a DICOM image to 'image.nii.gz' file..." << std::endl;
		     std::cout << "type in the terminal : " << argv[0] << " /home/DICOMfile.dcm " << "/home/DICOMimages/image" << std::endl;
		     std::cout << "Et voilà!" << std::endl << std::endl;
		     std::cout << "WARNING:" << std::endl;
		     std::cout << "This software manages the DICOM images with an intrinsic orientation to be re-oriented according to the orthogonal planes." << std::endl;
		     std::cout << "The output Converted Image will have orientation equal to: [1 0 0;0 1 0;0 0 1], after a proper interpolation." << std::endl;
		     std::cout << "Please Note: the more the intrinsic angle, the less the final accuracy." << std::endl;
		     std::cout << "Possible outputs: OutputImage_OP.nii.gz -- Re-Oriented Image" << std::endl;
		     std::cout << std::endl << "-------------------------------------------------" << std::endl << std::endl;

		return EXIT_FAILURE;
		}

	// Definizione del tipo di Pixel
	typedef float itkDCMPixelType;
	// Definizione della dimensione dell'immagine (3D - x,y,z)
	const unsigned int imageDimension = 3;

	// Definizione del tipo di immagine singola
	typedef itk::Image<itkDCMPixelType,imageDimension> ImageType;
	// Definizione del tipo di immagine 3D come DicomSeries
	typedef itk::ImageFileReader< ImageType > ReaderType;

	ReaderType::Pointer reader = ReaderType::New();

	typedef itk::GDCMImageIO ImageIOType;
	ImageIOType::Pointer dicomIO = ImageIOType::New();

	reader->SetImageIO( dicomIO );

	reader->SetFileName(argv[1]);

	try
	{
	reader->Update();
	}
	catch (itk::ExceptionObject &ex)
	{
	std::cout << ex << std::endl;
	std::cout << "Loading DICOM Image: Fail" << std::endl << std::endl;
	return EXIT_FAILURE;
	}

	// Forse qui ci vuole qualcosa per far capire che ha caricato correttamente
	std::cout << std::endl << "Loading DICOM Image: Complete" << std::endl<<std::endl;

	//std::cout << "-- DICOM Image Data --" << std::endl;
	//std::cout << "Image Size: " << reader->GetOutput()->GetLargestPossibleRegion().GetSize()<< std::endl;
	//std::cout << "Image Origin: " << reader->GetOutput()->GetOrigin() << std::endl;
	//std::cout << "Image Orientation: " << std::endl << reader->GetOutput()->GetDirection() << std::endl << std::endl;

	// Export dell'immagine in formato NIFTI o MHD in base agli input
	if (argc > 2)
	{
	    
	    typedef itk::ImageFileWriter< ImageType > WriterType;
	    WriterType::Pointer writerOriginal = WriterType::New();
	    std::string origARname = argv[2];
	   	std::string origARsuffix = ".nii.gz";
	    writerOriginal->SetFileName( origARname.append(origARsuffix) );
	    writerOriginal->SetInput( reader->GetOutput() );
	   
	    std::cout << "Writing the Image as: " << std::endl;
	    std::cout << argv[2] << std::endl << std::endl;

	    writerOriginal->Write();

	    // Finish writing original image

	    ImageType::DirectionType FinalDirection;
	    FinalDirection.SetIdentity();
	    
		    if (reader->GetOutput()->GetDirection() != FinalDirection)
		    {
			std::cout << "** WARNING: DICOM Image has an orientation different from orthogonal planes! **" << std::endl << std::endl;
			
			typedef itk::ResampleImageFilter<ImageType,ImageType> FilterType;
			FilterType::Pointer AntiRotationFilter = FilterType::New();

			typedef itk::VersorRigid3DTransform< double > TransformType;
			TransformType::Pointer AntiRotationTransform = TransformType::New();
			AntiRotationTransform->SetMatrix(reader->GetOutput()->GetDirection());
		    
			typedef itk::LinearInterpolateImageFunction<ImageType, double > InterpolatorType;
			InterpolatorType::Pointer LinearImageInterpolator = InterpolatorType::New();
			
			AntiRotationFilter->SetTransform(AntiRotationTransform);
			AntiRotationFilter->SetInterpolator( LinearImageInterpolator );
			AntiRotationFilter->SetDefaultPixelValue( 0 );
			AntiRotationFilter->SetOutputSpacing(reader->GetOutput()->GetSpacing());
			
			// Attenzione: va ruotato anche l'origine!
			
			vnl_vector< itk::SpacePrecisionType > AntiRotOrigVals = reader->GetOutput()->GetOrigin().GetVnlVector()*(reader->GetOutput()->GetDirection().GetVnlMatrix().as_matrix());
			
			const double RotatedOrigin[ imageDimension ] = { AntiRotOrigVals[0], AntiRotOrigVals[1], AntiRotOrigVals[2] };
			
			//AntiRotationFilter->SetOutputOrigin(reader->GetOutput()->GetOrigin());
			AntiRotationFilter->SetOutputOrigin(RotatedOrigin);
			
			AntiRotationFilter->SetOutputDirection(FinalDirection);

			AntiRotationFilter->SetSize(reader->GetOutput()->GetLargestPossibleRegion().GetSize());
			
			AntiRotationFilter->SetInput(reader->GetOutput());
			
			//Esportare l'Immagine Antiruotata
			WriterType::Pointer writerAntiRotImage = WriterType::New();
			
			std::string ARname = argv[2];
			std::string ARsuffix = "_OP";
			std::string finalARsuffix = ".nii.gz";
			
			//std::string ARFileName;
			//ARFileName.append(ARname.substr(0,ARname.rfind(".")));
			//ARFileName.append(ARsuffix);
			//ARFileName.append(ARname.substr(ARname.rfind("."),ARname.length()));
			
			ARname.append(ARsuffix);
			ARname.append(finalARsuffix);

			//writerAntiRotImage->SetFileName( ARFileName );
			writerAntiRotImage->SetFileName( ARname );
			writerAntiRotImage->SetInput( AntiRotationFilter->GetOutput() );
			
			//Stampo a video i Valori
			std::cout << "-- Restoring Image Orientation: Complete. " << std::endl << std::endl;
			std::cout << "Restored Image Size: " << reader->GetOutput()->GetLargestPossibleRegion().GetSize()<< std::endl;
			std::cout << "Restored Image Origin: " << AntiRotationFilter->GetOutputOrigin() << std::endl;
			std::cout << "Restored Image Orientation: " << std::endl << FinalDirection << std::endl;
			
			std::cout << "Writing the Restored Image as: " << std::endl;
			//std::cout << ARFileName << std::endl << std::endl;
			std::cout << ARname << std::endl << std::endl;
			
			writerAntiRotImage->Write();

			// Writing also the rotational transform matrix applied 
			std::string DICOMoutputTxTFileName = argv[2];
			DICOMoutputTxTFileName = DICOMoutputTxTFileName.substr(0,DICOMoutputTxTFileName.find_last_of("."));
			DICOMoutputTxTFileName.append(".txt");

			std::fstream DICOMoutputTxT;
			DICOMoutputTxT.open(DICOMoutputTxTFileName.c_str(), std::ios::out | std::ios::app);

			vnl_matrix< itk::SpacePrecisionType > AntiRotMatrix = reader->GetOutput()->GetDirection().GetVnlMatrix().as_matrix();

			DICOMoutputTxT << "Anti-Rotation Matrix:\n";

			for (int row = 0; row < 3; row ++)
			{	
				for (int col = 0; col < 3; col ++)
				{	
					if (col < 2) 
						DICOMoutputTxT << AntiRotMatrix[row][col] << ",";
					else
						DICOMoutputTxT << AntiRotMatrix[row][col] << ";";
				}
				if (row < 2)
					DICOMoutputTxT << "\n";
			}
			
			DICOMoutputTxT.close();

			}

	}

	std::cout << std::endl << "-- DICOM Conversion: Complete! " << std::endl << std::endl;

	return EXIT_SUCCESS;
	}

