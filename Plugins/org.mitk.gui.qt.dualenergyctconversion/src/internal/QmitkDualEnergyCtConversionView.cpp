/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <berryISelectionService.h>
#include <berryIWorkbenchWindow.h>
#include "berryIQtPreferencePage.h"
#include <berryIPreferences.h>
#include <berryIPreferencesService.h>
#include <berryIBerryPreferences.h>
#include <berryPlatform.h>
#include <mitkNodePredicateAnd.h>
#include <mitkNodePredicateDataType.h>
#include <mitkNodePredicateNot.h>
#include <mitkNodePredicateOr.h>
#include <mitkNodePredicateProperty.h>
#include <mitkLevelWindowProperty.h>
#include <mitkImage.h>

#include <usModuleRegistry.h>
#include <string>
#include <QMessageBox>
// include alpha blending module
#include <mitkAlphaBlendingTool.h>

#include "QmitkDualEnergyCtConversionView.h"

//helper function
void QmitkDualEnergyCtConversionView::InitAlphaBlendingTool()
{

    //initialize the alphablending module
    auto module = us::ModuleRegistry::GetModule("MitkAlphaBlending");
    if (nullptr != module)
    {
        m_BlendingTool = mitk::AlphaBlendingTool();
        m_BlendingTool.Initialize();

        MITK_INFO << "Alpha Blending module could be found and initialized";

        initializeBool = false;

    }else
    {
        MITK_ERROR << "Alpha Blending module couldn't be found";
        // gonna throw an error, but this plugin wont work without the alpha blending module so ...
        return;

    }

}
	
void QmitkDualEnergyCtConversionView::UpdateModeBox()
{
    m_Controls.modeBox->clear();
	
    QStringList qlist;

    for (auto it = m_BlendingTool.m_AlphaValueMap.begin(); it != m_BlendingTool.m_AlphaValueMap.end(); it++)
    {
        qlist << QString::fromStdString(it->first);
    }
    qlist << QString("custom alphaValue");

    m_Controls.modeBox->addItems(qlist);
}

// Don't forget to initialize the VIEW_ID.
const std::string QmitkDualEnergyCtConversionView::VIEW_ID = "org.mitk.views.dualenergyctconversion";

void QmitkDualEnergyCtConversionView::CreateQtPartControl(QWidget* parent)
{

    //OnPreferencesChanged(nullptr);
    // Setting up the UI is a true pleasure when using .ui files, isn't it?
    m_Controls.setupUi(parent);

    m_Controls.alphaSpinBox->setReadOnly(true);

    m_Controls.selectionWidget_lowEnergy->SetDataStorage(this->GetDataStorage());
    m_Controls.selectionWidget_lowEnergy->SetSelectionIsOptional(true);
    m_Controls.selectionWidget_lowEnergy->SetEmptyInfo(QStringLiteral("Select an low energy image"));


    m_Controls.selectionWidget_lowEnergy->SetNodePredicate(mitk::NodePredicateAnd::New(
        mitk::TNodePredicateDataType<mitk::Image>::New(),
        mitk::NodePredicateNot::New(mitk::NodePredicateOr::New(
            mitk::NodePredicateProperty::New("helper object"),
            mitk::NodePredicateProperty::New("hidden object")))));

    m_Controls.selectionWidget_highEnergy->SetDataStorage(this->GetDataStorage());
    m_Controls.selectionWidget_highEnergy->SetSelectionIsOptional(true);
    m_Controls.selectionWidget_highEnergy->SetEmptyInfo(QStringLiteral("Select an high energy image"));

    m_Controls.selectionWidget_highEnergy->SetNodePredicate(mitk::NodePredicateAnd::New(
        mitk::TNodePredicateDataType<mitk::Image>::New(),
        mitk::NodePredicateNot::New(mitk::NodePredicateOr::New(
            mitk::NodePredicateProperty::New("helper object"),
            mitk::NodePredicateProperty::New("hidden object")))));

    m_Controls.selectionWidget_huCube->SetDataStorage(this->GetDataStorage());
    m_Controls.selectionWidget_huCube->SetSelectionIsOptional(true);
    m_Controls.selectionWidget_huCube->SetEmptyInfo(QStringLiteral("Select HU image"));

    m_Controls.selectionWidget_huCube->SetNodePredicate(mitk::NodePredicateAnd::New(
        mitk::TNodePredicateDataType<mitk::Image>::New(),
        mitk::NodePredicateNot::New(mitk::NodePredicateOr::New(
            mitk::NodePredicateProperty::New("helper object"),
            mitk::NodePredicateProperty::New("hidden object")))));

    // hide the warning display
    m_Controls.warningLabel->setDisabled(true);
    m_Controls.warningLabel->setVisible(false);

	// get obj of alpha blending tool and load the defined alpha values in "AlphaBlendingTool/resources/alphaParameter.xml"
	if(initializeBool)
		InitAlphaBlendingTool();
	
	
    UpdateModeBox();

    // Wire up the UI widgets with our functionality.
    connect(m_Controls.selectionWidget_lowEnergy, &QmitkSingleNodeSelectionWidget::CurrentSelectionChanged, this, &QmitkDualEnergyCtConversionView::OnImageChanged);
    connect(m_Controls.selectionWidget_highEnergy, &QmitkSingleNodeSelectionWidget::CurrentSelectionChanged, this, &QmitkDualEnergyCtConversionView::OnImageChanged);
    connect(m_Controls.selectionWidget_huCube, &QmitkSingleNodeSelectionWidget::CurrentSelectionChanged, this, &QmitkDualEnergyCtConversionView::OnHuImageChanged);



	//Wire up the select mode Box and the alpha Values box
    connect(m_Controls.modeBox, SIGNAL(currentIndexChanged(int)), this, SLOT(OnModeChange(int)));
    // Wire up the UI blend button with the correlating funtion
    connect(m_Controls.blendingImageButton, SIGNAL(clicked()), this, SLOT(BlendSelectedImages()));
	// Wire up red conversion
    connect(m_Controls.redConversionButton, SIGNAL(clicked()), this, SLOT(ConvertToREDImage()));

    // Make sure to have a consistent UI state at the very beginning.
    this->OnImageChanged(m_Controls.selectionWidget_lowEnergy->GetSelectedNodes());
    this->OnImageChanged(m_Controls.selectionWidget_highEnergy->GetSelectedNodes());
    this->OnHuImageChanged(m_Controls.selectionWidget_huCube->GetSelectedNodes());
}

void QmitkDualEnergyCtConversionView::SetFocus()
{
    m_Controls.blendingImageButton->setFocus();
}

void QmitkDualEnergyCtConversionView::OnModeChange(int idx)
{
	//enable or disable the blending button
    this->EnableBlendingButton(m_Controls.selectionWidget_lowEnergy->GetSelectedNode().IsNotNull() && m_Controls.selectionWidget_highEnergy->GetSelectedNode().IsNotNull() && (m_Controls.modeBox->currentIndex() != -1));
	//enable edit for alpha value field if the custom alpha value box is selected
	if(idx == (m_Controls.modeBox->count()-1))
    {
        m_Controls.alphaSpinBox->setReadOnly(false);
	}else
	{
        m_Controls.alphaSpinBox->setReadOnly(true);
		// set the value of the alpha box to the corresponding alpha value to the selected mode
        //m_Controls.alphaSpinBox->setValue(m_BlendingTool.alphaValuesVector[idx]);
        m_Controls.alphaSpinBox->setValue(m_BlendingTool.m_AlphaValueMap[m_Controls.modeBox->currentText().toUtf8().constData()]);
	}
}

void QmitkDualEnergyCtConversionView::OnImageChanged(const QmitkSingleNodeSelectionWidget::NodeList&)
{
    this->EnableBlendingButton(m_Controls.selectionWidget_lowEnergy->GetSelectedNode().IsNotNull() && m_Controls.selectionWidget_highEnergy->GetSelectedNode().IsNotNull() && (m_Controls.modeBox->currentIndex() != -1));
}

void QmitkDualEnergyCtConversionView::EnableBlendingButton(bool enable)
{
    m_Controls.blendingImageButton->setEnabled(enable);
}

void QmitkDualEnergyCtConversionView::OnHuImageChanged(const QmitkSingleNodeSelectionWidget::NodeList&)
{
    this->EnableConversionButton(m_Controls.selectionWidget_huCube->GetSelectedNode().IsNotNull());
}

void QmitkDualEnergyCtConversionView::EnableConversionButton(bool enable)
{
    m_Controls.redConversionButton->setEnabled(enable);
}


void QmitkDualEnergyCtConversionView::BlendSelectedImages()
{

    auto selectedDataNodeLow = m_Controls.selectionWidget_lowEnergy->GetSelectedNode();
    auto selectedDataNodeHigh = m_Controls.selectionWidget_highEnergy->GetSelectedNode();

    auto dataLow = selectedDataNodeLow->GetData();
    auto dataHigh = selectedDataNodeHigh->GetData();

    //Get selected images
    mitk::Image::Pointer imageLow = dynamic_cast<mitk::Image*>(dataLow);
    mitk::Image::Pointer imageHigh = dynamic_cast<mitk::Image*>(dataHigh);

    auto imageName = selectedDataNodeLow->GetName();

    // get the level window for later
    mitk::LevelWindow levelWindow;
    selectedDataNodeLow->GetLevelWindow(levelWindow);

    MITK_INFO << "Blending images \"" << imageName << "\" ... ";

    //call alpha blending method of coresponding module
    mitk::Image::Pointer huCube  = m_BlendingTool.AlphaBlending(imageHigh, imageLow, m_Controls.alphaSpinBox->value());
    //mitk::Image::Pointer huCube = mitk::ArithmeticOperation::Add(mitk::ArithmeticOperation::Multiply(imageHigh, m_Controls.alphaSpinBox->value()), mitk::ArithmeticOperation::Multiply(imageLow, (1. - m_Controls.alphaSpinBox->value())));
    // create new datanode which contains the calculated alpha image

	auto huDataNode = mitk::DataNode::New();
    huDataNode->SetData(huCube);
	

    MITK_INFO << "  done";

    QString name = QString("%1 (rED)").arg(imageName.c_str());
    name = QString("%1 (HU)").arg(imageName.c_str());
	
    // set the name of the new data node
    huDataNode->SetName(name.toStdString());
    // set the level window to the same as the lowenergy data node because they wont differ much
    huDataNode->SetLevelWindow(levelWindow);
    // get the datastorage
    mitk::DataStorage::Pointer datastorage = this->GetDataStorage();

    // add the image to the datastorage
    datastorage->Add(huDataNode);

	// set the newly calculated datanode into the red conversion
    m_Controls.selectionWidget_huCube->SetCurrentSelectedNode(huDataNode);


}

void QmitkDualEnergyCtConversionView::ConvertToREDImage()
{
    auto selectedDataNode = m_Controls.selectionWidget_huCube->GetSelectedNode();
    auto data = selectedDataNode->GetData();
    auto imageName = selectedDataNode->GetName();

    mitk::Image::Pointer huCube = dynamic_cast<mitk::Image*>(data);

    mitk::Image::Pointer rEDCube = m_BlendingTool.ConvertToRED(huCube);


    MITK_INFO << "convert to RED Image \"" << imageName << "\" ... ";
	// create datanode containing the new red image
    auto rEDDataNode = mitk::DataNode::New();
    
    rEDDataNode->SetData(rEDCube);

    QString name = QString("%1 (rED)").arg(imageName.c_str());
    rEDDataNode->SetName(name.toStdString());	

	mitk::DataStorage::Pointer datastorage = this->GetDataStorage();
    datastorage->Add(rEDDataNode);

}

void QmitkDualEnergyCtConversionView::OnPreferencesChanged(const berry::IBerryPreferences*)
{
	
	// get the node for the preference page
    berry::IPreferences::Pointer prefNode = berry::Platform::GetPreferencesService()->GetSystemPreferences()->Node("/org.mitk.views.dualenergyctconversion");

    bool enableExternal = prefNode->GetBool("enable external", false);

    if (enableExternal)
    {
    	//load the alpha value preferences from defined path
        QString path = prefNode->Get("alpha path", "");
        bool append = prefNode->GetBool("append values", true);
	    int c = m_BlendingTool.ReadExternalResource(path.toStdString(), append);

        if(c == -1){
            // when the file in preference page is not valid enable and show the warning
            m_Controls.warningLabel->setDisabled(false);
            m_Controls.warningLabel->setText("Could not read external alpha resource file.");
            m_Controls.warningLabel->setToolTip("Select a valid alpha blending file or disable external resource, inside the DECT Preferences page.");
            m_Controls.warningLabel->setVisible(true);
            MITK_INFO << "Problem on reading external alpha value resource from preferences.";

        }else{
            // once no file is selected or the external resource is disabled hide the warning again.
            m_Controls.warningLabel->setVisible(false);
            m_Controls.warningLabel->setDisabled(true);
        }
    	
    }else
    {
        // disable warning label if no resource is read
        if(m_Controls.warningLabel->isEnabled()){
            m_Controls.warningLabel->setVisible(false);
            m_Controls.warningLabel->setDisabled(true);
        }
    	//if the preferences are disabled reset the alpha values and replace them with the internal defined resource
        m_BlendingTool.Reset();
    }

	//update the mode box with the newly loaded values from the alpha tool
    UpdateModeBox();
}

