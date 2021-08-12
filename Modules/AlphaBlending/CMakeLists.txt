mitk_create_module(AlphaBlending
  DEPENDS PUBLIC MitkCore MitkBasicImageProcessing
  PACKAGE_DEPENDS PRIVATE tinyxml2 
)

add_subdirectory(test)