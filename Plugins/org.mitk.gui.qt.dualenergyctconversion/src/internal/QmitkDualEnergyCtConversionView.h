/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkDualEnergyCtConversionView_h
#define QmitkDualEnergyCtConversionView_h

#include <berryISelectionListener.h>
#include <QmitkAbstractView.h>
#include <QmitkSingleNodeSelectionWidget.h>
#include <mitkAlphaBlendingTool.h>


// Include the Qt UI file which contains all the gui information for this plugin
#include <ui_QmitkDualEnergyCtConversionControls.h>

// All views in MITK derive from QmitkAbstractView. 
class QmitkDualEnergyCtConversionView : public QmitkAbstractView
{
  // Use Qt signal slot mechanic
  Q_OBJECT

public:
	
  void InitAlphaBlendingTool();
  
  // set the view id
  static const std::string VIEW_ID;

  // In this method we initialize the GUI components and connect the
  // associated signals and slots.
  void CreateQtPartControl(QWidget* parent) override;

  /**
   * @brief      Called when preferences page changed. 
   * Loads new alpha values when a fitting file is selected or displa a warning when the file is not valid.
   */
  void OnPreferencesChanged(const berry::IBerryPreferences* prefs) override;


private slots:
  void OnImageChanged(const QmitkSingleNodeSelectionWidget::NodeList& nodes);
  void OnHuImageChanged(const QmitkSingleNodeSelectionWidget::NodeList&);

  /**
   * @brief      Blend the two selected images togheter with the help of the alpha blending module.
   */
  void BlendSelectedImages();
  
  /**
   * @brief      Covnert the selected HU image to a RED image, with the help of the alpha blending module.
   */
  void ConvertToREDImage();
  
  /**
   * @brief      Called on mode change. Write the new alpha value in the spin box. 
   * And activate alpha spin box when custom value is activated.
   *
   */
  void OnModeChange(int idx);

private:
  // Typically a one-liner. Set the focus to the default widget.
  void SetFocus() override;
  
  /**
   * @brief      slot method called whenever a change happens inside the plugin ui elements. 
   * Activate the Blending button once all necessary elements are seleceted.
   */
  void EnableBlendingButton(bool enable);


  /**
   * @brief      Enables the conversion button. once all necessary elements are selected.
   *
   */
  void EnableConversionButton(bool enable);

  /**
   * @brief      Update the alpha mode box. Called when preference page change.
   */
  void UpdateModeBox();

  mitk::AlphaBlendingTool m_BlendingTool; // object of blendingTool from alphaBlending module performing all the arithmetic.
  bool initializeBool = true; // bool if the blending tool needs to be initialized
  

  // Generated from the associated UI file, it encapsulates all the widgets
  // of our view.
  Ui::QmitkDualEnergyCtConversionControls m_Controls;
};

#endif
