/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkDualEnergyCtConversionPreferencePage.h"
#include "mitkPluginActivator.h"

#include <berryIPreferencesService.h>
#include <berryIBerryPreferences.h>
#include <berryPlatform.h>

#include <QLabel>
#include <QPushButton>
#include <QFormLayout>
#include <QCheckBox>
#include <QGroupBox>
#include <QRadioButton>
#include <QLineEdit>
#include <QFileDialog>
#include <QVBoxLayout>


QmitkDualEnergyCtConversionPreferencePage::QmitkDualEnergyCtConversionPreferencePage()
  : m_MainControl(nullptr)
{
}

QmitkDualEnergyCtConversionPreferencePage::~QmitkDualEnergyCtConversionPreferencePage()
{
	
}

void QmitkDualEnergyCtConversionPreferencePage::Init(berry::IWorkbench::Pointer)
{
}


void QmitkDualEnergyCtConversionPreferencePage::CreateQtControl(QWidget* widget)
{
	berry::IPreferencesService* prefService = berry::Platform::GetPreferencesService();

	m_DualEnergyConversionPreferenceNode = prefService->GetSystemPreferences()->Node("/org.mitk.views.dualenergyctconversion");

	m_MainControl = new QWidget(widget);

	auto  formLayout = new QFormLayout;
	formLayout->setHorizontalSpacing(8);
	formLayout->setVerticalSpacing(24);

	auto disableOptionsLayout = new QHBoxLayout;
	m_EnableExternalCheckBox = new QCheckBox();
	disableOptionsLayout->addWidget(m_EnableExternalCheckBox);
	formLayout->addRow("Enable External Resource:",disableOptionsLayout);

	auto   pathOptionsLayout = new QHBoxLayout;
	m_PathEdit = new QLineEdit(m_MainControl);
	pathOptionsLayout->addWidget(m_PathEdit);
	m_PathSelect = new QPushButton("Select Path", m_MainControl);
	pathOptionsLayout->addWidget(m_PathSelect);
	formLayout->addRow("Local alpha value file:", pathOptionsLayout);

	auto readingOptionLayout = new QHBoxLayout;
	m_RadioOverwrite = new QRadioButton("Overwrite standard values", m_MainControl);
	readingOptionLayout->addWidget(m_RadioOverwrite);
	m_RadioAppend = new QRadioButton("Append to standard values", m_MainControl);
	readingOptionLayout->addWidget(m_RadioAppend);
	formLayout->addRow("Reading mode:", readingOptionLayout);

	// set tooltip for xml file. Displays an example xml file
	m_PathEdit->setToolTip("The xml file has to be in the following format: \n\n <AlphaBlendingTool>\n  <Mode description=\"descriptionTextOfMode\" alphaValue=\"1.0\"/>\n  <Mode description=\"descriptionTextOfMode\" alphaValue=\"1.5\"/>\n</AlphaBlendingTool>");

	m_MainControl->setLayout(formLayout);

	connect(m_PathSelect, SIGNAL(clicked()), this, SLOT(PathSelectButtonPushed()));
	connect(m_EnableExternalCheckBox, SIGNAL(stateChanged(int)), this, SLOT(CheckboxChanged(int)));
	
	this->Update();	
}

QWidget* QmitkDualEnergyCtConversionPreferencePage::GetQtControl() const
{
	return m_MainControl;
}

void QmitkDualEnergyCtConversionPreferencePage::PerformCancel()
{
}

bool QmitkDualEnergyCtConversionPreferencePage::PerformOk()
{
	auto t = m_DualEnergyConversionPreferenceNode->Keys();
	m_DualEnergyConversionPreferenceNode->PutBool("enable external", m_EnableExternalCheckBox->isChecked());
	m_DualEnergyConversionPreferenceNode->PutBool("append values", m_RadioAppend->isChecked());
	m_DualEnergyConversionPreferenceNode->PutBool("overwrite values", m_RadioOverwrite->isChecked());
	m_DualEnergyConversionPreferenceNode->Put("alpha path", m_PathEdit->text());
	return true;
}
void QmitkDualEnergyCtConversionPreferencePage::Update()
{
	m_EnableExternalCheckBox->setChecked(m_DualEnergyConversionPreferenceNode->GetBool("enable external", false));
	CheckboxChanged(m_EnableExternalCheckBox->checkState());
	
	if (m_DualEnergyConversionPreferenceNode->GetBool("append values", true))
	{
		m_RadioAppend->setChecked(true);
	}
	else
	{
		m_RadioOverwrite->setChecked(true);
	}

	QString path = m_DualEnergyConversionPreferenceNode->Get("alpha path", "");
	m_PathEdit->setText(path);
}

void QmitkDualEnergyCtConversionPreferencePage::PathSelectButtonPushed()
{
	QString path = QFileDialog::getOpenFileName(m_MainControl, "File for alpha value config", "", " XML Files (*.xml) ;; All Files (*.*)");
	if (!path.isEmpty())
	{
		m_PathEdit->setText(path);
	}
	
}

void QmitkDualEnergyCtConversionPreferencePage::CheckboxChanged(int state)
{
	if (state == Qt::Unchecked)
	{
		m_PathEdit->setDisabled(true);
		m_PathSelect->setDisabled(true);
		m_RadioAppend->setDisabled(true);
		m_RadioOverwrite->setDisabled(true);
		
	}else
	{
		m_PathEdit->setEnabled(true);
		m_PathSelect->setEnabled(true);
		m_RadioAppend->setEnabled(true);
		m_RadioOverwrite->setEnabled(true);
	}
}





