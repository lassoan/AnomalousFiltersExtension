#include "DTIMapSegmentation.h"

using namespace std;

DTIMapSegmentation::DTIMapSegmentation(const char *inputVolume, const char *outputLabels){
    this->numClasses=3;
    this->inputVolume=inputVolume;
    this->outputLabels=outputLabels;
}

int DTIMapSegmentation::getNumClasses() const
{
    return numClasses;
}

void DTIMapSegmentation::setNumClasses(int value)
{
    numClasses = value;
}

void DTIMapSegmentation::runSegmentation(DTIMapSegmentation::segMethod method)
{
    switch (method) {
    case BAYES:
        BayesSegmentation();
        break;
    case KMEANS:
        KMeansSegmentation();
        break;
    default:
        break;
    }
}

void DTIMapSegmentation::BayesSegmentation()
{

    const unsigned int              Dimension = 3;
    typedef float           InputPixelType;
    typedef itk::Image< InputPixelType, Dimension > ImageType;
    typedef itk::BayesianClassifierInitializationImageFilter< ImageType >         BayesianInitializerType;
    BayesianInitializerType::Pointer bayesianInitializer = BayesianInitializerType::New();

    typedef itk::ImageFileReader< ImageType > ReaderType;
    ReaderType::Pointer reader = ReaderType::New();
    reader->SetFileName( inputVolume );
    reader->Update();

    bayesianInitializer->SetInput( reader->GetOutput() );
    bayesianInitializer->SetNumberOfClasses( getNumClasses() );
    bayesianInitializer->Update();

    typedef unsigned char  LabelType;
    typedef float          PriorType;
    typedef float          PosteriorType;


    typedef itk::VectorImage< InputPixelType, Dimension > VectorInputImageType;
    typedef itk::BayesianClassifierImageFilter<
            VectorInputImageType,LabelType,
            PosteriorType,PriorType >   ClassifierFilterType;

    ClassifierFilterType::Pointer bayesClassifier = ClassifierFilterType::New();


    bayesClassifier->SetInput( bayesianInitializer->GetOutput() );

    //      TODO: See to add the AAD filter here!
    //      if( argv[3] )
    //        {
    //        filter->SetNumberOfSmoothingIterations( atoi( argv[3] ));
    //        typedef ClassifierFilterType::ExtractedComponentImageType ExtractedComponentImageType;
    //        typedef itk::GradientAnisotropicDiffusionImageFilter<
    //          ExtractedComponentImageType, ExtractedComponentImageType >  SmoothingFilterType;
    //        SmoothingFilterType::Pointer smoother = SmoothingFilterType::New();
    //        smoother->SetNumberOfIterations( 1 );
    //        smoother->SetTimeStep( 0.125 );
    //        smoother->SetConductanceParameter( 3 );
    //        filter->SetSmoothingFilter( smoother );
    //        }


    // SET FILTER'S PRIOR PARAMETERS
    // do nothing here to default to uniform priors
    // otherwise set the priors to some user provided values

    typedef itk::Image< unsigned char, Dimension >     OutputImageType;
    typedef itk::ImageFileWriter< OutputImageType >  WriterType;
    WriterType::Pointer writer = WriterType::New();
    writer->SetInput( bayesClassifier->GetOutput() );
    writer->SetFileName( outputLabels );

    writer->Update();
}

void DTIMapSegmentation::MRFSegmentation()
{
//    if( argc < 7 )
//        {
//        std::cerr << "Usage: " << std::endl;
//        std::cerr << argv[0];
//        std::cerr << " inputScalarImage inputLabeledImage";
//        std::cerr << " outputLabeledImage numberOfIterations";
//        std::cerr << " smoothingFactor numberOfClasses";
//        std::cerr << " mean1 mean2 ... meanN " << std::endl;
//        return EXIT_FAILURE;
//        }

//      const char * inputImageFileName      = ;
//      const char * inputLabelImageFileName = argv[2];
//      const char * outputImageFileName     = argv[3];

//      const unsigned int numberOfIterations = atoi( argv[4] );
      const double       smoothingFactor    = 1;
//      const unsigned int numberOfClasses    = atoi( argv[6] );

//      const unsigned int numberOfArgumentsBeforeMeans = 7;

//      if( static_cast<unsigned int>(argc) <
//          numberOfClasses + numberOfArgumentsBeforeMeans )
//        {
//        std::cerr << "Error: " << std::endl;
//        std::cerr << numberOfClasses << " classes have been specified ";
//        std::cerr << "but not enough means have been provided in the command ";
//        std::cerr << "line arguments " << std::endl;
//        return EXIT_FAILURE;

//        }


      typedef float                 PixelType;
      const unsigned int          Dimension = 3;

      typedef itk::Image<PixelType, Dimension > ImageType;

      typedef itk::ImageFileReader< ImageType > ReaderType;
      ReaderType::Pointer reader = ReaderType::New();
      reader->SetFileName( inputVolume );


      typedef unsigned char       LabelPixelType;
      typedef itk::Image<LabelPixelType, Dimension > LabelImageType;

    //First label approximation.
      typedef itk::ScalarImageKmeansImageFilter< ImageType > KMeansFilterType;
      KMeansFilterType::Pointer kmeansFilter = KMeansFilterType::New();
      kmeansFilter->SetInput( reader->GetOutput() );
      const unsigned int numberOfInitialClasses = getNumClasses();

      //TODO Apply the Gaussian Mixture Method to get the initial means guess.
      double means [getNumClasses()];
      for( unsigned k=0; k < numberOfInitialClasses; k++ )
      {
          means[k]=(k*50)+1;
          kmeansFilter->AddClassWithInitialMean( means[k] );
      }

//      typedef itk::ImageFileReader< LabelImageType > LabelReaderType;
//      LabelReaderType::Pointer labelReader = LabelReaderType::New();
//      labelReader->SetFileName( kmeansFilter->GetOutput() );


      typedef itk::FixedArray<LabelPixelType,1>  ArrayPixelType;

      typedef itk::Image< ArrayPixelType, Dimension > ArrayImageType;

      typedef itk::ComposeImageFilter<
                         ImageType, ArrayImageType > ScalarToArrayFilterType;

      ScalarToArrayFilterType::Pointer
        scalarToArrayFilter = ScalarToArrayFilterType::New();
      scalarToArrayFilter->SetInput( reader->GetOutput() );

      typedef itk::MRFImageFilter< ArrayImageType, LabelImageType > MRFFilterType;

      MRFFilterType::Pointer mrfFilter = MRFFilterType::New();

      mrfFilter->SetInput( scalarToArrayFilter->GetOutput() );

      mrfFilter->SetNumberOfClasses( getNumClasses() );
      mrfFilter->SetMaximumNumberOfIterations( 30 );
      mrfFilter->SetErrorTolerance( 1e-7 );

      mrfFilter->SetSmoothingFactor( smoothingFactor );

      typedef itk::ImageClassifierBase<
                                  ArrayImageType,
                                  LabelImageType >   SupervisedClassifierType;

      SupervisedClassifierType::Pointer classifier =
                                             SupervisedClassifierType::New();

      typedef itk::Statistics::MinimumDecisionRule DecisionRuleType;

      DecisionRuleType::Pointer  classifierDecisionRule = DecisionRuleType::New();

      classifier->SetDecisionRule( classifierDecisionRule.GetPointer() );

      typedef itk::Statistics::DistanceToCentroidMembershipFunction<
                                                        ArrayPixelType >
                                                           MembershipFunctionType;

      typedef MembershipFunctionType::Pointer MembershipFunctionPointer;


      double meanDistance = 0;
      MembershipFunctionType::CentroidType centroid(1);
      for( unsigned int i=0; i < static_cast<unsigned int>(getNumClasses()); i++ )
        {
        MembershipFunctionPointer membershipFunction =
                                             MembershipFunctionType::New();

//        centroid[0] = atof( argv[i+numberOfArgumentsBeforeMeans] );
        centroid[0]=kmeansFilter->GetFinalMeans()[i];

        membershipFunction->SetCentroid( centroid );

        classifier->AddMembershipFunction( membershipFunction );
        meanDistance += static_cast< double > (centroid[0]);
        }
//      if (getNumClasses() > 0)
//        {
        meanDistance /= getNumClasses();
//        }
//      else
//        {
//        std::cerr << "ERROR: numberOfClasses is 0" << std::endl;
//        return EXIT_FAILURE;
//        }

      mrfFilter->SetSmoothingFactor( smoothingFactor );

      mrfFilter->SetNeighborhoodRadius( 1 );

//TODO Review how is the matrix neighborhood is build!
//      std::vector< double > weights;
//      weights.push_back(1.5);
//      weights.push_back(2.0);
//      weights.push_back(1.5);
//      weights.push_back(2.0);
//      weights.push_back(0.0); // This is the central pixel
//      weights.push_back(2.0);
//      weights.push_back(1.5);
//      weights.push_back(2.0);
//      weights.push_back(1.5);

//      weights.push_back(1.5);
//      weights.push_back(2.0);
//      weights.push_back(1.5);
//      weights.push_back(2.0);
//      weights.push_back(0.0); // This is the central pixel
//      weights.push_back(2.0);
//      weights.push_back(1.5);
//      weights.push_back(2.0);
//      weights.push_back(1.5);
//      weights.push_back(1.5);
//      weights.push_back(2.0);
//      weights.push_back(1.5);
//      weights.push_back(2.0);
//      weights.push_back(0.0); // This is the central pixel
//      weights.push_back(2.0);
//      weights.push_back(1.5);
//      weights.push_back(2.0);
//      weights.push_back(1.5);

//      double totalWeight = 0;
//      for(std::vector< double >::const_iterator wcIt = weights.begin();
//          wcIt != weights.end(); ++wcIt )
//        {
//        totalWeight += *wcIt;
//        }
//      for(std::vector< double >::iterator wIt = weights.begin();
//          wIt != weights.end(); ++wIt )
//        {
//        *wIt = static_cast< double > ( (*wIt) * meanDistance / (2 * totalWeight));
//        }

//      mrfFilter->SetMRFNeighborhoodWeight( weights );

        mrfFilter->SetClassifier( classifier );

      typedef MRFFilterType::OutputImageType  OutputImageType;


      // Rescale outputs to the dynamic range of the display
//      typedef itk::Image< unsigned char, Dimension > RescaledOutputImageType;
//      typedef itk::RescaleIntensityImageFilter<
//                 OutputImageType, RescaledOutputImageType >   RescalerType;

//      RescalerType::Pointer intensityRescaler = RescalerType::New();
//      intensityRescaler->SetOutputMinimum(   0 );
//      intensityRescaler->SetOutputMaximum( 255 );
//      intensityRescaler->SetInput( mrfFilter->GetOutput() );


      typedef itk::ImageFileWriter< OutputImageType > WriterType;

      WriterType::Pointer writer = WriterType::New();

      writer->SetInput( mrfFilter->GetOutput() );

      writer->SetFileName( outputLabels );

        writer->Update();
//        }
//      catch( itk::ExceptionObject & excp )
//        {
//        std::cerr << "Problem encountered while writing ";
//        std::cerr << " image file : " << argv[2] << std::endl;
//        std::cerr << excp << std::endl;
//        return EXIT_FAILURE;
//        }
//      // Software Guide : EndCodeSnippet

//      std::cout << "Number of Iterations : ";
//      std::cout << mrfFilter->GetNumberOfIterations() << std::endl;
//      std::cout << "Stop condition: " << std::endl;
//      std::cout << "  (1) Maximum number of iterations " << std::endl;
//      std::cout << "  (2) Error tolerance:  "  << std::endl;
//      std::cout << mrfFilter->GetStopCondition() << std::endl;

}

void DTIMapSegmentation::KMeansSegmentation()
{

    typedef float       PixelType;
    const unsigned int         Dimension = 3;
    typedef itk::Image<PixelType, Dimension > ImageType;
    typedef itk::ImageFileReader< ImageType > ReaderType;
    ReaderType::Pointer reader = ReaderType::New();
    reader->SetFileName( inputVolume );

    typedef itk::ScalarImageKmeansImageFilter< ImageType > KMeansFilterType;
    KMeansFilterType::Pointer kmeansFilter = KMeansFilterType::New();
    kmeansFilter->SetInput( reader->GetOutput() );
    const unsigned int numberOfInitialClasses = getNumClasses();

    //TODO Apply the Gaussian Mixture Method to get the initial mean guess.
    double means [getNumClasses()];
    for( unsigned k=0; k < numberOfInitialClasses; k++ )
    {
        means[k]=(k*50)+1;
        kmeansFilter->AddClassWithInitialMean( means[k] );
    }


    typedef KMeansFilterType::OutputImageType  OutputImageType;
    typedef itk::ImageFileWriter< OutputImageType > WriterType;
    WriterType::Pointer writer = WriterType::New();
    writer->SetInput( kmeansFilter->GetOutput() );
    writer->SetFileName( outputLabels );

    writer->Update();
}
