
#include <mitkAlphaBlendingTool.h>
#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>
#include <mitkImage.h>
#include <mitkIOUtil.h>

#include <mitkEqual.h>
#include <mitkImageGenerator.h>
#include <itkImageRegionIterator.h>
#include <itkImage.h>

class mitkAlphaBlendingToolTestSuite : public mitk::TestFixture
{
	CPPUNIT_TEST_SUITE(mitkAlphaBlendingToolTestSuite);
	MITK_TEST(TestFailingDimensionConflict);
	MITK_TEST(TestNumericAlphaBlending);
	MITK_TEST(TestNumericREDConversion);
	MITK_TEST(TestFailingReadExResource);
	CPPUNIT_TEST_SUITE_END();

private:
	double m_Alpha;
	mitk::AlphaBlendingTool * m_BlendingTool = nullptr;
	mitk::Image::Pointer m_LowImage;
	mitk::Image::Pointer m_HighImage;
	mitk::Image::Pointer m_HUImage;
	mitk::Image::Pointer m_REDImage;
	mitk::Image::Pointer m_ExpectedHUImage;
	mitk::Image::Pointer m_ExpectedREDImage;
	mitk::Image::Pointer m_TwoDimensionImage;

public:
	/**
	 * @brief      Called once the test suite is selected.
	 * Sets up most of the test data.
	 */
	void setUp() override
	{
		// test alpha value
		m_Alpha = 1.45;
		
		// pointer of the alphabblendingtool
		m_BlendingTool = & mitk::AlphaBlendingTool();
		
		// test low image, double 3D image with values {0,1,2,3,4,5,6,7}
		m_LowImage = mitk::ImageGenerator::GenerateGradientImage<double>(2,2,2);
		
		// test high image with reversed values
		double vals1[8] = { 7., 6., 5., 4., 3., 2., 1., 0. };
		m_HighImage = createImage(vals1);

		// 2D image to test exception throw
		m_TwoDimensionImage = create2DImage();
		
		double vals2[8] = { -3.15, -1.25, 0.65, 2.55, 4.45, 6.35, 8.25, 10.15 };
		m_ExpectedHUImage = createImage(vals2);

		double vals3[8] = { 1., 1.001, 1.002, 1.003, 1.004, 1.005, 1.006, 1.007 };
		m_ExpectedREDImage = createImage(vals3);

		// assert if all images got created
		CPPUNIT_ASSERT_MESSAGE("Failed to create Image for testing.", m_LowImage.IsNotNull());
		CPPUNIT_ASSERT_MESSAGE("Failed to create Image for testing.", m_HighImage.IsNotNull());
		CPPUNIT_ASSERT_MESSAGE("Failed to create instance of alpha blending tool.", m_BlendingTool != nullptr);

		
		// check if expected image got created
		CPPUNIT_ASSERT_MESSAGE("Failed to create two dimensional Image for testing.", m_TwoDimensionImage.IsNotNull());
		CPPUNIT_ASSERT_MESSAGE("Failed to create expected HU Image for testing.", m_ExpectedHUImage.IsNotNull());
		CPPUNIT_ASSERT_MESSAGE("Failed to create expected RED Image for testing.", m_ExpectedREDImage.IsNotNull());

		m_REDImage = m_BlendingTool->ConvertToRED(m_LowImage);

		CPPUNIT_ASSERT_MESSAGE("Failed to create image with alpha blending.", m_REDImage.IsNotNull());
	
	}

	void TestFailingDimensionConflict()
	{
		
		// blending with two different dimensions should throw an exception
		CPPUNIT_ASSERT_THROW_MESSAGE(
			"Calling alpha blending with two images of different dimensions should throw an exception.",
			m_HUImage = m_BlendingTool->AlphaBlending(m_TwoDimensionImage, m_HighImage, m_Alpha),
			mitk::Exception);
		CPPUNIT_ASSERT_MESSAGE("HUImage should be still nullptr", m_HUImage.IsNull());
		
	}
	
	void TestNumericAlphaBlending()
	{
		// try catch because cpp units throws an error e.g. when the pixel type doesn't match up,
		// and the driver just shut downs instead of failing the test
		m_HUImage = m_BlendingTool->AlphaBlending(m_LowImage, m_HighImage, m_Alpha);

		CPPUNIT_ASSERT_MESSAGE("Failed to create image with alpha blending.", m_HUImage.IsNotNull());
		
		try
		{
		MITK_ASSERT_EQUAL(
			m_ExpectedHUImage,
			m_HUImage,
			"Blended image should be the same as expected image."
		);	
		}
		catch(CppUnit::Exception & e)
		{
			//cerr << e.what() << endl;
			CPPUNIT_FAIL("Problem with comparing expected and actual HU image.");
		}
		
	}

	void TestNumericREDConversion()
	{
		// try catch because cpp units throws an error e.g. when the pixel type doesn't match up,
		// and the driver just shut downs instead of failing the test
		m_REDImage = m_BlendingTool->ConvertToRED(m_LowImage); // we are using Low Image here because there is a smaller chance off floating point errors when using full numbers

		try
		{			
			MITK_ASSERT_EQUAL(
				m_ExpectedREDImage,
				m_REDImage,
				"Blended image should be the same as expected image."
			);
		}
		catch (CppUnit::Exception & e)
		{
			CPPUNIT_FAIL("Problem with comparing expected and actual HU image.");
		}
				
	}

	void TestFailingReadExResource()
	{
		int expected = m_BlendingTool->ReadExternalResource(" ", false);
		CPPUNIT_ASSERT_MESSAGE(
			"Loading a non existing or failing path config file should return -1.",
			expected == -1);

	}

	/**
	 * @brief      Creates an 3d 2|2|2 mitk image with given values.
	 *
	 * @param      array of 8 double values which are written into the image 
	 *
	 * @return     mitk image pointer
	 */
	static mitk::Image::Pointer createImage(double* vals)
	{
		// create image to compare
		typedef itk::Image<double, 3> ImageType;
		ImageType::RegionType region;
		ImageType::IndexType start;
		start.Fill(0);
		region.SetIndex(start);
		ImageType::SizeType size;
		size[0] = 2;
		size[1] = 2;
		size[2] = 2;

		region.SetSize(size);

		ImageType::SpacingType spacing;
		spacing[0] = 1;
		spacing[1] = 1;
		spacing[2] = 1;


		mitk::Point3D origin;
		origin.Fill(0.0);
		itk::Matrix<double, 3, 3> directionMatrix;
		directionMatrix.SetIdentity();


		ImageType::Pointer image = ImageType::New();
		image->SetSpacing(spacing);
		image->SetOrigin(origin);
		image->SetDirection(directionMatrix);
		image->SetRegions(region);
		image->Allocate();
		image->FillBuffer(0.0);

		itk::ImageRegionIterator<ImageType> it(image, region);
		it.GoToBegin();

		int idx = 0;

		while (!it.IsAtEnd())
		{
			it.Set(vals[idx]);
			++it;
			idx++;
		}

		mitk::Image::Pointer mitkImage = mitk::Image::New();
		mitkImage->InitializeByItk(image.GetPointer());
		mitkImage->SetVolume(image->GetBufferPointer());
		return mitkImage;
	}


	/**
	 * @brief      Creates an empty 2d mitk image.
	 *
	 * @return     2D mitk image
	 */
	static mitk::Image::Pointer create2DImage()
	{
		// create image to compare
		typedef itk::Image<double, 2> ImageType;
		ImageType::RegionType region;
		ImageType::IndexType start;
		start.Fill(0);
		region.SetIndex(start);
		ImageType::SizeType size;
		size.Fill(2);

		region.SetSize(size);

		mitk::Point2D origin;
		origin.Fill(0.0);

		ImageType::Pointer image = ImageType::New();
		image->SetOrigin(origin);
		image->SetRegions(region);
		image->Allocate();
		
		mitk::Image::Pointer mitkImage = mitk::Image::New();
		mitkImage->InitializeByItk(image.GetPointer());
		mitkImage->SetVolume(image->GetBufferPointer());
		return mitkImage;
	}


	/**
	 * @brief      Gives all the pointer free to surely clean up
	 */
	void tearDown() override
	{
	
		// delete all images and the instance of the blending tool
		m_HUImage = nullptr;
		m_HighImage = nullptr;
		m_LowImage = nullptr;
		m_ExpectedHUImage = nullptr;
		m_REDImage = nullptr;
		m_ExpectedREDImage = nullptr;
		m_TwoDimensionImage = nullptr;
		m_BlendingTool = nullptr;
		
	}
	
};


MITK_TEST_SUITE_REGISTRATION(mitkAlphaBlendingTool)
