/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkPluginActivator.h"
#include "QmitkDualEnergyCtConversionView.h"
#include "QmitkDualEnergyCtConversionPreferencePage.h"

void mitk::PluginActivator::start(ctkPluginContext* context)
{
	BERRY_REGISTER_EXTENSION_CLASS(QmitkDualEnergyCtConversionView, context)
	BERRY_REGISTER_EXTENSION_CLASS(QmitkDualEnergyCtConversionPreferencePage, context)
}

void mitk::PluginActivator::stop(ctkPluginContext*)
{
}
