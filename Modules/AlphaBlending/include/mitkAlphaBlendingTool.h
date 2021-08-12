/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef AlphaBlendingTool_h
#define AlphaBlendingTool_h

#include <mitkImage.h>
#include <mitkImageAccessByItk.h>
#include <mitkImageCast.h>

#include <usModuleResource.h>

#include <MitkAlphaBlendingExports.h>

#include <string>
#include <vector>
#include <map>

// See ExampleImageFilter.h for details on typical class declarations
// in MITK. The actual functionality of this class is commented in its
// implementation file.

namespace mitk{
	
	class MITKALPHABLENDING_EXPORT AlphaBlendingTool
	{
	public:

		AlphaBlendingTool() = default;
		~AlphaBlendingTool() = default;

		/**
		 * @brief      Initializes the object and read in alpha values from config file.
		 */
		void Initialize();
		/**
		 * @brief      Resets alpha values. Used when reading in new alpha values.
		 */
		void Reset();

		std::map<std::string, double> m_AlphaValueMap; // std::map which contain all the alpha values and their fitting descriptors

		/**
		 * @brief      Blends two given mitk images, with a given alpha value 
		 *
		 * @param      imageHigh  image with higher voltage level
		 * @param      imageLow   image with lower voltage level
		 * @param[in]  alpha      alpha value
		 *
		 * @return     double mitk image of same dimensions
		 */
		mitk::Image::Pointer AlphaBlending(mitk::Image::Pointer& imageHigh, mitk::Image::Pointer& imageLow, double alpha);

		/**
		 * @brief      Convert given HU image to an RED image
		 *
		 * @param      huCube  The hu image
		 *
		 * @return     mitk image 
		 */
		mitk::Image::Pointer ConvertToRED(mitk::Image::Pointer& huCube);

		/**
		 * @brief Read external resource defined in filepath and either append or overwrite the existing data.
		 * The data from the xml file get's written into m_AlphaValueMap 
		 *  
		 * @param filepath Full path to the external ressource file
		 * @param append Boolean if the data inside the file should be appended to the existing or overwrite
		*/
		int ReadExternalResource(const std::string& filepath, bool append);

	private:

		/**
		 * @brief      Reads a configuration resource.
		 *
		 * @param      resource  module resource
		 */
		void ReadConfigResource(us::ModuleResource& resource);

		/**
		 * @brief      Reads  internal resource
		 *
		 * @param[in]  resourcename  The resourcename
		 */
		void ReadConfigResource(const std::string& resourcename);


		/**
		 * @brief      Read resource from xml string.
		 *
		 * @param      xmlData  xml string data
		 */
		void AddConfig(std::string& xmlData);

	};

	// helper class for the arithmetic operations
	class MITKALPHABLENDING_EXPORT AlphaBlendingHelper
	{

	public:
		//Arithmetic part
		double m_AlphaValue;
		mitk::Image::Pointer m_ResultImage;

		/**
		 * @brief      Perform pixel wise addition between and image and a scaler
		 *
		 * @param      image  The image
		 * @param[in]  v      value to add
		 *
		 * @return     mitk image pointer
		 */
		mitk::Image::Pointer Add(mitk::Image::Pointer& image, double v);

		/**
		 * @brief      Perform pixel wise multiplikation between and image and a scaler
		 *
		 * @param      image  The image
		 * @param[in]  v      value
		 *
		 * @return     { description_of_the_return_value }
		 */
		mitk::Image::Pointer Mlp(mitk::Image::Pointer& image, double v);

		/**
		 * @brief      Perform pixel wise division between and image and a scaler
		 *
		 * @param      image  The image
		 * @param[in]  v      value 
		 *
		 * @return     result image
		 */
		mitk::Image::Pointer Div(mitk::Image::Pointer& image, double v);

		/**
		 * @brief      Adds two images together, pixel wise operation. Calls the AccessTwoImagesFixedDimensionByItk for the specific dimension.
		 *
		 * @param      imageA  The image a
		 * @param      imageB  The image b
		 *
		 * @return     returns added image
		 */
		mitk::Image::Pointer Add(mitk::Image::Pointer& imageA, mitk::Image::Pointer& imageB);

		/**
		 * @brief      Add two image functor for AccessTwoImagesFixedDimensionByItk.
		 *
		 * @param[in]  imageA            image a
		 * @param[in]  imageB            image b
		 *
		 * @tparam     TPixel1           pixel type of image one
		 * @tparam     VImageDimension1  dimension of image one
		 * @tparam     TPixel2           pixel type of image two
		 * @tparam     VImageDimension2  dimension of image two
		 */
		template<typename TPixel1, unsigned int VImageDimension1, typename TPixel2, unsigned int VImageDimension2 >
		void Add2Functor(const itk::Image<TPixel1, VImageDimension1>* imageA, const itk::Image<TPixel2, VImageDimension2>* imageB);

	};
	
}
#endif

