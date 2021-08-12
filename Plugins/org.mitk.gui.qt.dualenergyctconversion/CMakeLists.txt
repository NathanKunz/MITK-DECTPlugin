project(org_mitk_gui_qt_dualenergyctconversion)

mitk_create_plugin(
  EXPORT_DIRECTIVE dualEnergyCtConversion_EXPORT
  EXPORTED_INCLUDE_SUFFIXES src
  MODULE_DEPENDS MitkQtWidgetsExt MitkAlphaBlending 
)
