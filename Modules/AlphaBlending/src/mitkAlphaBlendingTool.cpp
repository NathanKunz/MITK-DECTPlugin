/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#include "mitkAlphaBlendingTool.h"

#include <mitkImage.h>
#include <mitkImageAccessByItk.h>
#include <mitkImageCast.h>

#include <usModuleContext.h>
#include <usGetModuleContext.h>
#include <usModuleResource.h>
#include <usModuleResourceStream.h>
#include <usModule.h>

#include <tinyxml2.h>
#include <itkArithmeticOpsFunctors.h>
#include <itkImage.h>
#include "itkUnaryFunctorImageFilter.h"
#include "itkAddImageFilter.h"
#include "itkMultiplyImageFilter.h"
#include "itkDivideImageFilter.h"

#include <string>



void mitk::AlphaBlendingTool::Initialize()
{
    ReadConfigResource("alphaParameter.xml");
}

void mitk::AlphaBlendingTool::Reset()
{
    m_AlphaValueMap.clear();
    ReadConfigResource("alphaParameter.xml");
}

void mitk::AlphaBlendingTool::ReadConfigResource(us::ModuleResource& resource)
{
    if (resource.IsValid())
    {
        us::ModuleResourceStream stream(resource);

        // read all into string s
        std::string s;

        stream.seekg(0, std::ios::end);
        s.reserve(stream.tellg());
        stream.seekg(0, std::ios::beg);

        s.assign((std::istreambuf_iterator<char>(stream)),
            std::istreambuf_iterator<char>());

        AddConfig(s);
    }
}

void mitk::AlphaBlendingTool::ReadConfigResource(const std::string & resourcename)
{
	
	if (nullptr == us::GetModuleContext()) return;

	us::ModuleResource r = us::GetModuleContext()->GetModule()->GetResource(resourcename);
    ReadConfigResource(r);
	
}

void mitk::AlphaBlendingTool::AddConfig(std::string& xmlData)
{
    tinyxml2::XMLDocument doc;
    doc.Parse(xmlData.c_str());

    tinyxml2::XMLElement* rootElement = doc.FirstChildElement("AlphaBlendingTool");

    if(rootElement)
    {
       
        //loop through the dom nodes to read all alpha values
        if (!rootElement->NoChildren())
        {       
            for (tinyxml2::XMLElement* dataElement = rootElement->FirstChildElement(); dataElement != NULL; dataElement = dataElement->NextSiblingElement())
            {
                const char* label = dataElement->Attribute("description");
                const char* alphaValue = dataElement->Attribute("alphaValue");

                m_AlphaValueMap[label] = std::stod(std::string(alphaValue));
            }
        }
        else
        {
            MITK_ERROR << "File should contain <AlphaBlendingTool> tag";
        }

    }
}

/**
 * @brief Read external resource defined in filepath and either append or overwrite the existing data.
 * The data from the xml file get's written into m_AlphaValueMap 
 *  
 * @param filepath Full path to the external ressource file
 * @param append Boolean if the data inside the file should be appended to the existing or overwrite
*/
int mitk::AlphaBlendingTool::ReadExternalResource(const std::string& filepath, bool append)
{

    tinyxml2::XMLDocument doc;
    if (doc.LoadFile(filepath.c_str()) == 0)
    {
        tinyxml2::XMLElement* rootElement = doc.FirstChildElement("AlphaBlendingTool");

        if (rootElement)
        {

            //loop through the dom nodes to read all alpha values
            if (!rootElement->NoChildren())
            {
            	//clear alpha value map when not appending and reset when appending
                if (append)                    
                    Reset();
                else
            		m_AlphaValueMap.clear();

                for (tinyxml2::XMLElement* dataElement = rootElement->FirstChildElement(); dataElement != NULL; dataElement = dataElement->NextSiblingElement())
                {
                    const char* label = dataElement->Attribute("description");
                    const char* alphaValue = dataElement->Attribute("alphaValue");

                    m_AlphaValueMap[label] = std::stod(std::string(alphaValue));
                }
                return 0;
            }
            else
            {
                MITK_INFO << "External alpha value file should contain <AlphaBlendingTool> tag";
                return -1;
            }

        }else
        {
            return -1;
        }

    }else
    {     
        return -1;
    }

}


/** This could be optimized by not converting to an MITK image after every function and do the whole alpha blending in only itk images and only convert once in the end,
 * but this would need a new macro in the form of AccessTwoImagesFixedDimensionsByItk_n in which it would be possible to give the additional alpha param,
 * or a manual cast to itk images what would require to know the pixel type.
 *
 */

mitk::Image::Pointer mitk::AlphaBlendingTool::AlphaBlending(mitk::Image::Pointer & imageHigh, mitk::Image::Pointer & imageLow, double alpha)
{
	if (imageHigh->GetDimension() != imageLow->GetDimension())
	{
        mitkThrow() << "Blending between images of different dimension is not supported by mitk::AlphaBlendingTool.";
	}
    //return mitk::ArithmeticOperation::Add(mitk::ArithmeticOperation::Multiply(imageHigh, alpha), mitk::ArithmeticOperation::Multiply(imageLow, (1. - alpha)));
    mitk::AlphaBlendingHelper helper;
    helper.m_AlphaValue = alpha;
    return helper.Add(helper.Mlp(imageHigh, alpha), helper.Mlp(imageLow, (1 - alpha)));
}

mitk::Image::Pointer mitk::AlphaBlendingTool::ConvertToRED(mitk::Image::Pointer & huCube)
{
    mitk::AlphaBlendingHelper helper;
    // Don't know why but it gave out wrong results when chaining the calls like this: return helper.Add(helper.Div(huCube, 1000.), 1.);
	// So we need to use an extra pointer just for that
    mitk::Image::Pointer tmpImage = helper.Div(huCube, 1000.);
    return helper.Add(tmpImage, 1);
}

//Type function in order to access by itk
template<typename TPixel, unsigned int VImageDimension>
static void AddValue(const itk::Image<TPixel, VImageDimension>* image, double v, mitk::Image::Pointer& resultImage)
{
    typedef itk::Image<TPixel, VImageDimension> ImageType;
    typedef itk::Image<double, VImageDimension> DoubleOutputType;
	
    typedef itk::AddImageFilter<ImageType, ImageType, DoubleOutputType> FilterType;

    auto filter = FilterType::New();
    filter->SetInput(image);
    //filter->GetFunctor().value = v;
    filter->SetConstant(v);
    filter->Update();


    mitk::CastToMitkImage(filter->GetOutput(), resultImage);

}

template<typename TPixel, unsigned int VImageDimension>
static void MlpValue(const itk::Image<TPixel, VImageDimension>* image, double v, mitk::Image::Pointer& resultImage)
{
    typedef itk::Image<TPixel, VImageDimension> ImageType;
    typedef itk::Image<double, VImageDimension> DoubleOutputType;

    typedef itk::MultiplyImageFilter<ImageType, ImageType, DoubleOutputType> FilterType;

    auto filter = FilterType::New();
    filter->SetInput(image);
    //filter->GetFunctor().value = v;
    filter->SetConstant(v);
    filter->Update();


    mitk::CastToMitkImage(filter->GetOutput(), resultImage);

}

template<typename TPixel, unsigned int VImageDimension>
static void DivValue(const itk::Image<TPixel, VImageDimension>* image, double v, mitk::Image::Pointer& resultImage)
{
    typedef itk::Image<TPixel, VImageDimension> ImageType;
    typedef itk::Image<double, VImageDimension> DoubleOutputType;

    typedef itk::DivideImageFilter<ImageType, ImageType, DoubleOutputType> FilterType;

    auto filter = FilterType::New();
    filter->SetInput(image);
    //filter->GetFunctor().value = v;
    filter->SetConstant(v);
    filter->Update();


    mitk::CastToMitkImage(filter->GetOutput(), resultImage);

}


mitk::Image::Pointer mitk::AlphaBlendingHelper::Add(mitk::Image::Pointer& image, double v)
{
    mitk::Image::Pointer resultImage;
    AccessByItk_n(image, AddValue, (v, resultImage));
    return resultImage;
}

mitk::Image::Pointer mitk::AlphaBlendingHelper::Mlp(mitk::Image::Pointer& image, double v)
{
    mitk::Image::Pointer resultImage;
    AccessByItk_n(image, MlpValue, (v, resultImage));
    return resultImage;
}

mitk::Image::Pointer mitk::AlphaBlendingHelper::Div(mitk::Image::Pointer& image, double v)
{
    mitk::Image::Pointer resultImage;
    AccessByItk_n(image, DivValue,(v, resultImage));
    return resultImage;	
}

mitk::Image::Pointer mitk::AlphaBlendingHelper::Add(mitk::Image::Pointer & imageA, mitk::Image::Pointer & imageB)
{
	switch (imageA->GetDimension())
	{
    case 1:
        AccessTwoImagesFixedDimensionByItk(imageA, imageB, mitk::AlphaBlendingHelper::Add2Functor, 1);
        break;
    case 2:
        AccessTwoImagesFixedDimensionByItk(imageA, imageB, mitk::AlphaBlendingHelper::Add2Functor, 2);
        break;
    case 3:
        AccessTwoImagesFixedDimensionByItk(imageA, imageB, mitk::AlphaBlendingHelper::Add2Functor, 3);
        break;
    case 4:
        AccessTwoImagesFixedDimensionByItk(imageA, imageB, mitk::AlphaBlendingHelper::Add2Functor, 4);
        break;
    default:
        mitkThrow() << "Image Dimension of " << imageA->GetDimension() << " is not supported";
        break;
	}
	
    return m_ResultImage;    
}
// functions for add arithmetic operation on two images
// in this case the pixel type should always be a double because of the image operation before
template<typename TPixel1, unsigned int VImageDimension1, typename TPixel2, unsigned int VImageDimension2 >
void mitk::AlphaBlendingHelper::Add2Functor(const itk::Image<TPixel1, VImageDimension1>* imageA, const itk::Image<TPixel2, VImageDimension2>* imageB)
{
    typedef itk::Image<TPixel1, VImageDimension1> ImageType1;
    typedef itk::Image<TPixel2, VImageDimension2> ImageType2;
    typedef itk::AddImageFilter<ImageType1, ImageType2, itk::Image<double,VImageDimension1>> FilterType;

    auto filter = FilterType::New();
    filter->SetInput1(imageA);
    filter->SetInput2(imageB);
    filter->Update();

    mitk::CastToMitkImage(filter->GetOutput(), m_ResultImage) ;

}



