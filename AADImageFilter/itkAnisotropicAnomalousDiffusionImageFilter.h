#ifndef __itkAnisotropicAnomalousDiffusionImageFilter_h
#define __itkAnisotropicAnomalousDiffusionImageFilter_h
#include "itkImageToImageFilter.h"
#include "itkImage.h"
#include "itkNumericTraits.h"
#include <itkConstNeighborhoodIterator.h>


namespace itk
{

template< typename TInputImage, typename TOutputImage >
class ITK_EXPORT AnisotropicAnomalousDiffusionImageFilter:
        public ImageToImageFilter< TInputImage, TOutputImage >
{
public:
    /** Extract dimension from input and output image. */
    itkStaticConstMacro(InputImageDimension, unsigned int,
                        TInputImage::ImageDimension);
    itkStaticConstMacro(OutputImageDimension, unsigned int,
                        TOutputImage::ImageDimension);

    /** Convenient typedefs for simplifying declarations. */
    typedef TInputImage  InputImageType;
    typedef TOutputImage OutputImageType;

    /** Standard class typedefs. */
    typedef AnisotropicAnomalousDiffusionImageFilter                              Self;
    typedef ImageToImageFilter< InputImageType, OutputImageType >                 Superclass;
    typedef SmartPointer< Self >                                                  Pointer;
    typedef SmartPointer< const Self >                                            ConstPointer;

    typedef typename Superclass::OutputImageRegionType OutputImageRegionType;

    typedef itk::ConstNeighborhoodIterator< OutputImageType >       NeighborIteratorType;

    /** Method for creation through the object factory. */
    itkNewMacro(Self)

    /** Run-time type information (and related methods). */
    itkTypeMacro(AnisotropicAnomalousDiffusionImageFilter, ImageToImageFilter)

    typedef typename InputImageType::PixelType                 InputPixelType;
    typedef typename OutputImageType::PixelType                OutputPixelType;
    //  typedef typename NumericTraits< InputPixelType >::RealType InputRealType;

    /** Set the conductance parameter. This is used to the edge preserving strategy in anisotropic filtering.*/
    itkSetMacro(Conductance, float)

//    /** Set the alpha parameter. This parameter is only used for the logistic edge function.*/
//    itkSetMacro(Alpha, float)

    itkSetMacro(H, float)

    itkSetMacro(Iterations, unsigned int)
    itkSetMacro(TimeStep, float)
    itkSetMacro(GeneralizedDiffusion, float)
    itkSetMacro(Q, float)
    itkSetMacro(EdgeFunction, unsigned int)
    itkGetMacro(Conductance, float)
    itkGetMacro(Alpha, float)
    itkGetMacro(H, float)
    itkGetMacro(Iterations, int)
    itkGetMacro(TimeStep, float)
    itkGetMacro(GeneralizedDiffusion, float)
    itkGetMacro(Q, float)
    itkGetMacro(EdgeFunction, unsigned int)

#ifdef ITK_USE_CONCEPT_CHECKING
    // Begin concept checking
    itkConceptMacro( InputHasNumericTraitsCheck,
                     ( Concept::HasNumericTraits< InputPixelType > ) );
    itkConceptMacro( SameDimensionCheck,
                     ( Concept::SameDimension< InputImageDimension, OutputImageDimension > ) );
    itkConceptMacro( InputPixelTypeIsFloatingPointCheck,
                     ( Concept::IsFloatingPoint< InputPixelType > ) );
    //  itkConceptMacro( OutputPixelTypeIsFloatingPointCheck,
    //                   ( Concept::IsFloatingPoint< OutputPixelType > ) );
    // End concept checking
#endif

    enum EdgeFunction {
        EXPONENTIAL=1, //Perona P., Malik J. "Scale-space and edge detection using anisotropic diffusion". 1990. doi: 10.1109/34.56205
        FRACTIONAL=2, //Perona P., Malik J. "Scale-space and edge detection using anisotropic diffusion". 1990. doi: 10.1109/34.56205
        LOGISTIC=3 //PAPER. doi:
    };

protected:
    AnisotropicAnomalousDiffusionImageFilter();
    virtual ~AnisotropicAnomalousDiffusionImageFilter() {}
    float m_GeneralizedDiffusion, m_TimeStep, m_Q, m_Conductance, m_Alpha, m_H;
    unsigned int m_Iterations, m_EdgeFunction;
    virtual void ThreadedGenerateData(const OutputImageRegionType &, ThreadIdType);

private:
    AnisotropicAnomalousDiffusionImageFilter(const Self &); //purposely not implemented
    void operator=(const Self &);  //purposely not implemented
    void TimeStepTestStability();
    float GeneralizedDiffCurve();
    float AlphaCalculation(float maximumGradient);
    float maxGrad=0.0;
    double EdgeWeightedController(InputPixelType idxValue, InputPixelType centerValue, unsigned int model, float maximumGradient);
    double meanNeighbors(NeighborIteratorType neighbors);
    InputPixelType up,down,left,right;
};
} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkAnisotropicAnomalousDiffusionImageFilter.hxx"
#endif

#endif
